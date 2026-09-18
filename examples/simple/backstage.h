#pragma once

class QWidget;
class QmRibbonWindow;

/// File → Backstage 的示例实现：左侧导航 + 右侧页面栈。
///
/// 框架级的 `QmRibbonBackstageView` / `QmRibbonBackstageNavigation` 属于第二阶段，
/// 这里演示业务层如何用 `QmRibbonWindow::setBackstageWidget()` 接入自己的 Backstage：
/// 只要按 `QmRibbonTheme` 约定的 objectName 搭页面，外观就与框架其它部分保持一致。
QWidget* makeBackstage(QmRibbonWindow* window);
