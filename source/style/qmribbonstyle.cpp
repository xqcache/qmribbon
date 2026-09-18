#include "qmribbonstyle.h"

QmRibbonStyle::QmRibbonStyle(QStyle* base)
    : QProxyStyle(base)
{}

int QmRibbonStyle::styleHint(StyleHint hint, const QStyleOption* option, const QWidget* widget,
                             QStyleHintReturn* return_data) const
{
    // 组合框弹出列表不要按「弹出菜单」画：那套画法给弹窗加的原生菜单外框
    // 在样式表里删不掉（Qt 把 QComboBoxPrivateContainer 标成了不可命中样式表），
    // 会让普通下拉框的弹出列表多出一层又重又旧的框。详见头文件注释。
    if (hint == QStyle::SH_ComboBox_Popup) {
        return 0;
    }

    return QProxyStyle::styleHint(hint, option, widget, return_data);
}
