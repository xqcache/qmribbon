#include "floatingtoolbar.h"

#include "exampleicons.h"
#include "qmribbonfloatingwidget.h"

#include <QAction>
#include <QColor>

namespace {

/// 工具栏上那种「小图标 + 文字」的扁平按钮由 addAction() 自动创建，
/// 这里只负责把 QAction 配好（图标 / 提示 / 可勾选）。
QAction* makeCommand(QObject* parent, const QString& text, const QColor& color, bool checkable)
{
    auto* action = new QAction(makeLetterIcon(text.left(1), color), text, parent);
    action->setCheckable(checkable);
    action->setToolTip(text);
    return action;
}

} // namespace

QmRibbonFloatingWidget* makeFloatingToolBar(QAction* toggle)
{
    auto* bar = new QmRibbonFloatingWidget;
    bar->setTitle(QObject::tr("浮动工具栏"));

    // 预设排列：这里先用「横向两行」，示例里可以用 View → Show 的两个控件实时切换。
    bar->setLayoutMode(QmRibbonFloatingWidget::LayoutMode::Rows, 2);

    // 一排命令（勾选态由 QAction 维护，按钮自动跟随）
    const QColor command_color(0x2b, 0x88, 0xd8);
    for (const QString& command :
         { QObject::tr("Bold"), QObject::tr("Italic"), QObject::tr("Underline"), QObject::tr("Highlight") }) {
        bar->addAction(makeCommand(bar, command, command_color, true));
    }

    // 「固定」：与浮动控件的 pinned 状态双向同步（双击拖动条也能切换）。
    auto* pin = makeCommand(bar, QObject::tr("固定"), QColor(0x10, 0x7c, 0x41), true);
    bar->addAction(pin);
    QObject::connect(pin, &QAction::toggled, bar, &QmRibbonFloatingWidget::setPinned);
    QObject::connect(bar, &QmRibbonFloatingWidget::pinnedChanged, pin, &QAction::setChecked);

    // 「关闭」= 取消 Ribbon 上那个勾选（勾选状态统一由 QAction 维护）。
    auto* close = makeCommand(bar, QObject::tr("关闭"), QColor(0xc4, 0x2b, 0x1c), false);
    bar->addAction(close);

    if (toggle != nullptr) {
        QObject::connect(close, &QAction::triggered, toggle, [toggle]() { toggle->setChecked(false); });
    } else {
        QObject::connect(close, &QAction::triggered, bar, &QWidget::hide);
    }

    return bar;
}
