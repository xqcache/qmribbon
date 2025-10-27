#include "qmribbontabbar.h"

#include <QTabBar>

struct QmRibbonTabBarPrivate {
    QTabBar* tabbar { nullptr };
};

QmRibbonTabBar::QmRibbonTabBar(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonTabBarPrivate)
{
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    initUi();
}

QmRibbonTabBar::~QmRibbonTabBar() noexcept
{
    delete d_;
}

void QmRibbonTabBar::initUi()
{
    d_->tabbar = new QTabBar(this);
}

void QmRibbonTabBar::addTab(const QString& title, const QIcon& icon)
{
    d_->tabbar->addTab(icon, title);
}

QSize QmRibbonTabBar::sizeHint() const
{
    return d_->tabbar->sizeHint();
}
