#include "gallery.h"

#include "exampleicons.h"
#include "qmribbongroup.h"
#include "qmribbonpage.h"
#include "qmribbontheme.h"
#include "qmribbonthememgr.h"

#include <QAction>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollBar>
#include <QSpinBox>
#include <QSplitter>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {

/// Ribbon 的一行高（和 Small 按钮一样是 22px）：
/// Group 的内容区固定 70px = 3 行 × 22 + 2 × 2，原生控件固定成这个高度才能整齐地三行一列。
constexpr int kRowHeight = 22;

void fitRibbonRow(QWidget* widget)
{
    widget->setFixedHeight(kRowHeight);
}

/// 一个带若干项的菜单（顺带展示 `QMenu` / `QMenu::separator` / 禁用项的样式）。
QMenu* makeSampleMenu(QWidget* parent)
{
    auto* menu = new QMenu(parent);

    auto* checkable = menu->addAction(QObject::tr("可勾选项"));
    checkable->setCheckable(true);
    checkable->setChecked(true);

    menu->addAction(QObject::tr("普通项"));
    menu->addSeparator();

    auto* disabled = menu->addAction(QObject::tr("禁用项"));
    disabled->setEnabled(false);

    return menu;
}

/// `QPushButton`：通用样式给了 4px 上下内边距，按钮自然高度约 26px，
/// 所以一列只放两个（三行会超出 Group 的 70px 内容区）。
void addPushButtonGroup(QmRibbonPage* page)
{
    auto* group = page->addGroup(QObject::tr("Push Button"));

    group->addWidget(new QPushButton(QObject::tr("Push Button")));

    auto* with_menu = new QPushButton(QObject::tr("Menu"));
    with_menu->setMenu(makeSampleMenu(with_menu));
    group->addWidget(with_menu);
}

/// 单行输入类：`QLineEdit` / `QSpinBox` / `QComboBox` 都是 22px 高，正好一列三行。
void addInputGroup(QmRibbonPage* page)
{
    auto* group = page->addGroup(QObject::tr("Input"));

    auto* edit = new QLineEdit(QObject::tr("QLineEdit"));
    edit->setPlaceholderText(QObject::tr("占位提示"));
    fitRibbonRow(edit);
    group->addWidget(edit);

    auto* spin = new QSpinBox;
    spin->setRange(0, 100);
    spin->setValue(42);
    fitRibbonRow(spin);
    group->addWidget(spin);

    auto* combo = new QComboBox;
    combo->addItems({ QObject::tr("QComboBox"), QObject::tr("第二项"), QObject::tr("第三项（长一点）") });
    fitRibbonRow(combo);
    group->addWidget(combo);
}

void addSelectionGroup(QmRibbonPage* page)
{
    auto* group = page->addGroup(QObject::tr("Selection"));

    auto* checked = new QCheckBox(QObject::tr("Checked"));
    checked->setChecked(true);
    fitRibbonRow(checked);
    group->addWidget(checked);

    auto* unchecked = new QCheckBox(QObject::tr("Check Box"));
    fitRibbonRow(unchecked);
    group->addWidget(unchecked);

    auto* disabled = new QCheckBox(QObject::tr("Disabled"));
    disabled->setEnabled(false);
    fitRibbonRow(disabled);
    group->addWidget(disabled);

    // 第 4 个控件起自动另起一列（Group 每列最多三行）。
    auto* radio_a = new QRadioButton(QObject::tr("Radio A"));
    radio_a->setChecked(true);
    fitRibbonRow(radio_a);
    group->addWidget(radio_a);

    auto* radio_b = new QRadioButton(QObject::tr("Radio B"));
    fitRibbonRow(radio_b);
    group->addWidget(radio_b);

    // 两个单选按钮落在不同的列里（列是 Group 内部各自独立的 QWidget），
    // 靠父子关系自动互斥会失效，所以显式放进一个 QButtonGroup。
    auto* radios = new QButtonGroup(group);
    radios->addButton(radio_a);
    radios->addButton(radio_b);
}

/// 指示类：`QLabel` / `QProgressBar` / `QScrollBar`（细把手、无箭头按钮）。
void addIndicatorGroup(QmRibbonPage* page)
{
    auto* group = page->addGroup(QObject::tr("Indicator"));

    auto* label = new QLabel(QObject::tr("QLabel"));
    fitRibbonRow(label);
    group->addWidget(label);

    auto* progress = new QProgressBar;
    progress->setRange(0, 100);
    progress->setValue(60);
    progress->setFixedHeight(14);
    group->addWidget(progress);

    auto* horizontal = new QScrollBar(Qt::Horizontal);
    horizontal->setRange(0, 100);
    horizontal->setValue(35);
    horizontal->setFixedHeight(12);
    group->addWidget(horizontal);

    // 另起一列放竖直滚动条（高度正好吃满 Group 内容区）。
    auto* vertical = new QScrollBar(Qt::Vertical);
    vertical->setRange(0, 100);
    vertical->setValue(40);
    vertical->setFixedHeight(QmRibbonMetrics::group_content_height);
    group->addWidget(vertical);
}

/// `QToolButton`：通用样式只改底色，所以尺寸仍由样式算，这里压到一行高。
void addToolButtonGroup(QmRibbonPage* page)
{
    auto* group = page->addGroup(QObject::tr("Tool Button"));

    const auto add_tool = [group](const QString& text, bool checkable, QMenu* menu) {
        auto* button = new QToolButton;
        button->setText(text);
        button->setToolButtonStyle(Qt::ToolButtonTextOnly);
        button->setCheckable(checkable);
        button->setCursor(Qt::PointingHandCursor);
        // 平铺的工具按钮：基础态由样式表直接画（透明），悬停 / 选中才上底色。
        // 不设 autoRaise 的话，基础态会走原生样式的按钮背景，外观就不受主题控制了。
        button->setAutoRaise(true);

        if (menu != nullptr) {
            button->setMenu(menu);
            button->setPopupMode(QToolButton::InstantPopup);
        }

        fitRibbonRow(button);
        group->addWidget(button);
        return button;
    };

    add_tool(QObject::tr("Tool Button"), false, nullptr);
    add_tool(QObject::tr("Checkable"), true, nullptr)->setChecked(true);
    add_tool(QObject::tr("Menu"), false, makeSampleMenu(group));
}

/// 放不进 Ribbon 的大件：用对话框展示（同时演示业务窗口如何接入主题）。
void showWidgetGallery(QWidget* parent)
{
    auto* dialog = new QDialog(parent, Qt::Window);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(QObject::tr("Widget Gallery - qmribbon"));
    dialog->resize(760, 560);

    auto* layout = new QVBoxLayout(dialog);
    layout->setContentsMargins(16, 16, 16, 16);
    layout->setSpacing(12);

    // ---------- QGroupBox + 单行控件 ----------
    auto* form_box = new QGroupBox(QObject::tr("QGroupBox：单行控件"));
    auto* form = new QFormLayout(form_box);
    form->setContentsMargins(12, 16, 12, 12);
    form->setSpacing(8);

    form->addRow(QObject::tr("QLineEdit"), new QLineEdit(QObject::tr("单行输入")));

    auto* spin = new QSpinBox;
    spin->setRange(0, 999);
    spin->setValue(128);
    form->addRow(QObject::tr("QSpinBox"), spin);

    auto* double_spin = new QDoubleSpinBox;
    double_spin->setRange(0.0, 100.0);
    double_spin->setValue(12.5);
    double_spin->setSuffix(QStringLiteral(" pt"));
    form->addRow(QObject::tr("QDoubleSpinBox"), double_spin);

    auto* combo = new QComboBox;
    combo->addItems({ QObject::tr("QComboBox（普通下拉）"), QObject::tr("第二项"), QObject::tr("第三项") });
    form->addRow(QObject::tr("QComboBox"), combo);

    auto* selections = new QWidget;
    auto* selections_layout = new QHBoxLayout(selections);
    selections_layout->setContentsMargins(0, 0, 0, 0);
    selections_layout->setSpacing(12);

    auto* check_box = new QCheckBox(QObject::tr("QCheckBox"));
    check_box->setChecked(true);
    auto* radio = new QRadioButton(QObject::tr("QRadioButton"));
    radio->setChecked(true);
    selections_layout->addWidget(check_box);
    selections_layout->addWidget(radio);
    selections_layout->addStretch(1);
    form->addRow(QObject::tr("选择"), selections);

    layout->addWidget(form_box);

    // ---------- 选项卡里的大件 ----------
    auto* tabs = new QTabWidget;

    auto* table = new QTableWidget(4, 3);
    table->setHorizontalHeaderLabels({ QObject::tr("列 A"), QObject::tr("列 B"), QObject::tr("列 C") });
    for (int row = 0; row < table->rowCount(); ++row) {
        for (int column = 0; column < table->columnCount(); ++column) {
            table->setItem(row, column,
                           new QTableWidgetItem(QObject::tr("R%1 C%2").arg(row + 1).arg(column + 1)));
        }
    }
    table->setAlternatingRowColors(true);
    table->verticalHeader()->setVisible(false);

    auto* tree = new QTreeWidget;
    tree->setHeaderLabels({ QObject::tr("节点"), QObject::tr("说明") });
    for (int i = 1; i <= 3; ++i) {
        auto* top = new QTreeWidgetItem(tree, { QObject::tr("节点 %1").arg(i), QObject::tr("QTreeView / QHeaderView") });
        for (int j = 1; j <= 2; ++j) {
            new QTreeWidgetItem(top, { QObject::tr("子节点 %1.%2").arg(i).arg(j), QString() });
        }
    }
    tree->expandAll();

    auto* list = new QListWidget;
    list->addItems({ QObject::tr("QListView 第 1 项"), QObject::tr("第 2 项"), QObject::tr("第 3 项") });
    list->setAlternatingRowColors(true);

    // QSplitter 的分隔件也是主题色（QSplitter::handle）。
    auto* splitter = new QSplitter(Qt::Horizontal);
    splitter->addWidget(table);
    splitter->addWidget(tree);
    splitter->addWidget(list);
    splitter->setSizes({ 320, 240, 200 });
    tabs->addTab(splitter, QObject::tr("表格 / 树 / 列表"));

    auto* text_page = new QWidget;
    auto* text_layout = new QVBoxLayout(text_page);
    text_layout->setContentsMargins(8, 8, 8, 8);
    text_layout->setSpacing(8);

    auto* editor = new QTextEdit;
    editor->setPlainText(QObject::tr("QTextEdit / QPlainTextEdit 是「正文区域」：不画边框，底色用 contentBackground；\n"
                                     "右侧滚动条是主题里的细把手样式（无箭头按钮）。"));
    text_layout->addWidget(editor, 1);

    auto* progress = new QProgressBar;
    progress->setRange(0, 100);
    progress->setValue(45);
    text_layout->addWidget(progress);

    tabs->addTab(text_page, QObject::tr("文本 / 进度"));
    layout->addWidget(tabs, 1);

    // ---------- 底部：菜单 + 标准按钮盒 ----------
    auto* bottom = new QHBoxLayout;
    bottom->setSpacing(8);

    auto* menu_button = new QPushButton(QObject::tr("QMenu"));
    menu_button->setMenu(makeSampleMenu(menu_button));
    bottom->addWidget(menu_button);
    bottom->addStretch(1);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QObject::connect(buttons, &QDialogButtonBox::accepted, dialog, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, dialog, &QDialog::reject);
    bottom->addWidget(buttons);

    layout->addLayout(bottom);

    // 业务自己的窗口只要登记一次就能跟主题走（QSS + 调色板），
    // 之后切换 Light / Dark 时这个对话框会实时刷新。
    QmRibbonThemeMgr::instance().apply(dialog);

    dialog->show();
}

} // namespace

void populateGalleryPage(QmRibbonPage* page)
{
    if (page == nullptr) {
        return;
    }

    addPushButtonGroup(page);
    addInputGroup(page);
    addSelectionGroup(page);
    addIndicatorGroup(page);
    addToolButtonGroup(page);

    auto* showcase = page->addGroup(QObject::tr("Showcase"));
    auto* action = new QAction(makeLetterIcon(QStringLiteral("G"), QColor(0x8a, 0x4f, 0xbd)),
                               QObject::tr("Widget Gallery"), page);
    action->setToolTip(QObject::tr("用对话框展示放不进 Ribbon 的大件控件"));
    showcase->addLargeAction(action);

    QObject::connect(action, &QAction::triggered, page, [page]() { showWidgetGallery(page->window()); });
}
