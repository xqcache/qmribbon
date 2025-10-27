#include "qmframelesswindow.h"
#include "qmribbon.h"
#include "qmribbontabbar.h"
#include "qmribbontitlebar.h"

#include <QApplication>
#include <QToolBar>
#include <QToolButton>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QmFramelessWindow win;
    win.setWindowTitle("Hello ");
    win.resize(1366, 768);

    auto* ribbon = QmRibbon::install(&win);
    ribbon->setFeatures(QmRibbon::NoRibbonButton | QmRibbon::NoUserInfoButton | QmRibbon::NoQuickAccessToolBar);
    ribbon->titleBar()->quickAccessToolBar()->addAction("Test");
    ribbon->tabBar()->applicationButton()->setText("File");

    auto* simu_page = ribbon->addPage("Simulate");
    auto* post_proc_page = ribbon->addPage("Post Process");
    auto* tools_page = ribbon->addPage("Tools");
    auto* view_page = ribbon->addPage("View");
    auto* help_page = ribbon->addPage("Help");

    win.show();

    return app.exec();
}