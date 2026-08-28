#include "qmribbon.h"
#include "qmribbonbackstageview.h"
#include "qmribbonmainwindow.h"
#include "qmribbonpage.h"
#include "qmribbonsection.h"
#include "qmribbontabbar.h"
#include "qmribbontitlebar.h"

#include <QApplication>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QToolBar>
#include <QToolButton>

class BackstageView : public QmRibbonBackstageView {
public:
    explicit BackstageView(QWidget* parent = nullptr)
        : QmRibbonBackstageView(parent)
    {
        auto* lyt_main = new QVBoxLayout(this);
        lyt_main->setContentsMargins(0, 0, 0, 0);
        lyt_main->setSpacing(0);

        auto* label = new QLabel("Backstage View", this);
        label->setAlignment(Qt::AlignCenter);

        QPushButton* btn_leave = new QPushButton("Leave Backstage View", this);
        connect(btn_leave, &QPushButton::clicked, this, [this] {
            emit leaveBackstageView();
        });
        lyt_main->addWidget(btn_leave);
        lyt_main->addWidget(label);
    }
};

class MainView : public QWidget {
public:
    explicit MainView(QWidget* parent = nullptr)
        : QWidget(parent)
    {
        auto* lyt_main = new QVBoxLayout(this);
        lyt_main->setContentsMargins(0, 0, 0, 0);
        lyt_main->setSpacing(0);

        auto* label = new QLabel("Main View", this);
        label->setAlignment(Qt::AlignCenter);
        lyt_main->addWidget(label);
    }
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QmRibbonMainWindow win(nullptr,
        QmRibbon::NoRibbonButton | QmRibbon::NoUserInfoButton | QmRibbon::NoQuickAccessToolBar | QmRibbon::NoDefaultTitleBar);
    win.setWindowTitle("Hello ");
    win.resize(1366, 768);
    win.setViewAnimationEnabled(true);

    win.setBackstageView(new BackstageView());
    win.setMainView(new MainView());
    win.showMainView();

    auto* ribbon = win.ribbon();
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
