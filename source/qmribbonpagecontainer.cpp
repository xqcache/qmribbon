#include "qmribbonpagecontainer.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyle>
#include <QStyleOption>

QmRibbonPageContainer::QmRibbonPageContainer(QWidget* parent)
    : QStackedWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    setAttribute(Qt::WA_StyledBackground);
}

void QmRibbonPageContainer::setFloating(bool floating)
{
    if (floating) {
        qApp->installEventFilter(this);
        setStyleSheet("QmRibbonPageContainer {margin: 2px; margin-top: 0px; border-radius: 5px;}");
    } else {
        qApp->removeEventFilter(this);
        setStyleSheet("QmRibbonPageContainer{margin: 0px; border-radius: 0px;}");
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

    return QStackedWidget::eventFilter(watched, event);
}

void QmRibbonPageContainer::showWithAnimation()
{
    if (isVisible()) {
        return;
    }
    setVisible(true);
    auto size = this->size();
    auto* anim = new QPropertyAnimation(this, "size", this);
    anim->setStartValue(QSize(size.width(), 0));
    anim->setEndValue(size);
    anim->setDuration(200);
    anim->setEasingCurve(QEasingCurve::InOutBack);
    anim->start(QPropertyAnimation::DeleteWhenStopped);
}

void QmRibbonPageContainer::hideWithAnimation()
{
    if (!isVisible()) {
        return;
    }
    auto size = this->size();
    auto* anim = new QPropertyAnimation(this, "size", this);
    connect(anim, &QPropertyAnimation::finished, this, [this, size] {
        resize(size);
        setVisible(false);
    });
    anim->setEndValue(QSize(size.width(), 0));
    anim->setStartValue(size);
    anim->setDuration(200);
    anim->setEasingCurve(QEasingCurve::InOutBack);
    anim->start(QPropertyAnimation::DeleteWhenStopped);
}
