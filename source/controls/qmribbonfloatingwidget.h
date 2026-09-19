#pragma once

#include "qmribbonexport.h"

#include <QFrame>
#include <QPoint>

class QAction;
class QMouseEvent;
class QPaintEvent;
class QToolButton;

/// 浮动控件：浮在父控件（通常是 MainView）之上的一小块面板，用来做**浮动工具栏**这类
/// 「不参与布局、可以拖来拖去、也能固定住」的东西。
///
/// 内容与排列：
///   - `addAction()` / `addWidget()` 往里放条目（一排命令按钮就是浮动工具栏）；
///   - `setLayoutMode()` 选**预设排列**：横向单行、纵向单列、横向多行、纵向多列；
///   - 条目变化时按内容自动调整尺寸（需要固定尺寸就在加完条目后自己 `resize()`）。
///
/// 交互：
///   - **拖动**：按住顶部拖动条（或面板空白处）拖动，全程夹在父控件可见区域内；
///     松手时离父控件边缘足够近就自动吸附过去；
///   - **固定**：`setPinned(true)` 后位置锁死、不再响应拖动（取消固定后恢复可拖动）；
///     双击拖动条也可以切固定；
///   - 拖动条（抓手 / 标题 / 固定标记）由控件用 `QmRibbonThemeMgr::current()` 自绘，
///     面板底色边框走 `#RibbonFloatingWidget`（见 `resources/themes/ribbon.qss`）。
///
/// 用法（配合 `QmRibbonWindow::addFloatingWidget()`）：
///
/// ```cpp
/// auto* bar = new QmRibbonFloatingWidget;
/// bar->setTitle(tr("浮动工具栏"));
/// bar->setLayoutMode(QmRibbonFloatingWidget::LayoutMode::Rows, 2);   // 横向两行
/// bar->addAction(bold_action);
/// bar->addAction(italic_action);
/// bar->addWidget(my_combo);
/// window->addFloatingWidget(bar, QPoint(24, 24));
/// ```
class QMRIBBON_EXPORT QmRibbonFloatingWidget : public QFrame {
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(bool movable READ isMovable WRITE setMovable)
    Q_PROPERTY(bool pinned READ isPinned WRITE setPinned NOTIFY pinnedChanged)
    Q_PROPERTY(bool snapToEdges READ snapToEdges WRITE setSnapToEdges)

public:
    /// 预设排列方式。
    enum class LayoutMode {
        Horizontal, ///< 横向单行
        Vertical,   ///< 纵向单列
        Rows,       ///< 横向排列、摊成 `lineCount()` 行（短行排在后面，中间不留空）
        Columns,    ///< 纵向排列、摊成 `lineCount()` 列（同上）
    };
    Q_ENUM(LayoutMode)

    explicit QmRibbonFloatingWidget(QWidget* parent = nullptr);
    ~QmRibbonFloatingWidget() noexcept override;

    // ---- 内容 ----

    /// 放一个命令按钮：内部是平铺的 `QToolButton`，文本 / 图标 / 勾选 / 菜单都跟着 action 走。
    QToolButton* addAction(QAction* action);
    /// 放任意控件（下拉框、输入框、分隔线、自定义面板…）。
    void addWidget(QWidget* widget);
    /// 条目数量。
    int count() const;
    /// 清空所有条目（控件会被删除）。
    void clear();

    // ---- 预设排列 ----

    LayoutMode layoutMode() const;
    /// 设置排列方式；`line_count` 只在 `Rows` / `Columns` 下有意义
    ///（**期望**的行数 / 列数，默认 2）。条目尽量均匀地摊开，**短的那几行/列排在后面**，
    /// 所以中间不会出现空位：6 个条目 4 行 → 2/2/1/1；条目比它少时按条目数算。
    void setLayoutMode(LayoutMode mode, int line_count = 2);
    /// `Rows` 的行数 / `Columns` 的列数，最小 1。
    int lineCount() const;
    void setLineCount(int line_count);

    // ---- 外观与交互 ----

    /// 拖动条上显示的文字；为空时只画抓手。
    void setTitle(const QString& title);
    QString title() const;

    /// 是否允许拖动，默认 true。固定（pinned）时无论如何都不可拖动。
    bool isMovable() const;
    void setMovable(bool movable);

    /// 固定：位置锁死，不再响应拖动。`movable` 的原值会保留，取消固定后照旧。
    bool isPinned() const;
    void setPinned(bool pinned);

    /// 松手时是否吸附到父控件边缘，默认 true。
    bool snapToEdges() const;
    void setSnapToEdges(bool snap);

    /// 拖动条高度（抓手 / 标题所在的那一条）。
    int dragBarHeight() const;

    /// 把当前位置夹回父控件内部（父控件尺寸变化时会自动调用）。
    void clampToParent();
    /// 吸附到最近的父控件边缘（在 `floating_snap_distance` 之内才吸附）。
    void snapToNearestEdge();

signals:
    /// 固定状态变化（点击 / 双击拖动条，或直接调 `setPinned()`）。
    void pinnedChanged(bool pinned);
    /// 拖动结束（松手或自动吸附之后），位置是相对父控件的坐标。
    void moved(const QPoint& position);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    bool event(QEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    /// 按当前 `layoutMode()` 重新摆放所有条目（条目增删、换模式、改行数时调用）。
    void relayout();
    /// 这个位置能不能作为拖动起点：拖动条上，或者面板的空白处（不含子控件）。
    bool isDragArea(const QPoint& pos) const;
    bool canDrag() const;
    /// 监听当前父控件的 resize，用来把面板夹回可见区域（换父控件时自动改听新的）。
    void watchParentResize();

    struct QmRibbonFloatingWidgetPrivate;
    QmRibbonFloatingWidgetPrivate* d_ { nullptr };
};
