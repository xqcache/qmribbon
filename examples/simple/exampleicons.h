#pragma once

#include <QColor>
#include <QIcon>
#include <QString>

/// 示例用的图标生成工具。
/// 真实项目中请替换为设计好的图标资源，这里只是为了演示时能看出区别。

/// 带字母的圆角彩色图标（Ribbon 命令、Quick Access 等用）。
QIcon makeLetterIcon(const QString& letter, const QColor& color);

/// 主题色板图标；split 为 true 时右半边画成深色，用于「跟随系统」。
QIcon makeSwatchIcon(const QColor& color, bool split);
