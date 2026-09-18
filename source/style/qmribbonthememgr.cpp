#include "qmribbonthememgr.h"

#include <QGuiApplication>
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
        QGuiApplication::setPalette(d_->theme.palette());
    }

    refreshStyleSheets();

    emit themeChanged(d_->theme);
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

    resolveTheme();
    refresh();
}
