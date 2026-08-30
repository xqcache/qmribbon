#include "qmribbonpage.h"
#include "qmribbongroup.h"

struct QmRibbonPagePrivate {
    QString title = "Unamed";
};

QmRibbonPage::QmRibbonPage(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonPagePrivate)
{
}

QmRibbonPage::~QmRibbonPage() noexcept
{
    delete d_;
}

void QmRibbonPage::setTitle(const QString& title)
{
    if (title == d_->title) {
        return;
    }
    d_->title = title;
    emit titleChanged(title);
}

QString QmRibbonPage::title() const
{
    return d_->title;
}

QmRibbonGroup* QmRibbonPage::addGroup(const QString& title, const QIcon& icon)
{
    auto* group = new QmRibbonGroup(title, icon, this);

    return group;
}