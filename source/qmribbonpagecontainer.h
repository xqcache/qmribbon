#pragma once

#include <QStackedWidget>

class QmRibbonPageContainer : public QStackedWidget {
    Q_OBJECT
public:
    explicit QmRibbonPageContainer(QWidget* parent = nullptr);
};