#include "qmribbontitlebar.h"

#include <QApplication>
#include <QFrame>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QMouseEvent>
#include <QToolBar>
#include <QToolButton>

struct QmRibbonTitleBarPrivate {
    QToolBar* quick_access_toolbar { nullptr };
    QToolButton* btn_user_info { nullptr };
    QToolButton* btn_options { nullptr };

    QToolButton* btn_win_minimized { nullptr };
    QToolButton* btn_win_maximized { nullptr };
    QToolButton* btn_win_close { nullptr };
};

QmRibbonTitleBar::QmRibbonTitleBar(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonTitleBarPrivate)
{
    setProperty("WindowTitleBar", true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    initUi();
    initStyleSheetKey();
    connectSignals();
}

QmRibbonTitleBar::~QmRibbonTitleBar() noexcept
{
    delete d_;
}

QToolBar* QmRibbonTitleBar::quickAccessToolBar() const
{
    return d_->quick_access_toolbar;
}

bool QmRibbonTitleBar::eventFilter(QObject* watched, QEvent* event)
{

    return QWidget::eventFilter(watched, event);
}

void QmRibbonTitleBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton && event->modifiers() == Qt::NoModifier) {
        toggleWindowMaximized();
        event->accept();
    }
}

void QmRibbonTitleBar::initUi()
{
    d_->quick_access_toolbar = new QToolBar(this);
    d_->btn_user_info = new QToolButton(this);
    d_->btn_options = new QToolButton(this);

    d_->btn_user_info->setToolTip(tr("User Information"));
    d_->btn_options->setToolTip(tr("Ribbon Options"));

    d_->btn_user_info->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    d_->btn_options->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    auto* blank_widget = new QWidget(this);
    blank_widget->setObjectName("hit_area");
    blank_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    blank_widget->setAttribute(Qt::WA_TransparentForMouseEvents);
    blank_widget->setAttribute(Qt::WA_StyledBackground, true);
    blank_widget->setMinimumWidth(300);

    auto* lyt_main = new QHBoxLayout(this);
    lyt_main->setContentsMargins(0, 0, 0, 0);
    lyt_main->setSpacing(0);
    lyt_main->addWidget(d_->quick_access_toolbar, 0);
    lyt_main->addWidget(blank_widget, 1);
    lyt_main->addWidget(d_->btn_user_info, 0);
    lyt_main->addWidget(d_->btn_options, 0);

    initWindowButtons();
}

void QmRibbonTitleBar::initWindowButtons()
{
    d_->btn_win_minimized = new QToolButton(this);
    d_->btn_win_maximized = new QToolButton(this);
    d_->btn_win_close = new QToolButton(this);

    d_->btn_win_minimized->setObjectName("btn_win_minimized");
    d_->btn_win_maximized->setObjectName("btn_win_maximized");
    d_->btn_win_close->setObjectName("btn_win_close");

    d_->btn_win_minimized->setToolTip(tr("Minimize the window"));
    d_->btn_win_maximized->setToolTip(tr("Maximize the window"));
    d_->btn_win_close->setToolTip(tr("Close window"));

    d_->btn_win_minimized->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    d_->btn_win_maximized->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
    d_->btn_win_close->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    if (auto* window = qApp->property("QmRibbon-Window").value<QMainWindow*>()) {
        d_->btn_win_maximized->setText(window->isMaximized() ? "\u2102" : "\u2101");
    } else {
        d_->btn_win_maximized->setText("\u2101");
    }

    auto* lyt_win_buttons = new QHBoxLayout();
    lyt_win_buttons->setContentsMargins(0, 0, 0, 0);
    lyt_win_buttons->setSpacing(0);
    lyt_win_buttons->addWidget(d_->btn_win_minimized);
    lyt_win_buttons->addWidget(d_->btn_win_maximized);
    lyt_win_buttons->addWidget(d_->btn_win_close);
    static_cast<QBoxLayout*>(layout())->addLayout(lyt_win_buttons, 0);
}

void QmRibbonTitleBar::initStyleSheetKey()
{
    d_->btn_user_info->setProperty("Style", "UserInfoButton");
    d_->btn_win_close->setProperty("Style", "WindowButton");
    d_->btn_win_minimized->setProperty("Style", "WindowButton");
    d_->btn_win_maximized->setProperty("Style", "WindowButton");
}

void QmRibbonTitleBar::connectSignals()
{
    connect(d_->btn_win_close, &QToolButton::clicked, this, [] {
        if (auto* window = qApp->property("QmRibbon-Window").value<QMainWindow*>()) {
            window->close();
        }
    });
    connect(d_->btn_win_maximized, &QToolButton::clicked, this, [this] {
        toggleWindowMaximized();
    });
    connect(d_->btn_win_minimized, &QToolButton::clicked, this, [] {
        if (auto* window = qApp->property("QmRibbon-Window").value<QMainWindow*>()) {
            window->showMinimized();
        }
    });
}

void QmRibbonTitleBar::toggleWindowMaximized()
{
    if (auto* window = qApp->property("QmRibbon-Window").value<QMainWindow*>()) {
        if (window->isMaximized()) {
            window->showNormal();
            d_->btn_win_maximized->setText("\u2101");
            d_->btn_win_maximized->setToolTip(tr("Maximize the window"));
        } else {
            window->showMaximized();
            d_->btn_win_maximized->setToolTip(tr("Normalize the window"));
            d_->btn_win_maximized->setText("\u2102");
        }
    }
}