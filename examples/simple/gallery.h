#pragma once

class QmRibbonPage;

/// Ribbon 的 Gallery 页：把我们**改过样式的 Qt 原生控件**摆出来，
/// 一眼就能看出「框架自己的控件」和「普通 Qt 控件」是不是同一套外观。
///
/// 分两部分：
///   - 页面里的分组直接用原生控件（`QPushButton` / `QToolButton` / `QLineEdit` /
///     `QSpinBox` / `QComboBox` / `QCheckBox` / `QRadioButton` / `QProgressBar` /
///     `QScrollBar` / `QLabel` / `QMenu`），高度压到 Ribbon 的一行高，正好塞进 Group；
///   - 塞不下的大件（`QGroupBox` / `QTabWidget` / `QSplitter` / `QTableWidget` /
///     `QTreeWidget` / `QListWidget` / `QTextEdit` / `QDialogButtonBox`）由分组里的
///     「Widget Gallery」按钮弹出一个**非模态对话框**展示。
///
/// 对话框同时演示了业务自己的窗口该怎么接入主题：只要 `QmRibbonThemeMgr::apply()` 一次，
/// QSS 与调色板都会跟着主题走 —— 对话框开着的时候在 Ribbon 里切 Light / Dark 能实时看到变化。
void populateGalleryPage(QmRibbonPage* page);
