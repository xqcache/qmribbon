#include "qmribbontitlebar.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QMainWindow>
#include <QMouseEvent>
#include <QToolBar>
#include <QToolButton>

struct QmRibbonTitleBarPrivate {
    QToolBar* quick_access_toolbar { nullptr };
    QToolButton* btn_user_info { nullptr };

    QToolButton* btn_win_minimum { nullptr };
    QToolButton* btn_win_maximum { nullptr };
    QToolButton* btn_win_close { nullptr };
};

QmRibbonTitleBar::QmRibbonTitleBar(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonTitleBarPrivate)
{
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

    auto* blank_widget = new QWidget(this);
    blank_widget->setAttribute(Qt::WA_TransparentForMouseEvents);
    blank_widget->setMinimumWidth(100);
    blank_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto* lyt_main = new QHBoxLayout(this);
    lyt_main->setContentsMargins(0, 0, 0, 0);
    lyt_main->setSpacing(0);
    lyt_main->addWidget(d_->quick_access_toolbar, 0);
    lyt_main->addWidget(blank_widget, 1);
    lyt_main->addWidget(d_->btn_user_info, 0);

    initWindowButtons();
}

void QmRibbonTitleBar::initWindowButtons()
{
    d_->btn_win_minimum = new QToolButton(this);
    d_->btn_win_maximum = new QToolButton(this);
    d_->btn_win_close = new QToolButton(this);

    d_->btn_win_minimum->setToolTip(tr("Minimize the window"));
    d_->btn_win_maximum->setToolTip(tr("Maximize the window"));
    d_->btn_win_close->setToolTip(tr("Close window"));

    auto* lyt_win_buttons = new QHBoxLayout();
    lyt_win_buttons->setContentsMargins(0, 0, 0, 0);
    lyt_win_buttons->setSpacing(0);
    lyt_win_buttons->addWidget(d_->btn_win_minimum);
    lyt_win_buttons->addWidget(d_->btn_win_maximum);
    lyt_win_buttons->addWidget(d_->btn_win_close);
    static_cast<QBoxLayout*>(layout())->addLayout(lyt_win_buttons, 0);
}

void QmRibbonTitleBar::initStyleSheetKey()
{
    d_->btn_user_info->setProperty("Style", "UserInfoButton");
    d_->btn_win_close->setProperty("Style", "WindowButton");
    d_->btn_win_minimum->setProperty("Style", "WindowButton");
    d_->btn_win_maximum->setProperty("Style", "WindowButton");
}

void QmRibbonTitleBar::connectSignals()
{
    connect(d_->btn_win_close, &QToolButton::clicked, this, [] {
        if (auto* window = qApp->property("QmRibbon-Window").value<QMainWindow*>()) {
            window->close();
        }
    });
    connect(d_->btn_win_maximum, &QToolButton::clicked, this, [this] {
        toggleWindowMaximized();
    });
    connect(d_->btn_win_minimum, &QToolButton::clicked, this, [] {
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
            d_->btn_win_maximum->setToolTip(tr("Maximize the window"));
        } else {
            window->showMaximized();
            d_->btn_win_maximum->setToolTip(tr("Normalize the window"));
        }
    }
}