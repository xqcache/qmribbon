#include "qmribbonsection.h"

QmRibbonSection::QmRibbonSection(QWidget* parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void QmRibbonSection::setTitle(const QString& title)
{
}

void QmRibbonSection::setIcon(const QIcon& icon)
{
}