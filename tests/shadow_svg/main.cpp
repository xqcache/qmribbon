#include "qmribbontheme.h"
#include "qmribbonthememgr.h"
#include "utils/qmribbonshadowgenerator.h"

#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QString>

namespace {

/// 把阴影渲染到一张预览图上，便于人工检查九宫格拉伸是否正确。
QImage makePreview(const QImage& shadow, int border)
{
    const QmRibbonTheme theme = QmRibbonThemeMgr::light();

    QImage preview(520, 360, QImage::Format_ARGB32_Premultiplied);
    preview.fill(Qt::white);

    QPainter painter(&preview);
    painter.setRenderHint(QPainter::Antialiasing, false);

    const QRect target(60, 60, 400, 240);
    QmRibbonShadowGenerator::draw(&painter, target, shadow, border);

    painter.fillRect(target, theme.windowBackground());
    painter.setPen(theme.border());
    painter.setBrush(Qt::NoBrush);
    painter.drawRect(target.adjusted(0, 0, -1, -1));

    return preview;
}

} // namespace

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);

    // 说明：QmRibbonWindow 现在使用系统边框（Win11 由 DWM 提供圆角与阴影），
    // 这个测试只用来单独检查九宫格阴影贴图工具本身的输出。
    QmRibbonShadowGenerator::Options options;
    options.spread = 12;
    options.radius = 0.0;
    options.center = 8;
    options.offset = QPointF(0.0, 3.0);
    options.color = QColor(0, 0, 0, 60);

    const QImage shadow = QmRibbonShadowGenerator::generate(options);
    if (shadow.isNull()) {
        return 1;
    }

    const QString output_dir = QCoreApplication::applicationDirPath();
    shadow.save(output_dir + QStringLiteral("/shadow_raw.png"));
    makePreview(shadow, options.spread).save(output_dir + QStringLiteral("/shadow_preview.png"));

    return 0;
}
