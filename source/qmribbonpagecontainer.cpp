#include "qmribbonpagecontainer.h"

#include "qmimageshadowwidget.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStackedWidget>
#include <QStyle>
#include <QStyleOption>
#include <QVBoxLayout>

struct QmRibbonPageContainerPrivate {

    QPropertyAnimation* animation { nullptr };
    QmImageShadowWidget* shadow_widget { nullptr };
    QStackedWidget* container_widget { nullptr };
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
    d_->container_widget = new QStackedWidget(d_->shadow_widget);

    auto* lyt_shadow = new QVBoxLayout(d_->shadow_widget);
    lyt_shadow->setContentsMargins(0, 0, 0, 0);
    lyt_shadow->addWidget(d_->container_widget);

    auto* lyt_main = new QVBoxLayout(this);
    lyt_main->setContentsMargins(0, 0, 0, 0);
    lyt_main->addWidget(d_->shadow_widget);
    d_->shadow_widget->setShadowEnabled(false);
}

void QmRibbonPageContainer::setFloating(bool floating)
{
    if (floating) {
        setProperty("geo", geometry());
        qApp->installEventFilter(this);
        setAttribute(Qt::WA_StyledBackground, false);
        d_->shadow_widget->setShadowEnabled(true);

        auto geo = geometry();
        geo.setHeight(geo.height() + d_->shadow_widget->contentGeometry().y());
        setGeometry(geo);

    } else {
        qApp->removeEventFilter(this);
        setAttribute(Qt::WA_StyledBackground, true);
        d_->shadow_widget->setShadowEnabled(false);
        setGeometry(property("geo").toRect());
    }
    setProperty("Floating", floating);
}

bool QmRibbonPageContainer::isFloating() const
{
    return property("Floating").toBool();
}

bool QmRibbonPageContainer::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress: {
        auto* widget = qApp->widgetAt(static_cast<QMouseEvent*>(event)->globalPosition().toPoint());
        if (widget && (isAncestorOf(widget) || widget->property("Style").toString() == "RibbonTabButton")) {
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
}

void QmRibbonPageContainer::showWithAnimation()
{
    if (isVisible() && !d_->animation) {
        return;
    }
    setVisible(true);

    if (d_->animation) {
        if (!d_->animation->property("Show").toBool()) {
            d_->animation->stop();
        } else {
            return;
        }
    }
    auto size = property("Size").toSize();
    d_->animation = new QPropertyAnimation(this, "size", this);
    d_->animation->setProperty("Show", true);
    connect(d_->animation, &QPropertyAnimation::finished, this, [this, size] {
        d_->animation = nullptr;
    });
    d_->animation->setStartValue(QSize(size.width(), 0));
    d_->animation->setEndValue(size);
    d_->animation->setDuration(200);
    d_->animation->setEasingCurve(QEasingCurve::InOutBack);
    d_->animation->start(QPropertyAnimation::DeleteWhenStopped);
}

void QmRibbonPageContainer::hideWithAnimation()
{
    if (!isVisible() && !d_->animation) {
        return;
    }

    if (d_->animation) {
        if (d_->animation->property("Show").toBool()) {
            d_->animation->stop();
        } else {
            return;
        }
    }

    auto size = property("Size").toSize();
    d_->animation = new QPropertyAnimation(this, "size", this);
    d_->animation->setProperty("Show", false);
    connect(d_->animation, &QPropertyAnimation::finished, this, [this, size] {
        d_->animation = nullptr;
        resize(size);
        setVisible(false);
    });
    d_->animation->setEndValue(QSize(size.width(), 0));
    d_->animation->setStartValue(size);
    d_->animation->setDuration(200);
    d_->animation->setEasingCurve(QEasingCurve::InOutBack);
    d_->animation->start(QPropertyAnimation::DeleteWhenStopped);
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
