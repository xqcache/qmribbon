#include "qmribbonwindow.h"

#include "qmribbon.h"
#include "qmribbonanimationutil.h"
#include "qmribbonquickaccessbar.h"
#include "qmribbonthememgr.h"
#include "qmribbontitlebar.h"

#include <QWKWidgets/widgetwindowagent.h>

#include <DockManager.h>

#include <QAction>
#include <QKeySequence>
#include <QPointer>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QToolButton>
#include <QVBoxLayout>

#include <utility>

// clang-format off
// <windows.h> 必须位于 Qt 头文件之后，且必须留在条件编译块内，
// 因此这里显式关闭格式化（.clang-format 中 SortIncludes 也已设为 Never）。
#ifdef Q_OS_WIN
#  ifndef NOMINMAX
#    define NOMINMAX
#  endif
#  include <windows.h>
#endif
// clang-format on

struct QmRibbonWindow::QmRibbonWindowPrivate {
    QVBoxLayout* layout { nullptr };

    QmRibbon* ribbon { nullptr };
    QStackedWidget* view_stack { nullptr };

    QWidget* main_view { nullptr };
    QPointer<QWidget> central_widget;
    QPointer<QWidget> backstage_widget;

    ViewMode view_mode { ViewMode::Main };

    QWK::WidgetWindowAgent* window_agent { nullptr };
    QPointer<ads::CDockManager> dock_manager;

    /// Backstage 切换动画（驱动 backstage_widget 的 pos）。
    QPropertyAnimation* view_animation { nullptr };

    /// 登记过的浮层控件（QmRibbonFloatingWidget 之类），随 MainView 一起可见。
    QList<QPointer<QWidget>> floating_widgets;
    /// 是否已经在 MainView 上装了 resize 过滤器。
    bool watching_main_view { false };
};

QmRibbonWindow::QmRibbonWindow(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , d_(new QmRibbonWindowPrivate)
{
    setObjectName(QStringLiteral("RibbonWindow"));
    // 关键：**不要**设置 Qt::FramelessWindowHint。
    //
    // QWindowKit 走的是 WM_NCCALCSIZE 方案：保留 WS_THICKFRAME / WS_CAPTION 等系统边框样式，
    // 由它把标题栏从客户区里去掉，DWM 因此仍然负责窗口的系统阴影与 Win11 圆角；
    // Qt 的 FramelessWindowHint 则会把窗口变成 WS_POPUP，是另一套渲染路径，
    // 混用会让窗口彻底失去系统边框 —— 表现就是没有系统阴影、没有圆角。
    // （QWindowKit 官方示例同样不设置该 flag。）
    //
    // 注意：QWindowKit 的 setup() 之后不能再调用 setWindowFlags()，因此这里一次设好。
    setWindowFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint |
                   Qt::WindowCloseButtonHint);
    setAttribute(Qt::WA_StyledBackground, true);

    QmRibbonThemeMgr::instance().apply(this);

    d_->layout = new QVBoxLayout(this);
    d_->layout->setContentsMargins(0, 0, 0, 0);
    d_->layout->setSpacing(0);

    // 不要让布局把「Ribbon 展开 / 折叠」引起的**最小尺寸**变化强加给窗口：
    // 否则每次折叠（或展开）Qt 都会重新应用一次窗口几何，Windows 再按边框微调
    // 一两像素，表现就是「双击折叠时窗口跳一下」。
    // 改成窗口尺寸固定不变，折叠只改变窗口内部的高度分配。
    d_->layout->setSizeConstraint(QLayout::SetNoConstraint);

    d_->ribbon = new QmRibbon(this);

    d_->view_stack = new QStackedWidget(this);
    d_->view_stack->setObjectName(QStringLiteral("RibbonViewStack"));

    d_->main_view = new QWidget(d_->view_stack);
    d_->main_view->setObjectName(QStringLiteral("RibbonMainView"));

    auto* main_view_layout = new QVBoxLayout(d_->main_view);
    main_view_layout->setContentsMargins(0, 0, 0, 0);
    main_view_layout->setSpacing(0);

    d_->view_stack->addWidget(d_->main_view);

    d_->layout->addWidget(d_->ribbon);
    d_->layout->addWidget(d_->view_stack, 1);

    connect(d_->ribbon, &QmRibbon::fileButtonClicked, this, [this]() {
        if (d_->backstage_widget != nullptr) {
            setViewMode(ViewMode::Backstage);
        }
    });

    // Backstage 模式下 Ribbon 整体隐藏，用 Esc 返回主界面（与 Word 行为一致）。
    auto* escape_action = new QAction(this);
    escape_action->setShortcut(QKeySequence(Qt::Key_Escape));
    escape_action->setShortcutContext(Qt::WindowShortcut);
    addAction(escape_action);

    connect(escape_action, &QAction::triggered, this, [this]() {
        if (d_->view_mode == ViewMode::Backstage) {
            setViewMode(ViewMode::Main);
        }
    });

    // 必须在所有子控件都建好之后调用。
    setupWindowAgent();

    // Backstage 滑出结束后再切回主工作区，并把位置复原，便于下次滑入。
    d_->view_animation = new QPropertyAnimation(this);
    connect(d_->view_animation, &QPropertyAnimation::finished, this, [this]() {
        QWidget* backstage = d_->backstage_widget.data();
        if (backstage == nullptr) {
            return;
        }

        if (d_->view_mode == ViewMode::Backstage) {
            backstage->move(0, 0);
            return;
        }

        d_->view_stack->setCurrentWidget(d_->main_view);
        backstage->move(0, 0);
    });

    // SetNoConstraint 之后布局不再限制窗口尺寸，这里补一个**固定不变**的最小尺寸：
    // 既避免窗口被压得过小，又因为它是常量而不会随折叠状态变化触发几何重算。
    d_->layout->activate();
    setMinimumSize(d_->layout->minimumSize());
}

QmRibbonWindow::~QmRibbonWindow() noexcept
{
    delete d_;
}

void QmRibbonWindow::setupWindowAgent()
{
    QmRibbonTitleBar* title_bar = d_->ribbon->titleBar();

    d_->window_agent = new QWK::WidgetWindowAgent(this);
    d_->window_agent->setup(this);

    // 顺序很重要：setTitleBar() 会清掉此前注册的系统按钮与 hit-test 目标。
    d_->window_agent->setTitleBar(title_bar);

    // 系统按钮：鼠标事件由 QWindowKit 直接处理（含 Win11 Snap Layout），
    // 同时把点击转发给 Qt，因此 QmRibbonTitleBar 里连接的 clicked 依然有效。
    d_->window_agent->setSystemButton(QWK::WindowAgentBase::WindowIcon, title_bar->appIconButton());
    d_->window_agent->setSystemButton(QWK::WindowAgentBase::Minimize, title_bar->minimizeButton());
    d_->window_agent->setSystemButton(QWK::WindowAgentBase::Maximize, title_bar->maximizeButton());
    d_->window_agent->setSystemButton(QWK::WindowAgentBase::Close, title_bar->closeButton());

    // 标题栏中需要交互的控件；标题栏里剩下的区域自动成为窗口拖动区。
    d_->window_agent->setHitTestVisible(title_bar->quickAccessBar(), true);
    d_->window_agent->setHitTestVisible(title_bar->accountButton(), true);

    // 让 DWM 的窗口边框跟随主题（QWindowKit 会设置 DWMWA_USE_IMMERSIVE_DARK_MODE）。
    updateWindowTheme();
    connect(&QmRibbonThemeMgr::instance(), &QmRibbonThemeMgr::themeChanged, this, [this]() {
        updateWindowTheme();
        updateDockManagerTheme();
    });
}

void QmRibbonWindow::updateWindowTheme()
{
    if (d_->window_agent == nullptr) {
        return;
    }

    d_->window_agent->setWindowAttribute(QStringLiteral("dark-mode"), QmRibbonThemeMgr::instance().isDark());
}

void QmRibbonWindow::updateDockManagerTheme()
{
    if (d_->dock_manager.isNull()) {
        return;
    }

    // ADS 自带浅色 / 深色两套样式表，连图标都分黑白两份
    //（close-button.svg 与 close-button_dark.svg）。它的默认策略 FollowPalette
    // 依赖「切换主题后自己能收到并处理调色板变化」，实测不可靠 ——
    // 表现为深色主题下停靠区的关闭 / 停靠 / 菜单按钮仍是黑色图标。
    // 这里按我们的主题显式指定，setColorSchemeMode() 内部会在需要时重载样式表。
    d_->dock_manager->setColorSchemeMode(QmRibbonThemeMgr::instance().isDark()
                                             ? ads::CDockManager::ColorSchemeMode::Dark
                                             : ads::CDockManager::ColorSchemeMode::Light);
}

QmRibbon* QmRibbonWindow::ribbon() const
{
    return d_->ribbon;
}

QStackedWidget* QmRibbonWindow::viewStack() const
{
    return d_->view_stack;
}

void QmRibbonWindow::setCentralWidget(QWidget* widget)
{
    if (d_->central_widget == widget) {
        return;
    }

    auto* layout = qobject_cast<QVBoxLayout*>(d_->main_view->layout());
    if (layout == nullptr) {
        return;
    }

    if (d_->central_widget != nullptr) {
        layout->removeWidget(d_->central_widget);
        d_->central_widget->deleteLater();
    }

    d_->central_widget = widget;

    if (widget != nullptr) {
        widget->setParent(d_->main_view);
        layout->addWidget(widget, 1);
    }
}

QWidget* QmRibbonWindow::centralWidget() const
{
    return d_->central_widget.data();
}

ads::CDockManager* QmRibbonWindow::dockManager()
{
    if (d_->dock_manager.isNull()) {
        auto* manager = new ads::CDockManager;
        manager->setObjectName(QStringLiteral("RibbonDockManager"));

        // setCentralWidget() 会把 manager 挂到 MainView 上并接管所有权。
        setCentralWidget(manager);
        d_->dock_manager = manager;

        // 用当前主题决定 ADS 用哪套样式表（含图标），否则深色下会出现黑色图标。
        updateDockManagerTheme();
    }

    return d_->dock_manager.data();
}

void QmRibbonWindow::addFloatingWidget(QWidget* widget, const QPoint& position)
{
    if (widget == nullptr) {
        return;
    }

    for (const QPointer<QWidget>& existing : std::as_const(d_->floating_widgets)) {
        if (existing.data() == widget) {
            widget->move(position);
            widget->show();
            widget->raise();
            return;
        }
    }

    d_->floating_widgets.append(QPointer<QWidget>(widget));
    registerFloatingWidget(widget, position);
}

void QmRibbonWindow::removeFloatingWidget(QWidget* widget)
{
    for (int i = d_->floating_widgets.size() - 1; i >= 0; --i) {
        if (d_->floating_widgets.at(i).data() == widget || d_->floating_widgets.at(i).isNull()) {
            d_->floating_widgets.removeAt(i);
        }
    }
}

QList<QWidget*> QmRibbonWindow::floatingWidgets() const
{
    QList<QWidget*> result;
    result.reserve(d_->floating_widgets.size());

    for (const QPointer<QWidget>& widget : d_->floating_widgets) {
        if (widget != nullptr) {
            result.append(widget.data());
        }
    }

    return result;
}

void QmRibbonWindow::registerFloatingWidget(QWidget* widget, const QPoint& position)
{
    // 挂到 MainView 上（而不是 view stack）：这样 Backstage 模式下会跟着一起隐藏，
    // 而且浮层始终位于停靠区之上。
    //
    // 尺寸：浮层不在布局里，没有谁替它 adjustSize()。刚 new 出来的控件是顶层窗口，
    // 默认 640×480（而且 `QWidget::resize()` 会置上 WA_Resized，所以不能只看这个标记 ——
    // 之前就是这里漏了，导致浮层一直保持 640×480，被 MainView 一夹就只能横向移动）。
    const bool was_window = widget->isWindow();
    widget->setParent(d_->main_view);

    if (was_window || !widget->testAttribute(Qt::WA_Resized)) {
        widget->adjustSize();
    }

    widget->move(position);
    widget->show();
    widget->raise();

    // MainView 尺寸变化时把浮层夹回可见区域（浮层自己也会监听父控件 resize，
    // 这里再兜一层，保证通过本函数挂上去的浮层一定不会跑丢）。
    if (!d_->watching_main_view) {
        d_->main_view->installEventFilter(this);
        d_->watching_main_view = true;
    }
}

void QmRibbonWindow::clampFloatingWidgets()
{
    const QRect bounds = d_->main_view->rect();

    for (int i = d_->floating_widgets.size() - 1; i >= 0; --i) {
        QWidget* widget = d_->floating_widgets.at(i).data();
        if (widget == nullptr) {
            d_->floating_widgets.removeAt(i);
            continue;
        }

        const QPoint pos = widget->pos();
        widget->move(qBound(bounds.left(), pos.x(), qMax(bounds.left(), bounds.right() - widget->width() + 1)),
                     qBound(bounds.top(), pos.y(), qMax(bounds.top(), bounds.bottom() - widget->height() + 1)));
    }
}

bool QmRibbonWindow::eventFilter(QObject* watched, QEvent* event)
{
    if (d_->watching_main_view && watched == d_->main_view && event->type() == QEvent::Resize) {
        clampFloatingWidgets();
    }

    return QWidget::eventFilter(watched, event);
}

void QmRibbonWindow::setBackstageWidget(QWidget* widget)
{
    if (d_->backstage_widget == widget) {
        return;
    }

    if (d_->backstage_widget != nullptr) {
        d_->view_stack->removeWidget(d_->backstage_widget);
        d_->backstage_widget->deleteLater();
    }

    d_->backstage_widget = widget;

    if (widget != nullptr) {
        d_->view_stack->addWidget(widget);
    } else if (viewMode() == ViewMode::Backstage) {
        setViewMode(ViewMode::Main);
    }
}

QWidget* QmRibbonWindow::backstageWidget() const
{
    return d_->backstage_widget.data();
}

QmRibbonWindow::ViewMode QmRibbonWindow::viewMode() const
{
    return d_->view_mode;
}

void QmRibbonWindow::setViewMode(ViewMode mode)
{
    if (mode == ViewMode::Backstage && d_->backstage_widget == nullptr) {
        return;
    }

    if (d_->view_mode == mode) {
        return;
    }

    d_->view_mode = mode;
    // 保留 TitleBar，保证 Backstage 下窗口仍可拖动 / 最小化 / 关闭。
    d_->ribbon->setTitleBarOnly(mode == ViewMode::Backstage);
    animateViewSwitch(mode);

    emit viewModeChanged(mode);
}

void QmRibbonWindow::animateViewSwitch(ViewMode mode)
{
    QWidget* backstage = d_->backstage_widget.data();
    if (backstage == nullptr) {
        // 没有 Backstage 页面时只需切换主工作区（例如 setBackstageWidget(nullptr)）。
        d_->view_stack->setCurrentWidget(d_->main_view);
        return;
    }

    d_->view_animation->stop();

    const bool animate = QmRibbonAnimationUtil::shouldAnimate() && isVisible();
    const int travel = qMax(d_->view_stack->width(), 1);

    if (!animate) {
        backstage->move(0, 0);
        d_->view_stack->setCurrentWidget(mode == ViewMode::Backstage ? backstage : d_->main_view);
        return;
    }

    // 时长与缓动曲线由 QmRibbonAnimationUtil 统一从当前主题取（含运行时覆盖）。
    if (mode == ViewMode::Backstage) {
        // 从左侧滑入，覆盖在主工作区之上。
        d_->view_stack->setCurrentWidget(backstage);
        backstage->move(-travel, 0);

        d_->view_animation->setTargetObject(backstage);
        d_->view_animation->setPropertyName("pos");
        QmRibbonAnimationUtil::prepare(d_->view_animation);
        d_->view_animation->setStartValue(QPoint(-travel, 0));
        d_->view_animation->setEndValue(QPoint(0, 0));
        d_->view_animation->start();
        return;
    }

    // 滑出后再切回主工作区（切换在 finished 处理里做）。
    d_->view_animation->setTargetObject(backstage);
    d_->view_animation->setPropertyName("pos");
    QmRibbonAnimationUtil::prepare(d_->view_animation);
    d_->view_animation->setStartValue(backstage->pos());
    d_->view_animation->setEndValue(QPoint(-travel, 0));
    d_->view_animation->start();
}

bool QmRibbonWindow::nativeEvent(const QByteArray& event_type, void* message, qintptr* result)
{
#ifdef Q_OS_WIN
    if (event_type == "windows_generic_MSG" && message != nullptr) {
        auto* msg = static_cast<MSG*>(message);

        // 双击标题栏：不让 DefWindowProc 走原生最大化，改为与最大化按钮完全一致的
        // Qt 侧切换（QWindowKit 在原生 caption 路径上有多处 "calling DefWindowProc()
        // here is really dangerous" 的处理，实测会导致界面挂死）。
        // 这里只拦 WM_NCLBUTTONDBLCLK，标题栏拖动、Aero Snap、边缘 Resize、
        // 最大化按钮的 Snap Layout 都仍然由 QWindowKit / 系统原生处理。
        if (msg->message == WM_NCLBUTTONDBLCLK && static_cast<int>(msg->wParam) == HTCAPTION) {
            d_->ribbon->titleBar()->toggleMaximizeRestore();
            *result = 0;
            return true;
        }
    }
#else
    Q_UNUSED(event_type)
    Q_UNUSED(message)
    Q_UNUSED(result)
#endif

    return QWidget::nativeEvent(event_type, message, result);
}
