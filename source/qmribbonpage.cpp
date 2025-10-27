#include "qmribbonpage.h"

#include "qmribbonsection.h"

#include <QFrame>
#include <QHBoxLayout>

struct QmRibbonPagePrivate {
    QHBoxLayout* lyt_section { nullptr };
};

QmRibbonPage::QmRibbonPage(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonPagePrivate)
{
    d_->lyt_section = new QHBoxLayout(this);
    d_->lyt_section->addSpacerItem(new QSpacerItem(20, 0, QSizePolicy::Expanding));
    d_->lyt_section->setContentsMargins(0, 0, 0, 0);
    d_->lyt_section->setSpacing(0);
}

QmRibbonPage::~QmRibbonPage() noexcept
{
    delete d_;
}

QmRibbonSection* QmRibbonPage::addSection(const QString& title, const QIcon& icon, QmRibbonSection::Features features)
{
    auto* section = new QmRibbonSection(this);
    section->setTitle(title);
    section->setIcon(icon);
    section->setFeatures(features);
    d_->lyt_section->insertWidget(d_->lyt_section->count() - 1, section);

    auto* separator = new QFrame(this);
    separator->setProperty("Style", "RibbonSectionSeparator");
    separator->setLineWidth(2);
    separator->setFrameShape(QFrame::VLine);
    d_->lyt_section->insertWidget(d_->lyt_section->count() - 1, separator);

    emit sectionCreated(section, QPrivateSignal());
    return section;
}

QSize QmRibbonPage::sizeHint() const
{
    return QSize(87, 100);
}