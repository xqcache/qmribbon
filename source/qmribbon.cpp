#include "qmribbon.h"
#include "qmribbonpage.h"
#include "qmribbontabbar.h"
#include "qmribbontitlebar.h"
#include <QStackedWidget>
#include <QVBoxLayout>

struct QmRibbonPrivate {
    QmRibbonTitleBar* titlebar { nullptr };
    QmRibbonTabBar* tabbar { nullptr };
    QStackedWidget* page_stack { nullptr };
};

QmRibbon::QmRibbon(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonPrivate)
{
    setObjectName("Ribbon");
    initUi();
    connectSignals();
}

QmRibbon::~QmRibbon() noexcept
{
    delete d_;
}

void QmRibbon::initUi()
{
    auto* lyt_main = new QVBoxLayout(this);
    lyt_main->setContentsMargins(0, 0, 0, 0);
    lyt_main->setSpacing(0);

    d_->titlebar = new QmRibbonTitleBar(this);
    d_->tabbar = new QmRibbonTabBar(this);
    d_->page_stack = new QStackedWidget(this);

    lyt_main->addWidget(d_->titlebar);
    lyt_main->addWidget(d_->tabbar);
    lyt_main->addWidget(d_->page_stack);
}

void QmRibbon::connectSignals()
{
}

QmRibbonPage* QmRibbon::addPage(const QString& title)
{
    auto* page = new QmRibbonPage(this);
    d_->page_stack->addWidget(page);
    return page;
}