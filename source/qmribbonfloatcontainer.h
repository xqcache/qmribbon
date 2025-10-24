#pragma once

#include <QWidget>

class QHBoxLayout;

class QmRibbonFloatContainer : public QWidget {
    Q_OBJECT
public:
    explicit QmRibbonFloatContainer(QWidget* parent = nullptr);

    void setWidget(QWidget* widget);
    void release();

private:
    QWidget* widget_ { nullptr };
    QWidget* widget_parent_ { nullptr };
    QHBoxLayout* layout_ { nullptr };
};