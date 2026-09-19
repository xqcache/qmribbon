// qmribbon 安装包消费验证（独立工程，见同目录 CMakeLists.txt）。
//
// 这里刻意「触达每一个公共类」：任何一类漏了 QMRIBBON_EXPORT，动态库（DLL）版本在
// **链接阶段**就会失败（找不到 __imp_ 符号），而不是拖到运行期才出奇怪问题。
//
// 只构造、不显示、不进事件循环：链接通过即算验证通过。

#include "qmribbon.h"
#include "qmribbonanimationutil.h"
#include "qmribbonbutton.h"
#include "qmribboncombobox.h"
#include "qmribbonfloatingwidget.h"
#include "qmribbongroup.h"
#include "qmribbonpage.h"
#include "qmribbonquickaccessbar.h"
#include "qmribbonstyle.h"
#include "qmribbontab.h"
#include "qmribbontabbar.h"
#include "qmribbontheme.h"
#include "qmribbonthememgr.h"
#include "qmribbonthemeswitchmask.h"
#include "qmribbontitlebar.h"
#include "qmribbonwindow.h"

#include <QAction>
#include <QApplication>
#include <QMetaObject>
#include <QPixmap>
#include <QTextStream>

#include <cstdio>
#include <iterator>

int main(int argc, char* argv[])
{
    // QWindowKit 的要求：必须在构造 QApplication 之前设置。
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    QApplication app(argc, argv);

    // 1) 14 个 Q_OBJECT 公共类：取 staticMetaObject 的地址。它是类的导出静态数据成员，
    //    取址即强制链接器去解析，漏宏必失败。
    const QMetaObject* const metas[] = {
        &QmRibbonWindow::staticMetaObject,         &QmRibbon::staticMetaObject,
        &QmRibbonTitleBar::staticMetaObject,       &QmRibbonQuickAccessBar::staticMetaObject,
        &QmRibbonTabBar::staticMetaObject,         &QmRibbonTab::staticMetaObject,
        &QmRibbonPage::staticMetaObject,           &QmRibbonGroup::staticMetaObject,
        &QmRibbonButton::staticMetaObject,         &QmRibbonComboBox::staticMetaObject,
        &QmRibbonFloatingWidget::staticMetaObject, &QmRibbonStyle::staticMetaObject,
        &QmRibbonThemeMgr::staticMetaObject,       &QmRibbonThemeSwitchMask::staticMetaObject,
    };

    // 2) 另外两个公共类不是 QObject，用静态工厂 / 静态函数触达。
    const QmRibbonTheme light_theme = QmRibbonThemeMgr::light();
    const int animation_duration = QmRibbonAnimationUtil::duration();
    const QStringList easing_names = QmRibbonAnimationUtil::easingNames();

    // 3) 正常用法走一遍：页面 / 分组 / 按钮 / 下拉框 / 浮动工具栏 / 停靠区。
    QmRibbonWindow window;
    window.setWindowTitle(QStringLiteral("qmribbon consumer"));
    window.resize(900, 600);

    QAction command(QStringLiteral("Paste"), &window);

    QmRibbon* ribbon = window.ribbon();
    QmRibbonPage* page = ribbon->addPage(QStringLiteral("Home"));
    QmRibbonGroup* group = page->addGroup(QStringLiteral("Clipboard"));
    group->addLargeAction(&command);
    ribbon->quickAccessBar()->addAction(&command);

    auto* combo = new QmRibbonComboBox;
    combo->addGroupHeader(QStringLiteral("主题字体"));
    combo->addEntry(QStringLiteral("Arial"), QStringLiteral("(正文)"));
    group->addWidget(combo);

    auto* floating = new QmRibbonFloatingWidget;
    floating->setTitle(QStringLiteral("浮动工具栏"));
    floating->setLayoutMode(QmRibbonFloatingWidget::LayoutMode::Rows, 2);
    floating->addAction(&command);
    window.addFloatingWidget(floating, QPoint(24, 24));

    // ADS 是 qmribbon 的公共依赖：这一行顺带验证安装包里「导入的 ADS 目标」可用。
    // 指针类型只需前置声明，所以这里不用 include ADS 的任何头文件。
    ads::CDockManager* dock_manager = window.dockManager();

    // 4) 主题管理器与过渡遮罩：跨模块取单例、构造遮罩。
    QmRibbonThemeMgr& manager = QmRibbonThemeMgr::instance();
    QmRibbonThemeSwitchMask mask(&window);
    mask.setBeforeSnapshot(QPixmap(4, 4));
    mask.setAfterSnapshot(QPixmap(4, 4));
    mask.setOrigin(window.rect().center());

    QTextStream(stdout) << "qmribbon consumer ok:"
                        << " classes=" << std::size(metas)
                        << " theme=" << (light_theme.isValid() ? light_theme.name() : QStringLiteral("<invalid>"))
                        << " dark=" << (light_theme.isDark() ? "true" : "false")
                        << " animationDuration=" << animation_duration
                        << " easingCurves=" << easing_names.size()
                        << " mode=" << static_cast<int>(manager.mode())
                        << " groups=" << page->groupCount()
                        << " floatingPinned=" << (floating->isPinned() ? "true" : "false")
                        << " dockManager=" << (dock_manager != nullptr ? "ok" : "null") << '\n';

    return 0;
}
