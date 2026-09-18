#include "backstage.h"

#include "exampleicons.h"
#include "qmribbonthememgr.h"
#include "qmribbonwindow.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QList>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>

namespace {

// ---------------------------------------------------------------------------
// 通用小部件
// ---------------------------------------------------------------------------

QLabel* makeLabel(const QString& text, const char* object_name)
{
    auto* label = new QLabel(text);
    label->setObjectName(QString::fromLatin1(object_name));
    return label;
}

/// 生成一张圆角缩略图（图标占位）。
QPixmap makeThumbnail(const QString& letter, const QColor& color, int size = 36)
{
    constexpr qreal ratio = 2.0;

    QPixmap pixmap(QSize(size, size) * static_cast<int>(ratio));
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawRoundedRect(QRectF(0.5, 0.5, size - 1.0, size - 1.0), size * 0.24, size * 0.24);

    QFont font = painter.font();
    font.setPixelSize(static_cast<int>(size * 0.48));
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(QRectF(0, 0, size, size), Qt::AlignCenter, letter);

    return pixmap;
}

/// 圆形头像。
QPixmap makeAvatar(const QString& initials, const QColor& color, int size = 48)
{
    constexpr qreal ratio = 2.0;

    QPixmap pixmap(QSize(size, size) * static_cast<int>(ratio));
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setPen(Qt::NoPen);
    painter.setBrush(color);
    painter.drawEllipse(QRectF(0, 0, size, size));

    QFont font = painter.font();
    font.setPixelSize(static_cast<int>(size * 0.36));
    font.setBold(true);
    painter.setFont(font);
    painter.setPen(Qt::white);
    painter.drawText(QRectF(0, 0, size, size), Qt::AlignCenter, initials);

    return pixmap;
}

/// 页面通用头部：大标题 + 副标题。
QWidget* makePageHeader(const QString& title, const QString& subtitle)
{
    auto* header = new QWidget;
    auto* layout = new QVBoxLayout(header);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);
    layout->addWidget(makeLabel(title, "BackstagePageTitle"));
    layout->addWidget(makeLabel(subtitle, "BackstagePageSubtitle"));
    return header;
}

/// 建好页面外框（含头部），返回布局供调用方继续追加内容。
QVBoxLayout* makePageLayout(QWidget* page, const QString& title, const QString& subtitle)
{
    auto* layout = new QVBoxLayout(page);
    layout->setContentsMargins(36, 24, 36, 24);
    layout->setSpacing(14);
    layout->addWidget(makePageHeader(title, subtitle));
    return layout;
}

/// 卡片：标题 + 说明。
QFrame* makeCard(const QString& title, const QString& text, int min_height = 78)
{
    auto* card = new QFrame;
    card->setObjectName(QStringLiteral("BackstageCard"));
    card->setMinimumHeight(min_height);

    auto* layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(4);
    layout->addWidget(makeLabel(title, "BackstageCardTitle"));
    layout->addWidget(makeLabel(text, "BackstageCardText"));
    layout->addStretch(1);

    return card;
}

/// 带缩略图的一行（最近使用的文档等）。
QFrame* makeRow(const QString& letter, const QColor& color, const QString& title, const QString& text)
{
    auto* row = new QFrame;
    row->setObjectName(QStringLiteral("BackstageRow"));
    row->setCursor(Qt::PointingHandCursor);

    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(12);

    auto* thumb = new QLabel;
    thumb->setPixmap(makeThumbnail(letter, color));
    thumb->setFixedSize(36, 36);
    layout->addWidget(thumb);

    auto* texts = new QVBoxLayout;
    texts->setContentsMargins(0, 0, 0, 0);
    texts->setSpacing(2);
    texts->addWidget(makeLabel(title, "BackstageRowTitle"));
    texts->addWidget(makeLabel(text, "BackstageRowText"));
    layout->addLayout(texts, 1);

    return row;
}

/// 「左标题 + 右侧控件/取值」的设置行。
QFrame* makeSettingRow(const QString& title, QWidget* control)
{
    auto* row = new QFrame;
    row->setObjectName(QStringLiteral("BackstageRow"));

    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(8, 6, 8, 6);
    layout->setSpacing(12);
    layout->addWidget(makeLabel(title, "BackstageRowTitle"));
    layout->addStretch(1);

    if (control != nullptr) {
        layout->addWidget(control);
    }

    return row;
}

/// 模板卡片：顶部色块 + 标题 + 说明。
QFrame* makeTile(const QString& title, const QString& text, const QColor& color)
{
    auto* tile = new QFrame;
    tile->setObjectName(QStringLiteral("BackstageTile"));
    tile->setFixedSize(150, 136);
    tile->setCursor(Qt::PointingHandCursor);

    auto* layout = new QVBoxLayout(tile);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* strip = new QFrame(tile);
    strip->setFixedHeight(72);
    strip->setAutoFillBackground(true);
    QPalette palette = strip->palette();
    palette.setColor(QPalette::Window, color);
    strip->setPalette(palette);
    layout->addWidget(strip);

    auto* texts = new QWidget(tile);
    auto* texts_layout = new QVBoxLayout(texts);
    texts_layout->setContentsMargins(10, 8, 10, 8);
    texts_layout->setSpacing(2);
    texts_layout->addWidget(makeLabel(title, "BackstageTileCaption"));
    texts_layout->addWidget(makeLabel(text, "BackstageTileText"));
    texts_layout->addStretch(1);
    layout->addWidget(texts, 1);

    return tile;
}

/// 一排模板卡片。
QWidget* makeTileRow(const QList<QFrame*>& tiles)
{
    auto* row = new QWidget;
    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    for (QFrame* tile : tiles) {
        layout->addWidget(tile);
    }
    layout->addStretch(1);

    return row;
}

QComboBox* makeCombo(const QStringList& items, const QString& current)
{
    auto* combo = new QComboBox;
    combo->addItems(items);
    combo->setCurrentText(current);
    return combo;
}

// ---------------------------------------------------------------------------
// 各页面
// ---------------------------------------------------------------------------

QWidget* makeHomePage()
{
    auto* page = new QWidget;
    auto* layout = makePageLayout(page, QObject::tr("Home"), QObject::tr("最近使用的文档和常用操作"));

    layout->addWidget(makeRow(QStringLiteral("Q"), QColor(0x0f, 0x6c, 0xbd), QObject::tr("季度报告.docx"),
                              QObject::tr("D:\\文档\\季度报告.docx  ·  今天 14:20")));
    layout->addWidget(makeRow(QStringLiteral("R"), QColor(0x10, 0x7c, 0x41), QObject::tr("需求评审记录.docx"),
                              QObject::tr("D:\\文档\\需求评审记录.docx  ·  昨天 09:05")));
    layout->addWidget(makeRow(QStringLiteral("P"), QColor(0xc5, 0x7a, 0x11), QObject::tr("产品路线图.pptx"),
                              QObject::tr("D:\\演示\\产品路线图.pptx  ·  3 天前")));
    layout->addWidget(makeRow(QStringLiteral("T"), QColor(0x8a, 0x4f, 0xbd), QObject::tr("排期表.xlsx"),
                              QObject::tr("D:\\表格\\排期表.xlsx  ·  上周")));

    layout->addSpacing(6);
    layout->addWidget(makeCard(QObject::tr("打开其他文档"),
                               QObject::tr("从这台电脑、OneDrive 或 SharePoint 中选择文件，也可以新建空白文档。"), 64));
    layout->addStretch(1);

    return page;
}

QWidget* makeNewPage()
{
    auto* page = new QWidget;
    auto* layout = makePageLayout(page, QObject::tr("New"), QObject::tr("选择模板或从空白文档开始"));

    layout->addWidget(
        makeTileRow({ makeTile(QObject::tr("空白文档"), QObject::tr("从零开始"), QColor(0xf3, 0xf3, 0xf3)),
                      makeTile(QObject::tr("报告"), QObject::tr("结构化排版"), QColor(0x0f, 0x6c, 0xbd)),
                      makeTile(QObject::tr("简历"), QObject::tr("单页模板"), QColor(0x10, 0x7c, 0x41)),
                      makeTile(QObject::tr("日程"), QObject::tr("周计划"), QColor(0xc5, 0x7a, 0x11)),
                      makeTile(QObject::tr("演示稿"), QObject::tr("宽屏 16:9"), QColor(0x8a, 0x4f, 0xbd)) }));

    layout->addSpacing(6);
    layout->addWidget(makeCard(QObject::tr("搜索联机模板"),
                               QObject::tr("从 Office 模板库中搜索更多样式，例如简历、日历、报表。"), 64));
    layout->addStretch(1);

    return page;
}

QWidget* makeOpenPage()
{
    auto* page = new QWidget;
    auto* layout = makePageLayout(page, QObject::tr("Open"), QObject::tr("从最近位置或其它位置打开文档"));

    layout->addWidget(makeRow(QStringLiteral("D"), QColor(0x0f, 0x6c, 0xbd), QObject::tr("文档"),
                              QObject::tr("D:\\文档  ·  12 个文件")));
    layout->addWidget(makeRow(QStringLiteral("P"), QColor(0x10, 0x7c, 0x41), QObject::tr("桌面"),
                              QObject::tr("C:\\Users\\Public\\Desktop  ·  3 个文件")));

    layout->addSpacing(6);
    auto* columns = new QWidget;
    auto* columns_layout = new QHBoxLayout(columns);
    columns_layout->setContentsMargins(0, 0, 0, 0);
    columns_layout->setSpacing(12);
    columns_layout->addWidget(makeCard(QObject::tr("这台电脑"), QObject::tr("浏览本地磁盘与网络位置。")));
    columns_layout->addWidget(makeCard(QObject::tr("OneDrive"), QObject::tr("打开云端已同步的文档。")));
    layout->addWidget(columns);

    layout->addStretch(1);
    return page;
}

QWidget* makeInfoPage()
{
    auto* page = new QWidget;
    auto* layout = makePageLayout(page, QObject::tr("Info"), QObject::tr("文档属性、保护与版本"));

    auto* columns = new QWidget;
    auto* columns_layout = new QHBoxLayout(columns);
    columns_layout->setContentsMargins(0, 0, 0, 0);
    columns_layout->setSpacing(16);

    // 左：属性
    auto* properties = new QWidget;
    auto* properties_layout = new QVBoxLayout(properties);
    properties_layout->setContentsMargins(0, 0, 0, 0);
    properties_layout->setSpacing(6);
    properties_layout->addWidget(makeLabel(QObject::tr("文档属性"), "BackstageCardTitle"));
    properties_layout->addWidget(
        makeSettingRow(QObject::tr("标题"), makeLabel(QObject::tr("季度报告"), "BackstageRowText")));
    properties_layout->addWidget(
        makeSettingRow(QObject::tr("作者"), makeLabel(QObject::tr("qmribbon"), "BackstageRowText")));
    properties_layout->addWidget(
        makeSettingRow(QObject::tr("页数"), makeLabel(QObject::tr("1 页"), "BackstageRowText")));
    properties_layout->addWidget(
        makeSettingRow(QObject::tr("修改时间"), makeLabel(QObject::tr("今天 14:20"), "BackstageRowText")));
    properties_layout->addStretch(1);
    columns_layout->addWidget(properties, 1);

    // 右：保护与版本
    auto* actions = new QWidget;
    auto* actions_layout = new QVBoxLayout(actions);
    actions_layout->setContentsMargins(0, 0, 0, 0);
    actions_layout->setSpacing(12);
    actions_layout->addWidget(
        makeCard(QObject::tr("保护文档"), QObject::tr("控制他人可对此文档进行的更改，例如标记为最终版本或设置密码。")));
    actions_layout->addWidget(makeCard(QObject::tr("版本"), QObject::tr("查看或恢复此文档的早期草稿（演示数据）。")));
    actions_layout->addStretch(1);
    columns_layout->addWidget(actions, 1);

    layout->addWidget(columns, 1);
    return page;
}

QWidget* makeSaveAsPage()
{
    auto* page = new QWidget;
    auto* layout = makePageLayout(page, QObject::tr("Save As"), QObject::tr("选择保存位置"));

    auto* columns = new QWidget;
    auto* columns_layout = new QHBoxLayout(columns);
    columns_layout->setContentsMargins(0, 0, 0, 0);
    columns_layout->setSpacing(12);
    columns_layout->addWidget(makeCard(QObject::tr("这台电脑"), QObject::tr("保存到本地磁盘，可自定义文件名与格式。")));
    columns_layout->addWidget(makeCard(QObject::tr("OneDrive"), QObject::tr("保存到云端，自动同步与共享。")));
    layout->addWidget(columns);

    layout->addWidget(
        makeCard(QObject::tr("其它格式"), QObject::tr("PDF、XPS、纯文本、网页等格式请在 Export 页面中创建。"), 64));
    layout->addStretch(1);
    return page;
}

QWidget* makePrintPage()
{
    auto* page = new QWidget;
    auto* layout = makePageLayout(page, QObject::tr("Print"), QObject::tr("打印设置与预览"));

    auto* columns = new QWidget;
    auto* columns_layout = new QHBoxLayout(columns);
    columns_layout->setContentsMargins(0, 0, 0, 0);
    columns_layout->setSpacing(24);

    // 左：打印按钮 + 设置
    auto* settings = new QWidget;
    auto* settings_layout = new QVBoxLayout(settings);
    settings_layout->setContentsMargins(0, 0, 0, 0);
    settings_layout->setSpacing(8);

    auto* print_button = new QPushButton(QObject::tr("打印"), settings);
    print_button->setObjectName(QStringLiteral("BackstagePrimaryButton"));
    print_button->setCursor(Qt::PointingHandCursor);
    print_button->setFixedHeight(32);
    settings_layout->addWidget(print_button, 0, Qt::AlignLeft);

    settings_layout->addSpacing(4);
    settings_layout->addWidget(
        makeSettingRow(QObject::tr("份数"), makeCombo({ QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3") },
                                                      QStringLiteral("1"))));
    settings_layout->addWidget(makeSettingRow(
        QObject::tr("打印机"), makeCombo({ QObject::tr("Microsoft Print to PDF"), QObject::tr("默认打印机") },
                                         QObject::tr("Microsoft Print to PDF"))));
    settings_layout->addWidget(
        makeSettingRow(QObject::tr("单面 / 双面"),
                       makeCombo({ QObject::tr("单面打印"), QObject::tr("双面打印") }, QObject::tr("单面打印"))));
    settings_layout->addWidget(makeSettingRow(
        QObject::tr("纸张大小"), makeCombo({ QStringLiteral("A4"), QStringLiteral("Letter") }, QStringLiteral("A4"))));
    settings_layout->addStretch(1);
    columns_layout->addWidget(settings, 1);

    // 右：纸张预览
    auto* preview = new QFrame;
    preview->setObjectName(QStringLiteral("BackstagePreview"));
    preview->setFixedSize(260, 340);
    columns_layout->addWidget(preview, 0, Qt::AlignTop);

    layout->addWidget(columns, 1);
    return page;
}

QWidget* makeSharePage()
{
    auto* page = new QWidget;
    auto* layout = makePageLayout(page, QObject::tr("Share"), QObject::tr("与他人共享此文档"));

    layout->addWidget(makeCard(QObject::tr("邀请他人"), QObject::tr("输入邮箱地址，设置为可编辑或只读后发送链接。")));
    layout->addWidget(makeCard(QObject::tr("发送副本"), QObject::tr("以附件形式发送副本，或发送 PDF 版本。")));
    layout->addWidget(makeCard(QObject::tr("联机展示"), QObject::tr("在浏览器中演示此文档（演示项）。")));
    layout->addStretch(1);
    return page;
}

QWidget* makeExportPage()
{
    auto* page = new QWidget;
    auto* layout = makePageLayout(page, QObject::tr("Export"), QObject::tr("导出为其它格式"));

    layout->addWidget(
        makeCard(QObject::tr("创建 PDF / XPS 文档"), QObject::tr("保留版式的只读副本，适合分发与归档。")));
    layout->addWidget(makeCard(QObject::tr("更改文件类型"), QObject::tr("导出为纯文本、网页或旧版格式。")));
    layout->addStretch(1);
    return page;
}

/// Light / Dark / 跟随系统 三选一；点击立即切换，QmRibbonThemeMgr 负责实时应用。
/// （View 页里还有一个等价的 Ribbon Group「Theme」，两边通过 modeChanged 保持同步。）
QWidget* makeThemeSelector()
{
    auto* row = new QWidget;
    auto* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(12);

    auto& manager = QmRibbonThemeMgr::instance();
    auto* group = new QButtonGroup(row);
    group->setExclusive(true);

    const auto add_mode = [&](QmRibbonThemeMgr::Mode mode, const QString& title, const QIcon& icon) {
        auto* button = new QPushButton(title, row);
        button->setObjectName(QStringLiteral("BackstageThemeButton"));
        button->setCheckable(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setFocusPolicy(Qt::NoFocus);
        button->setIcon(icon);
        button->setIconSize(QSize(18, 18));
        button->setFixedHeight(38);
        button->setMinimumWidth(136);
        button->setChecked(manager.mode() == mode);

        // 按钮 id 用 mode + 1：QButtonGroup 的 id 需要是正数。
        group->addButton(button, static_cast<int>(mode) + 1);
        layout->addWidget(button);

        // 单例活到进程结束，直接在里面取用即可，不需要捕获引用。
        QObject::connect(button, &QPushButton::clicked, &manager,
                         [mode]() { QmRibbonThemeMgr::instance().setMode(mode); });
    };

    add_mode(QmRibbonThemeMgr::Mode::Light, QObject::tr("Light"), makeSwatchIcon(QColor(0xf3, 0xf3, 0xf3), false));
    add_mode(QmRibbonThemeMgr::Mode::Dark, QObject::tr("Dark"), makeSwatchIcon(QColor(0x21, 0x21, 0x21), false));
    add_mode(QmRibbonThemeMgr::Mode::System, QObject::tr("跟随系统"), makeSwatchIcon(Qt::white, true));

    // 模式在别处被改（例如代码里调 setMode）时同步选中态。
    QObject::connect(&manager, &QmRibbonThemeMgr::modeChanged, row, [group](QmRibbonThemeMgr::Mode mode) {
        if (QAbstractButton* button = group->button(static_cast<int>(mode) + 1)) {
            button->setChecked(true);
        }
    });

    layout->addStretch(1);
    return row;
}

QWidget* makeAccountPage()
{
    auto* page = new QWidget;
    auto* layout = makePageLayout(page, QObject::tr("Account"), QObject::tr("账户与主题"));

    auto* user = new QFrame;
    user->setObjectName(QStringLiteral("BackstageRow"));
    auto* user_layout = new QHBoxLayout(user);
    user_layout->setContentsMargins(8, 8, 8, 8);
    user_layout->setSpacing(14);

    auto* avatar = new QLabel;
    avatar->setPixmap(makeAvatar(QStringLiteral("QM"), QColor(0x0f, 0x6c, 0xbd)));
    avatar->setFixedSize(48, 48);
    user_layout->addWidget(avatar);

    auto* user_texts = new QVBoxLayout;
    user_texts->setContentsMargins(0, 0, 0, 0);
    user_texts->setSpacing(2);
    user_texts->addWidget(makeLabel(QObject::tr("qmribbon 用户"), "BackstageRowTitle"));
    user_texts->addWidget(makeLabel(QObject::tr("user@example.com  ·  已登录"), "BackstageRowText"));
    user_layout->addLayout(user_texts, 1);
    layout->addWidget(user);

    layout->addSpacing(6);
    layout->addWidget(makeLabel(QObject::tr("主题"), "BackstageCardTitle"));
    layout->addWidget(makeThemeSelector());
    layout->addWidget(makeLabel(QObject::tr("选择后立即生效；「跟随系统」会随 Windows 的浅色 / 深色设置实时切换。"),
                                "BackstageRowText"));

    layout->addStretch(1);
    return page;
}

QWidget* makeOptionsPage()
{
    auto* page = new QWidget;
    auto* layout = makePageLayout(page, QObject::tr("Options"), QObject::tr("应用全局设置"));

    auto* card = new QFrame;
    card->setObjectName(QStringLiteral("BackstageCard"));
    auto* card_layout = new QVBoxLayout(card);
    card_layout->setContentsMargins(14, 12, 14, 12);
    card_layout->setSpacing(10);
    card_layout->addWidget(makeLabel(QObject::tr("常规"), "BackstageCardTitle"));

    for (const QString& text :
         { QObject::tr("显示迷你工具栏"), QObject::tr("启用实时预览"), QObject::tr("在屏幕提示中显示功能说明") }) {
        auto* box = new QCheckBox(text, card);
        box->setChecked(true);
        box->setCursor(Qt::PointingHandCursor);
        card_layout->addWidget(box);
    }

    card_layout->addStretch(1);
    layout->addWidget(card);
    layout->addStretch(1);
    return page;
}

} // namespace

QWidget* makeBackstage(QmRibbonWindow* window)
{
    auto* backstage = new QWidget(window);
    backstage->setObjectName(QStringLiteral("BackstageView"));
    backstage->setAttribute(Qt::WA_StyledBackground, true);

    auto* root = new QHBoxLayout(backstage);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    // ---------- 左侧导航 ----------
    auto* nav = new QWidget(backstage);
    nav->setObjectName(QStringLiteral("BackstageNav"));
    nav->setAttribute(Qt::WA_StyledBackground, true);
    nav->setFixedWidth(216);

    auto* nav_layout = new QVBoxLayout(nav);
    nav_layout->setContentsMargins(10, 10, 10, 12);
    nav_layout->setSpacing(2);

    auto* back_button = new QPushButton(QStringLiteral("\u2190"), nav); // ←
    back_button->setObjectName(QStringLiteral("BackstageBackButton"));
    back_button->setToolTip(QObject::tr("返回主界面（Esc）"));
    back_button->setFixedSize(36, 36);
    back_button->setCursor(Qt::PointingHandCursor);
    back_button->setFocusPolicy(Qt::NoFocus);
    nav_layout->addWidget(back_button, 0, Qt::AlignLeft);
    nav_layout->addSpacing(6);

    // ---------- 右侧页面栈 ----------
    auto* content = new QWidget(backstage);
    content->setObjectName(QStringLiteral("BackstageContent"));
    content->setAttribute(Qt::WA_StyledBackground, true);

    auto* content_layout = new QVBoxLayout(content);
    content_layout->setContentsMargins(0, 0, 0, 0);
    content_layout->setSpacing(0);

    auto* stack = new QStackedWidget(content);
    content_layout->addWidget(stack);

    auto* group = new QButtonGroup(backstage);
    group->setExclusive(true);

    QPushButton* first_button = nullptr;
    const auto add_page = [&](const QString& title, QWidget* page) {
        auto* button = new QPushButton(title, nav);
        button->setObjectName(QStringLiteral("BackstageNavItem"));
        button->setCheckable(true);
        button->setAutoDefault(false);
        button->setCursor(Qt::PointingHandCursor);
        button->setFocusPolicy(Qt::NoFocus);
        button->setFixedHeight(34);

        group->addButton(button);
        nav_layout->addWidget(button);
        stack->addWidget(page);

        QObject::connect(button, &QPushButton::clicked, stack, [stack, page]() { stack->setCurrentWidget(page); });

        if (first_button == nullptr) {
            first_button = button;
        }
        return button;
    };

    const auto add_separator = [&]() {
        auto* line = new QFrame(nav);
        line->setObjectName(QStringLiteral("BackstageNavSeparator"));
        line->setFixedHeight(1);
        nav_layout->addSpacing(6);
        nav_layout->addWidget(line);
        nav_layout->addSpacing(6);
    };

    add_page(QObject::tr("Home"), makeHomePage());
    add_page(QObject::tr("New"), makeNewPage());
    add_page(QObject::tr("Open"), makeOpenPage());
    add_separator();

    add_page(QObject::tr("Info"), makeInfoPage());
    add_separator();

    add_page(QObject::tr("Save As"), makeSaveAsPage());
    add_page(QObject::tr("Print"), makePrintPage());
    add_page(QObject::tr("Share"), makeSharePage());
    add_page(QObject::tr("Export"), makeExportPage());
    add_separator();

    // 命令项：不是页面，点了直接执行。
    auto* close_button = new QPushButton(QObject::tr("Close"), nav);
    close_button->setObjectName(QStringLiteral("BackstageNavCommand"));
    close_button->setCursor(Qt::PointingHandCursor);
    close_button->setFocusPolicy(Qt::NoFocus);
    close_button->setFixedHeight(34);
    close_button->setAutoDefault(false);
    nav_layout->addWidget(close_button);
    QObject::connect(close_button, &QPushButton::clicked, window, &QWidget::close);

    add_separator();
    add_page(QObject::tr("Account"), makeAccountPage());
    add_page(QObject::tr("Options"), makeOptionsPage());

    nav_layout->addStretch(1);

    if (first_button != nullptr) {
        first_button->setChecked(true);
        stack->setCurrentIndex(0);
    }

    QObject::connect(back_button, &QPushButton::clicked, window,
                     [window]() { window->setViewMode(QmRibbonWindow::ViewMode::Main); });

    root->addWidget(nav);
    root->addWidget(content, 1);

    return backstage;
}
