# qmribbon

基于 **Qt 6.8+ / Qt Widgets** 的 Ribbon UI 框架，布局与交互语义参考 Microsoft Word / Office，视觉风格偏 Fluent 2。

- 技术路线：`QWidget` 顶层窗口 + `Qt::FramelessWindowHint` + 自定义 TitleBar
- 设计原则：**只对 Ribbon 特有的结构与交互做自定义，能复用 Qt 标准控件的一律复用**
- 需求文档：[`docs/qmribbon_ui_requirements_v2.md`](docs/qmribbon_ui_requirements_v2.md)（现行）、[`docs/design.md`](docs/design.md)（初版，保留参考）

## 目录结构

```text
qmribbon/
├── source/
│   ├── window/      QmRibbonWindow —— 无边框顶层窗口（系统边框 / 圆角）、视图切换
│   ├── ribbon/      QmRibbon、TitleBar、QuickAccessBar、TabBar、Tab、Page、Group
│   ├── controls/    QmRibbonButton、QmRibbonComboBox、QmRibbonFloatingWidget 等 Ribbon 控件
│   ├── style/       QmRibbonTheme / QmRibbonThemeMgr —— 主题（配置驱动）、
│   │                QmRibbonThemeSwitchMask —— 切主题时的「挖空圆」过渡遮罩、
│   │                QmRibbonStyle —— 应用样式（QProxyStyle，收敛少数原生行为）、
│   │                QmRibbonAnimationUtil —— 动效（主题配置 + 运行时覆盖）、尺寸常量
│   ├── utils/       QmRibbonShadowGenerator —— 九宫格阴影贴图（窗口已改用系统边框，此工具当前不接入窗口）
│   └── qmribbonexport.h  QMRIBBON_EXPORT 导出宏（动态库构建用，公共头都包含它）
├── resources/themes/ 主题配置：light.json / dark.json / ribbon.qss（编译进库，只放框架样式）
├── thirdparty/      git submodule：QWindowKit（无边框窗口）、Qt Advanced Docking System（停靠）
├── examples/simple/ Word 风格的完整示例（含示例自己的样式表 simple.qss / simple.qrc）
├── tests/shadow_svg/ 阴影贴图与九宫格拉伸的目视验证（独立工具）
├── tests/consumer/  安装包消费验证（独立工程，不被主工程构建，见「安装与集成」）
└── docs/
```

## 构建

要求：CMake ≥ 3.24、C++17 以上编译器（MSVC 2022 已验证）、Qt 6.8+（Core / Gui / Widgets / Svg）。
第三方依赖通过 **git submodule** 提供，克隆时必须带 `--recursive`（详见
[`thirdparty/README.md`](thirdparty/README.md)）；首次 configure 会在构建目录内自动编译
`qmsetup`，之后无需联网。

```bash
git clone --recursive <this-repo>
# 已经克隆过：git submodule update --init --recursive

cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug   # 默认：静态库
cmake --build build
./build/examples/simple/qmribbon_simple
```

### 动态库（DLL / SO）

加 `-DQMRIBBON_BUILD_SHARED=ON`（等价于 CMake 约定的 `-DBUILD_SHARED_LIBS=ON`）构建动态库：

```bash
cmake -S . -B build-shared -G Ninja -DCMAKE_BUILD_TYPE=Debug -DQMRIBBON_BUILD_SHARED=ON
cmake --build build-shared
./build-shared/examples/simple/qmribbon_simple
```

| 构建类型 | 产物 | 运行期要部署的东西 |
| --- | --- | --- |
| 静态（默认） | `lib/qmribbond.lib` | 无（框架与第三方都在目标文件里） |
| 动态 | `bin/qmribbond.dll` + `lib/qmribbond.lib`（导入库） | `qmribbon.dll` 与 `qtadvanceddocking-qt6.dll` |

上表是 Debug 的名字：两个 DLL 都带 `d` 后缀（`qmribbond.dll` / `qtadvanceddocking-qt6d.dll`），
Release 则没有。

几点说明：

- **一次 configure 只产出一种类型**：两种都要就配两个目录（例如 `build/` 与
  `build-shared/`，`.gitignore` 已忽略 `build-*` / `install-*`）。
- 动态构建下 ADS（**公共**依赖）也跟着构建成 DLL，原因见根 `CMakeLists.txt`：静态 ADS 被
  链进 `qmribbon.dll` 之后，使用方还会再链一份，同一进程里就有两份 ADS，各自的静态状态
  互相看不见。顺带也满足 LGPL 对 ADS 的「动态链接」建议（见 `thirdparty/README.md` 的
  许可证提醒）。QWindowKit 反过来**始终静态** —— 它只出现在 `.cpp` 里（公共头不暴露它），
  会被整体链进 `qmribbon.dll`，所以不会多出一个要部署的 QWindowKit DLL。
- Windows 上示例与测试会把 DLL 自动拷到可执行文件旁边，所以上面两条运行命令在两种构建
  类型下都成立；你自己工程里的 exe 需要自己处理（见下一节）。
- **新增公共类时要在类名前加 `QMRIBBON_EXPORT`**（见 `source/qmribbonexport.h`）：静态库
  构建看不出问题，动态库构建下使用方会报 LNK2019。

作为子项目引入时（`QMRIBBON_BUILD_EXAMPLES` / `QMRIBBON_BUILD_TESTS` 默认关闭）：

```cmake
set(QMRIBBON_BUILD_SHARED ON)   # 可选：把 qmribbon 构建成动态库
add_subdirectory(thirdparty/qmribbon)
target_link_libraries(your_app PRIVATE qmribbon::qmribbon)
```

## 安装与集成（find_package）

```bash
cmake --install build        --prefix install-static
cmake --install build-shared --prefix install-shared
```

装出来的目录（以静态为例）：

```
install-static/
├── include/              qmribbon*.h（平铺）+ qmribbonexport.h
├── lib/                  qmribbond.lib（动态构建时这里是导入库）
├── bin/                  动态构建时的 qmribbond.dll 与 qtadvanceddocking-qt6d.dll
└── lib/cmake/qmribbon/   qmribbonConfig.cmake / qmribbonTargets.cmake / ConfigVersion
```

`cmake --install` 会连同一路构建的 QWindowKit、ADS 一起安装（它们各自也有 install 规则），
装完这个前缀就是个自洽的包。使用方：

```cmake
find_package(qmribbon 0.0.1 CONFIG REQUIRED)
add_executable(my_app main.cpp)
target_link_libraries(my_app PRIVATE qmribbon::qmribbon)
```

```bash
cmake -S . -B build -G Ninja -DCMAKE_PREFIX_PATH="<qmribbon 前缀>;<Qt6 前缀>"
```

注意事项：

- 动态库版本的 exe 运行时需要 `qmribbon.dll`（以及 `qtadvanceddocking-qt6.dll`，Debug 下
  都带 `d` 后缀）在它旁边或在 PATH 里 —— CMake 不会替你部署，`<prefix>/bin` 就是它们的位置。
- 静态库版本会在接口上带 `QMRIBBON_STATIC`，链接 `qmribbon::qmribbon` 即自动生效；
  **不用 CMake 目标、手工加 include 目录链接静态库时必须自己定义这个宏**，否则头文件里
  会展开成 `dllimport`，链接必然失败。
- 框架状态（主题管理器单例、动效覆盖等）都在库里：动态库方案下应用与 DLL 共享同一份。
- `tests/consumer/` 是一个**独立工程**（不被主工程构建），专门用来验证上面这套安装包：
  它触达每一个公共类，动态库版本能编译链接通过就说明导出宏齐全。

## 快速上手

> ⚠️ QWindowKit 要求：必须在构造 `QApplication` **之前**调用
> `QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings)`。

```cpp
#include "qmribbon.h"
#include "qmribbongroup.h"
#include "qmribbonpage.h"
#include "qmribbonwindow.h"

QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
QApplication app(argc, argv);

QmRibbonWindow window;
auto* ribbon = window.ribbon();

// Quick Access Toolbar 与 Ribbon 共享同一批 QAction
ribbon->quickAccessBar()->addAction(save_action);

auto* home = ribbon->addPage(tr("Home"));

auto* clipboard = home->addGroup(tr("Clipboard"));
clipboard->addLargeAction(paste_action);   // 图标在上、文字在下
clipboard->addAction(cut_action);          // 图标在左、文字在右，每三行换一列
clipboard->addAction(copy_action);

auto* font = home->addGroup(tr("Font"));
font->addWidget(font_family_combo);        // 任意 QWidget 都可以直接放入 Group
font->addWidget(font_size_combo);
font->addAction(bold_action);
font->addAction(italic_action);
font->addAction(underline_action);
font->setLauncherAction(font_dialog_action);   // 右下角 Dialog Box Launcher

window.show();
```

业务层只描述 Ribbon 语义（`addLargeAction` / `addAction` / `addWidget` / `setLauncherAction`），
控件尺寸、三行堆叠、Group 分隔线、Tab 状态等全部由框架负责。

Ribbon 的折叠 / 展开：**双击 Tab 栏**（双击到 Tab 上也可以）或点 Tab 栏右侧的折叠按钮，
与 Word 的 Collapse the Ribbon 一致。动画的开关 / 速度 / 缓动曲线统一由
`QmRibbonAnimationUtil` 提供（默认值写在主题配置文件的 `animation` 段里，见下文「主题」）：

```cpp
ribbon->setDisplayMode(QmRibbon::DisplayMode::TabsOnly);   // 也可以直接调 API
ribbon->isCollapsed();

QmRibbonAnimationUtil::shouldAnimate();                    // 当前配置下要不要做动画
QmRibbonAnimationUtil::setEnabled(false);                  // 运行时关掉全部动画
QmRibbonAnimationUtil::setDuration(160);                   // 毫秒，越小越快；0 = 不要动画
QmRibbonAnimationUtil::setEasingCurve(QEasingCurve::OutCubic);
QmRibbonAnimationUtil::clearOverrides();                   // 回到主题配置里的值
```

### QmRibbonComboBox（下拉框 / 字体框）

`QmRibbonComboBox` 继承 `QComboBox`，外观与弹出列表都对齐 Word 的「字体 / 字号」框：
直角、1px 细边框、右侧带分隔线的下拉按钮区（箭头自绘，跟着主题文字色走），
弹出列表支持**分组标题**、**右侧灰色标注**，以及**用条目自己的字体渲染**。
**弹出列表比输入框宽是正常的**：宽度按内容（最宽的一条 + 右侧标注）算，不会窄于组合框本身
—— Qt 默认只在「非可编辑 + 原生弹出菜单样式」下才会自己加宽，这里由控件显式处理：

```cpp
auto* combo = new QmRibbonComboBox;
combo->setEditable(true);                                  // 可选：Word 的字体框是可输入的
combo->setMinimumWidth(150);
font_group->addWidget(combo);                              // 放进 Group，和别的控件一样

combo->addGroupHeader(QStringLiteral("主题字体"));           // 分组标题：不可选中、不可点击
combo->addEntry(QStringLiteral("等线 Light"), QStringLiteral("(标题)"),
                QFont(QStringLiteral("等线 Light")));        // 第三项是「用它渲染这一项」的字体
combo->addEntry(QStringLiteral("等线"), QStringLiteral("(正文)"), QFont(QStringLiteral("等线")));
combo->addGroupHeader(QStringLiteral("所有字体"));
combo->addEntry(QStringLiteral("Arial"));                   // 标注与字体都可以省略

combo->setItemAnnotation(1, QStringLiteral("(正文)"));       // 也可以事后改
combo->setItemFont(1, QFont(QStringLiteral("等线")));
combo->isGroupHeader(0);                                    // true

combo->setPopupMinimumWidth(240);                           // 弹出列表最小宽度（0 = 只按内容算）
combo->popupMinimumWidth();                                 // 0
```

配色与尺寸见 `resources/themes/ribbon.qss` 的 `#RibbonComboBox` 段（主题切换自动跟随），
尺寸常量在 `QmRibbonMetrics`（`combo_dropdown_width` / `combo_item_height` / `combo_header_height` …）。
弹出列表的字号跟着组合框自身的 `font-size` 走（Qt 的 `QComboBoxListView` 就是这么取字体的）。

### 浮动控件（浮动工具栏）

`QmRibbonFloatingWidget` 是一块**不参与布局、浮在 MainView 之上**的面板，用来做浮动工具栏这类东西。
条目用 `addAction()` / `addWidget()` 放进去，**排列方式用预设模式指定**：

| `LayoutMode` | 效果 |
| --- | --- |
| `Horizontal` | 横向单行（默认） |
| `Vertical` | 纵向单列 |
| `Rows` | 横向排列、摊成 `lineCount()` 行 |
| `Columns` | 纵向排列、摊成 `lineCount()` 列 |

`lineCount()` 是**期望**的行 / 列数，条目尽量均匀地摊开，而且**短的那几行/列排在后面**，
所以中间不会出现空位：6 个条目要 4 行 → 2/2/1/1（不是 2/1/2/1，那样第二行末尾会空一格）；
条目比它少时按条目数算（3 个条目要 6 列就是 3 列）。

```cpp
auto* bar = new QmRibbonFloatingWidget;
bar->setTitle(tr("浮动工具栏"));
bar->setLayoutMode(QmRibbonFloatingWidget::LayoutMode::Rows, 2);   // 横向两行

bar->addAction(bold_action);                        // QToolButton（平铺），文本/图标/勾选跟随 QAction
bar->addAction(italic_action);
bar->addWidget(my_combo);                           // 也可以直接塞控件

window->addFloatingWidget(bar, QPoint(28, 28));     // 挂到 MainView 上（不接管生命周期）

bar->setPinned(true);                               // 固定位置（边框变强调色）
bar->snapToEdges();                                 // 需要的话手动吸边
```

拖动条可以拖（松手吸附到边缘、全程夹在可见区域内）、可以「固定」（位置锁死，双击拖动条也能切）。
`QmRibbonWindow::addFloatingWidget()` 会把控件 reparent 到 MainView、按位置摆放并置于最上层
（所以它始终浮在停靠区之上、Backstage 模式下跟着一起隐藏），MainView 尺寸变化时自动夹回可见区域；
条目变化时会按内容自动调整面板尺寸（要固定尺寸就在加完条目后自己 `resize()`）。
外观见 `ribbon.qss` 的 `#RibbonFloatingWidget`（固定状态是 `[pinned="true"]` 那条规则）。

Dialog Box Launcher（Group 右下角的 ↘）的响应由业务自己接：示例里 `setLauncherAction()` 的那个
QAction 被连到 `showFontLauncherPopup()`（[`examples/simple/launcherpopup.cpp`](examples/simple/launcherpopup.cpp)），
贴着按钮弹出一个模拟的「字体」面板，点外面或按 Esc 关闭。

主工作区使用内联的 **Qt Advanced Docking System**：

```cpp
#include <DockManager.h>
#include <DockWidget.h>

ads::CDockManager::setConfigFlag(ads::CDockManager::FocusHighlighting, true);

// 首次调用即创建 CDockManager 并接管 MainView，不需要 ADS 的窗口也用不上它
auto* dock_manager = window.dockManager();

auto* document_dock = dock_manager->createDockWidget(tr("Document1"));
document_dock->setWidget(document_editor);
dock_manager->addDockWidget(ads::CenterDockWidgetArea, document_dock);

auto* properties_dock = dock_manager->createDockWidget(tr("Properties"));
properties_dock->setWidget(properties_panel);
dock_manager->addDockWidget(ads::RightDockWidgetArea, properties_dock);
```

也可以完全不用 ADS，直接把任意控件放进主工作区：`window.setCentralWidget(widget)`。

## Windows 集成注意事项

以下三条是 QWindowKit 的硬性要求，**库无法代劳**，使用 `QmRibbonWindow` 的应用必须自己做：

1. **构造 `QApplication` 之前**设置属性：

   ```cpp
   QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
   ```

2. **不要设置 `Qt::FramelessWindowHint`**。QWindowKit 走 `WM_NCCALCSIZE` 方案——保留
   `WS_THICKFRAME` / `WS_CAPTION` 等系统边框样式，自己把标题栏从客户区里去掉，DWM 因此
   仍然负责窗口的**系统阴影与 Win11 圆角**；`Qt::FramelessWindowHint` 会把窗口变成
   `WS_POPUP`（另一套渲染路径），混用会让窗口失去系统边框，表现为没有阴影、没有圆角。
   `QmRibbonWindow` 已经按此设置好，业务侧不要再改（QWindowKit 的 `setup()` 之后也
   不应该再调用 `setWindowFlags()`）。

3. （可选）**Windows 11 贴靠布局面板**需要给 exe 加一份应用清单，在 `<compatibility>` 里声明
   支持 Windows 10/11（上游文档原话 "VERY VERY IMPORTANT"），而且没有等价的运行时 API。
   本仓库当前不启用该特性（`examples/simple` 未带清单）。需要时按下面做：

   ```xml
   <compatibility xmlns="urn:schemas-microsoft-com:compatibility.v1">
     <application>
       <supportedOS Id="{8e0f7a12-bfb3-4fe8-b9a5-48fd50a15a9a}"/>  <!-- Windows 10 / 11 -->
     </application>
   </compatibility>
   ```

   ```cmake
   if(WIN32)
       # 只加 /MANIFESTINPUT：CMake 的 vs_link_exe 会负责合并与嵌入，
       # 再加 /MANIFEST:EMBED 会得到 "CVT1100: 资源重复。类型: MANIFEST"。
       target_link_options(your_app PRIVATE
           "/MANIFESTINPUT:${CMAKE_CURRENT_SOURCE_DIR}/app.manifest")
   endif()
   ```

   两个坑：顶层 `<assemblyIdentity>` 不能写 `processorArchitecture="*"`（`mt.exe` 报
   `c10100b7`，只有 `<dependentAssembly>` 里的依赖标识才允许通配）；宿主机装了
   Windows SDK 的话可以直接用
   `mt.exe -manifest app.manifest -validate_manifest` 自查。

4. （可选）**过滤掉一条 Qt 的几何诊断**。无边框窗口在缩放显示器上会打印：

   ```text
   QWindowsWindow::setGeometry: Unable to set geometry 1770x1080+396+180
   (frame: 1792x1091+385+180) ... Resulting geometry: 1770x1080+396+182 ...
   ```

   它的含义只是「Windows 把请求的位置挪了 1~2 个像素」——**尺寸是按请求生效的**。
   原因是 QWindowKit 给窗口设的自绘边框（Qt 报 `margins: 11, 0, 11, 11`）与 Qt 内部的
   frame 模型在缩放显示器上会差一两像素，属无害噪声。只过滤这一条即可（示例做法见
   `examples/simple/main.cpp` 的 `filterGeometryNoise`）：

   ```cpp
   static QtMessageHandler previous = nullptr;
   previous = qInstallMessageHandler([](QtMsgType type, const QMessageLogContext& context,
                                        const QString& message) {
       if (message.startsWith(QStringLiteral("QWindowsWindow::setGeometry: Unable to set geometry"))) {
           return;
       }
       if (previous != nullptr) {
           previous(type, context, message);
       }
   });
   ```

窗口外观（边框、圆角、阴影）全部由系统提供，框架不自绘阴影、也不使用半透明背景：
半透明（layered）窗口不参与 DWM 圆角，而且按像素 alpha 做命中测试会让鼠标消息穿过
透明区域。`QmRibbonShadowGenerator`（`source/utils/`）那套九宫格阴影工具因此当前不接入
窗口，仅保留作为独立工具（`tests/shadow_svg` 可单独验证）。

## 主题

主题由 `QmRibbonTheme`（配置 + 由配置生成的样式表）和 `QmRibbonThemeMgr`（模式与实时应用）组成。
绘制代码取色统一走管理器上的静态入口，拿到的就是**当前生效**的主题：

```cpp
painter.fillRect(rect(), QmRibbonThemeMgr::current().contentBackground());
const QColor accent = QmRibbonThemeMgr::current().accent();
```

`QmRibbonThemeMgr::current()` 返回管理器内部主题的引用（等价于 `instance().theme()`），
换主题后同一个引用会读到新配色，可以安全长期持有；`QmRibbonTheme` 上的 `accent()` / `text()`
这类实例方法则作用于「某个具体主题」（例如 `QmRibbonThemeMgr::light()` 拿到的那一份）。

### 三种模式，实时生效

```cpp
auto& themes = QmRibbonThemeMgr::instance();

themes.setMode(QmRibbonThemeMgr::Mode::Light);   // 强制浅色
themes.setMode(QmRibbonThemeMgr::Mode::Dark);    // 强制深色
themes.setMode(QmRibbonThemeMgr::Mode::System);  // 跟随系统（默认）
```

- `QmRibbon` / `QmRibbonWindow` 构造时会把自己登记到管理器，主题变化时自动重新应用样式表，
  **不需要重启、也不需要手动刷新任何控件**；
- 每次切换主题，管理器会做两件事：
  1. **设置应用调色板**（`QmRibbonTheme::palette()` → `QGuiApplication::setPalette()`）。
     Qt 自绘控件（下拉列表、复选框、滚动条、工具提示）与第三方控件都读调色板 ——
     **ADS 的样式表就是用 `palette(window)` / `palette(light)` / `palette(dark)` /
     `palette(highlight)` 组织的**，所以停靠区会自动跟着主题换色，不需要另写 ADS 专用样式；
     示例里配了 `Fusion` 样式，它对调色板的支持最完整。
     另外 ADS 有浅色 / 深色**两套**样式表（图标也分黑白两份），它自己的 `FollowPalette`
     判定实测不可靠，会出现在深色下仍是黑色图标的问题，因此 `QmRibbonWindow` 会按主题
     显式调用 `CDockManager::setColorSchemeMode()`；
  2. 把主题 QSS 重新应用到登记过的框架控件（`QmRibbon` / `QmRibbonWindow`）；
- 窗口边框的明暗由 `QmRibbonWindow` 同步给 DWM（QWindowKit 的 `dark-mode` 属性），深色下不留白；
- 「跟随系统」模式用 `QStyleHints::colorScheme()` 读取系统明暗，并监听
  `colorSchemeChanged`，Windows 切换浅色 / 深色时界面**立即**跟着变（管理器不会去改系统配色方案）；
- 示例里有两个切换入口，都通过 `modeChanged` 保持选中态同步：
  **View → Theme 分组**（Light / Dark / 跟随系统，三个互斥的 Large 按钮）和
  **File → Account → 主题**（同样的三个选项，按钮式）。

### 主题切换动画

切主题不是硬闪，而是从窗口中心"擦"过去：管理器在切换**前**给每个可见的顶层窗口截一张旧主题快照，
切完后用 `QmRibbonThemeSwitchMask` 盖上整个窗口 —— 遮罩画的是「旧主题 + 中间一个不断扩大的洞
（`QPainterPath` 矩形 + 圆，`OddEvenFill` 奇偶填充）」，洞里露出的就是下面已经换成新主题的实时窗口，
所以看起来像新皮肤从圆心扩散开来。

只截**一张**快照（不是新旧各一张）：整窗 `grab()` 是这条链路上最贵的一步，少做一次，
切换时的停顿明显更短；正确性也没损失 —— 调色板 / 样式表此时已经换好，窗口的新主题重绘
在动画第一帧（圆还没露出来）之前就完成了。需要「完全不依赖下层窗口」的场合
（例如自己手动用这个遮罩）可以再 `setAfterSnapshot()` 补一张新快照，那样遮罩就自给自足。

它**全自动**，示例里四个切换入口（含跟随系统响应系统明暗）都会播，业务不用接任何东西；
过渡只对**登记过的顶层窗口**生效（`QmRibbonWindow`、`apply()` 过的对话框会一起播）。

```cpp
QmRibbonThemeMgr::instance().setThemeTransitionEnabled(false);   // 关掉过渡（默认开启）
QmRibbonAnimationUtil::setEnabled(false);                          // 或者干脆关掉所有动效
```

时长与缓动曲线复用主题 `animation` 段的设置（见下面「动画」一节），
所以深浅两个主题可以各自定速度；过渡期间遮罩对鼠标透明，窗口照常可拖动、可交互。
已知限制：窗口的**非客户区**（DWM 的系统边框 / 阴影）截不到也动画不了，那部分是瞬间切换的；
过渡途中有窗口尺寸变化时，遮罩会跟着贴合并按新尺寸缩放快照。

> 如果觉得切主题「点一下要等一下才动」，那部分基本不在这个动画上，而在换肤本身：
> 最贵的一步是窗口的 QSS 重新解析 + 整棵子树重新 polish（实测一个 1300 个控件的窗口，
> 一次 `setStyleSheet()` 约 85ms；控件越多线性增长），其次是整窗 `grab()`（1600×900 @150% DPI
> 约 8~25ms）。`setPalette()` 本身几乎不要钱（<0.1ms）。判断方法：
> `setThemeTransitionEnabled(false)` 再切一次，停顿依旧 → 就是换肤本身的开销。
>
> 框架为此做了两件事：过渡只截一张快照（少一次整窗 render）；换肤时**不重复设表** ——
> Qt 对「设成同一个字符串」不做短路（同串照样重新解析、重新 polish），所以
> `QmRibbonThemeMgr` 会先比对再设，重复 `apply()`、切到当前主题、系统明暗信号抖动等
> 都不会白刷整棵树。

### Qt 常用控件的统一外观

`ribbon.qss` 不只管框架自己的控件，也覆盖了 Qt 的常用控件，业务代码随手放一个 `QPushButton` /
`QTableView` 也不会「跳出」主题：

| 分组 | 覆盖的控件 |
| --- | --- |
| 按钮 | `QPushButton`（含默认按钮、选中态）、`QToolButton` |
| 输入 | `QLineEdit`（含只读）、`QAbstractSpinBox` + `QSpinBox` / `QDoubleSpinBox` 的上下按钮与箭头 |
| 编辑器 | `QTextEdit` / `QPlainTextEdit`（当作正文区域，不画边框） |
| 下拉 | `QComboBox`：右侧独立的下拉按钮区（分隔线 + 悬停）+ 主题箭头，以及弹出列表 |
| 选择 | `QCheckBox` / `QRadioButton` |
| 视图 | `QListView` / `QTreeView` / `QTableView` / `QColumnView` + `QHeaderView` |
| 容器 | `QTabWidget` / `QTabBar`、`QGroupBox`、`QSplitter` |
| 其它 | `QMenu` / `QMenuBar`、`QProgressBar`、`QScrollBar`（细把手、无箭头按钮）、`QDialog` |

`QLabel` 和复选框 / 单选的**文字颜色故意不写**进 QSS：它们由调色板负责（调色板是全局生效的），
写死 `color` 反而会压掉业务自己设过的颜色 —— 这类控件只调了间距。

下拉框和数值框的箭头不是原生画的，而是用主题自带的两份 SVG（明暗各一份，见
`QmRibbonTheme::assetKeys()`）：几何与 `QmRibbonComboBox` 自绘的那条 chevron 完全一致，
所以「Ribbon 里的下拉框」和「对话框里的普通下拉框」看起来是同一个东西。
只写 `QComboBox` 的边框是不够的 —— `::drop-down` 必须有几何，Qt 才会把文字区让出来，
否则箭头会浮在输入区上面、长条目会跑到箭头底下。

两条自我约束（写在 `ribbon.qss` 的段落注释里）：

- 只改**配色与状态**（悬停 / 按下 / 选中 / 禁用），**不动字体和最小尺寸** —— 字号和控件高度交给
  应用自己的设置与布局，免得把别人的版式挤变形。「工具按钮」还额外有三条：
  不写 `border` / `padding` / `border-radius`（否则 Qt 会改成「内容 + 盒模型」算尺寸，
  把原生样式的按钮边距丢掉，工具栏和停靠区标题栏的按钮大小就会跟着变）、
  **基础态也不写 `background`**（样式表会把 `QPalette::Button` 换成我们给的画刷再交给原生样式
  画按钮，而 `transparent` 这种「透明黑」在部分 Qt 版本上会被当成不透明黑画出来，
  表现就是「ToolButton 没悬停 / 没选中时是黑底黑字的一团」），以及
  想要平铺外观就让控件自己 `setAutoRaise(true)`（Ribbon 自己的按钮就是这么做的）；
- 只覆盖第三方控件**没有声明**的属性。控件自身的样式表优先级高于祖先（窗口）的样式表，
  所以 ADS 停靠区里已经声明过的 padding / border / background 不会被盖掉（停靠标签页、
  分隔件仍是它自己的样子），需要另一种外观时在那一层覆盖即可。

⚠️ 一个必须知道的副作用：`QPushButton` 的通用内边距会**吃掉固定尺寸按钮的内容区** ——
36×36 的纯图标按钮只剩 8px，里面的字形会被挤扁。所以固定尺寸 / 纯图标按钮要自己声明
`padding: 0`（示例里 Backstage 的返回按钮 `#BackstageBackButton` 就是这种情况）。

#### 下拉框弹出列表与 `QmRibbonStyle`

普通下拉框的弹出列表还有个 Qt 侧的硬限制：非可编辑的组合框，弹窗会走「弹出菜单」那套画法，
给弹窗加一层**原生菜单外框**；而弹窗容器 `QComboBoxPrivateContainer` 被 Qt 硬编码成
「不可被样式表命中」（`qstylesheetstyle.cpp` 的 `unstylable()`），所以那层框在 QSS 里删不掉 ——
要么我们的细框和它叠成「双层框」，要么只剩那层又重又旧的原生框。

`QmRibbonStyle`（一个 `QProxyStyle`，只有一处覆盖：`SH_ComboBox_Popup` 返回 0）
把「弹出菜单」那套关掉，弹窗于是完全由 `ribbon.qss` 控制，只剩一条 1px 细框：

```cpp
QApplication::setStyle(new QmRibbonStyle(QStyleFactory::create(QStringLiteral("Fusion"))));
```

不装也能用，只是普通下拉框的弹出列表会保留原生菜单外框；那种情况下把 `ribbon.qss` 里
`QComboBox QAbstractItemView` 的 `border` 去掉更合适（示例已经装了这个样式）。

QSS 是挂在窗口上的（`QmRibbonThemeMgr::apply()`），所以**对话框 / 独立窗口**也调一次 `apply()`
就能拿到同一套外观，并且跟着主题实时更新：

```cpp
auto* dialog = new QDialog(&window);
QmRibbonThemeMgr::instance().apply(dialog);   // 配色 + 样式表，主题切换时自动重新应用
dialog->exec();
```

### 主题配置文件

主题内容不在代码里，而是两份配色 JSON + 一份样式表模板（`resources/themes/`，编译进库的资源）：

```text
resources/themes/
├── light.json     配色（浅色）—— 内置主题 Light
├── dark.json      配色（深色）—— 内置主题 Dark
└── ribbon.qss     框架样式表模板，颜色写成 @{键名} 占位符
```

**框架样式表只放框架自己的东西**：窗口 / Ribbon / 各自定义控件，以及「Qt 常用控件」那一整段。
业务（和示例）自己的页面样式放各自的 qss 文件里，用同一套 `@{键名}` 占位符写法，
通过 `setExtraStyleSheet()` 交给主题管理器 —— 它会被追加在框架样式表之后应用到窗口上，
主题切换时自动重新解析，**不需要业务自己监听主题变化**：

```cpp
// 创建窗口之前设置即可（通常把 qss 塞进自己的 qrc）
QFile file(QStringLiteral(":/simple/simple.qss"));
if (file.open(QIODevice::ReadOnly)) {
    QmRibbonThemeMgr::instance().setExtraStyleSheet(QString::fromUtf8(file.readAll()));
}
```

示例就是这么做的：`examples/simple/simple.qss`（+ `simple.qrc`）里放 Backstage 页面、
Dialog Box Launcher 弹窗这些**只属于示例**的样式，框架的 `ribbon.qss` 里一条都没有。

**读取与注册内置主题是 `QmRibbonThemeMgr` 的职责**：管理器构造时会把这两份 JSON 读进来，
以 `"Light"` / `"Dark"` 注册到主题表（所以 `themeNames()`、`setTheme("Dark")` 里都有它们）。
需要单独拿一份内置主题时用静态工厂 `QmRibbonThemeMgr::light()` / `dark()`，
例如改几个颜色后注册成自己的主题：

```cpp
QmRibbonTheme mine = QmRibbonThemeMgr::dark();
mine.setColor(QStringLiteral("accent"), QColor(0x8a, 0x4f, 0xbd));
mine.setAnimationEnabled(false);                     // 动效也是主题配置的一部分
QmRibbonThemeMgr::instance().registerTheme(mine);     // 之后 setTheme(mine.name()) 切换
```

`QmRibbonTheme` 本身只是「一份主题」的值类型，不持有任何全局状态；
`QmRibbonTheme::fromFile()` 也支持 `:/...` 形式的 Qt 资源路径（内置主题就走这条路径读）。

配色 JSON 的结构（键的含义见 `resources/themes/light.json`）。
`animation` 段是**动效配置**，可以整个省略（用默认值 true / 160 ms / `OutCubic`），
也可以按主题分别设置 —— 例如某个主题想要「不要动画」，写 `"enabled": false` 即可：

```json
{
  "name": "Dark",
  "dark": true,
  "animation": {
    "enabled": true,
    "duration": 160,
    "easing": "OutCubic"
  },
  "assets": {
    "chevronDown": ":/my/arrow-down-dark.svg",
    "chevronUp": ":/my/arrow-up-dark.svg"
  },
  "colors": {
    "accent": "#4cc2ff",
    "windowBackground": "#212121",
    "contentBackground": "#2b2b2b",
    "text": "#e8e8e8",
    "...": "其余键见文件"
  }
}
```

`assets` 段是**可选的图标覆盖**。有几种小图标必须跟着主题明暗换色，而 QSS 的子控件
（`::down-arrow`）只接受 `image: url(...)`、给不了颜色，所以做法是**明暗各准备一份 SVG**：
内置主题按 `dark` 标记自动挑 `:/qmribbon/images/chevron-{down,up}-{light,dark}.svg`，
自定义主题可以用 `assets` 段换成自己的图（键见 `QmRibbonTheme::assetKeys()`）。

`easing` 可用的名字与 `QEasingCurve` 的枚举名一致，完整列表见
`QmRibbonAnimationUtil::easingNames()`（`"OutCubic"` / `"InOutSine"` / `"OutBack"` …），
写错时 `QmRibbonTheme::fromFile()` 会通过 `error` 参数报出来。

需要哪些键可以直接问代码：`QmRibbonTheme::colorKeys()` 是从 `ribbon.qss` 里推导出来的，
所以「模板里用到的键」就是唯一真相，不会出现两处维护。自绘控件（Tab、标题栏按钮、分隔线等）
通过 `QmRibbonTheme::accent()` / `text()` / `hover()` 这类语义化接口取色，同样跟着主题走。

### 自定义主题

```cpp
auto& themes = QmRibbonThemeMgr::instance();

QString error;
if (themes.loadThemeFile("D:/my-theme.json", &error)) {
    themes.setTheme(QStringLiteral("My Theme"));   // 模式会按其 dark 标记落到 Light / Dark
} else {
    qWarning() << error;
}

themes.themeNames();                     // 已注册的主题
themes.registerTheme(my_theme);          // 也可以直接注册一个 QmRibbonTheme
my_theme.setColor("accent", Qt::red);    // 键可自行扩展，只要模板里有 @{accent}
```

### 动画

动画是**主题的一部分**：`animation` 段（开关 / 时长 / 缓动）写在 `resources/themes/*.json` 里，
换主题时动效也跟着换；`QmRibbonAnimationUtil` 是统一的取用口，它把「默认值 → 主题配置 →
运行时覆盖」三层合并，并负责把设置套到 `QPropertyAnimation` 上：

```cpp
QmRibbonAnimationUtil::shouldAnimate();                  // 开关打开且时长 > 0
QmRibbonAnimationUtil::duration();                       // 真正生效的时长
QmRibbonAnimationUtil::easingCurve();                    // 真正生效的缓动曲线

QmRibbonAnimationUtil::setEnabled(false);                // 运行时覆盖（例如设置面板的「关闭动画」）
QmRibbonAnimationUtil::setDuration(160);                 // 毫秒，越小越快；0 = 不要动画
QmRibbonAnimationUtil::setEasingCurve(QEasingCurve::OutCubic);
QmRibbonAnimationUtil::clearOverrides();                 // 回到主题配置里的值
QmRibbonAnimationUtil::hasOverrides();                   // 设置界面可用它显示「已自定义」

// 框架内部（Ribbon 折叠 / 展开、Backstage 切换）统一这样写：
if (!QmRibbonAnimationUtil::prepare(animation)) {
    return;                                              // 直接跳到终态
}
animation->setStartValue(from);
animation->setEndValue(to);
animation->start();
```

## 代码风格

格式化配置见 [`.clang-format`](.clang-format)。

> ⚠️ **`SortIncludes` 必须保持 `Never`。**
> include 顺序是手工维护的：主头文件 → 项目头文件 → Qt 头文件 → 平台头文件。
> 典型反例是把 `<windows.h>` 排到 Qt 头文件之前，或把平台头文件挪出
> `#ifdef Q_OS_WIN` 条件块，都会直接导致编译失败。

需要真正禁止格式化的片段，请用 `// clang-format off` / `// clang-format on` 包裹。

> `thirdparty/` 下是 submodule，其中的文件由上游自带的 `.clang-format` 管辖，因此
> **不要对整个仓库执行 `clang-format -i`**，只格式化 `source/ examples/ tests/`，
> 否则会把 submodule 改脏。

## 实现状态

已完成：

- `QmRibbonWindow`：**不透明窗口 + 系统厚边框**（Win11 下圆角与阴影由 DWM 提供），
  MainView / Backstage 视图栈（切换带从左滑入 / 滑出的动画，动效同样走 `QmRibbonAnimationUtil`），
  Esc 从 Backstage 返回主界面。窗口拖动 / 边缘 Resize /
  Aero Snap / Windows 11 Snap Layout / 双击标题栏最大化全部由 **QWindowKit** 在原生层
  接管（见 [`thirdparty/README.md`](thirdparty/README.md)）
- `QmRibbon`：TitleBar + TabBar + Page Stack；**双击 Tab 栏**或点折叠按钮可折叠 / 展开
  （`DisplayMode::Expanded` / `TabsOnly`），带动画（开关 / 时长 / 缓动由主题配置决定，
  运行时可用 `QmRibbonAnimationUtil` 覆盖）；
  `setTitleBarOnly(true)` 供 Backstage 使用（保留标题栏与窗口按钮，隐藏 Tab 栏与页面）
- `QmRibbonTitleBar`：App 图标、Quick Access Toolbar、标题、账户、自绘窗口按钮
  （最小化 / 最大化 / 还原 / 关闭）；四个控件分别注册为 QWindowKit 的
  WindowIcon / Minimize / Maximize / Close 系统按钮，Quick Access 等交互控件注册为
  hit-test 可见，其余区域即窗口拖动区
- `QmRibbonTabBar` / `QmRibbonTab`：File 按钮、选中下划线、Contextual Tab 配色、右侧动作、折叠按钮
- `QmRibbonPage` / `QmRibbonGroup`：Group 横向布局与分隔线、Large/Small 三行堆叠、Dialog Box Launcher
- `QmRibbonButton`：Large / Medium / Small 三种语义尺寸
- `QmRibbonComboBox`：Word 风格的下拉框（直角 + 自绘箭头 + 分组标题 / 右侧标注 / 条目自带字体的弹出列表），
  示例中用作「字体 / 字号」两个框
- `QmRibbonFloatingWidget` + `QmRibbonWindow::addFloatingWidget()`：浮在 MainView 之上的面板
  （浮动工具栏），支持拖动、边缘吸附、固定（位置锁死），以及**预设排列**：
  横向单行 / 纵向单列 / 横向多行 / 纵向多列（`setLayoutMode()` + `lineCount()`）
- `QmRibbonTheme` / `QmRibbonThemeMgr`：配置驱动的主题（浅色 / 深色 / 跟随系统，实时切换）+ 尺寸常量。
  `QmRibbonTheme` 是「一份主题」的值类型（配色 / 动效配置 + 样式表与调色板）；
  `QmRibbonThemeMgr` 负责内置主题（`light()` / `dark()`，注册为 Light / Dark）的读取与注册、
  模式切换、实时应用，以及取色入口 `QmRibbonThemeMgr::current()`
- `QmRibbonAnimationUtil`：集中动效配置（默认值 → 主题 `animation` 段 → 运行时覆盖）与缓动曲线名转换，
  框架内的动画（Ribbon 折叠 / 展开、Backstage 切换、主题切换过渡）都从它取设置
- `QmRibbonThemeSwitchMask`：切主题时的过渡遮罩 —— 旧主题快照上挖一个不断扩大的圆露出新主题，
  由 `QmRibbonThemeMgr` 自动对登记过的顶层窗口播放（`setThemeTransitionEnabled(false)` 可关）
- `QmRibbonStyle`：可选的应用样式（`QProxyStyle`）。目前只覆盖 `SH_ComboBox_Popup`，
  让下拉框弹出列表完全由主题样式表控制，不再出现那层删不掉的原生菜单外框
- `QmRibbonWindow::dockManager()`：MainView 接入 Qt Advanced Docking System
  （`setCentralWidget()` 仍可用于不使用停靠的场景）
- 示例的 Backstage（[`examples/simple/backstage.cpp`](examples/simple/backstage.cpp)）：
  左侧导航（Home / New / Open / Info / Save As / Print / Share / Export / Account / Options +
  Close 命令项，含分组分隔线）+ 右侧页面栈（最近文档、模板卡片、属性与保护、打印设置与纸张预览、
  账户与主题、选项等）。外观走示例自己的样式表 [`examples/simple/simple.qss`](examples/simple/simple.qss)
  里的 `#Backstage*` 选择器（通过 `setExtraStyleSheet()` 接入主题，见「主题」一节）；
  框架级的 `QmRibbonBackstageView` 落地时沿用同一套 objectName 即可
- 示例的 **Gallery 页**（[`examples/simple/gallery.cpp`](examples/simple/gallery.cpp)）：
  把上面「Qt 常用控件」那一整段样式摆出来对照 —— 分组里直接放原生控件
  （`QPushButton` / `QToolButton` / `QLineEdit` / `QSpinBox` / `QComboBox` / `QCheckBox` /
  `QRadioButton` / `QProgressBar` / `QScrollBar` / `QLabel` / `QMenu`），
  塞不进 Ribbon 的大件（`QGroupBox` / `QTabWidget` / `QSplitter` / 表格 / 树 / 列表 /
  `QTextEdit` / `QDialogButtonBox`）由「Widget Gallery」按钮弹出**非模态对话框**展示；
  这个对话框同时演示了业务窗口如何接入主题：`QmRibbonThemeMgr::apply()` 一次，
  之后切换 Light / Dark 它会实时跟着变
- 示例的 **浮动工具栏**（[`examples/simple/floatingtoolbar.cpp`](examples/simple/floatingtoolbar.cpp)）：
  `QmRibbonFloatingWidget` + `addFloatingWidget()` 的完整用法 —— 一排命令按钮 + 「固定」/「关闭」，
  显隐由 **View → Show → Floating Toolbar** 这个 checkable 动作控制（关闭按钮会取消勾选它）
- 示例的 **Dialog Box Launcher 弹窗**（[`examples/simple/launcherpopup.cpp`](examples/simple/launcherpopup.cpp)）：
  Home → Font 组右下角的 ↘ 点开是一个模拟的「字体」面板（字体 / 字形 / 字号 + 效果 + 实时预览 +
  确定 / 取消），贴着按钮弹出、点外面或 Esc 关闭

待实现（第二阶段）：

- `QmRibbonGallery`（Styles / Themes 横向画廊，`QListView` + Model + Delegate）
  —— 注意它和示例里那个展示 Qt 控件的 **Gallery 页**不是一回事：前者是框架级的画廊控件，
  后者只是把控件样式摆出来对照的演示页
- `QmRibbonBackstageView` / `QmRibbonBackstageNavigation`（当前 `setBackstageWidget()` 只提供视图切换的接入点）
- Contextual Tab 分组管理、Dynamic Group Layout、Gallery Popup、Keyboard KeyTips
