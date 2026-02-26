#pragma once

#include "qmribbon_global.h"

#include <QWidget>

struct QmRibbonTabBarPrivate;
class QMRIBBON_API QmRibbonTabBar : public QWidget {
    Q_OBJECT
public:
    explicit QmRibbonTabBar(QWidget* parent = nullptr);
    ~QmRibbonTabBar() noexcept override;

    QToolButton* addTab(const QString& title, const QIcon& icon);
    QToolButton* applicationButton() const;

    void setApplicationButtonVisible(bool visible);

    void activeTab(int index);

signals:
    void requestToggleFloating(QPrivateSignal);
    void tabActivated(int index, QPrivateSignal);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void initUi();

private:
    QmRibbonTabBarPrivate* d_ { nullptr };
};