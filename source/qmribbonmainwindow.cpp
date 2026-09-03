#include "qmribbonmainwindow.h"

#include "qmribbon.h"
#include "qmribbonbackstageview.h"
#include "qmribbontitlebar.h"

#include <QEasingCurve>
#include <QParallelAnimationGroup>
#include <QPointer>
#include <QPropertyAnimation>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QToolBar>
#include <QToolButton>

struct QmRibbonMainWindowPrivate {
    QmRibbon* ribbon { nullptr };
    QmRibbonBackstageView* backstage_view { nullptr };
    QWidget* main_view { nullptr };
    QStackedWidget* central_widget { nullptr };
    QmRibbonMainWindow::ViewMode view_mode { QmRibbonMainWindow::ViewMode::MainView };
    QParallelAnimationGroup* view_animation { nullptr };
    QPointer<QWidget> transition_source;
    QPointer<QWidget> transition_target;
    QmRibbonMainWindow::ViewMode transition_mode { QmRibbonMainWindow::ViewMode::MainView };
    QPropertyAnimation* ribbon_animation { nullptr };
    bool ribbon_target_visible { true };
    int ribbon_expanded_height { 0 };
    bool view_animation_enabled { true };
};

QmRibbonMainWindow::QmRibbonMainWindow(QWidget* parent, QmRibbon::Features features, Qt::WindowFlags flags)
    : QmFramelessWindow(parent, flags, !features.testAnyFlag(QmRibbon::NoDefaultTitleBar))
    , d_(new QmRibbonMainWindowPrivate)
{
    initUi(features);
    connectSignals();
}

QmRibbonMainWindow::~QmRibbonMainWindow() noexcept
{
    cancelViewTransition();
    finishRibbonAnimation();
    delete d_;
}

void QmRibbonMainWindow::initUi(QmRibbon::Features features)
{
    d_->ribbon = QmRibbon::install(this, features);
    auto* title_bar = d_->ribbon->titleBar();
    setWindowTitleBar(title_bar);
    setWindowIconButton(title_bar->logoButton());
    setMinimizeButton(title_bar->minimizeButton());
    setMaximizeButton(title_bar->maximizeButton());
    setCloseButton(title_bar->closeButton());
    setHitTestVisible(title_bar->quickAccessToolBar());
    setHitTestVisible(title_bar->userInfoButton());
    setHitTestVisible(title_bar->ribbonOptionsButton());

    d_->central_widget = new QStackedWidget(this);
    setCentralWidget(d_->central_widget);
}

void QmRibbonMainWindow::connectSignals()
{
    connect(d_->ribbon, &QmRibbon::enterBackstageView, this, [this] {
        showBackstageView();
    });
}

void QmRibbonMainWindow::setBackstageView(QmRibbonBackstageView* view)
{
    cancelViewTransition();

    if (d_->backstage_view) {
        d_->central_widget->removeWidget(d_->backstage_view);
        d_->backstage_view->setParent(nullptr);
        d_->backstage_view->deleteLater();
    }

    d_->backstage_view = view;
    if (!d_->backstage_view) {
        return;
    }

    d_->central_widget->addWidget(d_->backstage_view);
    connect(d_->backstage_view, &QmRibbonBackstageView::leaveBackstageView, this, [this] {
        showMainView();
    });
}

QmRibbonBackstageView* QmRibbonMainWindow::backstageView() const
{
    return d_->backstage_view;
}

void QmRibbonMainWindow::setMainView(QWidget* widget)
{
    cancelViewTransition();

    if (d_->main_view) {
        d_->central_widget->removeWidget(d_->main_view);
        d_->main_view->setParent(nullptr);
        d_->main_view->deleteLater();
    }

    d_->main_view = widget;
    if (d_->main_view) {
        d_->central_widget->addWidget(d_->main_view);
    }
}

QWidget* QmRibbonMainWindow::mainView() const
{
    return d_->main_view;
}

QmRibbon* QmRibbonMainWindow::ribbon() const
{
    return d_->ribbon;
}

void QmRibbonMainWindow::showBackstageView()
{
    transitionToView(d_->backstage_view, ViewMode::BackstageView, -1);
}

void QmRibbonMainWindow::showMainView()
{
    transitionToView(d_->main_view, ViewMode::MainView, 1);
}

void QmRibbonMainWindow::setViewAnimationEnabled(bool enabled)
{
    if (d_->view_animation_enabled == enabled) {
        return;
    }

    d_->view_animation_enabled = enabled;
    if (!enabled) {
        finishViewTransition();
        finishRibbonAnimation();
    }
}

bool QmRibbonMainWindow::isViewAnimationEnabled() const
{
    return d_->view_animation_enabled;
}

void QmRibbonMainWindow::transitionToView(QWidget* target, ViewMode mode, int direction)
{
    if (!target) {
        return;
    }

    if (d_->view_animation) {
        if (d_->transition_target == target) {
            return;
        }
        finishViewTransition();
    }

    auto* source = d_->central_widget->currentWidget();
    if (source == target || !source || !isVisible() || !d_->view_animation_enabled) {
        setCurrentView(target, mode);
        return;
    }

    auto* stacked_layout = qobject_cast<QStackedLayout*>(d_->central_widget->layout());
    if (!stacked_layout) {
        setCurrentView(target, mode);
        return;
    }

    d_->view_mode = mode;
    animateRibbonVisibility(mode == ViewMode::MainView);
    if (layout()) {
        layout()->activate();
    }

    const int width = d_->central_widget->width();
    if (width <= 0) {
        setCurrentView(target, mode);
        return;
    }

    stacked_layout->setStackingMode(QStackedLayout::StackAll);
    d_->central_widget->setCurrentWidget(target);

    const QPoint origin(0, 0);
    const QPoint target_start(direction * width, 0);
    const QPoint source_end(-direction * width, 0);
    source->move(origin);
    target->move(target_start);

    auto* animation = new QParallelAnimationGroup(this);
    auto* source_animation = new QPropertyAnimation(source, "pos", animation);
    source_animation->setStartValue(origin);
    source_animation->setEndValue(source_end);
    source_animation->setDuration(200);
    source_animation->setEasingCurve(QEasingCurve::InOutCubic);

    auto* target_animation = new QPropertyAnimation(target, "pos", animation);
    target_animation->setStartValue(target_start);
    target_animation->setEndValue(origin);
    target_animation->setDuration(200);
    target_animation->setEasingCurve(QEasingCurve::InOutCubic);

    d_->view_animation = animation;
    d_->transition_source = source;
    d_->transition_target = target;
    d_->transition_mode = mode;

    connect(animation, &QParallelAnimationGroup::finished, this, [this, animation] {
        if (d_->view_animation == animation) {
            finishViewTransition();
        }
    });
    animation->start();
}

void QmRibbonMainWindow::setCurrentView(QWidget* target, ViewMode mode)
{
    if (!target) {
        return;
    }

    if (auto* stacked_layout = qobject_cast<QStackedLayout*>(d_->central_widget->layout())) {
        stacked_layout->setStackingMode(QStackedLayout::StackOne);
    }
    target->move(0, 0);
    d_->central_widget->setCurrentWidget(target);
    for (int index = 0; index < d_->central_widget->count(); ++index) {
        auto* widget = d_->central_widget->widget(index);
        widget->setVisible(widget == target);
    }
    animateRibbonVisibility(mode == ViewMode::MainView);
    d_->view_mode = mode;
}

void QmRibbonMainWindow::animateRibbonVisibility(bool visible)
{
    if (d_->ribbon_animation && d_->ribbon_target_visible == visible) {
        return;
    }

    const bool was_animating = d_->ribbon_animation != nullptr;
    int current_height = d_->ribbon->height();
    if (d_->ribbon_animation) {
        current_height = d_->ribbon->maximumHeight();
        d_->ribbon_animation->stop();
        d_->ribbon_animation->deleteLater();
        d_->ribbon_animation = nullptr;
    }

    if (!isVisible() || !d_->view_animation_enabled) {
        if (!visible && d_->ribbon->isVisible()) {
            d_->ribbon_expanded_height = qMax(d_->ribbon_expanded_height, current_height);
        }
        d_->ribbon->setMaximumHeight(QWIDGETSIZE_MAX);
        d_->ribbon->setVisible(visible);
        return;
    }

    if (visible) {
        if (d_->ribbon->isVisible() && current_height > 0 && !was_animating) {
            d_->ribbon->setMaximumHeight(QWIDGETSIZE_MAX);
            return;
        }
        if (!d_->ribbon->isVisible()) {
            current_height = 0;
        }
        d_->ribbon_expanded_height = qMax(d_->ribbon_expanded_height, d_->ribbon->sizeHint().height());
        current_height = qMax(0, current_height);
        d_->ribbon->setMaximumHeight(current_height);
        d_->ribbon->show();
    } else {
        if (!d_->ribbon->isVisible()) {
            return;
        }
        current_height = qMax(0, current_height);
        d_->ribbon_expanded_height = qMax(d_->ribbon_expanded_height, current_height);
    }

    const int target_height = visible ? qMax(1, d_->ribbon_expanded_height) : 0;
    if (current_height == target_height) {
        d_->ribbon->setMaximumHeight(QWIDGETSIZE_MAX);
        d_->ribbon->setVisible(visible);
        return;
    }

    d_->ribbon_target_visible = visible;
    auto* animation = new QPropertyAnimation(d_->ribbon, "maximumHeight", this);
    animation->setStartValue(current_height);
    animation->setEndValue(target_height);
    animation->setDuration(200);
    animation->setEasingCurve(QEasingCurve::InOutCubic);
    d_->ribbon_animation = animation;

    connect(animation, &QPropertyAnimation::finished, this, [this, animation] {
        if (d_->ribbon_animation != animation) {
            return;
        }
        d_->ribbon_animation = nullptr;
        d_->ribbon->setVisible(d_->ribbon_target_visible);
        d_->ribbon->setMaximumHeight(QWIDGETSIZE_MAX);
        animation->deleteLater();
    });
    animation->start();
}

void QmRibbonMainWindow::finishRibbonAnimation()
{
    if (!d_->ribbon_animation) {
        return;
    }

    auto* animation = d_->ribbon_animation;
    d_->ribbon_animation = nullptr;
    animation->stop();
    animation->deleteLater();
    d_->ribbon->setVisible(d_->ribbon_target_visible);
    d_->ribbon->setMaximumHeight(QWIDGETSIZE_MAX);
}

void QmRibbonMainWindow::finishViewTransition()
{
    if (!d_->view_animation) {
        return;
    }

    auto* animation = d_->view_animation;
    const auto source = d_->transition_source;
    const auto target = d_->transition_target;
    const auto mode = d_->transition_mode;

    d_->view_animation = nullptr;
    d_->transition_source = nullptr;
    d_->transition_target = nullptr;

    animation->stop();
    animation->deleteLater();

    if (source) {
        source->move(0, 0);
    }
    if (target) {
        setCurrentView(target, mode);
    } else if (auto* stacked_layout = qobject_cast<QStackedLayout*>(d_->central_widget->layout())) {
        stacked_layout->setStackingMode(QStackedLayout::StackOne);
    }
}

void QmRibbonMainWindow::cancelViewTransition()
{
    if (!d_->view_animation) {
        return;
    }

    auto* animation = d_->view_animation;
    const auto source = d_->transition_source;
    const auto target = d_->transition_target;

    d_->view_animation = nullptr;
    d_->transition_source = nullptr;
    d_->transition_target = nullptr;

    animation->stop();
    animation->deleteLater();

    if (source) {
        source->move(0, 0);
    }
    if (target) {
        target->move(0, 0);
    }

    if (auto* stacked_layout = qobject_cast<QStackedLayout*>(d_->central_widget->layout())) {
        stacked_layout->setStackingMode(QStackedLayout::StackOne);
    }
    auto* current = d_->central_widget->currentWidget();
    for (int index = 0; index < d_->central_widget->count(); ++index) {
        auto* widget = d_->central_widget->widget(index);
        widget->setVisible(widget == current);
    }
}

void QmRibbonMainWindow::showEvent(QShowEvent* event)
{
    QmFramelessWindow::showEvent(event);
    finishViewTransition();
}
