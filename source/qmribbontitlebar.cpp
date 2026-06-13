#include "qmribbontitlebar.h"

#include "qmribbon.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QSize>
#include <QToolBar>
#include <QToolButton>

struct QmRibbonTitleBarPrivate {
    QToolBar* quick_access_toolbar { nullptr };
    QToolButton* btn_user_info { nullptr };
    QToolButton* btn_ribbon_options { nullptr };

    QToolButton* btn_win_minimized { nullptr };
    QToolButton* btn_win_maximized { nullptr };
    QToolButton* btn_win_close { nullptr };

    QToolButton* btn_logo { nullptr };
};

QmRibbonTitleBar::QmRibbonTitleBar(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonTitleBarPrivate)
{
    setProperty("WindowTitleBar", true);
    setAttribute(Qt::WA_StyledBackground, true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    initUi();
    initStyleSheetKey();
    initWidgetData();
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

bool QmRibbonTitleBar::event(QEvent* event)
{
    switch (event->type()) {
    case QEvent::WindowTitleChange:
        update();
        break;
    default:
        break;
    }
    return QWidget::event(event);
}

bool QmRibbonTitleBar::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type()) {
    case QEvent::WindowStateChange:
        if (auto* window = qobject_cast<QMainWindow*>(watched); window) {
            if (window->isMaximized()) {
                d_->btn_win_maximized->setToolTip(tr("Normalize the window"));
                d_->btn_win_maximized->setText("\u2102");
            } else {
                d_->btn_win_maximized->setText("\u2101");
                d_->btn_win_maximized->setToolTip(tr("Maximize the window"));
            }
        }
        break;
    case QEvent::WindowTitleChange:
        if (auto* window = qobject_cast<QMainWindow*>(watched); window) {
            setWindowTitle(window->windowTitle());
        }
        break;
    case QEvent::WindowIconChange:
        if (auto* window = qobject_cast<QMainWindow*>(watched); window) {
            setLogoIcon(window->windowIcon());
        }
        break;
    default:
        break;
    }
    return QWidget::eventFilter(watched, event);
}

void QmRibbonTitleBar::paintEvent(QPaintEvent* event)
{
    if (windowTitle().isEmpty()) {
        return;
    }
    QPainter painter(this);
    initPainter(&painter);
    painter.setRenderHint(QPainter::TextAntialiasing);
    painter.setPen(Qt::black);
    painter.drawText(rect(), Qt::AlignCenter, windowTitle());
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

    d_->btn_logo = new QToolButton(this);
    d_->btn_logo->setProperty("Style", "RibbonApplicationLogo");
    d_->btn_logo->setToolButtonStyle(Qt::ToolButtonIconOnly);
    d_->btn_logo->setIconSize(QSize(18, 18));
    d_->btn_logo->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    d_->btn_user_info = new QToolButton(this);
    d_->btn_user_info->setToolTip(tr("User Information"));
    d_->btn_user_info->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    d_->btn_ribbon_options = new QToolButton(this);
    d_->btn_ribbon_options->setToolTip(tr("Ribbon Options"));
    d_->btn_ribbon_options->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

    auto* blank_widget = new QWidget(this);
    blank_widget->setObjectName("hit_area");
    blank_widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    blank_widget->setAttribute(Qt::WA_TransparentForMouseEvents);
    blank_widget->setMinimumWidth(300);

    auto* lyt_main = new QHBoxLayout(this);
    lyt_main->setContentsMargins(0, 0, 0, 0);
    lyt_main->setSpacing(0);
    lyt_main->addWidget(d_->btn_logo);
    lyt_main->addWidget(d_->quick_access_toolbar, 0);
    lyt_main->addWidget(blank_widget, 1);
    lyt_main->addWidget(d_->btn_user_info, 0);
    lyt_main->addWidget(d_->btn_ribbon_options, 0);

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

    if (auto* window = qApp->property(QmRibbon::kRibbonWindowPropName).value<QMainWindow*>()) {
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

void QmRibbonTitleBar::initWidgetData()
{
    if (auto* window = qApp->property(QmRibbon::kRibbonWindowPropName).value<QMainWindow*>()) {
        setWindowTitle(window->windowTitle());
    }
}

void QmRibbonTitleBar::connectSignals()
{
    connect(d_->btn_win_close, &QToolButton::clicked, this, [] {
        if (auto* window = qApp->property(QmRibbon::kRibbonWindowPropName).value<QMainWindow*>()) {
            window->close();
        }
    });
    connect(d_->btn_win_maximized, &QToolButton::clicked, this, [this] {
        toggleWindowMaximized();
    });
    connect(d_->btn_win_minimized, &QToolButton::clicked, this, [] {
        if (auto* window = qApp->property(QmRibbon::kRibbonWindowPropName).value<QMainWindow*>()) {
            window->showMinimized();
        }
    });
}

void QmRibbonTitleBar::toggleWindowMaximized()
{
    if (auto* window = qApp->property(QmRibbon::kRibbonWindowPropName).value<QMainWindow*>()) {
        if (window->isMaximized()) {
            window->showNormal();
        } else {
            window->showMaximized();
        }
    }
}

void QmRibbonTitleBar::setQuickAccessToolBarVisible(bool visible)
{
    d_->quick_access_toolbar->setVisible(visible);
}

void QmRibbonTitleBar::setUserInfoButtonVisible(bool visible)
{
    d_->btn_user_info->setVisible(visible);
}

void QmRibbonTitleBar::setRibbonButtonVisible(bool visible)
{
    d_->btn_ribbon_options->setVisible(visible);
}

void QmRibbonTitleBar::setLogoIcon(const QIcon& icon)
{
    d_->btn_logo->setIcon(icon);
}

void QmRibbonTitleBar::setLogoVisible(bool visible)
{
    d_->btn_logo->setVisible(visible);
}
