#include "qmribbon.h"
#include "qmribbontitlebar.h"

#include <QApplication>
#include <QMainWindow>
#include <QToolBar>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QMainWindow win;
    win.resize(640, 480);
    auto* ribbon = QmRibbon::install(&win);
    ribbon->titleBar()->quickAccessToolBar()->addAction("Test");

    win.show();

    return app.exec();
}