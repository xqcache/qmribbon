#pragma once

#include <QImage>

class QmRibbonShadowGenerator {
public:
    struct Options {
        QSize size { 96, 96 };

        qreal radius = 5.0;
        qreal blur = 10.0;
        QPointF offset { 0, 6.0 };

        QMarginsF content_margins { 10, 10, 10, 10 };
        QColor shadow_color = Qt::black;
        QColor center_color;
    };

    QImage generate() const;

    static QImage generate(const Options& options);

private:
    Options optoins_;
};