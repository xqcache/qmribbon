#include "qmribbonwelcomedialog.h"

#include "qmribbon.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QMainWindow>
#include <QResizeEvent>
#include <QScreen>
#include <QSpacerItem>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWindow>

namespace {
Qt::WindowStates windowStatesForSync(Qt::WindowStates states)
{
    return states & (Qt::WindowMinimized | Qt::WindowMaximized | Qt::WindowFullScreen);
}

void syncWindowFromMainWindow(QWidget* dialog, QMainWindow* main_win)
{
    if (!dialog || !main_win) {
        return;
    }

    const QSize minimum_size = { qMax(dialog->minimumSize().width(), main_win->minimumSize().width()),
        qMax(dialog->minimumSize().height(), main_win->minimumSize().height()) };

    main_win->setMinimumSize(minimum_size);
    dialog->setMinimumSize(minimum_size);

    if (dialog->isWindow()) {
        dialog->setGeometry(main_win->frameGeometry());
        dialog->setWindowState(windowStatesForSync(main_win->windowState()));
    }
}

void updateMaximizeButtonText(QToolButton* button, const QWidget* window)
{
    if (!button) {
        return;
    }
    button->setText(window && window->isMaximized() ? "\u2102" : "\u2101");
}

void updateBrandIcon(QLabel* label, const QIcon& icon)
{
    if (!label || icon.isNull()) {
        return;
    }
    label->setPixmap(icon.pixmap(QSize(18, 18)));
}
}

struct QmRibbonWelcomeDialogPrivate {
    QmRibbon* ribbon { nullptr };
    QLabel* brand_icon { nullptr };
    QToolButton* btn_win_minimized { nullptr };
    QToolButton* btn_win_maximized { nullptr };
    QToolButton* btn_win_close { nullptr };
};

QmRibbonWelcomeDialog::QmRibbonWelcomeDialog(QmRibbon* ribbon)
    : QmFramelessDialog()
    , d_(new QmRibbonWelcomeDialogPrivate)
{
    d_->ribbon = ribbon;

    initUi();
    connectSignals();
}

QmRibbonWelcomeDialog::~QmRibbonWelcomeDialog() noexcept
{
    delete d_;
}

int QmRibbonWelcomeDialog::execOverMainWindow(bool first_show)
{
    d_->ribbon->showBackstage(first_show);
    return QDialog::Accepted;
}

void QmRibbonWelcomeDialog::showEvent(QShowEvent* event)
{
    syncWindowFromMainWindow(this, d_->ribbon->mainWindow());
    updateMaximizeButtonText(d_->btn_win_maximized, this);
    if (auto* main_win = d_->ribbon->mainWindow(); main_win) {
        setWindowIcon(main_win->windowIcon());
        updateBrandIcon(d_->brand_icon, main_win->windowIcon());
    }
    QmFramelessDialog::showEvent(event);
}

bool QmRibbonWelcomeDialog::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == d_->ribbon->mainWindow()) {
        switch (event->type()) {
        case QEvent::WindowStateChange:
            updateMaximizeButtonText(d_->btn_win_maximized, d_->ribbon->mainWindow());
            [[fallthrough]];
        case QEvent::Move:
        case QEvent::Resize:
            syncWindowFromMainWindow(this, d_->ribbon->mainWindow());
            break;
        case QEvent::WindowIconChange:
            if (auto* main_win = d_->ribbon->mainWindow(); main_win) {
                setWindowIcon(main_win->windowIcon());
                updateBrandIcon(d_->brand_icon, main_win->windowIcon());
            }
            break;
        default:
            break;
        }
    }
    return QmFramelessDialog::eventFilter(watched, event);
}

void QmRibbonWelcomeDialog::initUi()
{
    QWidget* titlebar = new QWidget(this);
    titlebar->setProperty("WindowTitleBar", true);
    QHBoxLayout* lyt_titlebar = new QHBoxLayout(titlebar);
    lyt_titlebar->setContentsMargins(0, 0, 0, 0);

    d_->brand_icon = new QLabel(titlebar);
    d_->brand_icon->setObjectName("brand_icon");
    d_->brand_icon->setFixedSize(18, 18);
    d_->brand_icon->setScaledContents(true);

    lyt_titlebar->addSpacing(8);
    lyt_titlebar->addWidget(d_->brand_icon, 0, Qt::AlignVCenter);
    lyt_titlebar->addSpacing(8);
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

    d_->btn_win_minimized->setToolTip(tr("Minimize the window"));
    d_->btn_win_maximized->setToolTip(tr("Maximize the window"));
    d_->btn_win_close->setToolTip(tr("Close window"));

    d_->btn_win_minimized->setProperty("Style", "WindowButton");
    d_->btn_win_maximized->setProperty("Style", "WindowButton");
    d_->btn_win_close->setProperty("Style", "WindowButton");

    updateMaximizeButtonText(d_->btn_win_maximized, this);

    lyt_button_area->addWidget(d_->btn_win_minimized);
    lyt_button_area->addWidget(d_->btn_win_maximized);
    lyt_button_area->addWidget(d_->btn_win_close);
    lyt_titlebar->addLayout(lyt_button_area);

    setTitleBar(titlebar);
}

void QmRibbonWelcomeDialog::connectSignals()
{
    connect(d_->ribbon, &QmRibbon::mainWindowChanged, this, [this](QMainWindow* main_win) {
        if (!main_win) {
            return;
        }
        main_win->installEventFilter(this);
        setWindowIcon(main_win->windowIcon());
        updateBrandIcon(d_->brand_icon, main_win->windowIcon());
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
    connect(d_->btn_win_maximized, &QToolButton::clicked, this, [this] {
        auto* main_win = d_->ribbon->mainWindow();
        const bool maximized = (main_win && main_win->isMaximized()) || isMaximized();
        if (maximized) {
            if (main_win) {
                main_win->showNormal();
            }
            showNormal();
        } else {
            if (main_win) {
                main_win->showMaximized();
            }
            showMaximized();
        }
        updateMaximizeButtonText(d_->btn_win_maximized, this);
    });
}

void QmRibbonWelcomeDialog::setPage(QmRibbonWelcomePage* page)
{
    if (!page) {
        return;
    }
    setWidget(page);
}

QmRibbonWelcomePage* QmRibbonWelcomeDialog::page() const
{
    return qobject_cast<QmRibbonWelcomePage*>(widget());
}
