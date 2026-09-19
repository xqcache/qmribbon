#pragma once

#include "qmribbonexport.h"

#include "qmribbonanimationutil.h"

#include <QColor>
#include <QEasingCurve>
#include <QHash>
#include <QPalette>
#include <QString>

/// Ribbon 使用的尺寸常量（逻辑像素，DPI 缩放由 Qt 负责）。
/// 尺寸与配色分开：主题配置只改配色，尺寸仍然由代码常量决定。
namespace QmRibbonMetrics {

// 标题栏 / Tab 栏
inline constexpr int title_bar_height = 32;
inline constexpr int tab_bar_height = 32;
inline constexpr int tab_horizontal_padding = 14;
inline constexpr int tab_min_width = 52;
inline constexpr int tab_indicator_height = 3;
inline constexpr int window_button_width = 46;

// Ribbon Page / Group
inline constexpr int page_margin = 2;
inline constexpr int group_spacing = 8;
inline constexpr int group_content_height = 70;
inline constexpr int group_title_height = 16;
inline constexpr int group_total_height = group_content_height + group_title_height;

// 按钮
inline constexpr int large_button_icon = 32;
inline constexpr int large_button_min_width = 56;
inline constexpr int large_button_max_width = 112;
inline constexpr int medium_button_icon = 24;
inline constexpr int medium_button_height = 52;
inline constexpr int medium_button_min_width = 52;
inline constexpr int medium_button_max_width = 96;
inline constexpr int small_button_icon = 16;
inline constexpr int small_button_height = 22;
inline constexpr int small_button_spacing = 2;
inline constexpr int small_button_min_width = 64;
inline constexpr int small_button_max_width = 140;
inline constexpr int small_column_rows = 3;

// 组合框（QmRibbonComboBox）
inline constexpr int combo_dropdown_width = 20; ///< 下拉按钮区宽度（与 ribbon.qss 里 `::drop-down` 的 width 一致）
inline constexpr int combo_item_height = 30;    ///< 弹出列表条目高度
inline constexpr int combo_header_height = 26;  ///< 弹出列表分组标题高度
inline constexpr int combo_popup_padding = 10;  ///< 弹出列表左右内边距
inline constexpr int combo_annotation_gap = 16; ///< 条目文字与右侧标注之间的最小间隔
inline constexpr int combo_icon_gap = 6;        ///< 图标与文字之间的间隔

// 浮动控件（QmRibbonFloatingWidget）
inline constexpr int floating_drag_bar_height = 18; ///< 顶部拖动条高度（抓手 / 标题 / 固定标记）
inline constexpr int floating_snap_distance = 24;   ///< 松手时吸附到父控件边缘的距离

// 其它
inline constexpr int launcher_size = 16;
inline constexpr int quick_access_button_size = 24;
inline constexpr int icon_size = 16;
/// Ribbon 下边沿「阴影带」的高度（用于分隔 Ribbon 与 MainView）。
inline constexpr int ribbon_shadow_height = 6;

} // namespace QmRibbonMetrics

/// Ribbon 主题：一组配色 + 由配色生成样式表。
///
/// 主题内容来自 **JSON 配置文件**（`resources/themes/*.json`，编译进库的资源里），
/// 也可以用 `fromJson()` / `fromFile()` 加载自定义主题。配置结构：
///
/// ```json
/// {
///   "name": "Dark",
///   "dark": true,
///   "animation": { "enabled": true, "duration": 160, "easing": "OutCubic" },
///   "assets": { "chevronDown": ":/my/arrow-dark.svg" },
///   "colors": { "accent": "#4cc2ff", "windowBackground": "#212121", ... }
/// }
/// ```
///
/// 配色键见 `kColorKeys`（`QmRibbonTheme::color()` 也接受任意自定义键，
/// 只要样式表模板里出现 `@{键名}` 占位符就会被替换）。
///
/// 动效（开关 / 时长 / 缓动）和配色一样属于主题配置的一部分，
/// 见 `animation` 段与 `QmRibbonAnimationUtil`。
///
/// 这个类是「一份主题」的值类型；它不持有任何全局状态：
///   - **当前生效的主题**在管理器上：`QmRibbonThemeMgr::current()`（取色统一入口）；
///   - **内置主题**（Light / Dark）也由管理器提供：`QmRibbonThemeMgr::light()` / `dark()`。
class QMRIBBON_EXPORT QmRibbonTheme {
public:
    QmRibbonTheme() = default;

    static QmRibbonTheme fromJson(const QByteArray& json, QString* error = nullptr);

    /// 从文件加载主题；`:/...` 形式的 Qt 资源路径同样可用
    ///（库内置主题就在 `:/qmribbon/themes/` 下，见 `QmRibbonThemeMgr::light()`）。
    static QmRibbonTheme fromFile(const QString& file_path, QString* error = nullptr);

    /// 所有会被样式表模板引用的**配色**键（不含图标键，见 `assetKeys()`）。
    static QStringList colorKeys();

    // ---- 图标资源 ----
    //
    // 有几种小图标必须跟着主题明暗换色，而 QSS 的子控件（`::down-arrow`）只接受
    // `image: url(...)`、给不了颜色。做法是**明暗各准备一份 SVG**，样式表模板里写
    // `image: url(@{chevronDown})`，由本类替换成对应路径：
    //
    //   - 内置主题按 `dark` 标记自动挑一份（`:/qmribbon/images/chevron-down-{light,dark}.svg`）；
    //   - 自定义主题可以用 JSON 的 `assets` 段覆盖成自己的图。

    /// 模板里可以用 `@{键名}` 引用的图标键。
    static QStringList assetKeys();

    /// 图标键对应的资源路径；未定义的键返回空字符串。
    QString asset(const QString& key) const;

    bool isValid() const;
    QString name() const;
    bool isDark() const;

    QColor color(const QString& key) const;
    void setColor(const QString& key, const QColor& color);

    /// 用本主题的配色渲染样式表模板。
    QString styleSheet() const;

    /// 用本主题的值替换一段样式表模板里的 `@{键名}` 占位符（配色 + 图标资源）。
    ///
    /// `styleSheet()` 就是拿它解析内置模板的。业务自己的样式表也可以用同一套占位符写法，
    /// 交给 `QmRibbonThemeMgr::setExtraStyleSheet()` 之后就会跟着主题换色、换图标。
    QString resolveStyleSheet(const QString& template_text) const;

    /// 由本主题配色生成应用调色板。
    ///
    /// 这是主题的「另一半」：QSS 只管框架自己的控件，而 Qt 自绘控件（下拉列表、复选框、
    /// 滚动条、工具提示）以及第三方控件都读调色板 —— ADS 的样式表就是
    /// `palette(window)` / `palette(light)` / `palette(dark)` / `palette(highlight)` 组织的，
    /// 因此把调色板设成主题配色，停靠区就会跟着主题走。
    QPalette palette() const;

    // ---- 语义化取色 ----
    //
    // 这些都是实例方法，作用于「某个具体主题」：
    //     const QmRibbonTheme& theme = QmRibbonThemeMgr::light();
    //     theme.accent();
    //
    // 控件绘制时通常要拿「当前生效主题」的颜色，用管理器上的静态入口：
    //     QmRibbonThemeMgr::current().accent();
    QColor accent() const;
    QColor accentHover() const;
    QColor onAccent() const;
    QColor danger() const;
    QColor windowBackground() const;
    QColor contentBackground() const;
    QColor text() const;
    QColor secondaryText() const;
    QColor mutedText() const;
    QColor disabledText() const;
    QColor border() const;
    QColor borderStrong() const;
    QColor borderHover() const;
    QColor hover() const;
    QColor pressed() const;
    QColor buttonChecked() const;
    QColor buttonCheckedBorder() const;
    QColor surface() const;
    QColor surfaceBorder() const;
    QColor surfaceHover() const;
    QColor separator() const;
    QColor inputBackground() const;
    QColor navChecked() const;
    /// Ribbon 下边沿阴影带的颜色（**带 alpha**，配置里写成 `#AARRGGBB`）。
    QColor ribbonShadow() const;

    // ------------------------------------------------------------------
    // 动效设置（主题的一部分）
    //
    // 动画的开关 / 时长 / 缓动曲线写在主题配置文件的 `animation` 段里
    //（见 `resources/themes/light.json`），因此**每个主题可以有自己的动效**，
    // 换主题时动效设置也跟着换：
    //
    // ```json
    // "animation": { "enabled": true, "duration": 160, "easing": "OutCubic" }
    // ```
    //
    // 这里只保存「这个主题声明了什么」，是配置数据的读取口；
    // 真正跑动画的地方（Ribbon 折叠 / 展开、Backstage 切换）统一走
    // `QmRibbonAnimationUtil`，它会叠加运行时覆盖，并把设置套到 QPropertyAnimation 上。
    // ------------------------------------------------------------------

    /// 是否启用动画，默认 true。
    bool isAnimationEnabled() const;
    void setAnimationEnabled(bool enabled);

    /// 动画时长（毫秒），默认 160；越小越快，0 等同于关闭动画。
    int animationDuration() const;
    void setAnimationDuration(int milliseconds);

    /// 动画缓动曲线，默认 QEasingCurve::OutCubic。
    QEasingCurve animationEasingCurve() const;
    void setAnimationEasingCurve(const QEasingCurve& curve);

private:
    QString name_;
    bool dark_ { false };
    QHash<QString, QColor> colors_;
    /// JSON `assets` 段里的图标路径（没写的键走 `asset()` 里的内置默认值）。
    QHash<QString, QString> assets_;

    bool animation_enabled_ { QmRibbonAnimationUtil::defaultEnabled };
    int animation_duration_ { QmRibbonAnimationUtil::defaultDuration };
    QEasingCurve animation_easing_ { QEasingCurve::OutCubic };
};
