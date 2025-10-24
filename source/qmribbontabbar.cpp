#include "qmribbontabbar.h"

QmRibbonTabBar::QmRibbonTabBar(QWidget* parent)
    : QTabBar(parent)
{
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
}