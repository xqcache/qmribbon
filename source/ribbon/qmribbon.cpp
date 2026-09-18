#include "qmribbon.h"

#include "qmribbonanimationutil.h"
#include "qmribbonpage.h"
#include "qmribbonquickaccessbar.h"
#include "qmribbontheme.h"
#include "qmribbonthememgr.h"
#include "qmribbontab.h"
#include "qmribbontabbar.h"
#include "qmribbontitlebar.h"

#include <QPainter>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QVBoxLayout>

struct QmRibbon::QmRibbonPrivate {
    QmRibbonTitleBar* title_bar { nullptr };
    QmRibbonTabBar* tab_bar { nullptr };
    QStackedWidget* page_stack { nullptr };
    /// 下边沿阴影带：占位用，本身透明，渐变由 QmRibbon::paintEvent 绘制。
    QWidget* shadow_band { nullptr };
    /// 折叠 / 展开动画：驱动 page_stack 的 maximumHeight。
    QPropertyAnimation* page_animation { nullptr };
    QList<QmRibbonPage*> pages;
    DisplayMode display_mode { DisplayMode::Expanded };
    /// 只显示 TitleBar：进入 Backstage 时使用，保证窗口仍可拖动 / 最小化 / 关闭。
    bool title_bar_only { false };
};

QmRibbon::QmRibbon(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonPrivate)
{
    setObjectName(QStringLiteral("Ribbon"));
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setAttribute(Qt::WA_StyledBackground, true);

    QmRibbonThemeMgr::instance().apply(this);

    auto* layout_main = new QVBoxLayout(this);
    layout_main->setContentsMargins(0, 0, 0, 0);
    layout_main->setSpacing(0);

    d_->title_bar = new QmRibbonTitleBar(this);
    d_->tab_bar = new QmRibbonTabBar(this);
    d_->page_stack = new QStackedWidget(this);
    d_->page_stack->setObjectName(QStringLiteral("RibbonPageStack"));
    d_->page_stack->setAttribute(Qt::WA_StyledBackground, true);

    // 三个部分一律显式顶对齐，末尾再放一个 stretch 吸收多余高度：
    // 这样无论 Ribbon 拿到多少高度（例如折叠动画的中间帧），
    // 标题栏都固定在 y = 0，不会被布局往下顶。
    layout_main->addWidget(d_->title_bar, 0, Qt::AlignTop);
    layout_main->addWidget(d_->tab_bar, 0, Qt::AlignTop);
    layout_main->addWidget(d_->page_stack, 0, Qt::AlignTop);
    layout_main->addStretch(1);

    // 下边沿的阴影带：固定高度占位，本身不绘制（透明），
    // 渐变由 QmRibbon::paintEvent 画，末端渐隐到窗口底色，视觉上像 Ribbon 投下的阴影。
    d_->shadow_band = new QWidget(this);
    d_->shadow_band->setObjectName(QStringLiteral("RibbonShadowBand"));
    d_->shadow_band->setFixedHeight(QmRibbonMetrics::ribbon_shadow_height);
    d_->shadow_band->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    layout_main->addWidget(d_->shadow_band, 0, Qt::AlignTop);

    // 折叠 / 展开动画：动画 page_stack 的 maximumHeight，
    // 页面区域高度归零后把 Tab 栏以下的部分收起来（Word 的 Collapse the Ribbon）。
    d_->page_animation = new QPropertyAnimation(d_->page_stack, "maximumHeight", this);
    connect(d_->page_animation, &QPropertyAnimation::finished, this, [this]() {
        // 收尾时移除高度约束，避免以后页面尺寸变化被这个上限裁掉。
        d_->page_stack->setMaximumHeight(QWIDGETSIZE_MAX);
        d_->page_stack->setVisible(!d_->title_bar_only && d_->display_mode == DisplayMode::Expanded);
        updateGeometry();
    });

    connect(d_->tab_bar, &QmRibbonTabBar::currentIndexChanged, this, [this](int index) {
        if (index >= 0 && index < d_->page_stack->count()) {
            d_->page_stack->setCurrentIndex(index);
        }
        emit currentIndexChanged(index);
        emit currentPageChanged(pageAt(index));
    });

    connect(d_->tab_bar, &QmRibbonTabBar::tabClicked, this, [this](int index) {
        Q_UNUSED(index)
        // 折叠状态下点击 Tab 会重新展开 Ribbon。
        if (d_->display_mode == DisplayMode::TabsOnly) {
            setDisplayMode(DisplayMode::Expanded);
        }
    });

    connect(d_->tab_bar, &QmRibbonTabBar::fileButtonClicked, this, &QmRibbon::fileButtonClicked);

    // 双击 Tab 栏 = 折叠 / 展开（与 Word 一致），带动画。
    //
    // 目标模式按「手势开始（按下）那一刻」的状态决定，而不是按当前状态取反：
    // 双击的第一下落在 Tab 上时会先触发 clicked，而折叠状态下点击 Tab 会先把
    // Ribbon 展开，此后再取反就会立刻又收起来 —— 表现成「双击展不开」。
    // 按手势开始的状态判断时，这种情况只是重复设置「展开」，等于空操作。
    connect(d_->tab_bar, &QmRibbonTabBar::barDoubleClicked, this, [this](bool collapsed_before_press) {
        setDisplayMode(collapsed_before_press ? DisplayMode::Expanded : DisplayMode::TabsOnly);
    });

    connect(d_->tab_bar, &QmRibbonTabBar::collapseButtonClicked, this, [this]() {
        setDisplayMode(d_->display_mode == DisplayMode::Expanded ? DisplayMode::TabsOnly : DisplayMode::Expanded);
    });
}

QmRibbon::~QmRibbon() noexcept
{
    delete d_;
}

QmRibbonTitleBar* QmRibbon::titleBar() const
{
    return d_->title_bar;
}

QmRibbonTabBar* QmRibbon::tabBar() const
{
    return d_->tab_bar;
}

QmRibbonQuickAccessBar* QmRibbon::quickAccessBar() const
{
    return d_->title_bar->quickAccessBar();
}

QStackedWidget* QmRibbon::pageStack() const
{
    return d_->page_stack;
}

QmRibbonPage* QmRibbon::addPage(const QString& title)
{
    auto* page = new QmRibbonPage(title, d_->page_stack);
    d_->pages.append(page);
    d_->page_stack->addWidget(page);

    auto* tab = d_->tab_bar->addTab(title);

    // Page 标题变化时同步 Tab 文案。
    connect(page, &QmRibbonPage::titleChanged, tab, [tab](const QString& text) {
        tab->setText(text);
        tab->updateGeometry();
    });

    return page;
}

int QmRibbon::pageCount() const
{
    return static_cast<int>(d_->pages.size());
}

QmRibbonPage* QmRibbon::pageAt(int index) const
{
    if (index < 0 || index >= d_->pages.size()) {
        return nullptr;
    }
    return d_->pages.at(index);
}

QmRibbonPage* QmRibbon::currentPage() const
{
    return pageAt(currentIndex());
}

int QmRibbon::currentIndex() const
{
    return d_->tab_bar->currentIndex();
}

void QmRibbon::setCurrentIndex(int index)
{
    if (index < 0 || index >= d_->pages.size()) {
        return;
    }

    d_->tab_bar->setCurrentIndex(index);

    // 没有发生切换时（例如重复设置同一个 index），仍然保证页面同步。
    if (d_->page_stack->currentIndex() != index) {
        d_->page_stack->setCurrentIndex(index);
        emit currentIndexChanged(index);
        emit currentPageChanged(pageAt(index));
    }
}

void QmRibbon::setCurrentPage(QmRibbonPage* page)
{
    const int index = static_cast<int>(d_->pages.indexOf(page));
    if (index >= 0) {
        setCurrentIndex(index);
    }
}

QmRibbon::DisplayMode QmRibbon::displayMode() const
{
    return d_->display_mode;
}

bool QmRibbon::isCollapsed() const
{
    return d_->display_mode == DisplayMode::TabsOnly;
}

void QmRibbon::setDisplayMode(DisplayMode mode)
{
    if (d_->display_mode == mode) {
        return;
    }

    d_->display_mode = mode;
    d_->tab_bar->setCollapsed(mode == DisplayMode::TabsOnly);
    applyDisplayMode(true);

    emit displayModeChanged(mode);
}

bool QmRibbon::isTitleBarOnly() const
{
    return d_->title_bar_only;
}

void QmRibbon::setTitleBarOnly(bool title_bar_only)
{
    if (d_->title_bar_only == title_bar_only) {
        return;
    }

    d_->title_bar_only = title_bar_only;
    updateVisibility();
}

void QmRibbon::updateVisibility()
{
    d_->tab_bar->setVisible(!d_->title_bar_only);
    d_->shadow_band->setVisible(!d_->title_bar_only);
    // 进入 Backstage 时直接到位，不播动画。
    applyDisplayMode(false);
}

void QmRibbon::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    const QmRibbonTheme& theme = QmRibbonThemeMgr::current();

    QPainter painter(this);
    // 相当于原来的 QSS #Ribbon 背景（覆写 paintEvent 后由我们自己画）。
    painter.fillRect(rect(), theme.windowBackground());

    if (!d_->shadow_band->isVisible()) {
        return;
    }

    // 下边沿阴影：从半透明阴影色渐变到全透明。
    // 渐变末端与窗口底色（也是 MainView / 停靠区的底色）一致，
    // 因此看起来就是 Ribbon 投在 MainView 上的阴影，而不是一条硬边。
    constexpr int band_height = QmRibbonMetrics::ribbon_shadow_height;

    const QColor shadow = theme.ribbonShadow();
    if (!shadow.isValid() || shadow.alpha() == 0) {
        return;
    }

    const QRect band_rect(0, height() - band_height, width(), band_height);
    QLinearGradient gradient(band_rect.topLeft(), band_rect.bottomLeft());
    gradient.setColorAt(0.0, shadow);
    gradient.setColorAt(1.0, QColor(shadow.red(), shadow.green(), shadow.blue(), 0));

    painter.fillRect(band_rect, gradient);
}

int QmRibbon::expandedPageHeight() const
{
    // 优先用当前页面的高度：页面高度是固定的，不会因为动画中的高度约束而变化。
    if (QWidget* page = d_->page_stack->currentWidget()) {
        return page->sizeHint().height();
    }

    return d_->page_stack->sizeHint().height();
}

void QmRibbon::applyDisplayMode(bool animate)
{
    const bool expanded = (d_->display_mode == DisplayMode::Expanded);

    // Backstage：整块区域都不显示，也没有动画可言。
    if (d_->title_bar_only) {
        d_->page_animation->stop();
        d_->page_stack->setMaximumHeight(QWIDGETSIZE_MAX);
        d_->page_stack->setVisible(false);
        updateGeometry();
        return;
    }

    const int full_height = expandedPageHeight();
    // 折叠 / 展开是否做动画：主题里的动效设置（开关 / 时长）说了算，
    // 再叠加调用方与当前状态的条件。
    const bool use_animation = animate && QmRibbonAnimationUtil::shouldAnimate() && full_height > 0 && isVisible();

    if (!use_animation) {
        d_->page_animation->stop();
        d_->page_stack->setMaximumHeight(QWIDGETSIZE_MAX);
        d_->page_stack->setVisible(expanded);
        updateGeometry();
        return;
    }

    // 从当前高度起步：快速连续双击时动画会平滑地反向，而不是跳变。
    const int start_height =
        (d_->page_animation->state() == QAbstractAnimation::Running)
            ? d_->page_animation->currentValue().toInt()
            : (expanded ? (d_->page_stack->isVisible() ? d_->page_stack->height() : 0) : d_->page_stack->height());

    // 展开和折叠都需要页面区域在这期间保持可见，动画才看得见高度变化。
    d_->page_stack->setVisible(true);

    d_->page_animation->stop();
    // 时长与缓动曲线由 QmRibbonAnimationUtil 统一从当前主题取（含运行时覆盖）。
    QmRibbonAnimationUtil::prepare(d_->page_animation);
    d_->page_animation->setStartValue(start_height);
    d_->page_animation->setEndValue(expanded ? full_height : 0);
    d_->page_animation->start();

    updateGeometry();
}
