#pragma once

class QAction;
class QmRibbonFloatingWidget;

/// 示例用的**浮动工具栏**：一个浮在 MainView 之上的小面板，
/// 里面是一排命令按钮 + 「固定」/「关闭」。
///
/// 演示 `QmRibbonFloatingWidget` 的三件事：
///   - 条目用 `addAction()` 放进去（文本 / 图标 / 勾选都跟着 QAction 走）；
///   - **预设排列**由 `setLayoutMode()` 决定：横向单行 / 纵向单列 / 横向多行 / 纵向多列
///     —— 示例里 View → Show 那两个控件（布局下拉框 + 行数列数）会实时切换它；
///   - 拖顶部拖动条（或面板空白处）可以自由移动，松手吸附到 MainView 边缘；
///     「固定」按钮（或双击拖动条）切换固定状态，固定后位置锁死、边框变强调色。
///
/// `toggle` 传 Ribbon 上那个控制显隐的 checkable QAction：工具栏的「关闭」会取消勾选它，
/// 勾选状态与工具栏可见性始终保持同步。工具栏本身用
/// `QmRibbonWindow::addFloatingWidget()` 挂到 MainView 上。
QmRibbonFloatingWidget* makeFloatingToolBar(QAction* toggle);
