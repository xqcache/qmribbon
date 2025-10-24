#include "qmribbonpagecontainer.h"

QmRibbonPageContainer::QmRibbonPageContainer(QWidget* parent)
    : QStackedWidget(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
}