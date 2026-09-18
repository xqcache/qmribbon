#include "qmribbonanimationutil.h"

#include "qmribbontheme.h"
#include "qmribbonthememgr.h"

#include <QPropertyAnimation>

#include <iterator>
#include <optional>

namespace {

/// 配置文件里能写的缓动曲线名（`"easing": "OutCubic"`）与 QEasingCurve 的对应表。
/// 名称与 QEasingCurve::Type 的枚举名一致，方便对照 Qt 文档。
struct EasingCurveEntry {
    const char* name;
    QEasingCurve::Type type;
};

constexpr EasingCurveEntry kEasingCurves[] = {
    { "Linear", QEasingCurve::Linear },
    { "InQuad", QEasingCurve::InQuad },
    { "OutQuad", QEasingCurve::OutQuad },
    { "InOutQuad", QEasingCurve::InOutQuad },
    { "OutInQuad", QEasingCurve::OutInQuad },
    { "InCubic", QEasingCurve::InCubic },
    { "OutCubic", QEasingCurve::OutCubic },
    { "InOutCubic", QEasingCurve::InOutCubic },
    { "OutInCubic", QEasingCurve::OutInCubic },
    { "InQuart", QEasingCurve::InQuart },
    { "OutQuart", QEasingCurve::OutQuart },
    { "InOutQuart", QEasingCurve::InOutQuart },
    { "OutInQuart", QEasingCurve::OutInQuart },
    { "InQuint", QEasingCurve::InQuint },
    { "OutQuint", QEasingCurve::OutQuint },
    { "InOutQuint", QEasingCurve::InOutQuint },
    { "OutInQuint", QEasingCurve::OutInQuint },
    { "InSine", QEasingCurve::InSine },
    { "OutSine", QEasingCurve::OutSine },
    { "InOutSine", QEasingCurve::InOutSine },
    { "OutInSine", QEasingCurve::OutInSine },
    { "InExpo", QEasingCurve::InExpo },
    { "OutExpo", QEasingCurve::OutExpo },
    { "InOutExpo", QEasingCurve::InOutExpo },
    { "OutInExpo", QEasingCurve::OutInExpo },
    { "InCirc", QEasingCurve::InCirc },
    { "OutCirc", QEasingCurve::OutCirc },
    { "InOutCirc", QEasingCurve::InOutCirc },
    { "OutInCirc", QEasingCurve::OutInCirc },
    { "InElastic", QEasingCurve::InElastic },
    { "OutElastic", QEasingCurve::OutElastic },
    { "InOutElastic", QEasingCurve::InOutElastic },
    { "OutInElastic", QEasingCurve::OutInElastic },
    { "InBack", QEasingCurve::InBack },
    { "OutBack", QEasingCurve::OutBack },
    { "InOutBack", QEasingCurve::InOutBack },
    { "OutInBack", QEasingCurve::OutInBack },
    { "InBounce", QEasingCurve::InBounce },
    { "OutBounce", QEasingCurve::OutBounce },
    { "InOutBounce", QEasingCurve::InOutBounce },
    { "OutInBounce", QEasingCurve::OutInBounce },
    { "InCurve", QEasingCurve::InCurve },
    { "OutCurve", QEasingCurve::OutCurve },
    { "SineCurve", QEasingCurve::SineCurve },
    { "CosineCurve", QEasingCurve::CosineCurve },
    { "BezierSpline", QEasingCurve::BezierSpline },
    { "TCBSpline", QEasingCurve::TCBSpline },
    { "Custom", QEasingCurve::Custom },
};

// 运行时覆盖：空表示用当前主题配置里的值。
std::optional<bool> g_enabled_override;
std::optional<int> g_duration_override;
std::optional<QEasingCurve> g_easing_override;

} // namespace

QString QmRibbonAnimationUtil::defaultEasingName()
{
    return QStringLiteral("OutCubic");
}

bool QmRibbonAnimationUtil::isEnabled()
{
    if (g_enabled_override.has_value()) {
        return *g_enabled_override;
    }
    return QmRibbonThemeMgr::current().isAnimationEnabled();
}

int QmRibbonAnimationUtil::duration()
{
    if (g_duration_override.has_value()) {
        return *g_duration_override;
    }
    return QmRibbonThemeMgr::current().animationDuration();
}

QEasingCurve QmRibbonAnimationUtil::easingCurve()
{
    if (g_easing_override.has_value()) {
        return *g_easing_override;
    }
    return QmRibbonThemeMgr::current().animationEasingCurve();
}

bool QmRibbonAnimationUtil::shouldAnimate()
{
    return isEnabled() && duration() > 0;
}

void QmRibbonAnimationUtil::setEnabled(bool enabled)
{
    g_enabled_override = enabled;
}

void QmRibbonAnimationUtil::setDuration(int milliseconds)
{
    // 0 表示不要动画；上限只是防止误传一个夸张的值。
    g_duration_override = qBound(0, milliseconds, maxDuration);
}

void QmRibbonAnimationUtil::setEasingCurve(const QEasingCurve& curve)
{
    g_easing_override = curve;
}

void QmRibbonAnimationUtil::clearOverrides()
{
    g_enabled_override.reset();
    g_duration_override.reset();
    g_easing_override.reset();
}

bool QmRibbonAnimationUtil::hasOverrides()
{
    return g_enabled_override.has_value() || g_duration_override.has_value() || g_easing_override.has_value();
}

QString QmRibbonAnimationUtil::easingName(const QEasingCurve& curve)
{
    for (const EasingCurveEntry& entry : kEasingCurves) {
        if (entry.type == curve.type()) {
            return QString::fromLatin1(entry.name);
        }
    }
    return defaultEasingName();
}

QEasingCurve QmRibbonAnimationUtil::easingFromName(const QString& name, bool* ok)
{
    for (const EasingCurveEntry& entry : kEasingCurves) {
        // 大小写不敏感：配置文件里写 "outcubic" 也认。
        if (name.compare(QString::fromLatin1(entry.name), Qt::CaseInsensitive) == 0) {
            if (ok != nullptr) {
                *ok = true;
            }
            return QEasingCurve(entry.type);
        }
    }

    if (ok != nullptr) {
        *ok = false;
    }
    return QEasingCurve(QEasingCurve::OutCubic);
}

QStringList QmRibbonAnimationUtil::easingNames()
{
    static const QStringList names = []() {
        QStringList result;
        result.reserve(int(std::size(kEasingCurves)));
        for (const EasingCurveEntry& entry : kEasingCurves) {
            result.append(QString::fromLatin1(entry.name));
        }
        return result;
    }();
    return names;
}

bool QmRibbonAnimationUtil::prepare(QPropertyAnimation* animation)
{
    if (animation == nullptr || !shouldAnimate()) {
        return false;
    }

    animation->setDuration(duration());
    animation->setEasingCurve(easingCurve());
    return true;
}
