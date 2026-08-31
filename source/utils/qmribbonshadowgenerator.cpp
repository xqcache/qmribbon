#include "qmribbonshadowgenerator.h"
#include <QDebug>
#include <QImageReader>
#include <format>

namespace {

constexpr std::string_view kShadowSvgTemplate = R"SVG(<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg"
     width="{0}"
     height="{1}"
     viewBox="0 0 {0} {1}">

  <defs>
    <filter id="soft"
            x="-40%"
            y="-30%"
            width="180%"
            height="180%"
            filterUnits="objectBoundingBox">
      <feGaussianBlur stdDeviation="{2}"/>
    </filter>
  </defs>

  <path d="M{3} {4}
           V{5}
           Q{3} {6} {7} {6}
           H{8}
           Q{9} {6} {9} {5}
           V{4}"
        fill="none"
        stroke="{10}"
        stroke-width="{11}"
        stroke-linecap="round"
        stroke-linejoin="round"
        stroke-opacity="{12}"
        transform="translate({13} {14})"
        filter="url(#soft)"/>

  <rect x="{15}"
        y="{16}"
        width="{17}"
        height="{18}"
        rx="{19}"
        ry="{19}"
        fill="{20}"
        fill-opacity="{21}"/>

</svg>
)SVG";

} // namespace

QImage QmRibbonShadowGenerator::generate(const Options& options)
{
    if (options.size.isEmpty()) {
        return { };
    }

    const qreal radius = options.radius;

    const qreal left = options.content_margins.left();
    const qreal top = options.content_margins.top();
    const qreal right = options.size.width() - options.content_margins.right();
    const qreal bottom = options.size.height() - options.content_margins.bottom();

    const qreal top_radius_y = top + radius;
    const qreal bottom_radius_y = bottom - radius;
    const qreal left_radius_x = left + radius;
    const qreal right_radius_x = right - radius;

    const QString shadow_rgb = options.shadow_color.name(QColor::HexRgb);
    const QString center_rgb = options.center_color.name(QColor::HexRgb);

    const qreal shadow_opacity = options.shadow_color.alphaF();
    const qreal center_opacity = options.center_color.alphaF();

    const qreal stroke_width = std::max<qreal>(1.0, radius * 0.8);

    // clang-format off
    const QString svg = QString::fromStdString(std::format(
        kShadowSvgTemplate,
        options.size.width(),        // {0}
        options.size.height(),       // {1}
        options.blur,                // {2}

        left,                        // {3}
        top_radius_y,                  // {4}
        bottom_radius_y,               // {5}
        bottom,                      // {6}
        left_radius_x,                 // {7}
        right_radius_x,                // {8}
        right,                       // {9}

        shadow_rgb.toStdString(),     // {10}
        options.blur,                 // {11}
        shadow_opacity,               // {12}

        options.offset.x(),          // {13}
        options.offset.y(),          // {14}

        left,     // {15}
        top,     // {16}
        (options.size.width() - options.content_margins.left() - options.content_margins.right()), // {17}
        (options.size.height() - options.content_margins.top() - options.content_margins.bottom()),// {18}

        radius,                      // {19}
        center_rgb.toStdString(),     // {20}
        center_opacity                // {21}
    ));
    // clang-format on

    qDebug() << svg.toStdString().data();
    qDebug() << QImageReader::supportedImageFormats();

    QImage image;

    if (!image.loadFromData(svg.toUtf8(), "SVG")) {
        qWarning() << "Failed to load SVG from generated data.";
        return { };
    }

    return image;
}