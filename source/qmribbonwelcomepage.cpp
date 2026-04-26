#include "qmribbonwelcomepage.h"

#include "qmribbon.h"
#include "qmribbonwelcomedialog.h"

#include <QApplication>

QmRibbonWelcomePage::QmRibbonWelcomePage(QWidget* parent)
    : QWidget(parent)
{
}

QmRibbonWelcomePage::~QmRibbonWelcomePage() noexcept
{
}

void QmRibbonWelcomePage::setAsFirstShow(bool first_show)
{
}

QmRibbonWelcomeDialog* QmRibbonWelcomePage::welcomeDialog() const
{
    if (auto* ribbon = qApp->property(QmRibbon::kRibbonPropName).value<QmRibbon*>()) {
        return ribbon->welcomeDialog();
    }
    return nullptr;
}