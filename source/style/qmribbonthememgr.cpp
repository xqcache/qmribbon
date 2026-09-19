#include "qmribbonthememgr.h"

#include "qmribbonanimationutil.h"
#include "qmribbonthemeswitchmask.h"

#include <QGuiApplication>
#include <QPair>
#include <QPalette>
#include <QStyleHints>
#include <QWidget>

#include <utility>

namespace {

constexpr auto kLightName = "Light";
constexpr auto kDarkName = "Dark";

// 内置主题的资源配置（由 source/style/qmribbon.qrc 提供）。
// 只要 QmRibbonTheme::fromFile() 就能读到 —— 它会先确保 qrc 初始化过。
constexpr auto kLightResource = ":/qmribbon/themes/light.json";
constexpr auto kDarkResource = ":/qmribbon/themes/dark.json";

bool systemPrefersDark()
{
    if (QStyleHints* hints = QGuiApplication::styleHints()) {
        return hints->colorScheme() == Qt::ColorScheme::Dark;
    }
    return false;
}

} // namespace

struct QmRibbonThemeMgr::Private {
    Mode mode { Mode::System };
    /// setTheme() 显式指定的主题名；为空表示按 mode 使用内置的 Light / Dark。
    QString selected_theme;
    QHash<QString, QmRibbonTheme> themes;
    QmRibbonTheme theme;
    /// 已登记的应用目标（QmRibbon / QmRibbonWindow），主题变化时自动重新应用。
    QList<QPointer<QWidget>> roots;
    /// 业务自己的附加样式表模板（见 setExtraStyleSheet()）。
    QString extra_style_sheet;

    // ---- 主题切换过渡 ----

    /// 是否启用过渡（见 setThemeTransitionEnabled()）。
    bool theme_transition_enabled { true };
    /// 切换前抓到的旧主题快照，等 refresh() 末尾消费（窗口 → 快照）。
    QList<QPair<QPointer<QWidget>, QPixmap>> pending_snapshots;
    /// 正在播放的遮罩。
    QList<QPointer<QmRibbonThemeSwitchMask>> masks;
    /// 切换前的主题名，用来判断这次是不是真的换了主题。
    QString transition_from_theme;
};

QmRibbonThemeMgr& QmRibbonThemeMgr::instance()
{
    // 故意不回收：QObject 的静态析构顺序不可控，而主题管理器要活到进程结束。
    static QmRibbonThemeMgr* manager = new QmRibbonThemeMgr;
    return *manager;
}

const QmRibbonTheme& QmRibbonThemeMgr::current()
{
    return instance().theme();
}

QmRibbonTheme QmRibbonThemeMgr::light()
{
    return QmRibbonTheme::fromFile(QString::fromLatin1(kLightResource));
}

QmRibbonTheme QmRibbonThemeMgr::dark()
{
    return QmRibbonTheme::fromFile(QString::fromLatin1(kDarkResource));
}

QmRibbonThemeMgr::QmRibbonThemeMgr(QObject* parent)
    : QObject(parent)
    , d_(new Private)
{
    d_->themes.insert(QString::fromLatin1(kLightName), QmRibbonThemeMgr::light());
    d_->themes.insert(QString::fromLatin1(kDarkName), QmRibbonThemeMgr::dark());

    // 「跟随系统」模式下实时响应系统明暗切换。
    if (QStyleHints* hints = QGuiApplication::styleHints()) {
        connect(hints, &QStyleHints::colorSchemeChanged, this, &QmRibbonThemeMgr::handleColorSchemeChanged);
    }

    resolveTheme();
}

QmRibbonThemeMgr::~QmRibbonThemeMgr()
{
    delete d_;
}

QmRibbonThemeMgr::Mode QmRibbonThemeMgr::mode() const
{
    return d_->mode;
}

void QmRibbonThemeMgr::setMode(Mode mode)
{
    const bool changed = (d_->mode != mode) || !d_->selected_theme.isEmpty();

    if (changed) {
        beginThemeTransition();
    }

    d_->mode = mode;
    // 三种模式固定对应内置的 Light / Dark 主题；自定义主题用 setTheme() 切换。
    d_->selected_theme.clear();

    resolveTheme();
    refresh();

    if (changed) {
        emit modeChanged(d_->mode);
    }
}

const QmRibbonTheme& QmRibbonThemeMgr::theme() const
{
    return d_->theme;
}

bool QmRibbonThemeMgr::isDark() const
{
    return d_->theme.isDark();
}

bool QmRibbonThemeMgr::registerTheme(const QmRibbonTheme& theme)
{
    if (!theme.isValid()) {
        return false;
    }

    d_->themes.insert(theme.name(), theme);
    return true;
}

bool QmRibbonThemeMgr::loadThemeFile(const QString& file_path, QString* error)
{
    const QmRibbonTheme theme = QmRibbonTheme::fromFile(file_path, error);
    if (!theme.isValid()) {
        return false;
    }
    return registerTheme(theme);
}

QStringList QmRibbonThemeMgr::themeNames() const
{
    QStringList names = d_->themes.keys();
    names.sort();
    return names;
}

bool QmRibbonThemeMgr::setTheme(const QString& name)
{
    const QmRibbonTheme theme = d_->themes.value(name);
    if (!theme.isValid()) {
        return false;
    }

    if (d_->selected_theme != name) {
        beginThemeTransition();
    }

    d_->selected_theme = name;
    d_->mode = theme.isDark() ? Mode::Dark : Mode::Light;

    resolveTheme();
    refresh();

    emit modeChanged(d_->mode);
    return true;
}

void QmRibbonThemeMgr::apply(QWidget* root)
{
    if (root == nullptr) {
        return;
    }

    for (const QPointer<QWidget>& existing : std::as_const(d_->roots)) {
        if (existing.data() == root) {
            return; // 已登记，样式表在主题变化时统一刷新
        }
    }

    // 已经有登记的祖先（例如 QmRibbonWindow 里的 QmRibbon、Group 里的 QmRibbonComboBox）时，
    // 不再给它单独一份样式表：祖先那份会顺着父子关系作用到它，重复设一份只会让每次换主题
    // 多刷一遍它的整棵子树 —— 而换主题本来就是「全体重新 polish」，这一下能省不少。
    for (const QPointer<QWidget>& other : std::as_const(d_->roots)) {
        QWidget* ancestor = other.data();
        if (ancestor != nullptr && ancestor != root && ancestor->isAncestorOf(root)) {
            return;
        }
    }

    d_->roots.append(QPointer<QWidget>(root));
    root->setStyleSheet(currentStyleSheet());
    root->update();
}

void QmRibbonThemeMgr::setExtraStyleSheet(const QString& template_text)
{
    if (d_->extra_style_sheet == template_text) {
        return;
    }

    d_->extra_style_sheet = template_text;
    refreshStyleSheets();
}

QString QmRibbonThemeMgr::extraStyleSheet() const
{
    return d_->extra_style_sheet;
}

void QmRibbonThemeMgr::setThemeTransitionEnabled(bool enabled)
{
    d_->theme_transition_enabled = enabled;
}

bool QmRibbonThemeMgr::isThemeTransitionEnabled() const
{
    return d_->theme_transition_enabled;
}

void QmRibbonThemeMgr::beginThemeTransition()
{
    // 先收拾上一次还没播完的遮罩：**必须隐藏**，否则接下来 grab() 会把遮罩自己拍进快照。
    for (const QPointer<QmRibbonThemeSwitchMask>& mask : std::as_const(d_->masks)) {
        if (mask != nullptr) {
            mask->hide();
            mask->deleteLater();
        }
    }
    d_->masks.clear();
    d_->pending_snapshots.clear();

    if (!d_->theme_transition_enabled || !QmRibbonAnimationUtil::shouldAnimate()) {
        return;
    }

    // 只对**顶层窗口**做过渡：roots 里既有窗口（QmRibbonWindow、apply() 过的对话框），
    // 也有子控件（QmRibbon / QmRibbonComboBox），后者不是窗口、截不了。
    for (const QPointer<QWidget>& root : std::as_const(d_->roots)) {
        QWidget* window = root.data();
        if (window == nullptr || !window->isWindow() || !window->isVisible() || window->size().isEmpty()) {
            continue;
        }

        d_->pending_snapshots.append(QPair<QPointer<QWidget>, QPixmap>(QPointer<QWidget>(window), window->grab()));
    }

    d_->transition_from_theme = d_->theme.name();
}

void QmRibbonThemeMgr::playThemeTransition()
{
    const QList<QPair<QPointer<QWidget>, QPixmap>> snapshots = d_->pending_snapshots;
    d_->pending_snapshots.clear();

    if (snapshots.isEmpty()) {
        return;
    }

    // 主题名没变（例如重复 setMode(Dark)）就没必要播。
    if (d_->theme.name() == d_->transition_from_theme) {
        return;
    }

    for (const QPair<QPointer<QWidget>, QPixmap>& entry : snapshots) {
        QWidget* window = entry.first.data();
        if (window == nullptr || !window->isVisible() || window->size().isEmpty()) {
            continue;
        }

        // 新旧两张快照都必须截，不能省第二张（这里踩过坑，别再来一次）：
        //
        // 遮罩是带 WA_OpaquePaintEvent 的不透明子控件，而 Qt 的重绘管理器会把「被不透明兄弟
        // 完全盖住」的窗口的重绘请求直接丢掉 —— 窗口 update() 之后紧跟着 show() 遮罩，
        // 窗口一次 paintEvent 都不会来（连 repaint() 都丢）。也就是说圆里**不可能**露出
        // 「已经换成新主题的实时窗口」，窗口还停在旧主题上；少了这张新主题快照，整段动画
        // 就是静止的旧主题，播完瞬间硬切，看起来像动画没播。
        //
        // 先截新主题、再建遮罩：这样快照里绝不可能有遮罩自己，和 grab() 对隐藏子控件的
        // 处理方式无关。
        const QPixmap after = window->grab();

        auto* mask = new QmRibbonThemeSwitchMask(window);
        mask->setBeforeSnapshot(entry.second);
        mask->setAfterSnapshot(after);

        d_->masks.append(QPointer<QmRibbonThemeSwitchMask>(mask));
        mask->start();
    }
}

QString QmRibbonThemeMgr::currentStyleSheet() const
{
    if (d_->extra_style_sheet.isEmpty()) {
        return d_->theme.styleSheet();
    }

    // 附加样式表排后面：同一特异性时它能覆盖框架规则（业务要的就是这个）。
    return d_->theme.styleSheet() + QLatin1Char('\n') + d_->theme.resolveStyleSheet(d_->extra_style_sheet);
}

void QmRibbonThemeMgr::resolveTheme()
{
    if (!d_->selected_theme.isEmpty()) {
        const QmRibbonTheme selected = d_->themes.value(d_->selected_theme);
        if (selected.isValid()) {
            d_->theme = selected;
            return;
        }
        d_->selected_theme.clear();
    }

    bool dark = false;
    switch (d_->mode) {
    case Mode::Light:
        dark = false;
        break;
    case Mode::Dark:
        dark = true;
        break;
    case Mode::System:
        dark = systemPrefersDark();
        break;
    }

    const QmRibbonTheme theme = d_->themes.value(QString::fromLatin1(dark ? kDarkName : kLightName));
    if (theme.isValid()) {
        d_->theme = theme;
    }
}

void QmRibbonThemeMgr::refresh()
{
    // 调色板是主题的「另一半」，而且必须排在最前面：
    // QSS 只覆盖框架自己的控件，Qt 自绘控件（下拉列表、复选框、滚动条）与第三方控件
    // （ADS 的停靠区就是 palette(window)/palette(light)/palette(dark)/palette(highlight)
    // 组织的）都读调色板，因此把主题配色设成应用调色板，它们就会一起跟着主题走。
    if (QGuiApplication::instance() != nullptr) {
        const QPalette palette = d_->theme.palette();
        // 一样就不设：setPalette() 会给每个窗口推一遍 PaletteChange 事件，虽然本身很便宜，
        // 但重复设置等于白让全体控件重算一遍配色（切到当前主题时就会走到这里）。
        if (palette != QGuiApplication::palette()) {
            QGuiApplication::setPalette(palette);
        }
    }

    refreshStyleSheets();

    emit themeChanged(d_->theme);

    // 业务侧（DWM dark-mode、ADS 配色）已经在上面这个信号里改完了，
    // 到这里再抓新主题快照、盖遮罩起动画：这一次同步调用返回事件循环前不会上屏，
    // 所以用户看到的第一帧就已经是「旧主题 + 开始扩散的圆」。
    playThemeTransition();
}

void QmRibbonThemeMgr::refreshStyleSheets()
{
    const QString sheet = currentStyleSheet();

    for (int i = d_->roots.size() - 1; i >= 0; --i) {
        QWidget* root = d_->roots.at(i).data();
        if (root == nullptr) {
            d_->roots.removeAt(i);
            continue;
        }

        // Qt 对「设成同一个样式表」不做短路：QStyleSheetStyle 照样重新解析、重新 polish
        // 整棵子树（实测 1300 个控件的窗口，一次 setStyleSheet() 约 85ms，与「字符串真的
        // 变了」同价）。所以这里得自己挡一次 —— 重复 apply()、切到当前主题、系统明暗信号
        // 抖动等等都会走到这里，不挡的话每次都要白刷一整棵树。
        if (root->styleSheet() == sheet) {
            continue;
        }

        root->setStyleSheet(sheet);
        root->update();
    }
}

void QmRibbonThemeMgr::handleColorSchemeChanged()
{
    // 我们自己切换主题时不会改变系统的配色方案，所以这个信号只会由系统设置变化触发；
    // 只有「跟随系统」才需要跟着系统走。
    if (d_->mode != Mode::System) {
        return;
    }

    // 系统明暗变了：同样走一次过渡（若是同名主题，playThemeTransition() 会自己丢掉）。
    beginThemeTransition();

    resolveTheme();
    refresh();
}
