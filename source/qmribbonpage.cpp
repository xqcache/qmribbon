#include "qmribbonpage.h"

#include "qmribbonsection.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QPropertyAnimation>

struct QmRibbonPagePrivate {
    QHBoxLayout* lyt_section { nullptr };
};

QmRibbonPage::QmRibbonPage(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonPagePrivate)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d_->lyt_section = new QHBoxLayout(this);
    d_->lyt_section->setContentsMargins(0, 0, 0, 0);
    d_->lyt_section->setSpacing(0);
}

QmRibbonPage::~QmRibbonPage() noexcept
{
    delete d_;
}

QmRibbonSection* QmRibbonPage::addSection(const QString& title, const QIcon& icon, QmRibbonSection::Features features)
{
    if (d_->lyt_section->count() > 0) {
        auto* separator = new QFrame(this);
        separator->setProperty("Style", "RibbonSectionSeparator");
        separator->setLineWidth(2);
        separator->setFrameShape(QFrame::VLine);
        d_->lyt_section->insertWidget(d_->lyt_section->count() , separator);
    }

    auto* section = new QmRibbonSection(this);
    section->setTitle(title);
    section->setIcon(icon);
    section->setFeatures(features);
    d_->lyt_section->insertWidget(d_->lyt_section->count() , section);

    emit sectionCreated(section, QPrivateSignal());
    return section;
}

QSize QmRibbonPage::sizeHint() const
{
    return QSize(87, 100);
}

void QmRibbonPage::showAnimation()
{
    auto pos = this->pos();
    auto* anim = new QPropertyAnimation(this, "pos", this);
    anim->setStartValue(QPoint(width(), pos.y()));
    anim->setEndValue(pos);
    anim->setDuration(200);
    anim->setEasingCurve(QEasingCurve::InOutBack);
    anim->start(QPropertyAnimation::DeleteWhenStopped);
}