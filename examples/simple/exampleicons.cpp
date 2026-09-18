#include "exampleicons.h"

#include <QFont>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

QIcon makeLetterIcon(const QString& letter, const QColor& color)
{
    constexpr int size = 32;
    constexpr qreal ratio = 2.0;

    QPixmap pixmap(QSize(size, size) * static_cast<int>(ratio));
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(QRectF(1.5, 1.5, size - 3.0, size - 3.0), 7, 7);

    QFont font = painter.font();
    font.setPixelSize(16);
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(QRectF(0, 0, size, size), Qt::AlignCenter, letter);

    return QIcon(pixmap);
}

QIcon makeSwatchIcon(const QColor& color, bool split)
{
    constexpr int size = 18;
    constexpr qreal ratio = 2.0;

    QPixmap pixmap(QSize(size, size) * static_cast<int>(ratio));
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF box(0.5, 0.5, size - 1.0, size - 1.0);
    painter.setPen(QPen(QColor(0x88, 0x88, 0x88), 1.0));
    painter.setBrush(color);
    painter.drawRoundedRect(box, 4, 4);

    if (split) {
        QPainterPath clip;
        clip.addRoundedRect(box, 4, 4);
        painter.setClipPath(clip);
        painter.setPen(Qt::NoPen);
        painter.setBrush(QColor(0x21, 0x21, 0x21));
        painter.drawRect(QRectF(size / 2.0, 0.0, size / 2.0, size));
    }

    return QIcon(pixmap);
}
