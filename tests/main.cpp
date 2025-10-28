#include "qmframelesswindow.h"
#include "qmribbon.h"
#include "qmribbonpage.h"
#include "qmribbonsection.h"
#include "qmribbontabbar.h"
#include "qmribbontitlebar.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QToolBar>
#include <QToolButton>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QmFramelessWindow win;
    win.setWindowTitle("Hello ");
    win.resize(1366, 768);

    auto* centeral_widget = new QWidget(&win);
    centeral_widget->setAttribute(Qt::WA_StyledBackground);
    centeral_widget->setStyleSheet("background: blue;");
    win.setCentralWidget(centeral_widget);

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

    auto* btn1 = new QToolButton(conf_widget);
    btn1->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    btn1->setToolButtonStyle(Qt::ToolButtonIconOnly);

    QIcon icon(CURRENT_PATH "/config.svg");
    qDebug() << icon.isNull();
    btn1->setIcon(icon);
    btn1->setIconSize(QSize(50, 50));

    auto* btn2 = new QToolButton(conf_widget);
    btn2->setText("开始模拟");
    btn2->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    lyt_conf->addWidget(btn1);
    lyt_conf->addWidget(btn2);

    win.show();

    return app.exec();
}