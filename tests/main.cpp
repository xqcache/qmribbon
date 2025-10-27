#include "qmframelesswindow.h"
#include "qmribbon.h"
#include "qmribbonpage.h"
#include "qmribbonsection.h"
#include "qmribbontabbar.h"
#include "qmribbontitlebar.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
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

    auto* conf_section = simu_page->addSection("Model Config", QIcon(), QmRibbonSection::NoTitle | QmRibbonSection::NoExpandButton);

    auto* conf_widget = new QWidget(simu_page);
    conf_section->setWidget(conf_widget);

    auto* lyt_conf = new QHBoxLayout(conf_widget);
    lyt_conf->addWidget(new QPushButton("模型配置"));
    lyt_conf->addWidget(new QPushButton("开始仿真"));

    win.show();

    return app.exec();
}