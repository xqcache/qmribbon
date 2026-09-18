#pragma once

#include "qmribbontheme.h"

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QStringList>

class QWidget;

/// 主题管理器（`QmRibbonThemeMgr`）：注册主题、切换「浅色 / 深色 / 跟随系统」三种模式，
/// 并把当前主题实时应用到整个界面。
///
/// 每次切换主题会一起做两件事：
///   1. `QGuiApplication::setPalette(主题调色板)` —— Qt 自绘控件（下拉列表、复选框、
///      滚动条、工具提示）与第三方控件都读调色板。ADS 的样式表正是用
///      `palette(window)` / `palette(light)` / `palette(dark)` / `palette(highlight)`
///      组织的，所以停靠区会自动跟着主题走，不需要另写一套 ADS 专用样式；
///   2. 把主题生成的 QSS 应用到登记过的框架控件（`QmRibbon` / `QmRibbonWindow`）。
/// 窗口边框的明暗由 `QmRibbonWindow` 同步给 DWM（QWindowKit 的 `dark-mode` 属性）。
///
/// 「跟随系统」模式用 `QStyleHints::colorScheme()` 读取系统明暗，并监听
/// `colorSchemeChanged` 实时响应系统切换；管理器不会去改系统/应用的配色方案。
///
/// 主题本身（配色 + 由配色生成的样式表）来自 JSON 配置文件，
/// 见 `QmRibbonTheme` 与 `resources/themes/*.json`。
class QmRibbonThemeMgr : public QObject {
    Q_OBJECT

public:
    enum class Mode {
        Light,  ///< 强制浅色
        Dark,   ///< 强制深色
        System, ///< 跟随系统
    };
    Q_ENUM(Mode)

    /// 全局单例：首次调用时创建，进程内唯一，生命周期直到进程结束（不会为空）。
    static QmRibbonThemeMgr& instance();

    // ---- 内置主题 ----
    //
    // 两份内置主题的内容不在代码里，而是编译进库的资源
    //（`resources/themes/light.json` / `dark.json` → `:/qmribbon/themes/`）。
    // 读取与注册都归管理器负责：构造时它们会以名字 `"Light"` / `"Dark"` 注册进主题表，
    // 因此 `setTheme("Dark")`、`themeNames()` 里都能看到它们。
    //
    // 这里的两个静态工厂只是「按需再构造一份内置主题」，
    // 例如拿去改几个颜色后 `registerTheme()` 成自己的主题。

    /// 内置浅色主题（对应 `Light`）。
    static QmRibbonTheme light();
    /// 内置深色主题（对应 `Dark`）。
    static QmRibbonTheme dark();

    /// 当前生效的主题（等价于 `instance().theme()`），**绘制代码取色的统一入口**：
    ///
    /// ```cpp
    /// painter.fillRect(rect(), QmRibbonThemeMgr::current().contentBackground());
    /// QColor accent = QmRibbonThemeMgr::current().accent();
    /// ```
    ///
    /// 返回管理器内部主题的引用，不会产生拷贝；主题切换后同一个引用会读到新配色，
    /// 因此可以安全地长期持有。
    static const QmRibbonTheme& current();

    Mode mode() const;
    void setMode(Mode mode);

    /// 当前生效的主题；「跟随系统」模式下是解析后的实际主题。
    /// 只比 `current()` 少一层语法糖，需要 `instance()` 才能调用。
    const QmRibbonTheme& theme() const;
    bool isDark() const;

    /// 注册自定义主题（配色键可自行扩展，只要样式表模板里有同名占位符）。
    bool registerTheme(const QmRibbonTheme& theme);
    /// 从 JSON 文件加载并注册主题，成功后可用 `setTheme(主题名)` 切换。
    bool loadThemeFile(const QString& file_path, QString* error = nullptr);

    /// 已注册的主题名（内置的 Light / Dark 也在其中，按名称排序）。
    QStringList themeNames() const;
    /// 切换到指定主题；模式会按其 dark 标记落到 Light 或 Dark。
    bool setTheme(const QString& name);

    /// 把当前主题应用到 root。root 会被登记，之后主题变化时自动重新应用。
    /// QmRibbon / QmRibbonWindow 构造时会自动调用。
    /// 主题是「两层」的：调色板（`QGuiApplication::setPalette()`）全局生效，QSS 只作用于
    /// 这里登记过的控件及其子孙。所以业务自己开的窗口（对话框、独立窗口）也调一次本函数，
    /// 就能拿到同一套控件外观（`ribbon.qss` 除了框架自己的控件，也覆盖了 Qt 常用控件）：
    ///
    /// ```cpp
    /// auto* dialog = new QDialog(&window);
    /// QmRibbonThemeMgr::instance().apply(dialog);
    /// ```
    ///
    /// 注意：控件自身的样式表优先级高于祖先的样式表，所以第三方控件（例如 ADS 停靠区）
    /// 自己声明过的属性不会被这里盖掉。
    void apply(QWidget* root);

    /// 业务自己的**附加样式表**（模板形式，可以用 `@{键名}` 占位符取主题配色 / 图标）。
    ///
    /// 它会被追加在框架样式表之后，一起应用到登记过的 root（及其子孙），
    /// 所以业务页面（例如示例的 Backstage、自定义对话框）可以有自己的样式，
    /// 又不用自己处理主题切换 —— 换主题时管理器会重新解析并应用它。
    ///
    /// ```cpp
    /// // 创建窗口之前调用；内容通常来自自己的 qrc
    /// QmRibbonThemeMgr::instance().setExtraStyleSheet(QString::fromUtf8(readMyQss()));
    /// ```
    void setExtraStyleSheet(const QString& template_text);
    QString extraStyleSheet() const;

signals:
    void modeChanged(Mode mode);
    void themeChanged(const QmRibbonTheme& theme);

private:
    explicit QmRibbonThemeMgr(QObject* parent = nullptr);
    ~QmRibbonThemeMgr() override;

    void resolveTheme();
    /// 框架样式表 + 业务附加样式表（都已解析占位符）。
    QString currentStyleSheet() const;
    /// 只把样式表重新刷到已登记的 root 上（不碰调色板、不发信号）。
    void refreshStyleSheets();
    void refresh();
    void handleColorSchemeChanged();

    struct Private;
    Private* d_ { nullptr };
};
