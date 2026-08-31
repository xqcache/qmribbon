#include "qmribbonmainwindow.h"
#include "utils/qmribbonshadowgenerator.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QmRibbonMainWindow win;
    win.resize(640, 480);
    win.show();

    QmRibbonShadowGenerator::Options opts;
    opts.offset = { 2, 2 };
    opts.blur = 5;
    opts.radius = 5;
    opts.size = { 96, 96 };
    opts.content_margins = { 10, 0, 14, 16 };
    opts.shadow_color = Qt::black;
    opts.shadow_color.setAlphaF(0.31);

    opts.center_color = QColor("#f3f3f3");
    QImage img = QmRibbonShadowGenerator::generate(opts);
    img.save("D:/1.png", "PNG");

    return app.exec();
}
