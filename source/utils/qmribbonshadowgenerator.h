#pragma once

#include "qmribbonexport.h"

#include <QColor>
#include <QImage>
#include <QPointF>
#include <QRect>

class QPainter;

/// 生成可九宫格（9-slice）拉伸的窗口阴影贴图。
///
/// 生成的图片中心为全透明，只包含四周的柔和阴影，
/// 因此可以按边/角切片后拉伸到任意窗口尺寸。
///
/// 这是**私有**工具（头文件不安装、库内部也没有调用点），带导出宏仅仅是因为
/// `tests/shadow_svg` 用构建树里的相对路径包含它：动态库构建下这两个静态函数必须能从
/// qmribbon.dll 里导出，否则那个测试链接不过。
class QMRIBBON_EXPORT QmRibbonShadowGenerator {
public:
    struct Options {
        /// 阴影向外扩散的距离，同时也是九宫格边框宽度。
        int spread { 12 };
        /// 窗口圆角半径，取值范围 [0, spread]。
        qreal radius { 0.0 };
        /// 中间可拉伸区域的边长，只需大于 0。
        int center { 8 };
        /// 阴影整体偏移，通常向下偏移一点。
        QPointF offset { 0.0, 3.0 };
        /// 阴影颜色（alpha 决定深浅）。
        QColor color { 0, 0, 0, 60 };
    };

    /// 生成阴影贴图，尺寸为 2 * spread + center。
    static QImage generate(const Options& options);

    /// 把阴影绘制到 target 之外，target 为窗口内容矩形，border 必须等于生成时的 spread。
    static void draw(QPainter* painter, const QRect& target, const QImage& shadow, int border);
};
