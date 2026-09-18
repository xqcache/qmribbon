#include "qmribbon.h"
#include "qmribbonbutton.h"
#include "qmribboncombobox.h"
#include "qmribbongroup.h"
#include "qmribbonpage.h"
#include "qmribbonquickaccessbar.h"
#include "qmribbontitlebar.h"
#include "qmribbonwindow.h"

#include "backstage.h"
#include "exampleicons.h"
#include "floatingtoolbar.h"
#include "gallery.h"
#include "launcherpopup.h"
#include "qmribbonfloatingwidget.h"
#include "qmribbonstyle.h"
#include "qmribbonthememgr.h"

// Qt Advanced Docking System
#include <DockManager.h>
#include <DockWidget.h>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QComboBox>
#include <QCoreApplication>
#include <QFile>
#include <QLabel>
#include <QListWidget>
#include <QSpinBox>
#include <QStyleFactory>
#include <QTextEdit>

namespace {

QAction* makeAction(QObject* parent, const QString& text, const QColor& color,
                    const QKeySequence& shortcut = QKeySequence())
{
    auto* action = new QAction(makeLetterIcon(text.left(1), color), text, parent);
    action->setToolTip(shortcut.isEmpty() ? text : QStringLiteral("%1 (%2)").arg(text, shortcut.toString()));
    if (!shortcut.isEmpty()) {
        action->setShortcut(shortcut);
    }
    return action;
}

// 上一个消息处理器：链式调用，保证其它日志照常输出。
QtMessageHandler g_previous_message_handler = nullptr;

/// 过滤 Qt 在 Windows 上针对「无边框 + 自绘边框」窗口打印的几何诊断。
///
/// 形如：
///   QWindowsWindow::setGeometry: Unable to set geometry 1770x1080+396+180
///   (frame: 1792x1091+385+180) ... Resulting geometry: 1770x1080+396+182 ...
///
/// 含义只是「Windows 把请求的位置挪了 1~2 个像素」，尺寸是按请求生效的：
/// QWindowKit 给窗口设的自绘边框（margins 11, 0, 11, 11）与 Qt 内部的 frame 模型
/// 在缩放显示器上会差一两像素，所以属无害噪声，这里只屏蔽这一条前缀。
void filterGeometryNoise(QtMsgType type, const QMessageLogContext& context, const QString& message)
{
    static const QString prefix = QStringLiteral("QWindowsWindow::setGeometry: Unable to set geometry");
    if (message.startsWith(prefix)) {
        return;
    }

    if (g_previous_message_handler != nullptr) {
        g_previous_message_handler(type, context, message);
    }
}

} // namespace

int main(int argc, char* argv[])
{
    // QWindowKit 要求：必须在构造 QApplication 之前设置。
    QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);

    QApplication app(argc, argv);

    // 屏蔽无边框窗口在缩放显示器上必然出现的几何诊断（见函数注释）。
    g_previous_message_handler = qInstallMessageHandler(filterGeometryNoise);

    // Fusion 会完整遵循应用调色板，这样深色主题下 Qt 自己绘制的控件
    // （下拉列表、复选框、滚动条等）以及 ADS 的停靠区才能一起变暗。
    // 外面再包一层 QmRibbonStyle：它只改组合框弹出列表的画法
    //（不让它走「弹出菜单」那套，否则会多出一层删不掉的原生菜单外框，见头文件注释）。
    QApplication::setStyle(new QmRibbonStyle(QStyleFactory::create(QStringLiteral("Fusion"))));

    // 主题：浅色 / 深色 / 跟随系统。默认跟随系统，切换入口在 Backstage → Account。
    // 想固定某种模式：
    //   QmRibbonThemeMgr::instance().setMode(QmRibbonThemeMgr::Mode::Dark);
    QmRibbonThemeMgr::instance().setMode(QmRibbonThemeMgr::Mode::System);

    // 示例自己的样式表（simple.qrc / simple.qss）：Backstage 页面、Dialog Box Launcher 弹窗等。
    // 交给主题管理器追加应用 —— 它同样用 @{键名} 占位符，换主题时会自动重新解析。
    // 注意要在创建窗口之前设置，这样窗口第一次 apply() 就带上它了。
    {
        QFile style_file(QStringLiteral(":/simple/simple.qss"));
        if (style_file.open(QIODevice::ReadOnly)) {
            QmRibbonThemeMgr::instance().setExtraStyleSheet(QString::fromUtf8(style_file.readAll()));
        }
    }

    QmRibbonWindow window;
    window.setWindowTitle(QStringLiteral("Document1 - qmribbon"));
    window.resize(1180, 720);

    // ---------------- MainView：Qt Advanced Docking System ----------------
    ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);

    auto* dock_manager = window.dockManager();

    auto* document = new QTextEdit;
    document->setPlainText(QStringLiteral("MainView\n\n这里是 Dock 中央文档区，"
                                          "拖动标签页即可浮动 / 停靠到其它区域。"));
    auto* document_dock = dock_manager->createDockWidget(QStringLiteral("Document1"));
    document_dock->setWidget(document);
    dock_manager->addDockWidget(ads::CenterDockWidgetArea, document_dock);

    auto* outline = new QListWidget;
    outline->addItems({ QStringLiteral("Heading 1"), QStringLiteral("Heading 2"), QStringLiteral("Heading 3") });
    auto* outline_dock = dock_manager->createDockWidget(QStringLiteral("Navigation"));
    outline_dock->setWidget(outline);
    dock_manager->addDockWidget(ads::LeftDockWidgetArea, outline_dock);

    auto* properties = new QListWidget;
    properties->addItems({ QStringLiteral("Title: Document1"), QStringLiteral("Author: —"), QStringLiteral("Pages: 1"),
                           QStringLiteral("Modified: —") });
    auto* properties_dock = dock_manager->createDockWidget(QStringLiteral("Properties"));
    properties_dock->setWidget(properties);
    dock_manager->addDockWidget(ads::RightDockWidgetArea, properties_dock);

    // 命令触发的反馈显示在底部 Output 面板里。
    auto* status = new QLabel(QStringLiteral("Ribbon 命令的触发结果会显示在这里"));
    status->setAlignment(Qt::AlignCenter);
    auto* output_dock = dock_manager->createDockWidget(QStringLiteral("Output"));
    output_dock->setWidget(status);
    dock_manager->addDockWidget(ads::BottomDockWidgetArea, output_dock);

    auto* ribbon = window.ribbon();
    auto* title_bar = ribbon->titleBar();
    auto* quick_access = title_bar->quickAccessBar();

    const auto connectAction = [status](QAction* action) {
        QObject::connect(action, &QAction::triggered, status,
                         [status, action]() { status->setText(QStringLiteral("已触发命令：%1").arg(action->text())); });
    };

    // ---------------- Quick Access Toolbar ----------------
    auto* save_action = makeAction(&window, QStringLiteral("Save"), QColor(0x0f, 0x6c, 0xbd), QKeySequence::Save);
    auto* undo_action = makeAction(&window, QStringLiteral("Undo"), QColor(0x60, 0x60, 0x60), QKeySequence::Undo);
    auto* redo_action = makeAction(&window, QStringLiteral("Redo"), QColor(0x60, 0x60, 0x60), QKeySequence::Redo);
    auto* print_action = makeAction(&window, QStringLiteral("Quick Print"), QColor(0x4f, 0x6b, 0x8a));

    quick_access->addAction(save_action);
    quick_access->addAction(undo_action);
    quick_access->addAction(redo_action);
    quick_access->addSeparator();
    quick_access->addAction(print_action);

    for (QAction* action : { save_action, undo_action, redo_action, print_action }) {
        connectAction(action);
    }

    // ---------------- Home ----------------
    auto* home = ribbon->addPage(QStringLiteral("Home"));

    auto* clipboard = home->addGroup(QStringLiteral("Clipboard"));
    auto* paste_action = makeAction(&window, QStringLiteral("Paste"), QColor(0x0f, 0x6c, 0xbd), QKeySequence::Paste);
    auto* cut_action = makeAction(&window, QStringLiteral("Cut"), QColor(0x2b, 0x88, 0xd8), QKeySequence::Cut);
    auto* copy_action = makeAction(&window, QStringLiteral("Copy"), QColor(0x2b, 0x88, 0xd8), QKeySequence::Copy);
    auto* format_painter_action = makeAction(&window, QStringLiteral("Format Painter"), QColor(0x2b, 0x88, 0xd8));
    clipboard->addLargeAction(paste_action);
    clipboard->addAction(cut_action);
    clipboard->addAction(copy_action);
    clipboard->addAction(format_painter_action);

    auto* font = home->addGroup(QStringLiteral("Font"));

    // 字体框：Word 那种「分组标题 + 右侧灰色标注 + 条目用自己的字体渲染」的列表。
    auto* family_combo = new QmRibbonComboBox;
    family_combo->setEditable(true);
    family_combo->setMinimumWidth(150);

    family_combo->addGroupHeader(QStringLiteral("主题字体"));
    family_combo->addEntry(QStringLiteral("等线 Light"), QStringLiteral("(标题)"), QFont(QStringLiteral("等线 Light")));
    family_combo->addEntry(QStringLiteral("等线"), QStringLiteral("(正文)"), QFont(QStringLiteral("等线")));

    family_combo->addGroupHeader(QStringLiteral("所有字体"));
    const QStringList families {
        QStringLiteral("Arial"),        QStringLiteral("Consolas"),      QStringLiteral("Microsoft YaHei UI"),
        QStringLiteral("MS Gothic"),    QStringLiteral("Noto Sans SC"),  QStringLiteral("Segoe UI"),
        QStringLiteral("Times New Roman"),
    };
    for (const QString& family : families) {
        family_combo->addEntry(family, QString(), QFont(family));
    }
    // 默认选中「正文」字体（分组标题不可选中，findText 只会命中真正的条目）。
    family_combo->setCurrentIndex(family_combo->findText(QStringLiteral("等线")));

    // 字号框：没有分组，也没有标注，只是一个普通下拉。
    auto* size_combo = new QmRibbonComboBox;
    size_combo->setEditable(true);
    size_combo->setMinimumWidth(60);
    for (int size : { 8, 9, 10, 11, 12, 14, 16, 18, 20, 24, 28, 36, 48, 72 }) {
        size_combo->addEntry(QString::number(size));
    }
    size_combo->setCurrentIndex(size_combo->findText(QStringLiteral("11")));

    auto* bold_action = makeAction(&window, QStringLiteral("Bold"), QColor(0x1b, 0x1b, 0x1b), QKeySequence::Bold);
    auto* italic_action = makeAction(&window, QStringLiteral("Italic"), QColor(0x1b, 0x1b, 0x1b), QKeySequence::Italic);
    auto* underline_action =
        makeAction(&window, QStringLiteral("Underline"), QColor(0x1b, 0x1b, 0x1b), QKeySequence::Underline);
    bold_action->setCheckable(true);
    italic_action->setCheckable(true);
    underline_action->setCheckable(true);

    font->addWidget(family_combo);
    font->addWidget(size_combo);
    font->addAction(bold_action);
    font->addAction(italic_action);
    font->addAction(underline_action);

    auto* font_dialog_action = makeAction(&window, QStringLiteral("Font Dialog"), QColor(0x0f, 0x6c, 0xbd));
    font->setLauncherAction(font_dialog_action);

    // Dialog Box Launcher（↘）的弹窗响应：贴着这个按钮弹出一个模拟的「字体」面板
    // （见 launcherpopup.cpp；真实项目里这里换成自己的对话框即可）。
    QObject::connect(font_dialog_action, &QAction::triggered, font,
                     [font]() { showFontLauncherPopup(font->launcherButton()); });

    auto* paragraph = home->addGroup(QStringLiteral("Paragraph"));
    paragraph->addAction(makeAction(&window, QStringLiteral("Align Left"), QColor(0x77, 0x77, 0x77)));
    paragraph->addAction(makeAction(&window, QStringLiteral("Center"), QColor(0x77, 0x77, 0x77)));
    paragraph->addAction(makeAction(&window, QStringLiteral("Align Right"), QColor(0x77, 0x77, 0x77)));
    paragraph->addAction(makeAction(&window, QStringLiteral("Bullets"), QColor(0x77, 0x77, 0x77)));
    paragraph->addAction(makeAction(&window, QStringLiteral("Numbering"), QColor(0x77, 0x77, 0x77)));
    paragraph->addAction(makeAction(&window, QStringLiteral("Line Spacing"), QColor(0x77, 0x77, 0x77)));

    auto* styles = home->addGroup(QStringLiteral("Styles"));
    styles->addLargeAction(makeAction(&window, QStringLiteral("Styles"), QColor(0x8a, 0x4f, 0xbd)));
    styles->addAction(makeAction(&window, QStringLiteral("Heading 1"), QColor(0x8a, 0x4f, 0xbd)));
    styles->addAction(makeAction(&window, QStringLiteral("Heading 2"), QColor(0x8a, 0x4f, 0xbd)));

    auto* editing = home->addGroup(QStringLiteral("Editing"));
    editing->addAction(makeAction(&window, QStringLiteral("Find"), QColor(0x2b, 0x88, 0xd8)));
    editing->addAction(makeAction(&window, QStringLiteral("Replace"), QColor(0x2b, 0x88, 0xd8)));
    editing->addAction(makeAction(&window, QStringLiteral("Select"), QColor(0x2b, 0x88, 0xd8)));

    // ---------------- Insert ----------------
    auto* insert = ribbon->addPage(QStringLiteral("Insert"));

    auto* pages = insert->addGroup(QStringLiteral("Pages"));
    pages->addLargeAction(makeAction(&window, QStringLiteral("Cover Page"), QColor(0x0f, 0x6c, 0xbd)));
    pages->addAction(makeAction(&window, QStringLiteral("Blank Page"), QColor(0x2b, 0x88, 0xd8)));
    pages->addAction(makeAction(&window, QStringLiteral("Page Break"), QColor(0x2b, 0x88, 0xd8)));

    auto* tables = insert->addGroup(QStringLiteral("Tables"));
    tables->addLargeAction(makeAction(&window, QStringLiteral("Table"), QColor(0x10, 0x7c, 0x41)));

    auto* illustrations = insert->addGroup(QStringLiteral("Illustrations"));
    illustrations->addLargeAction(makeAction(&window, QStringLiteral("Pictures"), QColor(0xc5, 0x7a, 0x11)));
    illustrations->addAction(makeAction(&window, QStringLiteral("Shapes"), QColor(0xe0, 0x8a, 0x1e)));
    illustrations->addAction(makeAction(&window, QStringLiteral("Icons"), QColor(0xe0, 0x8a, 0x1e)));
    illustrations->addAction(makeAction(&window, QStringLiteral("Chart"), QColor(0xe0, 0x8a, 0x1e)));

    // ---------------- View ----------------
    auto* view = ribbon->addPage(QStringLiteral("View"));

    auto* views = view->addGroup(QStringLiteral("Views"));
    views->addLargeAction(makeAction(&window, QStringLiteral("Read Mode"), QColor(0x0f, 0x6c, 0xbd)));
    views->addAction(makeAction(&window, QStringLiteral("Print Layout"), QColor(0x2b, 0x88, 0xd8)));
    views->addAction(makeAction(&window, QStringLiteral("Web Layout"), QColor(0x2b, 0x88, 0xd8)));

    auto* show = view->addGroup(QStringLiteral("Show"));
    for (const QString& name :
         { QStringLiteral("Ruler"), QStringLiteral("Gridlines"), QStringLiteral("Navigation Pane") }) {
        auto* action = makeAction(&window, name, QColor(0x60, 0x60, 0x60));
        action->setCheckable(true);
        show->addAction(action);
    }

    // ---------------- 浮动工具栏（MainView 之上的浮层） ----------------
    // QmRibbonFloatingWidget + QmRibbonWindow::addFloatingWidget()：
    // 不参与布局、浮在停靠区之上，可拖动（松手吸附到边缘）、可固定。
    auto* floating_action = makeAction(&window, QStringLiteral("Floating Toolbar"), QColor(0x8a, 0x4f, 0xbd));
    floating_action->setCheckable(true);
    floating_action->setChecked(true);
    floating_action->setToolTip(
        QObject::tr("显示 / 隐藏浮动工具栏（可拖动，双击它的拖动条可固定）"));
    show->addAction(floating_action);

    auto* floating_bar = makeFloatingToolBar(floating_action);
    window.addFloatingWidget(floating_bar, QPoint(28, 28));
    QObject::connect(floating_action, &QAction::toggled, floating_bar, &QWidget::setVisible);

    // 浮动工具栏的**预设排列**：横向单行 / 纵向单列 / 横向多行 / 纵向多列。
    // 这两个控件直接改浮动面板的排列方式，方便看效果（行数 / 列数也跟着走）。
    auto* layout_combo = new QmRibbonComboBox;
    layout_combo->addEntry(QObject::tr("横向单行"));
    layout_combo->addEntry(QObject::tr("纵向单列"));
    layout_combo->addEntry(QObject::tr("横向多行"));
    layout_combo->addEntry(QObject::tr("纵向多列"));
    layout_combo->setFixedHeight(22);
    show->addWidget(layout_combo);

    auto* line_spin = new QSpinBox;
    line_spin->setRange(1, 8);
    line_spin->setValue(2);
    line_spin->setToolTip(QObject::tr("「横向多行 / 纵向多列」时的行数 / 列数"));
    line_spin->setFixedHeight(22);
    show->addWidget(line_spin);

    const auto apply_floating_layout = [floating_bar, layout_combo, line_spin]() {
        using LayoutMode = QmRibbonFloatingWidget::LayoutMode;

        switch (layout_combo->currentIndex()) {
        case 1:
            floating_bar->setLayoutMode(LayoutMode::Vertical);
            break;
        case 2:
            floating_bar->setLayoutMode(LayoutMode::Rows, line_spin->value());
            break;
        case 3:
            floating_bar->setLayoutMode(LayoutMode::Columns, line_spin->value());
            break;
        case 0:
        default:
            floating_bar->setLayoutMode(LayoutMode::Horizontal);
            break;
        }
    };

    QObject::connect(layout_combo, &QComboBox::currentIndexChanged, &window,
                     [apply_floating_layout](int) { apply_floating_layout(); });
    QObject::connect(line_spin, &QSpinBox::valueChanged, &window,
                     [apply_floating_layout](int) { apply_floating_layout(); });

    auto* window_group = view->addGroup(QStringLiteral("Window"));
    window_group->addLargeAction(makeAction(&window, QStringLiteral("New Window"), QColor(0x4f, 0x6b, 0x8a)));
    window_group->addAction(makeAction(&window, QStringLiteral("Arrange All"), QColor(0x6b, 0x7f, 0x95)));
    window_group->addAction(makeAction(&window, QStringLiteral("Split"), QColor(0x6b, 0x7f, 0x95)));

    // ---------------- Theme（主题配置组）----------------
    auto& themes = QmRibbonThemeMgr::instance();

    auto* theme_group = view->addGroup(QStringLiteral("Theme"));

    // 三个选项互斥；勾选状态由 QActionGroup 维护，点击后立即切换主题。
    auto* theme_actions = new QActionGroup(&window);
    theme_actions->setExclusive(true);

    const auto add_theme_action = [&](const QString& text, const QIcon& icon, QmRibbonThemeMgr::Mode mode) {
        auto* action = new QAction(icon, text, &window);
        action->setCheckable(true);
        action->setActionGroup(theme_actions);
        theme_group->addLargeAction(action);

        QObject::connect(action, &QAction::triggered, &themes, [&themes, mode]() { themes.setMode(mode); });
        connectAction(action);
        return action;
    };

    auto* light_action = add_theme_action(QStringLiteral("Light"), makeSwatchIcon(QColor(0xf3, 0xf3, 0xf3), false),
                                          QmRibbonThemeMgr::Mode::Light);
    auto* dark_action = add_theme_action(QStringLiteral("Dark"), makeSwatchIcon(QColor(0x21, 0x21, 0x21), false),
                                         QmRibbonThemeMgr::Mode::Dark);
    auto* system_action = add_theme_action(QStringLiteral("跟随系统"), makeSwatchIcon(Qt::white, true),
                                           QmRibbonThemeMgr::Mode::System);

    // 与当前模式对齐，并在别处（例如 Backstage → Account）改动时保持同步。
    const auto sync_theme_actions = [light_action, dark_action, system_action](QmRibbonThemeMgr::Mode mode) {
        switch (mode) {
        case QmRibbonThemeMgr::Mode::Light:
            light_action->setChecked(true);
            break;
        case QmRibbonThemeMgr::Mode::Dark:
            dark_action->setChecked(true);
            break;
        case QmRibbonThemeMgr::Mode::System:
            system_action->setChecked(true);
            break;
        }
    };

    sync_theme_actions(themes.mode());
    QObject::connect(&themes, &QmRibbonThemeMgr::modeChanged, &window, sync_theme_actions);

    // ---------------- Gallery（Qt 常用控件一览） ----------------
    // 把 ribbon.qss 覆盖过的原生控件摆出来，方便肉眼核对「框架控件 / 普通控件」是否同一套外观；
    // 具体实现见 gallery.cpp。
    populateGalleryPage(ribbon->addPage(QStringLiteral("Gallery")));

    // ---------------- Backstage ----------------
    window.setBackstageWidget(makeBackstage(&window));

    window.show();
    return app.exec();
}
