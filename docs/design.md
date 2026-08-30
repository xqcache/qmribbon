# Ribbon 界面开发需求文档

## 1. 项目目标

基于 Qt 6.9+ / Qt Widgets 实现一套 Ribbon UI 框架，整体布局、组件语义和交互参考 Microsoft Word / Microsoft Office，并兼顾 Fluent 2 Design 风格。

技术路线：

```text
Qt 6.9+
Qt Widgets
QWidget 顶层窗口
Qt::FramelessWindowHint
自定义 TitleBar
Qt Advanced Docking System 作为 MainView
Backstage View 模拟 Microsoft Word / Office
视觉风格偏 Fluent 2
```

整体需要支持：

- 自定义 TitleBar
- Ribbon Tab
- Ribbon Page
- Ribbon Group
- Ribbon Button
- Quick Access Toolbar
- Contextual Tab
- Backstage View
- Main View
- 窗口控制按钮
- 系统拖动 / Resize
- Windows Aero Snap
- Windows 11 Snap Layout

---

## 2. 总体布局结构

```text
QmRibbonWindow
│
├── QmRibbon
│   │
│   ├── QmRibbonTitleBar
│   │   ├── QmRibbonQuickAccessBar
│   │   ├── QmRibbonWindowTitle
│   │   ├── 自定义内容区域
│   │   └── QmWindowButtonGroup
│   │
│   ├── QmRibbonTabBar
│   │   ├── QmRibbonFileButton
│   │   ├── QmRibbonTab
│   │   ├── QmRibbonTab
│   │   ├── QmRibbonContextualTab
│   │   └── RightActions
│   │
│   └── QmRibbonPageStack
│       ├── QmRibbonPage
│       │   ├── QmRibbonGroup
│       │   ├── QmRibbonGroup
│       │   └── ...
│       └── ...
│
└── QStackedWidget
    ├── MainView
    │   └── ads::CDockManager
    │
    └── QmRibbonBackstageView
        ├── QmRibbonBackstageNavigation
        └── QStackedWidget
            ├── HomePage
            ├── NewPage
            ├── OpenPage
            ├── InfoPage
            ├── PrintPage
            └── ...
```

---

## 3. 命名规范

统一使用 `QmRibbonXXX` 表示 Ribbon 框架组件。

### 3.1 窗口相关

```cpp
QmRibbonWindow
QmRibbonTitleBar
QmWindowButtonGroup
QmWindowButton
```

### 3.2 Ribbon 核心组件

```cpp
QmRibbon
QmRibbonTabBar
QmRibbonTab
QmRibbonPage
QmRibbonGroup
```

### 3.3 Ribbon 命令控件

```cpp
QmRibbonButton
QmRibbonToolButton
QmRibbonSplitButton
QmRibbonDropDownButton
QmRibbonGallery
QmRibbonComboBox
QmRibbonLineEdit
QmRibbonCheckBox
QmRibbonSeparator
```

### 3.4 特殊 Ribbon 组件

```cpp
QmRibbonQuickAccessBar
QmRibbonContextualTab
QmRibbonContextualTabGroup

QmRibbonBackstageView
QmRibbonBackstageNavigation
QmRibbonBackstagePage
```

### 3.5 Private 类

```cpp
QmRibbonPrivate
QmRibbonWindowPrivate
QmRibbonPagePrivate
QmRibbonGroupPrivate
```

### 3.6 View Mode

```cpp
enum class QmRibbonViewMode {
    Main,
    Backstage
};
```

不建议枚举值使用 `MainView` / `BackstageView`，因为和 `ViewMode` 组合后语义重复。

---

# 4. QmRibbonWindow

## 4.1 作用

应用程序最外围的顶层窗口。

对应 Word 中：

> Microsoft Word 最外层窗口。

建议：

```cpp
class QmRibbonWindow : public QWidget
```

不依赖 `QMainWindow`。

原因：

- TitleBar 完全自定义
- Ribbon 完全自定义
- Docking 使用 Qt Advanced Docking System
- Central View 自定义
- 使用 `Qt::FramelessWindowHint`

## 4.2 布局

```text
QmRibbonWindow
│
├── QmRibbon
│
└── QStackedWidget
    ├── MainView
    └── BackstageView
```

示例成员：

```cpp
QmRibbon* ribbon_;
QStackedWidget* view_stack_;

QWidget* main_view_;
QmRibbonBackstageView* backstage_view_;
```

## 4.3 窗口行为

需要支持：

- 窗口拖动
- 窗口 Resize
- 最小化
- 最大化
- Restore
- 关闭
- Windows Aero Snap
- Windows 11 Snap Layout
- 多显示器
- DPI

推荐实现：

```text
TitleBar Drag
    → QWindow::startSystemMove()

Resize
    → QWindow::startSystemResize()

Win11 Snap Layout
    → WM_NCHITTEST + HTMAXBUTTON
```

---

# 5. QmRibbon

## 5.1 作用

整个 Ribbon 区域的顶层容器。

对应 Word 中：

> 从窗口顶部标题区开始，到 Clipboard / Font / Paragraph 等命令区域结束的整体 Ribbon 区域。

## 5.2 布局

```text
QmRibbon
├── QmRibbonTitleBar
├── QmRibbonTabBar
└── QmRibbonPageStack
```

建议接口：

```cpp
class QmRibbon : public QWidget
{
public:
    QmRibbonTitleBar* titleBar() const;
    QmRibbonTabBar* tabBar() const;

    QmRibbonPage* addPage(const QString& title);
};
```

---

# 6. QmRibbonTitleBar

## 6.1 Word 中的对应区域

Word 顶部类似：

```text
┌────────────────────────────────────────────────────────────┐
│ AutoSave  [Save][Undo][Redo]   Document1 - Word   Search  _ □ × │
└────────────────────────────────────────────────────────────┘
```

这整条区域对应 `QmRibbonTitleBar`。

## 6.2 推荐结构

```text
QmRibbonTitleBar
│
├── LeftArea
│   ├── AppIcon（可选）
│   └── QmRibbonQuickAccessBar
│
├── CenterArea
│   ├── QmRibbonWindowTitle
│   └── Search（可选）
│
└── RightArea
    ├── Account（可选）
    └── QmWindowButtonGroup
```

---

# 7. QmRibbonQuickAccessBar

## 7.1 Word 中的对应功能

Word 左上角常见：

```text
Save
Undo
Redo
```

这部分叫：

> Quick Access Toolbar（快速访问工具栏）

典型功能：

- Save
- Undo
- Redo
- Repeat
- Customize Quick Access Toolbar

## 7.2 作用

用于放置始终可见的高频命令。

即使当前 Ribbon Tab 是：

```text
Home
Insert
Layout
```

Save / Undo / Redo 依然保持可见。

## 7.3 推荐实现

```cpp
class QmRibbonQuickAccessBar : public QWidget
```

内部通常：

```cpp
QHBoxLayout
```

主要放置：

```cpp
QmRibbonToolButton
```

---

# 8. QmRibbonWindowTitle

## 8.1 Word 中的对应功能

例如：

```text
Document1 - Word
```

或者新版 Office：

```text
Document1
Saved to this PC
```

## 8.2 作用

可用于显示：

- 当前文档名
- 当前工程名
- 应用名称
- 文档修改状态
- 保存状态

第一版可直接使用：

```cpp
QLabel
```

后续再封装：

```cpp
QmRibbonWindowTitle
```

---

# 9. QmWindowButtonGroup

对应 Windows 右上角：

```text
─   □   ×
```

结构：

```text
QmWindowButtonGroup
├── Minimize
├── Maximize / Restore
└── Close
```

建议：

```cpp
enum class QmWindowButtonType {
    Minimize,
    Maximize,
    Restore,
    Close
};
```

典型行为：

```text
Minimize
    → showMinimized()

Maximize
    → showMaximized()

Restore
    → showNormal()

Close
    → close()
```

其中最大化按钮额外承担：

> Windows 11 Snap Layout 触发区域。

建议通过：

```text
WM_NCHITTEST
    → HTMAXBUTTON
```

实现。

---

# 10. QmRibbonTabBar

这是用户最常理解为“Ribbon 菜单栏”的部分。

Word 对应：

```text
File   Home   Insert   Draw   Design   Layout   References   Mailings   Review   View
```

对应组件：

```cpp
QmRibbonTabBar
```

建议不要直接将整个 TabBar 等同于 `QTabBar`，因为该区域除了普通 Tab 之外，还可能包含：

- File Button
- Contextual Tab
- Help
- Collapse Ribbon
- 右侧动作按钮

---

# 11. QmRibbonFileButton

## 11.1 Word 中的对应功能

Word：

```text
File
```

点击 File 后不会显示普通 Ribbon Page，而是进入：

```text
Backstage View
```

因此 File 不应该被设计成普通 `QmRibbonTab`。

建议：

```cpp
class QmRibbonFileButton : public QAbstractButton
```

行为：

```text
File clicked
    ↓
QmRibbonWindow::setViewMode(QmRibbonViewMode::Backstage)
```

---

# 12. QmRibbonTab

Word 中：

```text
Home
Insert
Draw
Design
Layout
References
Mailings
Review
View
```

每一个就是一个 `QmRibbonTab`。

作用：

```text
RibbonTab
    ↓
对应一个 RibbonPage
```

例如：

```text
Home
    ↓
HomePage

Insert
    ↓
InsertPage
```

建议接口：

```cpp
QmRibbonTab* addTab(
    const QString& title,
    QmRibbonPage* page);
```

---

# 13. QmRibbonPage

一个 Ribbon Tab 对应一个 Ribbon Page。

例如 Word 的 Home：

```text
Clipboard
Font
Paragraph
Styles
Editing
```

结构：

```text
Home Tab
    ↓

QmRibbonPage
├── Clipboard Group
├── Font Group
├── Paragraph Group
├── Styles Group
└── Editing Group
```

建议：

```cpp
class QmRibbonPage : public QWidget
```

内部通常使用：

```cpp
QHBoxLayout
```

用于横向排列多个 Ribbon Group。

---

# 14. QmRibbonGroup

Ribbon 中最核心的布局单元之一。

## 14.1 Word 中的对应功能

Word Home 页面：

```text
Clipboard | Font | Paragraph | Styles | Editing
```

每一块就是一个 Ribbon Group。

例如：

```text
┌─────────────┐
│ Paste       │
│ Cut Copy    │
│             │
│ Clipboard   │
└─────────────┘
```

对应：

```cpp
QmRibbonGroup
```

## 14.2 Group 结构

```text
QmRibbonGroup
│
├── ContentArea
│   ├── RibbonButton
│   ├── RibbonButton
│   └── ...
│
├── GroupTitle
│
└── GroupLauncherButton（可选）
```

---

# 15. QmRibbonGroupLauncherButton

这是 Ribbon 中一个重要但容易遗漏的控件。

Word 的 Font、Paragraph 等 Group 右下角通常有一个非常小的：

```text
↘
```

按钮。

该按钮官方通常称为：

> Dialog Box Launcher

建议类名：

```cpp
QmRibbonGroupLauncherButton
```

典型 Word 行为：

```text
Font Group Launcher
    → Font Dialog

Paragraph Group Launcher
    → Paragraph Dialog
```

---

# 16. QmRibbonButton

Ribbon 中最基本的命令按钮。

Word Home 中：

```text
Paste
Bold
Italic
Underline
Align Left
Center
```

都可视为 Ribbon Button。

Ribbon Button 一般需要支持多种尺寸。

## 16.1 Large Button

例如 Word 的 Paste：

```text
┌────────┐
│        │
│  ICON  │
│        │
│ Paste  │
└────────┘
```

## 16.2 Small Button

例如：

```text
[Icon] Text
```

建议：

```cpp
enum class QmRibbonButtonSize {
    Large,
    Medium,
    Small
};
```

或者：

```cpp
enum class QmRibbonButtonLayout {
    Large,
    Small
};
```

---

# 17. QmRibbonToolButton

用于小型、偏工具型按钮。

Word 中典型：

- Bold
- Italic
- Underline
- Undo
- Redo

可以基于：

```cpp
QToolButton
```

封装 Fluent / Ribbon 风格行为。

---

# 18. QmRibbonSplitButton

Ribbon 中非常重要的一类按钮。

Word 中典型：

```text
Paste ▼
```

按钮逻辑上拆为两个区域：

```text
┌───────────┐
│ Paste     │ → 执行默认命令
├───────────┤
│     ▼     │ → 打开菜单
└───────────┘
```

这类控件叫：

> Split Button

建议：

```cpp
QmRibbonSplitButton
```

行为：

```text
MainArea
    → trigger default action

ArrowArea
    → open menu
```

Word 中典型应用：

- Paste
- New Slide
- Borders
- Styles

---

# 19. QmRibbonDropDownButton

和 Split Button 不同。

DropDown Button：

```text
整个按钮点击
    ↓
都打开菜单
```

建议：

```cpp
QmRibbonDropDownButton
```

典型场景：

- Change Case
- Line Spacing
- Styles
- 各种选项菜单

---

# 20. QmRibbonGallery

Ribbon 中较高级的组件。

## 20.1 Word 中的对应功能

Home → Styles：

```text
Normal
No Spacing
Heading 1
Heading 2
Title
...
```

这些横向展示的样式卡片就是：

> Ribbon Gallery

其他典型场景：

```text
Design → Themes
Picture Format → Picture Styles
```

建议：

```cpp
QmRibbonGallery
```

内部推荐：

```text
QListView
+
QAbstractItemModel
+
QStyledItemDelegate
```

不建议堆大量 QWidget。

---

# 21. QmRibbonComboBox

Word 中典型：

```text
Font Family
Calibri ▼
```

以及：

```text
Font Size
11 ▼
```

建议：

```cpp
QmRibbonComboBox
```

可以基于 `QComboBox` 封装统一 Ribbon / Fluent 风格。

---

# 22. QmRibbonLineEdit

典型用途：

- Search
- Tell me what you want to do
- 尺寸 / 参数输入

建议：

```cpp
QmRibbonLineEdit
```

---

# 23. QmRibbonCheckBox

Word View Tab 中：

```text
Ruler
Gridlines
Navigation Pane
```

这些属于布尔状态开关。

建议：

```cpp
QmRibbonCheckBox
```

---

# 24. QmRibbonSeparator

用于 Ribbon Group 内部的小型分隔。

Page 中不同 Group 之间常见：

```text
Clipboard | Font | Paragraph
```

建议优先由 `QmRibbonPage` 或 Group 布局自动绘制 Group 分隔线，而不是业务层手工插入大量 Separator。

---

# 25. QmRibbonContextualTab

完整 Ribbon 框架需要支持 Contextual Tab。

## 25.1 Word 中的对应功能

选中图片后出现：

```text
Picture Format
```

选中表格后出现：

```text
Table Design
Layout
```

这些 Tab 平时不显示，只在特定上下文中显示。

称为：

> Contextual Tabs

建议：

```cpp
QmRibbonContextualTab
QmRibbonContextualTabGroup
```

例如：

```text
Table Tools
├── Table Design
└── Layout
```

Contextual Tab Group 可以有独立颜色标识。

---

# 26. Ribbon Collapse

Word 支持：

> Collapse the Ribbon

即只显示：

```text
File Home Insert ...
```

Ribbon Page 默认隐藏，点击 Tab 时临时展开。

建议：

```cpp
enum class QmRibbonDisplayMode {
    Expanded,
    TabsOnly
};
```

未来可以继续扩展：

```cpp
AutoHide
```

---

# 27. Backstage View

点击 Word 的：

```text
File
```

进入：

```cpp
QmRibbonBackstageView
```

此时主工作区切换为：

```text
Backstage Navigation
+
Backstage Page
```

---

# 28. QmRibbonBackstageNavigation

## 28.1 Word 中的对应功能

左侧通常包含：

```text
Home
New
Open

Info
Save
Save As

Print
Share
Export

Close
Account
Options
```

建议：

```cpp
QmRibbonBackstageNavigation
QmRibbonBackstageItem
```

## 28.2 Item 类型

需要区分：

### Page Item

例如：

```text
Home
New
Open
Info
Print
```

点击后切换右侧页面。

### Command Item

例如：

```text
Save
Close
```

点击后直接执行命令。

建议：

```cpp
enum class QmBackstageItemType {
    Page,
    Command,
    Separator
};
```

---

# 29. QmRibbonBackstagePage

建议定义基类：

```cpp
class QmRibbonBackstagePage : public QWidget
```

页面实现可包括：

```cpp
QmRibbonBackstageHomePage
QmRibbonBackstageNewPage
QmRibbonBackstageOpenPage
QmRibbonBackstageInfoPage
QmRibbonBackstageSaveAsPage
QmRibbonBackstagePrintPage
QmRibbonBackstageSharePage
QmRibbonBackstageExportPage
QmRibbonBackstageAccountPage
QmRibbonBackstageOptionsPage
```

---

# 30. Word Backstage 页面功能映射

| Page | Word 中的功能 | 作用 |
|---|---|---|
| Home | 文件首页 | 最近文件、模板、推荐操作 |
| New | 新建 | 创建空白文档或基于模板创建 |
| Open | 打开 | 最近文件、本地文件、云端文件 |
| Info | 信息 | 文档属性、权限、保护、版本 |
| Save | 保存 | 保存当前文件 |
| Save As | 另存为 | 选择路径和文件类型 |
| Print | 打印 | 打印设置与打印预览 |
| Share | 共享 | 分享文档 |
| Export | 导出 | 导出 PDF / XPS 等 |
| Close | 关闭 | 关闭当前文档 |
| Account | 账户 | 用户、主题、License |
| Options | 选项 | 应用全局设置 |

---

# 31. MainView

Backstage View 对应的正常应用工作区域建议命名：

```cpp
MainView
```

这是项目内部术语。

结构：

```text
QmRibbonWindow
└── ViewStack
    ├── MainView
    └── BackstageView
```

MainView 中：

```text
ads::CDockManager
```

成员示例：

```cpp
QWidget* main_view_;
QmRibbonBackstageView* backstage_view_;
```

---

# 32. 三层 Stack 的职责

整个应用中实际存在三个不同职责的 `QStackedWidget`。

## 32.1 Window View Stack

```text
Window View Stack
├── MainView
└── BackstageView
```

负责：

> 正常工作区 / File Backstage 之间切换。

## 32.2 Ribbon Page Stack

```text
Ribbon Page Stack
├── HomePage
├── InsertPage
└── ViewPage
```

负责：

> Ribbon Tab 对应 Page 的切换。

## 32.3 Backstage Page Stack

```text
Backstage Page Stack
├── HomePage
├── NewPage
└── OpenPage
```

负责：

> Backstage 左侧导航对应页面的切换。

这三个 Stack 不应混用。

---

# 33. 推荐类关系

```text
QmRibbonWindow
│
├── QmRibbon
│   │
│   ├── QmRibbonTitleBar
│   │   ├── QmRibbonQuickAccessBar
│   │   └── QmWindowButtonGroup
│   │
│   ├── QmRibbonTabBar
│   │   ├── QmRibbonFileButton
│   │   ├── QmRibbonTab
│   │   └── QmRibbonContextualTab
│   │
│   └── QStackedWidget
│       └── QmRibbonPage
│           └── QmRibbonGroup
│               ├── QmRibbonButton
│               ├── QmRibbonSplitButton
│               ├── QmRibbonGallery
│               ├── QmRibbonComboBox
│               └── ...
│
└── QStackedWidget
    ├── MainView
    │   └── ads::CDockManager
    │
    └── QmRibbonBackstageView
        │
        ├── QmRibbonBackstageNavigation
        │
        └── QStackedWidget
            └── QmRibbonBackstagePage
```

---

# 34. 推荐目录结构

```text
qmribbon/
├── window/
│   ├── qmribbonwindow.h
│   ├── qmribbonwindow.cpp
│   └── qmribbonwindow_win.cpp
│
├── ribbon/
│   ├── qmribbon.h
│   ├── qmribbon.cpp
│   │
│   ├── qmribbontitlebar.h
│   ├── qmribbontabbar.h
│   ├── qmribbontab.h
│   ├── qmribbonpage.h
│   └── qmribbongroup.h
│
├── controls/
│   ├── qmribbonbutton.h
│   ├── qmribbonsplitbutton.h
│   ├── qmribbondropdownbutton.h
│   ├── qmribbongallery.h
│   ├── qmribboncombobox.h
│   ├── qmribbonlineedit.h
│   └── qmribboncheckbox.h
│
├── backstage/
│   ├── qmribbonbackstageview.h
│   ├── qmribbonbackstagenavigation.h
│   ├── qmribbonbackstagepage.h
│   └── pages/
│
├── style/
│   ├── qmribbonstyle.h
│   ├── qmribbontheme.h
│   ├── qmribboncolors.h
│   └── qmribbonmetrics.h
│
└── private/
    ├── qmribbon_p.h
    ├── qmribbonwindow_p.h
    └── ...
```

---

# 35. 第一阶段实现范围

第一阶段建议实现：

```text
QmRibbonWindow
QmRibbon
QmRibbonTitleBar
QmRibbonQuickAccessBar

QmRibbonTabBar
QmRibbonTab

QmRibbonPage
QmRibbonGroup

QmRibbonButton
QmRibbonSplitButton
QmRibbonComboBox
QmRibbonGallery

QmRibbonBackstageView
QmRibbonBackstageNavigation
```

同时实现：

```text
Frameless Window
QWindow::startSystemMove()
QWindow::startSystemResize()
Windows Aero Snap
HTMAXBUTTON Snap Layout
```

第二阶段再实现：

```text
Contextual Tabs
Ribbon Collapse
Dynamic Layout
Gallery Popup
Customization
Keyboard KeyTips
```

---

# 36. Word Home 页面 API 映射示例

## 36.1 Clipboard

业务层期望写法：

```cpp
auto* home = ribbon->addPage(tr("Home"));

auto* clipboard =
    home->addGroup(tr("Clipboard"));

clipboard->addLargeButton(
    paste_action);

clipboard->addButton(
    cut_action);

clipboard->addButton(
    copy_action);
```

对应 Word：

```text
Home
┌────────────────┐
│     Paste      │
│   Cut   Copy   │
│                │
│   Clipboard    │
└────────────────┘
```

## 36.2 Font

业务层期望写法：

```cpp
auto* font =
    home->addGroup(tr("Font"));

font->addWidget(font_family_combo);
font->addWidget(font_size_combo);

font->addButton(bold_action);
font->addButton(italic_action);
font->addButton(underline_action);

font->setLauncherAction(font_dialog_action);
```

对应 Word：

```text
Font
┌──────────────────────────────────┐
│ Calibri ▼     11 ▼               │
│ B   I   U   abc   A▼             │
│                              ↘   │
│              Font                │
└──────────────────────────────────┘
```

---

# 37. API 设计原则

业务层应该描述 Ribbon 语义，例如：

```cpp
group->addLargeButton(action);
group->addButton(action);
group->setLauncherAction(action);
```

而不应该直接暴露布局实现：

```cpp
layout->addWidget(button);
layout->addLayout(...);
```

Ribbon 框架应该负责：

- 控件尺寸
- 控件排列
- Large / Small Button 布局
- Group 分隔
- Fluent / Ribbon Style
- DPI
- Hover / Pressed / Disabled 状态
- Ribbon 折叠
- Dynamic Layout

这样整个库才能成为真正意义上的 Ribbon Framework，而不仅仅是一套 Ribbon 风格的 QWidget。
