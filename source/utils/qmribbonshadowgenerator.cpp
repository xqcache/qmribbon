#include "qmribbonshadowgenerator.h"

#include <QDebug>
#include <QPainter>
#include <QSvgRenderer>

#include <format>

namespace {

constexpr std::string_view kShadowSvgTemplate = R"SVG(<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg"
     width="{0}"
     height="{0}"
     viewBox="0 0 {0} {0}">

  <defs>
    <filter id="soft"
            filterUnits="userSpaceOnUse"
            x="-{1}"
            y="-{1}"
            width="{2}"
            height="{2}">
      <feGaussianBlur stdDeviation="{3}"/>
    </filter>
  </defs>

  <rect x="{4}"
        y="{4}"
        width="{5}"
        height="{5}"
        rx="{6}"
        ry="{6}"
        fill="none"
        stroke="{7}"
        stroke-width="{8}"
        stroke-opacity="{9}"
        transform="translate({10} {11})"
        filter="url(#soft)"/>

</svg>
)SVG";

} // namespace

QImage QmRibbonShadowGenerator::generate(const Options& options)
{
    const int spread = qMax(2, options.spread);
    const int center = qMax(1, options.center);
    const int size = 2 * spread + center;

    const qreal radius = qBound(0.0, options.radius, static_cast<qreal>(spread));
    const qreal sigma = spread / 4.0;
    const qreal stroke_width = 2.0;
    const int pad = spread;
    const int filter_size = size + 2 * pad;

    const QString rgb = options.color.name(QColor::HexRgb);
    const qreal opacity = options.color.alphaF();

    // clang-format off
    const QString svg = QString::fromStdString(std::format(
        kShadowSvgTemplate,
        size,                          // {0}
        pad,                           // {1}
        filter_size,                   // {2}
        sigma,                         // {3}
        spread,                        // {4}
        center,                        // {5}
        radius,                        // {6}
        rgb.toStdString(),             // {7}
        stroke_width,                  // {8}
        opacity,                       // {9}
        options.offset.x(),            // {10}
        options.offset.y()             // {11}
    ));
    // clang-format on

    // 直接使用 Qt6::Svg 模块渲染，避免依赖运行时的 svg imageformat 插件。
    QSvgRenderer renderer(svg.toUtf8());
    if (!renderer.isValid()) {
        qWarning() << "QmRibbonShadowGenerator: failed to render shadow SVG";
        return {};
    }

    QImage image(size, size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);

    {
        QPainter painter(&image);
        renderer.render(&painter);
    }

    return image;
}

void QmRibbonShadowGenerator::draw(QPainter* painter, const QRect& target, const QImage& shadow, int border)
{
    if (painter == nullptr || shadow.isNull() || border <= 0) {
        return;
    }

    const int width = shadow.width();
    const int height = shadow.height();

    if (width <= 2 * border || height <= 2 * border) {
        return;
    }

    const int band_width = width - 2 * border;
    const int band_height = height - 2 * border;

    // 四角：按原尺寸绘制
    const QRect source_corners[4] = {
        { 0, 0, border, border },
        { width - border, 0, border, border },
        { 0, height - border, border, border },
        { width - border, height - border, border, border },
    };
    const QRect target_corners[4] = {
        { target.left() - border, target.top() - border, border, border },
        { target.right() + 1, target.top() - border, border, border },
        { target.left() - border, target.bottom() + 1, border, border },
        { target.right() + 1, target.bottom() + 1, border, border },
    };

    for (int i = 0; i < 4; ++i) {
        painter->drawImage(target_corners[i], shadow, source_corners[i]);
    }

    // 四边：单方向拉伸
    painter->drawImage(QRect(target.left(), target.top() - border, target.width(), border), shadow,
                       QRect(border, 0, band_width, border));
    painter->drawImage(QRect(target.left(), target.bottom() + 1, target.width(), border), shadow,
                       QRect(border, height - border, band_width, border));
    painter->drawImage(QRect(target.left() - border, target.top(), border, target.height()), shadow,
                       QRect(0, border, border, band_height));
    painter->drawImage(QRect(target.right() + 1, target.top(), border, target.height()), shadow,
                       QRect(width - border, border, border, band_height));
}
