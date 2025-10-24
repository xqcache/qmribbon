#include "qmribbonfloatcontainer.h"

#include <QHBoxLayout>

QmRibbonFloatContainer::QmRibbonFloatContainer(QWidget* parent)
    : QWidget(parent)
    , layout_(new QHBoxLayout(this))
{
    layout_->setContentsMargins(0, 0, 0, 0);
}

void QmRibbonFloatContainer::setWidget(QWidget* widget)
{
    if (!widget) {
        return;
    }
    release();
    widget_ = widget;
    widget_parent_ = widget->parentWidget();
    layout_->addWidget(widget_);
}

void QmRibbonFloatContainer::release()
{
    if (widget_) {
        layout_->removeWidget(widget_);
        widget_->setParent(widget_parent_);
    }
    widget_ = nullptr;
    widget_parent_ = nullptr;
}