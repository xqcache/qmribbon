#pragma once

#include "qmribbonexport.h"

#include <QProxyStyle>

/// Ribbon 风格的应用样式：包一层应用当前的样式（通常就是 Fusion），
/// 只收敛少数几处与「Office 风格 + 主题样式表」不合拍的原生行为。
///
/// 目前只有一条，但很关键：**`SH_ComboBox_Popup` 一律返回 0**，
/// 也就是不让组合框的弹出列表走「弹出菜单」那套画法。原因是：
///
///   - 那套画法会给弹窗加一层**原生菜单外框**，并在左右各留 `PM_MenuPanelWidth` 的边
///     （表现为又重又旧的一圈框）；
///   - 而 Qt 把弹出容器 `QComboBoxPrivateContainer` 硬编码成「不可被样式表命中」
///     （`qstylesheetstyle.cpp` 的 `unstylable()`：父对象是 QComboBox 的 QFrame 直接返回 true），
///     所以这层框在 QSS 里**删不掉** —— 要么我们的细框和它叠成两层框，要么只剩那层原生框。
///
/// 关掉之后弹出列表完全由 `ribbon.qss` 控制，只剩我们的一条 1px 细框。
///
/// 用法（创建 `QApplication` 之后、创建窗口之前）：
///
/// ```cpp
/// QApplication::setStyle(new QmRibbonStyle(QStyleFactory::create(QStringLiteral("Fusion"))));
/// ```
///
/// 不装这个样式也能正常用，只是普通下拉框的弹出列表会保留 Qt 原生的菜单外框；
/// 那种情况下把 `ribbon.qss` 里 `QComboBox QAbstractItemView` 的 border 去掉更合适。
class QMRIBBON_EXPORT QmRibbonStyle : public QProxyStyle {
    Q_OBJECT

public:
    /// `base` 为空时以应用当前样式为基础；`base` 的所有权转移给本对象（与 QProxyStyle 一致）。
    explicit QmRibbonStyle(QStyle* base = nullptr);

    int styleHint(StyleHint hint, const QStyleOption* option = nullptr, const QWidget* widget = nullptr,
                  QStyleHintReturn* return_data = nullptr) const override;
};
