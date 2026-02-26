#include "qmribbonapppage.h"

#include "qmribbon.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QResizeEvent>
#include <QSpacerItem>
#include <QToolButton>
#include <QVBoxLayout>

namespace {
Qt::WindowStates windowStatesForSync(Qt::WindowStates states)
{
    return states & (Qt::WindowMinimized | Qt::WindowMaximized | Qt::WindowFullScreen);
}

void syncWindowFromMainWindow(QWidget* window, QMainWindow* main_win)
{
    if (!window || !main_win) {
        return;
    }
    window->setGeometry(main_win->geometry());
    window->setWindowState(windowStatesForSync(main_win->windowState()));
}
}

struct QmRibbonAppPagePrivate {
    QmRibbon* ribbon { nullptr };
    QToolButton* btn_back { nullptr };
    QToolButton* btn_win_minimized { nullptr };
    QToolButton* btn_win_maximized { nullptr };
    QToolButton* btn_win_close { nullptr };
};

QmRibbonAppPage::QmRibbonAppPage(QmRibbon* ribbon)
    : QmFramelessDialog(ribbon)
    , d_(new QmRibbonAppPagePrivate)
{
    d_->ribbon = ribbon;

    initUi();
    connectSignals();
}

QmRibbonAppPage::~QmRibbonAppPage() noexcept
{
    delete d_;
}

void QmRibbonAppPage::showEvent(QShowEvent* event)
{
    d_->btn_back->setVisible(d_->ribbon->mainWindow()->isVisible());

    syncWindowFromMainWindow(this, d_->ribbon->mainWindow());
    QmFramelessDialog::showEvent(event);
}

bool QmRibbonAppPage::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == d_->ribbon->mainWindow()) {
        switch (event->type()) {
        case QEvent::WindowStateChange:
        case QEvent::Move:
        case QEvent::Resize:
            syncWindowFromMainWindow(this, d_->ribbon->mainWindow());
            break;
        default:
            break;
        }
    }

    return QmFramelessDialog::eventFilter(watched, event);
}

QToolButton* QmRibbonAppPage::backButton() const
{
    return d_->btn_back;
}

void QmRibbonAppPage::initUi()
{
    QWidget* titlebar = new QWidget(this);
    QHBoxLayout* lyt_titlebar = new QHBoxLayout(titlebar);
    lyt_titlebar->setContentsMargins(0, 0, 0, 0);

    d_->btn_back = new QToolButton(this);
    d_->btn_back->setObjectName("AppPageBackButton");
    d_->btn_back->setText(tr("Back"));
    d_->btn_back->setToolTip(tr("Return to the main interface."));
    lyt_titlebar->addSpacerItem(new QSpacerItem(10, 0));
    lyt_titlebar->addWidget(d_->btn_back);
    lyt_titlebar->addSpacerItem(new QSpacerItem(10, 0, QSizePolicy::Expanding));

    QHBoxLayout* lyt_button_area = new QHBoxLayout;
    lyt_button_area->setSpacing(0);
    lyt_button_area->setContentsMargins(0, 0, 0, 0);
    d_->btn_win_minimized = new QToolButton(this);
    d_->btn_win_maximized = new QToolButton(this);
    d_->btn_win_close = new QToolButton(this);

    d_->btn_win_minimized->setObjectName("btn_win_minimized");
    d_->btn_win_maximized->setObjectName("btn_win_maximized");
    d_->btn_win_close->setObjectName("btn_win_close");

    d_->btn_win_minimized->setProperty("Style", "WindowButton");
    d_->btn_win_maximized->setProperty("Style", "WindowButton");
    d_->btn_win_close->setProperty("Style", "WindowButton");

    lyt_button_area->addWidget(d_->btn_win_minimized);
    lyt_button_area->addWidget(d_->btn_win_maximized);
    lyt_button_area->addWidget(d_->btn_win_close);
    lyt_titlebar->addLayout(lyt_button_area);

    setTitleBar(titlebar);
}

void QmRibbonAppPage::connectSignals()
{
    connect(d_->ribbon, &QmRibbon::mainWindowChanged, this, [this](QMainWindow* main_win) {
        if (!main_win) {
            return;
        }
        main_win->installEventFilter(this);
    });
    connect(d_->btn_win_close, &QToolButton::clicked, this, [this] {
        reject();
        if (auto* main_win = d_->ribbon->mainWindow(); main_win) {
            main_win->close();
        }
        qApp->quit();
    });
    connect(d_->btn_win_minimized, &QToolButton::clicked, this, [this] {
        if (auto* main_win = d_->ribbon->mainWindow(); main_win) {
            main_win->showMinimized();
        }
        showMinimized();
    });
    connect(d_->btn_back, &QToolButton::clicked, this, [this] {
        accept();
    });
    connect(this, &QmFramelessDialog::accepted, this, [this] {
        if (!d_->ribbon->mainWindow()->isVisible()) {
            d_->ribbon->mainWindow()->show();
        }
    });
}
