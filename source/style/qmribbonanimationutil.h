#pragma once

#include "qmribbonexport.h"

#include <QEasingCurve>
#include <QString>
#include <QStringList>

class QPropertyAnimation;

/// 动效工具：框架里**所有动画**的开关 / 时长 / 缓动曲线都由它提供，
/// 并把设置一次性套到 `QPropertyAnimation` 上。
///
/// 配置分三层，优先级由低到高：
///   1. 类里的默认值（`defaultEnabled` / `defaultDuration` / `defaultEasingName()`）；
///   2. **主题配置文件**的 `animation` 段（`resources/themes/*.json`）—— 每个主题可以有
///      自己的动效，换主题时动效也跟着换；
///   3. 运行时的 `setEnabled()` / `setDuration()` / `setEasingCurve()` 覆盖，例如设置面板里的
///      「关闭动画」：用户偏好不会被下一次换主题重置，`clearOverrides()` 可回到主题配置。
///
/// 框架内部用到动画的地方（Ribbon 折叠 / 展开、Backstage 与主工作区切换）统一这样写：
///
/// ```cpp
/// QPropertyAnimation* animation = ...;   // 目标与属性由调用方决定
/// if (!QmRibbonAnimationUtil::prepare(animation)) {
///     // 当前配置下不要动画（开关关闭或时长为 0）：直接跳到终态
///     return;
/// }
/// animation->setStartValue(from);
/// animation->setEndValue(to);
/// animation->start();
/// ```
class QMRIBBON_EXPORT QmRibbonAnimationUtil {
public:
    // ---- 默认值（主题配置里的 `animation` 段缺项时使用）----

    static constexpr bool defaultEnabled = true;
    static constexpr int defaultDuration = 160;
    /// 时长上限：配置里误写一个夸张的值时不至于让界面「卡住」。
    static constexpr int maxDuration = 5000;
    /// 默认缓动曲线的名字（与 QEasingCurve 的枚举名一致）。
    static QString defaultEasingName();

    // ---- 生效值（主题配置 + 运行时覆盖）----

    static bool isEnabled();
    static int duration();
    static QEasingCurve easingCurve();
    /// 是否应该播放动画：开关打开且时长大于 0。
    /// 只表示「配置允许动画」，是否真的播还要看调用方自己的条件（例如控件是否可见）。
    static bool shouldAnimate();

    // ---- 运行时覆盖 ----

    static void setEnabled(bool enabled);
    static void setDuration(int milliseconds);
    static void setEasingCurve(const QEasingCurve& curve);
    /// 清除运行时覆盖，回到当前主题配置里的动效设置。
    static void clearOverrides();
    /// 是否有运行时覆盖（设置界面可以用它显示「已自定义」）。
    static bool hasOverrides();

    // ---- 缓动曲线名 ↔ QEasingCurve ----
    //
    // 名字与 QEasingCurve::Type 的枚举名一致（"OutCubic" / "InOutSine" ...），
    // 既用于解析主题配置，也可以直接拿来填设置界面的下拉框。

    static QString easingName(const QEasingCurve& curve);
    /// 解析失败时返回默认曲线并把 `ok` 置为 false（`ok` 允许为空）。
    static QEasingCurve easingFromName(const QString& name, bool* ok = nullptr);
    static QStringList easingNames();

    // ---- 套用到动画 ----

    /// 把当前生效的时长与缓动曲线设置到 `animation` 上。
    ///
    /// 返回 true 表示已经配置好，调用方接着设 start / end 值并 `start()` 即可；
    /// 返回 false 表示当前配置下不该播放动画（开关关闭或时长为 0），
    /// 调用方应当直接跳到终态（而不是 `start()` 一个 0 时长的动画）。
    /// `animation` 为空时同样返回 false。
    ///
    /// 头文件只前置声明 `QPropertyAnimation`，调用方需要自己包含 `<QPropertyAnimation>`。
    static bool prepare(QPropertyAnimation* animation);
};
