# thirdparty

本目录下的第三方依赖以 **git submodule** 方式管理，仓库里只保存引用（gitlink）与
`.gitmodules`，不保存上游源码。

| 目录 | 来源 | 版本 / 提交 | 许可证 |
|---|---|---|---|
| `qwindowkit/` | [stdware/qwindowkit](https://github.com/stdware/qwindowkit) | 1.5.1.0 @ `4f683f2e0e4f3a7d6061d0224de9ed30e7c17e0f` | Apache-2.0 |
| `qwindowkit/qmsetup/` | [stdware/qmsetup](https://github.com/stdware/qmsetup)（QWindowKit 嵌套的 submodule） | `a63c44c9a452f016fe849470d66f825609354c92` | MIT |
| `qwindowkit/qmsetup/src/stdcorelib/` | [stdware/stdcorelib](https://github.com/stdware/stdcorelib)（qmsetup 嵌套的 submodule） | `2d134f10852475e3c90d0c0495e5b3f8ab868079` | MIT |
| `qtadvanceddocking/` | [githubuser0xFFFF/Qt-Advanced-Docking-System](https://github.com/githubuser0xFFFF/Qt-Advanced-Docking-System) | v5.1.1 @ `023ce95934fecfd4cf5c672c9c404fabe0f54923` | **LGPL-2.1** |

依赖链是三层的：`qwindowkit → qmsetup → stdcorelib`。qmsetup 是 QWindowKit 的
configure 期依赖（它在 configure 阶段被单独 build 一次），而 qmsetup 又需要
`stdcorelib` 才能配置，所以**必须递归获取**，否则会在 configure 阶段报
`... /qmsetup/src/stdcorelib does not contain a CMakeLists.txt file`。

## 获取源码

```bash
# 首次克隆（含所有嵌套 submodule）
git clone --recursive <this-repo>

# 已经克隆过、或忘了加 --recursive
git submodule update --init --recursive
```

## 更新依赖

```bash
# 以 ADS 为例，切到新 tag
git -C thirdparty/qtadvanceddocking fetch --depth 1 origin tag v5.1.2
git -C thirdparty/qtadvanceddocking checkout v5.1.2
git add thirdparty/qtadvanceddocking        # 记录新的 gitlink

# QWindowKit 同理，可以直接跟上游分支
git -C thirdparty/qwindowkit fetch --depth 1 origin main
git -C thirdparty/qwindowkit checkout FETCH_HEAD
git add thirdparty/qwindowkit
```

更新 QWindowKit 后，如果它 pin 的 `qmsetup` / `stdcorelib` 也变了，需要一并更新并记录：

```bash
git -C thirdparty/qwindowkit submodule update --init --recursive
git add thirdparty/qwindowkit
```

更新后请同步修改本文件表格里的版本与提交号。

> 本地这三份克隆是用 `--depth 1`（stdcorelib 为完整克隆）拉的，因此切换到别的提交前
> 需要先 `fetch`（上面的命令已经包含）。重新 `git clone --recursive` 得到的是完整克隆。

## ⚠️ 许可证提醒

- QWindowKit 是 **Apache-2.0**，商业友好。
- Qt Advanced Docking System 是 **LGPL-2.1**，本项目以 `BUILD_STATIC=ON` 静态链接它。
  静态链接 LGPL 库后对外分发二进制时，需要同时提供可重新链接的目标文件（或改用动态链接）。
  内部使用没有问题；**如果要对外发布，建议改成动态链接**：

  ```powershell
  cmake -S . -B build -DBUILD_STATIC=OFF   # ADS 改为构建 DLL
  ```

  此时需要把 `qtadvanceddocking-qt6.dll` 与可执行文件一起分发（Debug 为 `d` 后缀）。

## QWindowKit：无边框窗口的原生行为

`QmRibbonWindow` 是无边框窗口，需要处理标题栏拖动、边缘 Resize、Aero Snap、
Windows 11 Snap Layout、双击标题栏最大化等行为。这些必须在原生层完成：

- 标题栏可拖动区域要返回 `HTCAPTION`，让系统接管拖动与双击最大化；
- 最大化按钮区域要返回 `HTMAXBUTTON`，**同时**窗口必须有正确的窗口样式
  （`WS_THICKFRAME` / `WS_MAXIMIZEBOX` 等），`DefWindowProc` 才会真正执行最大化
  并弹出贴靠布局面板 —— 只返回 `HTMAXBUTTON` 而不调整样式，会把该区域变成
  "无人处理"的非客户区，表现为按钮点不动。

QWindowKit 在 `WM_NCHITTEST` 中先取 `DefWindowProc` 的结果再修正，并同步维护窗口样式，
因此上述行为都由它接管；框架自身只保留 Ribbon 内容与 MainView 布局，窗口外观
（边框、圆角、阴影）全部交给系统。

集成时有两条硬性要求（库无法代劳，详见项目根 `README.md` 的「Windows 集成注意事项」）：

1. 构造 `QApplication` 之前设置 `Qt::AA_DontCreateNativeWidgetSiblings`；
2. **不要设置 `Qt::FramelessWindowHint`** —— 它会让窗口变成 `WS_POPUP`，从而失去系统
   边框，表现为没有系统阴影、没有圆角。

另外，Windows 11 的贴靠布局面板还要求 exe 带一份声明 `supportedOS` 的应用清单
（本仓库当前未启用该特性，需要时的做法见根 `README.md`）。

## Qt Advanced Docking System：MainView 的停靠系统

`QmRibbonWindow::dockManager()` 首次调用时创建 `ads::CDockManager` 并把它设为主工作区内容
（未调用的窗口不会创建任何 ADS 对象），业务层随后用标准 ADS API 搭布局：

```cpp
auto* dock_manager = window.dockManager();
auto* dock = dock_manager->createDockWidget(tr("Properties"));
dock->setWidget(properties_widget);
dock_manager->addDockWidget(ads::RightDockWidgetArea, dock);
```

`QmRibbonWindow` 的公共头文件只用前置声明 `namespace ads { class CDockManager; }`，
所以包含它本身不需要 ADS 的包含路径。

## 构建方式

顶层 `CMakeLists.txt` 直接：

```cmake
add_subdirectory(thirdparty/qwindowkit)
add_subdirectory(thirdparty/qtadvanceddocking)
```

链接目标分别是 `QWindowKit::Widgets`（`QWKWidgets` 的 ALIAS）与
`ads::qtadvanceddocking`（`qtadvanceddocking-qt6` 的版本无关 ALIAS）。
使用的关键配置：

- `QWINDOWKIT_BUILD_STATIC=ON`：静态链接，应用不需要额外部署 QWindowKit 的 DLL；
- `QWINDOWKIT_BUILD_QUICK/EXAMPLES/TESTS/DOCUMENTATIONS=OFF`：本项目用不到；
- `QWINDOWKIT_ENABLE_WINDOWS_SYSTEM_BORDERS=ON`（上游默认）：窗口使用系统厚边框，
  Win11 的圆角与阴影由 DWM 提供，我们**不自绘阴影、也不使用半透明背景**。
  关闭它会同时踩到三个坑：layered（半透明）窗口不参与 DWM 圆角 → 窗口没有圆角；
  阴影留白处 alpha=0 → 鼠标消息穿透，QWindowKit 布置在客户区最外侧的 Resize 边框失效；
  最大化时 QWindowKit 会把客户区四边内缩 8px（`win32windowcontext.cpp` 的
  `isSystemBorderEnabled()` 分支）→ 表现为"最大化后仍能拖动边缘调整大小"；
- `BUILD_STATIC=ON`、`BUILD_EXAMPLES=OFF`（ADS 的选项没有项目前缀，因此只在
  `add_subdirectory` 前后的一小段作用域内设置，用完立刻 `unset`）。

`qmsetup` 是 QWindowKit 的构建期依赖，QWindowKit 会优先 `find_package(qmsetup)`，
找不到时自动使用 `qwindowkit/qmsetup` 这份 submodule 源码，所以无需单独安装。

## ⚠️ 修改约定

**不要修改本目录下 submodule 里的任何代码。** 四个 submodule 都必须保持

```bash
git -C thirdparty/qwindowkit status --porcelain -uall          # 应为空
git -C thirdparty/qwindowkit/qmsetup status --porcelain -uall  # 应为空
git -C thirdparty/qwindowkit/qmsetup/src/stdcorelib status --porcelain -uall  # 应为空
git -C thirdparty/qtadvanceddocking status --porcelain -uall   # 应为空
```

一旦在 submodule 内改动，`git submodule status` 会显示脏标记，别人拉下来也无法复现。
第三方行为不符合预期时，按优先级选择：

1. **在自己的代码里适配**：例如 `QmRibbonWindow` 通过 `nativeEvent()` 拦截某个消息，
   或者调整我们传给 QWindowKit 的注册方式（titleBar / systemButton / hitTestVisible）；
2. **只用上游提供的开关**：如 `QWINDOWKIT_ENABLE_WINDOWS_SYSTEM_BORDERS`、`BUILD_STATIC`，
   或运行期的 `WindowAgentBase::setWindowAttribute()`；
3. **确实必须改上游源码时**：fork 上游仓库，把 `.gitmodules` 里的 url 改成 fork 并
   记录 diff，不要就地修改（就地改的代价是后续完全无法升级）。

## 关于格式化与仓库体积

- **格式化**：submodule 里的文件由上游自带的 `.clang-format` 管辖，本项目的
  `thirdparty/.clang-format`（`DisableFormat: true`）只对 submodule 之外的文件生效。
  因此**不要对整个仓库执行 `clang-format -i`**，只格式化
  `source/ examples/ tests/`；否则 submodule 会被改脏（上游配置与本地 clang-format
  版本不一致时会出现大量重排）。
- **体积**：ADS 上游的 `doc/`（约 22.6 MB 截图）等目录仍会出现在工作区里，
  但不进入本仓库历史，也不参与构建（`BUILD_EXAMPLES=OFF`）。
