#pragma once

#include "qmribbon_global.h"

#include <QWidget>

class QMRIBBON_API QmRibbonBackstageView : public QWidget {
    Q_OBJECT
public:
    explicit QmRibbonBackstageView(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());

private:
    void initUi();

signals:
    void leaveBackstageView();
};
