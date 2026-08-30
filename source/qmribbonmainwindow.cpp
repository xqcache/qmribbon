#include "qmribbonmainwindow.h"

#include "qmribbon.h"
#include <QDebug>
#include <QPointer>
#include <QWindow>

struct QmRibbonMainWindowPrivate {
    QPointer<QWidget> central_widget;
    QPointer<QmRibbon> ribbon;
};

QmRibbonMainWindow::QmRibbonMainWindow(QWidget* parent, Qt::WindowFlags flags)
    : QWidget(parent, flags)
    , d_(new QmRibbonMainWindowPrivate)
{
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_NativeWindow);
    initUi();
    connectSignals();
}

QmRibbonMainWindow::~QmRibbonMainWindow() noexcept
{
    delete d_;
}

void QmRibbonMainWindow::initUi()
{
}

void QmRibbonMainWindow::connectSignals()
{
}

void QmRibbonMainWindow::setCentralWidget(QWidget* widget)
{
}