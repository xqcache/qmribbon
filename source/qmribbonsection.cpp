#include "qmribbonsection.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>

struct QmRibbonSectionPrivate {
    QWidget* widget { nullptr };
    QLabel* lbl_title { nullptr };
    QToolButton* btn_expand { nullptr };
    QToolButton* btn_section { nullptr };
};

QmRibbonSection::QmRibbonSection(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    initUi();
}

QmRibbonSection::~QmRibbonSection() noexcept
{
    delete d_;
}

void QmRibbonSection::initUi()
{
    d_->btn_section = new QToolButton(this);
    d_->btn_section->setIconSize(QSize(50, 50));
    d_->btn_section->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    auto* lyt_bottom = new QHBoxLayout();
    d_->btn_expand = new QToolButton(this);
    d_->lbl_title = new QLabel(this);

    auto* lyt_main = new QVBoxLayout(this);
    lyt_main->addWidget(d_->btn_section);
    if (d_->widget) {
        if (!isWidthSufficient()) {
            d_->btn_section->setVisible(true);
            d_->widget->setVisible(false);
        } else {
            d_->btn_section->setVisible(false);
            d_->widget->setVisible(true);
        }
    }

    lyt_main->addLayout(lyt_bottom);
}

void QmRibbonSection::setTitle(const QString& title)
{
    d_->btn_section->setText(title);
    d_->lbl_title->setText(title);
}

void QmRibbonSection::setIcon(const QIcon& icon)
{
    d_->btn_section->setIcon(icon);
}

void QmRibbonSection::setWidget(QWidget* widget)
{
    if (!widget) {
        return;
    }
    if (d_->widget) {
        layout()->removeWidget(d_->widget);
        d_->widget->deleteLater();
    }
    d_->widget = widget;
    d_->widget->setParent(this);
    static_cast<QBoxLayout*>(layout())->insertWidget(1, d_->widget);
}

QSize QmRibbonSection::minimumSizeHint() const
{
    return d_->btn_section->sizeHint();
}

QSize QmRibbonSection::sizeHint() const
{
    if (!d_->widget || !d_->widget->isVisible()) {
        return minimumSizeHint();
    }
    return d_->widget->sizeHint();
}

void QmRibbonSection::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);

    if (!d_->widget) {
        d_->btn_section->setVisible(true);
        return;
    }
    if (!isWidthSufficient()) {
        d_->btn_section->setVisible(true);
        d_->widget->setVisible(false);
    } else {
        d_->btn_section->setVisible(false);
        d_->widget->setVisible(true);
    }
}

bool QmRibbonSection::isWidthSufficient() const
{
    if (!d_->widget) {
        return false;
    }
    return width() >= d_->widget->sizeHint().width();
}
