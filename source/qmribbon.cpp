#include "qmribbon.h"

#include "qmribbonfloatcontainer.h"
#include "qmribbonpage.h"
#include "qmribbonpagecontainer.h"
#include "qmribbontabbar.h"
#include "qmribbontitlebar.h"

#include <QApplication>
#include <QEvent>
#include <QFile>
#include <QFontDatabase>
#include <QMainWindow>
#include <QVBoxLayout>

int qInitResources_qmribbon_assets();
int qCleanupResources_qmribbon_assets();
namespace {
struct AssetsInitializer {
    AssetsInitializer()
    {
        qInitResources_qmribbon_assets();
        QFontDatabase::addApplicationFont(":/qmribbon/fonts/uifont");
    }
    ~AssetsInitializer() noexcept
    {
        qCleanupResources_qmribbon_assets();
    }
};

void initializeAssets()
{
    static AssetsInitializer ins;
}
}

struct QmRibbonPrivate {
    QmRibbonTitleBar* titlebar { nullptr };
    QmRibbonTabBar* tabbar { nullptr };
    QmRibbonPageContainer* page_container { nullptr };

    QmRibbonFloatContainer* float_container { nullptr };
    QmRibbon::Features feature = QmRibbon::Features();
};

QmRibbon::QmRibbon(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonPrivate)
{
    initializeAssets();
    initUi();

    QFile style(":/qmribbon/styles/default");
    if (style.open(QFile::ReadOnly)) {
        setStyleSheet(style.readAll());
        style.close();
    }
}

QmRibbon::~QmRibbon() noexcept
{
    delete d_->page_container;
    delete d_;
    qApp->setProperty("QmRibbon-Window", QVariant());
}

QmRibbonTitleBar* QmRibbon::titleBar() const
{
    return d_->titlebar;
}

void QmRibbon::initUi()
{
    QVBoxLayout* lyt_main = new QVBoxLayout(this);
    lyt_main->setContentsMargins(0, 0, 0, 0);
    lyt_main->setSpacing(0);

    d_->titlebar = new QmRibbonTitleBar(this);
    d_->tabbar = new QmRibbonTabBar(this);
    d_->page_container = new QmRibbonPageContainer(this);

    lyt_main->addWidget(d_->titlebar);
    lyt_main->addWidget(d_->tabbar);
    lyt_main->addWidget(d_->page_container);

    d_->float_container = new QmRibbonFloatContainer(parentWidget());
    d_->float_container->setVisible(false);
}

bool QmRibbon::event(QEvent* event)
{

    return QWidget::event(event);
}

void QmRibbon::setWindow(QMainWindow* window)
{
    qApp->setProperty("QmRibbon-Window", QVariant::fromValue(window));
    window->setMenuWidget(this);
}

QmRibbon* QmRibbon::install(QMainWindow* window)
{
    auto* ribbon = new QmRibbon(window);
    ribbon->setWindow(window);
    return ribbon;
}

QmRibbonPage* QmRibbon::addPage(const QString& title, const QIcon& icon)
{
    auto* page = new QmRibbonPage(title, icon, d_->page_container);
    d_->page_container->addWidget(page);
    d_->tabbar->addTab(icon, title);
    return page;
}

void QmRibbon::updateWidgetGeometry()
{
}