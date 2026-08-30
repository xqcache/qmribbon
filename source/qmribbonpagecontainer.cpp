#include "qmribbonpagecontainer.h"

#include "qmimageshadowwidget.h"

#include <QActionGroup>
#include <QApplication>
#include <QHBoxLayout>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QStyle>
#include <QStyleOption>
#include <QToolButton>
#include <QVBoxLayout>

struct QmRibbonPageContainerPrivate {

    QPropertyAnimation* animation { nullptr };
    QmImageShadowWidget* shadow_widget { nullptr };
    QHBoxLayout* lyt_shadow { nullptr };
    QStackedWidget* container_widget { nullptr };
    QToolButton* btn_ribbon_fold { nullptr };
    QMenu* fold_menu { nullptr };
    QAction* always_show_action { nullptr };
    QAction* tabs_only_action { nullptr };
};

QmRibbonPageContainer::QmRibbonPageContainer(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonPageContainerPrivate)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    setAttribute(Qt::WA_StyledBackground, true);
    initUi();
}

QmRibbonPageContainer::~QmRibbonPageContainer() noexcept
{
    delete d_;
}

void QmRibbonPageContainer::initUi()
{
    d_->shadow_widget = new QmImageShadowWidget(this);
    d_->shadow_widget->setPatchMargins(QMargins(20, 8, 20, 24));
    d_->shadow_widget->setShadowMargins(QMargins(8, 0, 8, 10));
    d_->container_widget = new QStackedWidget(d_->shadow_widget);

    d_->btn_ribbon_fold = new QToolButton(d_->shadow_widget);
    d_->btn_ribbon_fold->setObjectName("btn_ribbon_fold");
    d_->btn_ribbon_fold->setProperty("Style", "RibbonFoldButton");
    d_->btn_ribbon_fold->setArrowType(Qt::DownArrow);
    d_->btn_ribbon_fold->setToolTip(tr("Ribbon display options"));
    d_->btn_ribbon_fold->setFixedSize(20, 20);

    d_->fold_menu = new QMenu(d_->btn_ribbon_fold);
    auto* display_mode_group = new QActionGroup(d_->fold_menu);
    display_mode_group->setExclusive(true);
    d_->always_show_action = d_->fold_menu->addAction(tr("Always Show Ribbon"));
    d_->tabs_only_action = d_->fold_menu->addAction(tr("Show Tabs Only"));
    d_->always_show_action->setCheckable(true);
    d_->tabs_only_action->setCheckable(true);
    d_->always_show_action->setChecked(true);
    display_mode_group->addAction(d_->always_show_action);
    display_mode_group->addAction(d_->tabs_only_action);
    d_->btn_ribbon_fold->setMenu(d_->fold_menu);
    d_->btn_ribbon_fold->setPopupMode(QToolButton::InstantPopup);
    d_->shadow_widget->installEventFilter(this);

    d_->lyt_shadow = new QHBoxLayout(d_->shadow_widget);
    d_->lyt_shadow->setContentsMargins(0, 0, 30, 0);
    d_->lyt_shadow->setSpacing(0);
    d_->lyt_shadow->addWidget(d_->container_widget);

    auto* lyt_main = new QVBoxLayout(this);
    lyt_main->setContentsMargins(0, 0, 0, 0);
    lyt_main->addWidget(d_->shadow_widget);
    d_->shadow_widget->setShadowEnabled(false);

    connect(d_->always_show_action, &QAction::triggered, this, [this] {
        emit floatingRequested(false);
    });
    connect(d_->tabs_only_action, &QAction::triggered, this, [this] {
        emit floatingRequested(true);
        if (isFloating()) {
            hideWithAnimation();
        }
    });
}

void QmRibbonPageContainer::setFloating(bool floating)
{
    const QSize expanded_size = property("Size").toSize();
    stopAnimation();
    if (expanded_size.isValid() && !expanded_size.isEmpty()) {
        resize(expanded_size);
    }

    if (floating) {
        setProperty("geo", geometry());
        qApp->installEventFilter(this);
        setAttribute(Qt::WA_StyledBackground, false);
        d_->shadow_widget->setShadowEnabled(true);

        auto geo = geometry();
        const QMargins shadow_margins = d_->shadow_widget->shadowMargins();
        d_->lyt_shadow->setContentsMargins(shadow_margins.left(), 0, 30, shadow_margins.bottom());
        geo.setHeight(geo.height() + shadow_margins.top() + shadow_margins.bottom());
        setGeometry(geo);
    } else {
        qApp->removeEventFilter(this);
        setAttribute(Qt::WA_StyledBackground, true);
        d_->shadow_widget->setShadowEnabled(false);
        d_->lyt_shadow->setContentsMargins(0, 0, 30, 0);
        setGeometry(property("geo").toRect());
        show();
    }
    setProperty("Floating", floating);
    d_->always_show_action->setChecked(!floating);
    d_->tabs_only_action->setChecked(floating);
    updateFoldButtonGeometry();
}

bool QmRibbonPageContainer::isFloating() const
{
    return property("Floating").toBool();
}

void QmRibbonPageContainer::setFoldButtonVisible(bool visible)
{
    d_->btn_ribbon_fold->setVisible(visible);
    updateFoldButtonGeometry();
}

bool QmRibbonPageContainer::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type()) {
    case QEvent::Resize: {
        if (watched == d_->shadow_widget) {
            updateFoldButtonGeometry();
        } else if (isFloating() && watched == parentWidget()) {
            const QMargins parent_margins = parentWidget()->contentsMargins();
            const int available_width = qMax(0, parentWidget()->width() - parent_margins.left() - parent_margins.right());

            auto geo = geometry();
            geo.setWidth(available_width);
            setGeometry(geo);

            auto expanded_size = property("Size").toSize();
            expanded_size.setWidth(available_width);
            setProperty("Size", expanded_size);
        }
    } break;
    case QEvent::MouseButtonPress: {
        auto* widget = qApp->widgetAt(static_cast<QMouseEvent*>(event)->globalPosition().toPoint());
        const bool is_fold_menu = widget && (widget == d_->fold_menu || d_->fold_menu->isAncestorOf(widget));
        if (widget && (isAncestorOf(widget) || is_fold_menu || widget->property("Style").toString() == "RibbonTabButton")) {
            showWithAnimation();
        } else {
            hideWithAnimation();
        }
    } break;
    default:
        break;
    }

    return QWidget::eventFilter(watched, event);
}

void QmRibbonPageContainer::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (!d_->animation) {
        setProperty("Size", event->size());
    }
    updateFoldButtonGeometry();
}

void QmRibbonPageContainer::updateFoldButtonGeometry()
{
    if (!d_->btn_ribbon_fold || !d_->btn_ribbon_fold->isVisible()) {
        return;
    }

    QMargins content_margins;

    if (isFloating()) {
        content_margins = d_->shadow_widget->shadowMargins();
        content_margins.setRight(content_margins.right() + d_->shadow_widget->blurRadius() / 2);
        content_margins.setBottom(content_margins.bottom() + d_->shadow_widget->blurRadius() / 2);
    }
    const QRect content_rect = d_->shadow_widget->rect().marginsRemoved(content_margins);
    const int x = qMax(content_rect.left(), content_rect.right() - d_->btn_ribbon_fold->width() + 1);
    const int y = qMax(content_rect.top(), content_rect.bottom() - d_->btn_ribbon_fold->height() + 1);

    d_->btn_ribbon_fold->move(x, y);
    d_->btn_ribbon_fold->raise();
}

void QmRibbonPageContainer::stopAnimation()
{
    if (!d_->animation) {
        return;
    }

    auto* animation = d_->animation;
    d_->animation = nullptr;
    animation->stop();
    animation->deleteLater();
}

void QmRibbonPageContainer::showWithAnimation()
{
    if (isVisible() && !d_->animation) {
        return;
    }
    if (d_->animation) {
        if (d_->animation->property("Show").toBool()) {
            return;
        }
    }

    const QSize expanded_size = property("Size").toSize();
    const QSize start_size = isVisible() ? size() : QSize(expanded_size.width(), 0);
    stopAnimation();

    auto* animation = new QPropertyAnimation(this, "size", this);
    d_->animation = animation;
    animation->setProperty("Show", true);
    connect(animation, &QPropertyAnimation::finished, this, [this, animation, expanded_size] {
        if (d_->animation != animation) {
            return;
        }
        d_->animation = nullptr;
        resize(expanded_size);
        animation->deleteLater();
    });
    animation->setStartValue(start_size);
    animation->setEndValue(expanded_size);
    animation->setDuration(200);
    animation->setEasingCurve(QEasingCurve::OutCubic);
    setVisible(true);
    animation->start();
}

void QmRibbonPageContainer::hideWithAnimation()
{
    if (!isVisible() && !d_->animation) {
        return;
    }

    if (d_->animation) {
        if (!d_->animation->property("Show").toBool()) {
            return;
        }
    }

    const QSize expanded_size = property("Size").toSize();
    const QSize start_size = size();
    stopAnimation();

    auto* animation = new QPropertyAnimation(this, "size", this);
    d_->animation = animation;
    animation->setProperty("Show", false);
    connect(animation, &QPropertyAnimation::finished, this, [this, animation, expanded_size] {
        if (d_->animation != animation) {
            return;
        }
        d_->animation = nullptr;
        resize(expanded_size);
        setVisible(false);
        animation->deleteLater();
    });
    animation->setStartValue(start_size);
    animation->setEndValue(QSize(expanded_size.width(), 0));
    animation->setDuration(200);
    animation->setEasingCurve(QEasingCurve::InCubic);
    animation->start();
}

int QmRibbonPageContainer::addWidget(QWidget* widget)
{
    return d_->container_widget->addWidget(widget);
}

QWidget* QmRibbonPageContainer::currentWidget() const
{
    return d_->container_widget->currentWidget();
}

int QmRibbonPageContainer::currentIndex() const
{
    return d_->container_widget->currentIndex();
}

void QmRibbonPageContainer::setCurrentIndex(int index)
{
    return d_->container_widget->setCurrentIndex(index);
}
