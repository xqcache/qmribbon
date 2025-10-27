#include "qmribbonsection.h"

#include <QLabel>
#include <QPainter>
#include <QToolButton>
#include <QVBoxLayout>

struct QmRibbonSectionPrivate {
    QWidget* widget { nullptr };
    QToolButton* btn_section { nullptr };
    QToolButton* btn_expand { nullptr };

    QmRibbonSection::Features features = QmRibbonSection::Features();
};

QmRibbonSection::QmRibbonSection(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonSectionPrivate)
{
    setAttribute(Qt::WA_StyledBackground);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    initUi();
}

QmRibbonSection::~QmRibbonSection() noexcept
{
    delete d_;
}

void QmRibbonSection::initUi()
{
    d_->btn_section = new QToolButton(this);
    d_->btn_section->setText("Untitled");
    d_->btn_section->setIconSize(QSize(50, 50));
    d_->btn_section->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    d_->btn_expand = new QToolButton(this);
    d_->btn_expand->setProperty("Style", "RibbonSectionExpandButton");
    d_->btn_expand->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    d_->btn_expand->setText("Expand");

    auto* lyt_main = new QVBoxLayout(this);
    lyt_main->setContentsMargins(0, 0, 0, 0);
    lyt_main->addWidget(d_->btn_section, 1);
    lyt_main->addWidget(d_->btn_expand, 0, Qt::AlignRight);
    lyt_main->setSpacing(0);

    if (d_->widget) {
        if (!isWidthSufficient()) {
            d_->btn_section->setVisible(true);
            d_->widget->setVisible(false);
        } else {
            d_->btn_section->setVisible(false);
            d_->widget->setVisible(true);
        }
    }
}

void QmRibbonSection::setTitle(const QString& title)
{
    d_->btn_section->setText(title);
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
    d_->widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    static_cast<QBoxLayout*>(layout())->insertWidget(1, d_->widget, 1);

    if (!isWidthSufficient()) {
        d_->btn_section->setVisible(true);
        d_->widget->setVisible(false);
    } else {
        d_->btn_section->setVisible(false);
        d_->widget->setVisible(true);
    }
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

QSize QmRibbonSection::sizeHint() const
{
    QSize size = QWidget::sizeHint();
    int expand_width = d_->features.testAnyFlag(NoExpandButton) ? 0 : d_->btn_expand->width() * 2;
    int title_width = d_->features.testAnyFlag(NoTitle) ? 0 : fontMetrics().boundingRect(d_->btn_section->text()).width();
    size.setWidth(qMax(expand_width + layout()->spacing() + expand_width, size.width()));
    return size;
}

bool QmRibbonSection::isWidthSufficient() const
{
    if (!d_->widget) {
        return false;
    }
    return width() >= d_->widget->sizeHint().width();
}

void QmRibbonSection::paintEvent(QPaintEvent* event)
{
    if (d_->features.testAnyFlag(NoTitle)) {
        return;
    }
    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing, true);

    auto title_r = rect();
    title_r.setTop(d_->btn_expand->geometry().top());
    title_r.setBottom(d_->btn_expand->geometry().bottom());
    painter.drawText(title_r, Qt::AlignCenter, d_->btn_section->text());
}

void QmRibbonSection::setFeature(Feature feature, bool on)
{
    if (on) {
        d_->features |= feature;
    } else {
        d_->features &= ~feature;
    }
    setFeatures(d_->features, on);
}

void QmRibbonSection::setFeatures(Features features, bool on)
{
    if (on) {
        d_->features |= features;
    } else {
        d_->features &= ~features;
    }
    d_->btn_expand->setVisible(!d_->features.testAnyFlag(NoExpandButton));
    update();
}

QmRibbonSection::Features QmRibbonSection::features() const
{
    return d_->features;
}