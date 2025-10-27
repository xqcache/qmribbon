#include "qmframelesswindow.h"
#include "qmribbon.h"
#include "qmribbontitlebar.h"

#include <QApplication>
#include <QToolBar>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QmFramelessWindow win;
    win.resize(640, 480);
    auto* ribbon = QmRibbon::install(&win);
    ribbon->setFeatures(QmRibbon::NoRibbonButton | QmRibbon::NoUserInfoButton | QmRibbon::NoQuickAccessToolBar);
    ribbon->titleBar()->quickAccessToolBar()->addAction("Test");
    ribbon->addPage("File");
    ribbon->addPage("View");

    win.setWindowTitle("Hello ");
    win.show();

    return app.exec();
}