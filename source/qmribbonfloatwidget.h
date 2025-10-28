#pragma once

#include <QWidget>

class QmRibbonFloatWidget : public QWidget {
    Q_OBJECT
public:
    explicit QmRibbonFloatWidget(QWidget* parent = nullptr);

    void setWidget(QWidget* widget);

private:
};