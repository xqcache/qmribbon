#include "qmribbon.h"

#include "qmribbonpage.h"
#include "qmribbonpagecontainer.h"
#include "qmribbonsection.h"
#include "qmribbontabbar.h"
#include "qmribbontitlebar.h"

#include <QApplication>
#include <QEvent>
#include <QFile>
#include <QFontDatabase>
#include <QMainWindow>
#include <QStyle>
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

    QmRibbon::Features features = QmRibbon::Features();
};

QmRibbon::QmRibbon(QWidget* parent, Features features)
    : QWidget(parent)
    , d_(new QmRibbonPrivate)
{
    d_->features = features;
    if (!d_->features.testAnyFlag(NoDefaultStyle)) {
        initializeAssets();
        QFile style(":/qmribbon/styles/default");
        if (style.open(QFile::ReadOnly)) {
            qApp->setStyleSheet(style.readAll());
            style.close();
        }
    }
    setAttribute(Qt::WA_StyledBackground, true);
    initUi();
    connectSignals();
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

QmRibbonTabBar* QmRibbon::tabBar() const
{
    return d_->tabbar;
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

    setFeatures(d_->features);
}

bool QmRibbon::event(QEvent* event)
{
    return QWidget::event(event);
}

void QmRibbon::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (d_->page_container->property("Floating").toBool()) {
        auto geo = d_->page_container->geometry();
        geo.setWidth(d_->tabbar->width());
        d_->page_container->setGeometry(geo);
    }
}

void QmRibbon::setWindow(QMainWindow* window)
{
    qApp->setProperty("QmRibbon-Window", QVariant::fromValue(window));
    window->setMenuWidget(this);
    window->installEventFilter(d_->titlebar);
    d_->titlebar->setWindowTitle(window->windowTitle());
}

QmRibbon* QmRibbon::install(QMainWindow* window, Features features)
{
    auto* ribbon = new QmRibbon(window, features);
    ribbon->setWindow(window);
    return ribbon;
}

QmRibbonPage* QmRibbon::addPage(const QString& title, const QIcon& icon)
{
    auto* page = new QmRibbonPage(d_->page_container);
    d_->page_container->addWidget(page);
    d_->tabbar->addTab(title, icon);
    return page;
}

void QmRibbon::setFeature(Feature feature, bool on)
{
    if (on) {
        d_->features |= feature;
    } else {
        d_->features &= ~feature;
    }
    setFeatures(d_->features, on);
}

void QmRibbon::setFeatures(Features features, bool on)
{
    if (on) {
        d_->features |= features;
    } else {
        d_->features &= ~features;
    }
    d_->titlebar->setQuickAccessToolBarVisible(!d_->features.testAnyFlag(NoQuickAccessToolBar));
    d_->titlebar->setUserInfoButtonVisible(!d_->features.testAnyFlag(NoUserInfoButton));
    d_->titlebar->setRibbonButtonVisible(!d_->features.testAnyFlag(NoRibbonButton));
    d_->titlebar->setLogoVisible(!d_->features.testAnyFlag(NoApplicationLogo));
    d_->tabbar->setApplicationButtonVisible(!d_->features.testAnyFlag(NoApplicationButton));
}

void QmRibbon::connectSignals()
{
    connect(d_->tabbar, &QmRibbonTabBar::tabActivated, this, [this](int index) {
        if (index == d_->page_container->currentIndex()) {
            return;
        }
        d_->page_container->setCurrentIndex(index);
        if (auto* widget = qobject_cast<QmRibbonPage*>(d_->page_container->currentWidget()); widget) {
            widget->showAnimation();
        }
    });
    connect(d_->tabbar, &QmRibbonTabBar::requestToggleFloating, this, &QmRibbon::onContainerFloatingRequested);
}

void QmRibbon::onContainerFloatingRequested()
{
    if (d_->page_container->parentWidget() == this) {
        QPoint top_left(0, 0);
        auto page_geo = d_->page_container->geometry();
        if (auto* window = qApp->property("QmRibbon-Window").value<QMainWindow*>()) {
            top_left.setX(window->contentsMargins().left());
            top_left.setY(window->contentsMargins().top());
            page_geo.setWidth(window->width() - (window->contentsMargins().left() + window->contentsMargins().right()));
        }
        static_cast<QBoxLayout*>(layout())->removeWidget(d_->page_container);
        page_geo.moveTopLeft(page_geo.topLeft() + top_left);
        d_->page_container->setParent(parentWidget());
        d_->page_container->setGeometry(page_geo);
        d_->page_container->show();
        d_->page_container->setFloating(true);
        d_->page_container->updateGeometry();
        updateGeometry();
    } else {
        static_cast<QBoxLayout*>(layout())->addWidget(d_->page_container);
        d_->page_container->setFloating(false);
    }
}

void QmRibbon::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
        // ui_->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}