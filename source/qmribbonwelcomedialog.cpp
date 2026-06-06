#include "qmribbonwelcomedialog.h"

#include "qmribbon.h"

#include <QEvent>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QPointer>
#include <QResizeEvent>
#include <QScreen>
#include <QSpacerItem>
#include <QTimer>
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

void refreshWidgetTree(QWidget* widget)
{
    if (!widget) {
        return;
    }

    if (auto* widget_layout = widget->layout(); widget_layout) {
        widget_layout->invalidate();
        widget_layout->activate();
    }
    widget->updateGeometry();
    widget->update();
    widget->repaint();

    const auto child_widgets = widget->findChildren<QWidget*>();
    for (QWidget* child_widget : child_widgets) {
        if (auto* child_layout = child_widget->layout(); child_layout) {
            child_layout->invalidate();
            child_layout->activate();
        }
        child_widget->updateGeometry();
        child_widget->update();
        child_widget->repaint();
    }
}
}

struct QmRibbonWelcomeDialogPrivate {
    QmRibbon* ribbon { nullptr };
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
    auto* main_win = d_->ribbon->mainWindow();
    if (!main_win) {
        return exec();
    }
    main_win->setWindowOpacity(0);
    syncWindowFromMainWindow(this, main_win);

    if (auto* welcome_page = qobject_cast<QmRibbonWelcomePage*>(widget()); welcome_page) {
        welcome_page->setAsFirstShow(first_show);
    }

    const int dialog_code = exec();

    raise();
    activateWindow();

    main_win->setGeometry(geometry());
    main_win->setWindowOpacity(1.0);
    main_win->show();
    main_win->raise();
    main_win->activateWindow();
    refreshWidgetTree(main_win);

    QPointer<QMainWindow> guarded_main_win(main_win);
    QTimer::singleShot(0, main_win, [guarded_main_win] {
        if (!guarded_main_win || !guarded_main_win->isVisible()) {
            return;
        }
        refreshWidgetTree(guarded_main_win);
    });

    return dialog_code;
}

void QmRibbonWelcomeDialog::showEvent(QShowEvent* event)
{
    syncWindowFromMainWindow(this, d_->ribbon->mainWindow());
    QmFramelessDialog::showEvent(event);
}

bool QmRibbonWelcomeDialog::eventFilter(QObject* watched, QEvent* event)
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

void QmRibbonWelcomeDialog::initUi()
{
    QWidget* titlebar = new QWidget(this);
    titlebar->setProperty("WindowTitleBar", true);
    QHBoxLayout* lyt_titlebar = new QHBoxLayout(titlebar);
    lyt_titlebar->setContentsMargins(0, 0, 0, 0);

    lyt_titlebar->addSpacerItem(new QSpacerItem(10, 0));
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

void QmRibbonWelcomeDialog::connectSignals()
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
