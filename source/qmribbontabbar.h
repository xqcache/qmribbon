#pragma once

#include "qmribbon_global.h"

#include <QTabBar>

class QMRIBBON_API QmRibbonTabBar : public QTabBar {
    Q_OBJECT
public:
    explicit QmRibbonTabBar(QWidget* parent = nullptr);
};