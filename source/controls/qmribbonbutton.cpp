#include "qmribbonbutton.h"

#include "qmribbontheme.h"

#include <QAction>

QmRibbonButton::QmRibbonButton(QWidget* parent)
    : QToolButton(parent)
{
    init();
}

QmRibbonButton::QmRibbonButton(QAction* action, Size size, QWidget* parent)
    : QToolButton(parent)
{
    init();
    setDefaultAction(action);
    setSize(size);
}

void QmRibbonButton::init()
{
    setAutoRaise(true);
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::ArrowCursor);
    applySize();
}

QmRibbonButton::Size QmRibbonButton::size() const
{
    return size_;
}

void QmRibbonButton::setSize(Size size)
{
    size_ = size;
    applySize();
}

void QmRibbonButton::applySize()
{
    switch (size_) {
    case Size::Large:
        setObjectName(QStringLiteral("RibbonLargeButton"));
        setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        setIconSize(QSize(QmRibbonMetrics::large_button_icon, QmRibbonMetrics::large_button_icon));
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        setMinimumWidth(QmRibbonMetrics::large_button_min_width);
        setMaximumWidth(QmRibbonMetrics::large_button_max_width);
        setFixedHeight(QmRibbonMetrics::group_content_height);
        break;

    case Size::Medium:
        setObjectName(QStringLiteral("RibbonMediumButton"));
        setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        setIconSize(QSize(QmRibbonMetrics::medium_button_icon, QmRibbonMetrics::medium_button_icon));
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        setMinimumWidth(QmRibbonMetrics::medium_button_min_width);
        setMaximumWidth(QmRibbonMetrics::medium_button_max_width);
        setFixedHeight(QmRibbonMetrics::medium_button_height);
        break;

    case Size::Small:
        setObjectName(QStringLiteral("RibbonSmallButton"));
        setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        setIconSize(QSize(QmRibbonMetrics::small_button_icon, QmRibbonMetrics::small_button_icon));
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        setMinimumWidth(QmRibbonMetrics::small_button_min_width);
        setMaximumWidth(QmRibbonMetrics::small_button_max_width);
        setFixedHeight(QmRibbonMetrics::small_button_height);
        break;
    }
}

QSize QmRibbonButton::sizeHint() const
{
    QSize hint = QToolButton::sizeHint();

    switch (size_) {
    case Size::Large:
        hint.setWidth(
            qBound(QmRibbonMetrics::large_button_min_width, hint.width(), QmRibbonMetrics::large_button_max_width));
        hint.setHeight(QmRibbonMetrics::group_content_height);
        break;

    case Size::Medium:
        hint.setWidth(
            qBound(QmRibbonMetrics::medium_button_min_width, hint.width(), QmRibbonMetrics::medium_button_max_width));
        hint.setHeight(QmRibbonMetrics::medium_button_height);
        break;

    case Size::Small:
        hint.setWidth(
            qBound(QmRibbonMetrics::small_button_min_width, hint.width(), QmRibbonMetrics::small_button_max_width));
        hint.setHeight(QmRibbonMetrics::small_button_height);
        break;
    }

    return hint;
}

QSize QmRibbonButton::minimumSizeHint() const
{
    return sizeHint();
}
