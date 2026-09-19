#pragma once

#include "qmribbonexport.h"

#include <QWidget>

class QAction;
class QToolButton;

/// Quick Access Toolbar（保存 / 撤销 / 重做 ...）。
///
/// 始终可见的高频命令区域，通过 QAction 与 Ribbon 其它位置共享同一份命令。
class QMRIBBON_EXPORT QmRibbonQuickAccessBar : public QWidget {
    Q_OBJECT

public:
    explicit QmRibbonQuickAccessBar(QWidget* parent = nullptr);
    ~QmRibbonQuickAccessBar() noexcept override;

    QToolButton* addAction(QAction* action);
    QToolButton* insertAction(QAction* before, QAction* action);
    void addSeparator();
    void removeAction(QAction* action);
    void clear();

    QList<QAction*> actions() const;
    QToolButton* buttonForAction(QAction* action) const;

    QSize iconSize() const;
    void setIconSize(const QSize& size);

private:
    struct QmRibbonQuickAccessBarPrivate;
    QmRibbonQuickAccessBarPrivate* d_ { nullptr };
};
