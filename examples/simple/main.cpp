#include "qmribbonmainwindow.h"
#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QmRibbonMainWindow win;
    win.resize(640, 480);
    win.show();

    return app.exec();
}
