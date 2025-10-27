#pragma once

#include "qmribbon_global.h"

#include <QWidget>

struct QmRibbonTabBarPrivate;
class QMRIBBON_API QmRibbonTabBar : public QWidget {
    Q_OBJECT
public:
    explicit QmRibbonTabBar(QWidget* parent = nullptr);
    ~QmRibbonTabBar() noexcept override;

    QSize sizeHint() const override;

    void addTab(const QString& title, const QIcon& icon);

private:
    void initUi();

private:
    QmRibbonTabBarPrivate* d_ { nullptr };
};