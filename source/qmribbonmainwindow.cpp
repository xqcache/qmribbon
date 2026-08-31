#include "qmribbonmainwindow.h"

#include "qmribbon.h"
#include <QDebug>
#include <QPainter>
#include <QPointer>
#include <QVBoxLayout>
#include <QWindow>

struct QmRibbonMainWindowPrivate {
    QmRibbon* ribbon { nullptr };
    QVBoxLayout* lyt_main { nullptr };
    QPointer<QWidget> central_widget { nullptr };

    QMargins shadow_margins { 10, 10, 10, 10 };
};

QmRibbonMainWindow::QmRibbonMainWindow(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , d_(new QmRibbonMainWindowPrivate)
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NativeWindow);
    setAttribute(Qt::WA_TranslucentBackground);
    initUi();
    connectSignals();
}

QmRibbonMainWindow::~QmRibbonMainWindow() noexcept
{
    delete d_;
}

void QmRibbonMainWindow::initUi()
{
    d_->ribbon = new QmRibbon(this);
    // 占位
    d_->central_widget = new QWidget(this);
    d_->central_widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    d_->lyt_main = new QVBoxLayout(this);
    d_->lyt_main->setSpacing(0);

    if (isMaximized()) {
        d_->lyt_main->setContentsMargins(0, 0, 0, 0);
    } else {
        d_->lyt_main->setContentsMargins(d_->shadow_margins);
    }

    d_->lyt_main->addWidget(d_->ribbon, 0, Qt::AlignHCenter | Qt::AlignTop);
    d_->lyt_main->addWidget(d_->central_widget, 1, Qt::AlignHCenter | Qt::AlignBottom);
}

void QmRibbonMainWindow::connectSignals()
{
}

void QmRibbonMainWindow::setCentralWidget(QWidget* widget)
{
}

QWidget* QmRibbonMainWindow::centralWidget() const
{
    return d_->central_widget.data();
}

QmRibbon* QmRibbonMainWindow::ribbon() const
{
    return d_->ribbon;
}

void QmRibbonMainWindow::paintEvent(QPaintEvent* event)
{
    QPainter painter(this);
    initPainter(&painter);

    QRect base_rect = rect();
    if (!isMaximized()) {
        base_rect = base_rect.marginsRemoved(d_->shadow_margins);
    }

    painter.fillRect(rect().marginsRemoved(d_->shadow_margins), palette().base());
}
