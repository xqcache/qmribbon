#include "qmribbonmainwindow.h"

#include "qmribbon.h"
#include "qmribbonbackstageview.h"

#include <QEasingCurve>
#include <QParallelAnimationGroup>
#include <QPointer>
#include <QPropertyAnimation>
#include <QStackedLayout>
#include <QStackedWidget>
#include <QVBoxLayout>

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
};

QmRibbonMainWindow::QmRibbonMainWindow(QWidget* parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags)
    , d_(new QmRibbonMainWindowPrivate)
{
    initUi();
    connectSignals();
}

QmRibbonMainWindow::~QmRibbonMainWindow() noexcept
{
    cancelViewTransition();
    delete d_;
}

void QmRibbonMainWindow::initUi()
{
    d_->ribbon = QmRibbon::install(this);
    d_->central_widget = new QStackedWidget(this);
    setCentralWidget(d_->central_widget);

    auto* lyt_main = new QVBoxLayout(d_->central_widget);
    lyt_main->setContentsMargins(0, 0, 0, 0);
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
    if (source == target || !source || !isVisible()) {
        setCurrentView(target, mode);
        return;
    }

    auto* stacked_layout = qobject_cast<QStackedLayout*>(d_->central_widget->layout());
    if (!stacked_layout) {
        setCurrentView(target, mode);
        return;
    }

    d_->view_mode = mode;
    d_->ribbon->setVisible(mode == ViewMode::MainView);
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
    d_->ribbon->setVisible(mode == ViewMode::MainView);
    d_->view_mode = mode;
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
    QMainWindow::showEvent(event);
    finishViewTransition();
}
