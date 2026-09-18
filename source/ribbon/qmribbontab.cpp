#include "qmribbontab.h"

#include "qmribbontheme.h"
#include "qmribbonthememgr.h"

#include <QFontMetrics>
#include <QPainter>

QmRibbonTab::QmRibbonTab(QWidget* parent)
    : QAbstractButton(parent)
{
    setObjectName(QStringLiteral("RibbonTab"));
    setCheckable(true);
    setFocusPolicy(Qt::NoFocus);
    setCursor(Qt::ArrowCursor);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFixedHeight(QmRibbonMetrics::tab_bar_height);
}

QmRibbonTab::QmRibbonTab(const QString& text, QWidget* parent)
    : QmRibbonTab(parent)
{
    setText(text);
}

QString QmRibbonTab::contextualGroup() const
{
    return contextual_group_;
}

void QmRibbonTab::setContextualGroup(const QString& group)
{
    if (contextual_group_ == group) {
        return;
    }
    contextual_group_ = group;
    update();
}

QColor QmRibbonTab::contextualColor() const
{
    return contextual_color_;
}

void QmRibbonTab::setContextualColor(const QColor& color)
{
    if (contextual_color_ == color) {
        return;
    }
    contextual_color_ = color;
    update();
}

bool QmRibbonTab::isContextual() const
{
    return !contextual_group_.isEmpty();
}

QSize QmRibbonTab::sizeHint() const
{
    const int width = fontMetrics().horizontalAdvance(text()) + 2 * QmRibbonMetrics::tab_horizontal_padding;

    return { qMax(width, QmRibbonMetrics::tab_min_width), QmRibbonMetrics::tab_bar_height };
}

QSize QmRibbonTab::minimumSizeHint() const
{
    return sizeHint();
}

void QmRibbonTab::enterEvent(QEnterEvent* event)
{
    QAbstractButton::enterEvent(event);
    hovered_ = true;
    update();
}

void QmRibbonTab::leaveEvent(QEvent* event)
{
    QAbstractButton::leaveEvent(event);
    hovered_ = false;
    update();
}

void QmRibbonTab::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QmRibbonTheme& theme = QmRibbonThemeMgr::current();
    const bool selected = isChecked();
    const QRect body = rect().adjusted(2, 3, -2, 0);

    if (selected) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(theme.contentBackground());
        painter.drawRoundedRect(body, 4, 4);
    } else if (hovered_) {
        painter.setPen(Qt::NoPen);
        painter.setBrush(theme.hover());
        painter.drawRoundedRect(body, 4, 4);
    }

    QColor indicator = contextual_color_.isValid() ? contextual_color_ : theme.accent();

    if (selected) {
        const int inset = 10;
        const QRect bar(body.left() + inset, height() - QmRibbonMetrics::tab_indicator_height,
                        qMax(body.width() - 2 * inset, 8), QmRibbonMetrics::tab_indicator_height);

        painter.setPen(Qt::NoPen);
        painter.setBrush(indicator);
        painter.drawRoundedRect(bar, 1.5, 1.5);
    }

    QColor text_color = theme.text();
    if (selected) {
        text_color = indicator;
    } else if (isContextual()) {
        text_color = theme.secondaryText();
    }

    painter.setPen(text_color);
    painter.drawText(body, Qt::AlignCenter, text());
}
