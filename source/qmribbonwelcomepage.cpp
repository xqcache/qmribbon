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
    if (auto* ribbon = this->ribbon()) {
        return ribbon->welcomeDialog();
    }
    return nullptr;
}

QmRibbon* QmRibbonWelcomePage::ribbon() const
{
    return qApp->property(QmRibbon::kRibbonPropName).value<QmRibbon*>();
}

void QmRibbonWelcomePage::closeBackstage()
{
    if (auto* ribbon = this->ribbon()) {
        ribbon->hideBackstage();
    }
}
