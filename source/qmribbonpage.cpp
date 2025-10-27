#include "qmribbonpage.h"

#include "qmribbonsection.h"

#include <QHBoxLayout>

struct QmRibbonPagePrivate {
    QString title;
    QIcon icon;
};

QmRibbonPage::QmRibbonPage(QWidget* parent)
    : QmRibbonPage("", QIcon(), parent)
{
}

QmRibbonPage::QmRibbonPage(const QString& title, QWidget* parent)
    : QmRibbonPage(title, QIcon(), parent)
{
}

QmRibbonPage::QmRibbonPage(const QString& title, const QIcon& icon, QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonPagePrivate)
{
    setLayout(new QHBoxLayout(this));
}

QmRibbonPage::~QmRibbonPage() noexcept
{
    delete d_;
}

void QmRibbonPage::setTitle(const QString& title)
{
    d_->title = title;
}

void QmRibbonPage::setIcon(const QIcon& icon)
{
    d_->icon = icon;
}

QmRibbonSection* QmRibbonPage::addSection(const QString& title, const QIcon& icon)
{
    auto* section = new QmRibbonSection(this);
    section->setTitle(title);
    section->setIcon(icon);
    static_cast<QHBoxLayout*>(layout())->addWidget(section);
    return section;
}

QSize QmRibbonPage::sizeHint() const
{
    return QSize(87, 100);
}