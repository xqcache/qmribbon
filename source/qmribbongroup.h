#pragma once

#include <QWidget>

class QmRibbonGroup : public QWidget {
    Q_OBJECT
public:
    explicit QmRibbonGroup(QWidget* parent = nullptr);
    explicit QmRibbonGroup(const QString& title, const QIcon& icon = QIcon(), QWidget* parent = nullptr);
};
