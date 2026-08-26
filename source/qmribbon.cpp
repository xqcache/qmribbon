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
#include <QLayout>
#include <QMainWindow>
#include <QMargins>
#include <QPoint>
#include <QRect>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
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

QRect backstageGeometry(QMainWindow* window, QmRibbonTitleBar* titlebar)
{
    if (!window || !titlebar) {
        return { };
    }

    const QMargins margins = window->contentsMargins();
    const int top = titlebar->mapTo(window, QPoint(0, titlebar->height())).y();
    return QRect(margins.left(), top, window->width() - margins.left() - margins.right(), window->height() - top - margins.bottom());
}
}

struct QmRibbonPrivate {
    QMainWindow* window { nullptr };
    QmRibbonTitleBar* titlebar { nullptr };
    QmRibbonTabBar* tabbar { nullptr };
    QmRibbonWelcomePage* welcome_page { nullptr };
    QmRibbonPageContainer* page_container { nullptr };

    QmRibbon::Features features = QmRibbon::Features();

    std::map<QmRibbonPage*, QToolButton*> page_buttons;
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
    if (d_->window) {
        d_->window->removeEventFilter(this);
        if (d_->titlebar) {
            d_->window->removeEventFilter(d_->titlebar);
        }
    }
    qApp->setProperty(QmRibbon::kRibbonWindowPropName, QVariant());
    qApp->setProperty(QmRibbon::kRibbonPropName, QVariant());

    if (d_->page_container && !d_->page_container->parent()) {
        delete d_->page_container;
    }
    delete d_;
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

void QmRibbon::setMainWindow(QMainWindow* window)
{
    if (d_->window == window) {
        return;
    }
    d_->window = window;
    qApp->setProperty(QmRibbon::kRibbonWindowPropName, QVariant::fromValue(window));
    qApp->setProperty(QmRibbon::kRibbonPropName, QVariant::fromValue(this));
    if (window->metaObject()->indexOfSlot("setTitleBar(QWidget*)") >= 0) {
        QMetaObject::invokeMethod(window, "setTitleBar", Q_ARG(QWidget*, this));
    } else {
        window->setMenuWidget(this);
    }
    window->installEventFilter(this);
    window->installEventFilter(d_->titlebar);
    d_->titlebar->setWindowTitle(window->windowTitle());
    d_->titlebar->setLogoIcon(window->windowIcon());
    emit mainWindowChanged(d_->window);
}

QMainWindow* QmRibbon::mainWindow() const
{
    return d_->window;
}

QmRibbon* QmRibbon::install(QMainWindow* window, Features features)
{
    auto* ribbon = new QmRibbon(window, features);
    ribbon->setMainWindow(window);
    return ribbon;
}

QmRibbonPage* QmRibbon::addPage(const QString& title, const QIcon& icon)
{
    auto* page = new QmRibbonPage(d_->page_container);
    d_->page_container->addWidget(page);
    auto* tab_button = d_->tabbar->addTab(title, icon);
    d_->page_buttons.emplace(page, tab_button);
    return page;
}

QToolButton* QmRibbon::pageButton(QmRibbonPage* page) const
{
    if (!page) {
        return nullptr;
    }
    auto it = d_->page_buttons.find(page);
    if (it != d_->page_buttons.end()) {
        return it->second;
    }
    return nullptr;
}

QmRibbonPage* QmRibbon::findPage(const QString& object_name) const
{
    return d_->page_container->findChild<QmRibbonPage*>(object_name);
}

QToolButton* QmRibbon::findPageButton(const QString& object_name) const
{
    return pageButton(findPage(object_name));
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

    d_->titlebar->setVisible(!d_->features.testAnyFlag(NoDefaultTitleBar));
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

    connect(d_->tabbar->applicationButton(), &QToolButton::clicked, this, &QmRibbon::enterBackstageView);
}

void QmRibbon::onContainerFloatingRequested()
{
    if (d_->page_container->parentWidget() == this) {
        QPoint top_left(0, 0);
        auto page_geo = d_->page_container->geometry();
        if (auto* window = qApp->property(QmRibbon::kRibbonWindowPropName).value<QMainWindow*>()) {
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
