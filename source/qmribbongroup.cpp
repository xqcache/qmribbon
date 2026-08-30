#include "qmribbongroup.h"

QmRibbonGroup::QmRibbonGroup(const QString& title, const QIcon& icon, QWidget* parent)
    : QWidget(parent)
{
}

QmRibbonGroup::QmRibbonGroup(QWidget* parent)
    : QmRibbonGroup("", QIcon(), parent)
{
}