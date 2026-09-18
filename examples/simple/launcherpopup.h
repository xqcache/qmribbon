#pragma once

class QWidget;

/// Dialog Box Launcher（Ribbon Group 右下角的 ↘）的**模拟弹窗**示例。
///
/// 真实项目里这个箭头通常打开一个功能完整的对话框（Word 的「字体」对话框就是这样）；
/// 这里给出一份精简版的模拟实现：贴在该按钮下方弹出一个面板，
/// 里面是字体 / 字形 / 字号 + 效果 + 预览 + 确定 / 取消，点面板外面或按 Esc 关闭。
///
/// 面板配色走主题（`#RibbonLauncherPopup*`，见 `resources/themes/ribbon.qss`），
/// 所以它跟 Ribbon 其它部分是同一套外观。
///
/// `anchor` 传 launcher 按钮（`QmRibbonGroup::launcherButton()`），面板贴着它弹出。
void showFontLauncherPopup(QWidget* anchor);
