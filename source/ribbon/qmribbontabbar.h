#pragma once

#include "qmribbonexport.h"

#include <QWidget>

class QAction;
class QButtonGroup;
class QHBoxLayout;
class QToolButton;
class QmRibbonTab;

/// Ribbon Tab 栏。
///
/// 该区域除了普通 Tab 之外还包含 File 按钮与右侧动作（Help / Collapse），
/// 因此不使用 QTabBar，而是自行组织布局。
class QMRIBBON_EXPORT QmRibbonTabBar : public QWidget {
    Q_OBJECT

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

public:
    explicit QmRibbonTabBar(QWidget* parent = nullptr);
    ~QmRibbonTabBar() noexcept override;

    QmRibbonTab* addTab(const QString& title);
    QmRibbonTab* insertTab(int index, const QString& title);
    void removeTab(int index);
    void clear();

    int count() const;
    int currentIndex() const;
    QmRibbonTab* tabAt(int index) const;
    QmRibbonTab* currentTab() const;
    int indexOf(QmRibbonTab* tab) const;

    /// File 按钮，点击应进入 Backstage View。
    QToolButton* fileButton() const;
    void setFileButtonVisible(bool visible);

    /// 折叠 / 展开 Ribbon 的按钮。
    QToolButton* collapseButton() const;
    void setCollapseButtonVisible(bool visible);

    /// 同步折叠按钮的图标方向。
    void setCollapsed(bool collapsed);

    /// 在 Tab 右侧、折叠按钮左侧追加动作按钮。
    QToolButton* addRightAction(QAction* action);

signals:
    void currentIndexChanged(int index);
    /// 每次点击 Tab 都会发出（包括点击当前 Tab），用于「折叠状态下的临时展开」。
    void tabClicked(int index);
    /// 双击 Tab 栏（含双击某个 Tab）——与 Word 一致，用于折叠 / 展开 Ribbon。
    ///
    /// 参数是**手势开始时**（按下那一刻）Ribbon 是否处于折叠状态。
    /// 因为双击的第一下已经在 Tab 上触发了「折叠时点击 Tab 自动展开」，
    /// 接收方必须按手势开始时的状态决定目标模式，而不是简单地取反，
    /// 否则会出现「双击展不开」的问题。
    void barDoubleClicked(bool collapsed_before_press);
    void fileButtonClicked();
    void collapseButtonClicked();

public slots:
    void setCurrentIndex(int index);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    /// 监听 Tab：记录按下状态，并在双击落在 Tab 上时代为转发成 barDoubleClicked()。
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    struct QmRibbonTabBarPrivate;
    QmRibbonTabBarPrivate* d_ { nullptr };
};
