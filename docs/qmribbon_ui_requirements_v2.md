# Ribbon 界面开发需求文档（Qt Widgets 版）

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
- Quick Access Toolbar
- Contextual Tab
- Backstage View
- Main View
- 窗口控制按钮
- 系统拖动 / Resize
- Windows Aero Snap
- Windows 11 Snap Layout

---

# 2. 总体布局结构

```text
QmRibbonWindow
│
├── QmRibbon
│   │
│   ├── QmRibbonTitleBar
│   │   ├── QmRibbonQuickAccessBar
│   │   ├── QLabel / QmRibbonWindowTitle
│   │   ├── 可选 Search / Account 区域
│   │   └── WindowButtonGroup
│   │
│   ├── QmRibbonTabBar
│   │   ├── File Button
│   │   ├── Ribbon Tab
│   │   ├── Ribbon Tab
│   │   ├── Contextual Tab
│   │   └── Right Actions
│   │
│   └── QStackedWidget
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

# 3. 实现原则

Ribbon 框架不应把所有控件都重新实现。

总体原则：

> 只对 Ribbon 特有的结构和交互语义做自定义；Qt 已经成熟提供的基础控件应尽量直接复用。

推荐分为三类：

### 3.1 必须自定义的 Ribbon 结构组件

这些组件本身带有 Ribbon 特有布局、语义或状态，不适合直接用 Qt 标准控件替代：

```text
QmRibbonWindow
QmRibbon
QmRibbonTitleBar
QmRibbonQuickAccessBar
QmRibbonTabBar
QmRibbonTab
QmRibbonPage
QmRibbonGroup
QmRibbonGallery
QmRibbonBackstageView
QmRibbonBackstageNavigation
```

### 3.2 建议基于 Qt 控件轻量封装

这些控件 Qt 已经提供了成熟能力，只需要增加 Ribbon 尺寸、布局或样式语义：

```text
QmRibbonButton        → 基于 QToolButton
QmRibbonTab           → 基于 QAbstractButton / QToolButton
```

### 3.3 直接使用 Qt 控件

以下控件原则上没有必要重新定义类：

```text
QToolButton
QPushButton
QComboBox
QLineEdit
QCheckBox
QMenu
QAction
QLabel
QFrame
QListView
QStackedWidget
```

---

# 4. 组件实现方式总表

| Ribbon 概念 | Word 中对应功能 | 推荐 Qt 实现 | 是否自定义 |
|---|---|---|---|
| RibbonWindow | Word 主窗口 | `QWidget` + Frameless | 是 |
| Ribbon | 整个 Ribbon 区域 | `QWidget` | 是 |
| RibbonTitleBar | Word 顶部标题栏 | `QWidget` | 是 |
| QuickAccessBar | Save / Undo / Redo | `QWidget + QHBoxLayout + QToolButton` | 是 |
| WindowTitle | `Document1 - Word` | `QLabel` | 否 |
| WindowButtonGroup | `─ □ ×` 容器 | `QWidget + QHBoxLayout` | 可内部实现 |
| Min/Max/Close Button | 窗口控制按钮 | `QToolButton` | 否 |
| RibbonTabBar | File / Home / Insert... | `QWidget` + 自定义布局 | 是 |
| File Button | File | `QToolButton` / `QAbstractButton` | 否或轻量封装 |
| RibbonTab | Home / Insert / View | `QAbstractButton` / `QToolButton` | 建议轻量封装 |
| RibbonPage | Home 页内容区 | `QWidget` | 是 |
| RibbonGroup | Clipboard / Font / Paragraph | `QWidget` | 是 |
| RibbonButton | Paste / Bold / Align | `QToolButton` | 建议轻量封装 |
| SplitButton | Paste + 下拉菜单 | `QToolButton::MenuButtonPopup` | 否 |
| DropDownButton | 整体点击打开菜单 | `QToolButton::InstantPopup` | 否 |
| ComboBox | Font / Font Size | `QComboBox` | 否 |
| LineEdit | Search / 参数输入 | `QLineEdit` | 否 |
| CheckBox | Ruler / Gridlines | `QCheckBox` | 否 |
| Gallery | Styles / Themes / Picture Styles | `QListView + Model + Delegate` | 是 |
| Separator | Group 内部分隔 | `QFrame` 或自绘 | 否 |
| Group Launcher | Font 右下角 ↘ | `QToolButton` | 否 |
| Contextual Tab | Picture Format / Table Design | RibbonTab + 属性 | 不单独建类 |
| BackstageView | 点击 File 后整个界面 | `QWidget` | 是 |
| BackstageNavigation | 左侧 Home/New/Open | `QListView` 或自定义 QWidget | 建议自定义组件 |
| BackstagePage | Home / Open / Print | 任意 `QWidget` | 否 |
| Page Stack | Tab 对应 Page | `QStackedWidget` | 否 |
| View Stack | Main / Backstage | `QStackedWidget` | 否 |

---

# 5. 命名规范

统一使用 `QmRibbonXXX` 表示 Ribbon 框架组件。

## 5.1 窗口相关

```cpp
QmRibbonWindow
QmRibbonTitleBar
```

窗口按钮组可以作为内部实现，不一定暴露公共类。

如果需要公开：

```cpp
QmWindowButtonGroup
```

窗口按钮本身直接使用：

```cpp
QToolButton
```

无需定义：

```cpp
QmWindowButton
```

除非以后需要统一窗口按钮绘制和状态管理。

---

# 6. QmRibbonWindow

## 6.1 作用

应用程序最外围顶层窗口。

对应 Word：

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

## 6.2 布局

```text
QmRibbonWindow
│
├── QmRibbon
│
└── QStackedWidget
    ├── MainView
    └── BackstageView
```

成员示例：

```cpp
QmRibbon* ribbon_;
QStackedWidget* view_stack_;

QWidget* main_view_;
QmRibbonBackstageView* backstage_view_;
```

## 6.3 窗口行为

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

推荐：

```text
TitleBar Drag
    → QWindow::startSystemMove()

Resize
    → QWindow::startSystemResize()

Win11 Snap Layout
    → WM_NCHITTEST + HTMAXBUTTON
```

---

# 7. QmRibbon

## 7.1 作用

Ribbon 整体容器。

对应 Word：

> 从标题栏开始，到 Clipboard / Font / Paragraph 等命令区域结束的完整 Ribbon 区域。

## 7.2 布局

```text
QmRibbon
├── QmRibbonTitleBar
├── QmRibbonTabBar
└── QStackedWidget
```

建议：

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

# 8. QmRibbonTitleBar

## 8.1 Word 对应区域

Word 顶部大致为：

```text
┌────────────────────────────────────────────────────────────┐
│ AutoSave [Save][Undo][Redo]  Document1 - Word  Search  _ □ × │
└────────────────────────────────────────────────────────────┘
```

## 8.2 推荐结构

```text
QmRibbonTitleBar
│
├── LeftArea
│   ├── AppIcon（可选）
│   └── QmRibbonQuickAccessBar
│
├── CenterArea
│   ├── QLabel（Window Title）
│   └── QLineEdit（Search，可选）
│
└── RightArea
    ├── Account Button（可选）
    └── Window Buttons
```

这里真正需要自定义的是 TitleBar 的整体行为和布局。

里面的基础控件尽量使用 Qt 自带控件。

---

# 9. Quick Access Toolbar

## 9.1 Word 对应功能

Word 左上角常见：

```text
Save
Undo
Redo
```

称为：

> Quick Access Toolbar

## 9.2 推荐实现

自定义：

```cpp
QmRibbonQuickAccessBar : public QWidget
```

内部使用：

```cpp
QHBoxLayout
QToolButton
QAction
```

例如：

```cpp
quick_access_bar->addAction(save_action);
quick_access_bar->addAction(undo_action);
quick_access_bar->addAction(redo_action);
```

按钮本身无需自定义。

---

# 10. Window Title

对应：

```text
Document1 - Word
```

直接使用：

```cpp
QLabel
```

即可。

没有必要单独实现 `QmRibbonWindowTitle`，除非未来需要：

- 保存状态
- 文档修改状态
- 云同步状态
- 多行标题
- 特殊绘制

---

# 11. Window Buttons

对应：

```text
─   □   ×
```

推荐直接使用：

```cpp
QToolButton
```

连接：

```cpp
showMinimized()
showMaximized()
showNormal()
close()
```

最大化按钮额外通过 Win32：

```text
WM_NCHITTEST
    → HTMAXBUTTON
```

支持 Windows 11 Snap Layout。

不需要自定义 `QmWindowButton`。

---

# 12. QmRibbonTabBar

## 12.1 Word 对应

```text
File
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

这整行对应：

```cpp
QmRibbonTabBar
```

## 12.2 为什么建议自定义

该区域不仅包含普通 Tab，还包含：

- File
- 普通 Ribbon Tab
- Contextual Tab
- Help
- Collapse Ribbon
- 右侧动作

因此不建议简单直接使用一个 `QTabBar` 完成整个区域。

---

# 13. File Button

Word：

```text
File
```

点击后进入：

```text
Backstage View
```

推荐直接使用：

```cpp
QToolButton
```

或者：

```cpp
QAbstractButton
```

只需要在 RibbonTabBar 中给它特殊布局和行为。

没有必要单独定义：

```cpp
QmRibbonFileButton
```

除非希望 API 更语义化。

---

# 14. Ribbon Tab

Word 中：

```text
Home
Insert
Draw
Design
Layout
References
Review
View
```

建议：

```cpp
class QmRibbonTab : public QAbstractButton
```

或者基于：

```cpp
QToolButton
```

轻量封装。

自定义的原因不是按钮本身，而是需要增加：

- 当前选中状态
- Ribbon Page 关联
- Contextual Group 信息
- Contextual Color
- Hover / Selected 绘制

例如：

```cpp
QmRibbonTab* addTab(
    const QString& title,
    QmRibbonPage* page);
```

---

# 15. QmRibbonPage

一个 Ribbon Tab 对应一个 Page。

Word Home：

```text
Clipboard
Font
Paragraph
Styles
Editing
```

布局：

```text
QmRibbonPage
├── QmRibbonGroup
├── QmRibbonGroup
├── QmRibbonGroup
└── ...
```

必须自定义：

```cpp
class QmRibbonPage : public QWidget
```

主要负责：

- Group 横向布局
- Group 间距
- Group separator
- Overflow
- Ribbon collapse 后恢复

---

# 16. QmRibbonGroup

Word：

```text
Clipboard | Font | Paragraph | Styles | Editing
```

每一块对应：

```cpp
QmRibbonGroup
```

建议结构：

```text
QmRibbonGroup
│
├── ContentArea
│
├── GroupTitle
│
└── LauncherButton（可选）
```

这是 Ribbon 特有组件，需要自定义。

---

# 17. Ribbon Button

Ribbon Button 没必要从零实现。

推荐：

```cpp
class QmRibbonButton : public QToolButton
```

只做轻量封装。

主要增加 Ribbon 语义：

```cpp
enum class Size {
    Large,
    Medium,
    Small
};
```

例如：

```cpp
button->setToolButtonStyle(
    Qt::ToolButtonTextUnderIcon);
```

对应 Word Large Button：

```text
┌────────┐
│  ICON  │
│ Paste  │
└────────┘
```

Small Button：

```cpp
button->setToolButtonStyle(
    Qt::ToolButtonTextBesideIcon);
```

---

# 18. Split Button

Word 中典型：

```text
Paste ▼
```

点击主体执行默认动作，点击箭头打开菜单。

Qt 已经直接支持：

```cpp
QToolButton::MenuButtonPopup
```

例如：

```cpp
auto* button = new QToolButton;

button->setDefaultAction(paste_action);
button->setMenu(paste_menu);
button->setPopupMode(
    QToolButton::MenuButtonPopup);
```

所以：

> 第一版不需要实现 `QmRibbonSplitButton`。

如果后续发现 Word 风格的：

- Hit Area
- 箭头区域
- Large Button 垂直布局
- Hover 绘制

无法通过 Style 达到要求，再考虑自定义。

---

# 19. DropDown Button

整个按钮点击后打开菜单。

Qt：

```cpp
QToolButton::InstantPopup
```

例如：

```cpp
button->setMenu(menu);
button->setPopupMode(
    QToolButton::InstantPopup);
```

因此不需要：

```cpp
QmRibbonDropDownButton
```

---

# 20. ComboBox

Word：

```text
Font Family: 等线（中文正文）
Font Size: 11
```

直接使用：

```cpp
QComboBox
```

即可。

样式统一交给 Ribbon Style / Fluent Theme。

不建议定义：

```cpp
QmRibbonComboBox
```

---

# 21. LineEdit

Word 中类似：

```text
Search
```

或业务参数输入。

直接：

```cpp
QLineEdit
```

无需定义：

```cpp
QmRibbonLineEdit
```

---

# 22. CheckBox

Word View Tab：

```text
Ruler
Gridlines
Navigation Pane
```

直接：

```cpp
QCheckBox
```

无需定义：

```cpp
QmRibbonCheckBox
```

---

# 23. Separator

内部小分隔可以使用：

```cpp
QFrame
```

例如：

```cpp
frame->setFrameShape(QFrame::VLine);
```

但是 Ribbon Group 之间的分隔线，更推荐由：

```cpp
QmRibbonPage
```

统一绘制。

---

# 24. Group Launcher Button

Word Font / Paragraph Group 右下角：

```text
↘
```

对应：

> Dialog Box Launcher

直接使用：

```cpp
QToolButton
```

即可。

没有必要定义独立 Launcher Button 类。

例如：

```cpp
font_group->setLauncherAction(
    font_dialog_action);
```

内部创建普通 `QToolButton`。

---

# 25. Ribbon Gallery

Word 中：

```text
Home → Styles
Design → Themes
Picture Format → Picture Styles
```

这些是 Ribbon Gallery。

这是 Qt 标准控件无法直接完整表达的 Ribbon 特有组件，因此建议自定义：

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

Gallery 主要负责：

- 横向 item 展示
- Preview
- Hover
- Selection
- More Button
- Popup Gallery

---

# 26. Contextual Tab

Word：

选中图片：

```text
Picture Format
```

选中表格：

```text
Table Design
Layout
```

不建议单独实现：

```cpp
QmRibbonContextualTab
```

更建议普通：

```cpp
QmRibbonTab
```

增加属性：

```cpp
bool contextual;
QString contextual_group;
QColor contextual_color;
```

例如：

```cpp
tab->setContextualGroup("Picture Tools");
tab->setContextualColor(color);
```

Contextual Tab Group 如果需要统一管理多个 Tab，可以单独定义轻量数据对象，而不一定做 QWidget。

---

# 27. Backstage View

点击：

```text
File
```

进入：

```cpp
QmRibbonBackstageView
```

必须自定义，因为它负责：

```text
Navigation
+
Backstage Page Stack
```

---

# 28. Backstage Navigation

Word 左边：

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
```

内部有两种实现方案：

### 方案 A：QListView

推荐。

```text
QListView
+
Model
+
Delegate
```

优点：

- Item 数量多时性能好
- Hover/Selected 统一
- Separator 容易实现
- Page / Command 类型容易通过 Model 表达

### 方案 B：QWidget + Buttons

适合第一版简单实现。

---

# 29. Backstage Item 类型

建议：

```cpp
enum class QmBackstageItemType {
    Page,
    Command,
    Separator
};
```

### Page

例如：

```text
Home
New
Open
Info
Print
```

点击切换页面。

### Command

例如：

```text
Save
Close
```

点击直接执行 QAction。

---

# 30. Backstage Page

不建议要求业务页面继承：

```cpp
QmRibbonBackstagePage
```

直接接受：

```cpp
QWidget*
```

更灵活。

例如：

```cpp
backstage->addPage(
    open_action,
    open_page);

backstage->addPage(
    print_action,
    print_page);
```

其中：

```cpp
open_page
print_page
```

可以是任何 QWidget。

因此：

> 不必定义 `QmRibbonBackstagePage` 基类。

---

# 31. QAction 应作为命令核心

Ribbon 框架应尽量以：

```cpp
QAction
```

表示业务命令。

例如：

```text
Save
Undo
Redo
Copy
Paste
Bold
Print
```

都应该首先是 QAction。

例如：

```cpp
auto* save_action =
    new QAction(save_icon, tr("Save"), this);

save_action->setShortcut(
    QKeySequence::Save);
```

然后同一个 Action 可以出现在：

```text
Quick Access Toolbar
Ribbon Group
Menu
Context Menu
Keyboard Shortcut
```

例如：

```cpp
quick_access_bar->addAction(
    save_action);

file_group->addAction(
    save_action);
```

这比为每种 Ribbon Button 设计独立业务 API 更重要。

---

# 32. View Mode

建议：

```cpp
enum class QmRibbonViewMode {
    Main,
    Backstage
};
```

结构：

```text
QmRibbonWindow
└── QStackedWidget
    ├── MainView
    └── BackstageView
```

---

# 33. 三层 QStackedWidget

整个框架会存在三个不同职责的 Stack。

## 33.1 Window View Stack

```text
MainView
BackstageView
```

## 33.2 Ribbon Page Stack

```text
HomePage
InsertPage
ViewPage
```

## 33.3 Backstage Page Stack

```text
HomePage
NewPage
OpenPage
PrintPage
```

三者职责必须分开。

---

# 34. 推荐最终公共类数量

第一版公共 API 建议控制在：

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
QmRibbonGallery

QmRibbonBackstageView
QmRibbonBackstageNavigation
```

只有约 12 个核心公共类。

---

# 35. 不建议第一版定义的类

以下类可以删除或延后：

```text
QmRibbonWindowTitle
QmWindowButton
QmRibbonFileButton
QmRibbonSplitButton
QmRibbonDropDownButton
QmRibbonComboBox
QmRibbonLineEdit
QmRibbonCheckBox
QmRibbonSeparator
QmRibbonGroupLauncherButton
QmRibbonContextualTab
QmRibbonBackstagePage
```

对应直接使用：

```text
QLabel
QToolButton
QToolButton::MenuButtonPopup
QToolButton::InstantPopup
QComboBox
QLineEdit
QCheckBox
QFrame
QWidget
```

---

# 36. 推荐目录结构

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
│   ├── qmribbontitlebar.h
│   ├── qmribbonquickaccessbar.h
│   ├── qmribbontabbar.h
│   ├── qmribbontab.h
│   ├── qmribbonpage.h
│   └── qmribbongroup.h
│
├── controls/
│   ├── qmribbonbutton.h
│   ├── qmribbonbutton.cpp
│   ├── qmribbongallery.h
│   └── qmribbongallery.cpp
│
├── backstage/
│   ├── qmribbonbackstageview.h
│   ├── qmribbonbackstageview.cpp
│   ├── qmribbonbackstagenavigation.h
│   └── qmribbonbackstagenavigation.cpp
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

# 37. 第一阶段实现范围

建议第一阶段实现：

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
QmRibbonGallery

QmRibbonBackstageView
QmRibbonBackstageNavigation
```

同时实现：

```text
Qt::FramelessWindowHint
QWindow::startSystemMove()
QWindow::startSystemResize()
Windows Aero Snap
WM_NCHITTEST + HTMAXBUTTON
Windows 11 Snap Layout
```

第二阶段再实现：

```text
Contextual Tab Group
Ribbon Collapse
Dynamic Group Layout
Gallery Popup
Quick Access Customization
Ribbon Customization
Keyboard KeyTips
```

---

# 38. Word Home 页面映射示例

## 38.1 Clipboard

```cpp
auto* home =
    ribbon->addPage(tr("Home"));

auto* clipboard =
    home->addGroup(tr("Clipboard"));

clipboard->addLargeAction(
    paste_action);

clipboard->addAction(
    cut_action);

clipboard->addAction(
    copy_action);
```

对应：

```text
┌────────────────┐
│     Paste      │
│   Cut   Copy   │
│                │
│   Clipboard    │
└────────────────┘
```

Paste 若需要 Split Button：

```cpp
auto* paste_button =
    clipboard->buttonForAction(
        paste_action);

paste_button->setMenu(
    paste_menu);

paste_button->setPopupMode(
    QToolButton::MenuButtonPopup);
```

无需额外定义 SplitButton 类。

---

# 39. Font Group 映射示例

```cpp
auto* font =
    home->addGroup(tr("Font"));

auto* family =
    new QComboBox;

auto* size =
    new QComboBox;

font->addWidget(family);
font->addWidget(size);

font->addAction(bold_action);
font->addAction(italic_action);
font->addAction(underline_action);

font->setLauncherAction(
    font_dialog_action);
```

对应 Word：

```text
┌──────────────────────────────────┐
│ 等线（中文正文） ▼     11 ▼       │
│ B   I   U   abc   A▼             │
│                              ↘   │
│              字体                 │
└──────────────────────────────────┘
```

其中：

```text
Font Family  → QComboBox
Font Size    → QComboBox
B/I/U        → QToolButton + QAction
Launcher     → QToolButton
```

---

# 40. API 设计原则

业务层应该描述 Ribbon 语义：

```cpp
group->addLargeAction(action);
group->addAction(action);
group->addWidget(widget);
group->setLauncherAction(action);
```

而不是直接操作布局：

```cpp
layout->addWidget(...);
layout->addLayout(...);
```

Ribbon Framework 负责：

- 控件尺寸
- Large / Medium / Small 排列
- Group 布局
- Group 分隔
- Tab 状态
- Fluent / Ribbon Style
- DPI
- Hover / Pressed / Disabled
- Ribbon Collapse
- Dynamic Layout
- Backstage 切换

业务程序只负责：

```text
Action
Widget
Page
业务逻辑
```

这样可以最大限度复用 Qt 自带控件，同时保持 Ribbon 框架自身的结构清晰和维护成本可控。
