#include "qmribbontheme.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QRegularExpression>

namespace {

// 由 source/style/qmribbon.qrc 提供的资源。
// 内置主题（light.json / dark.json）由 QmRibbonThemeMgr 负责读取，
// 这里只管绘制主题时用到的样式表模板。
constexpr auto kStyleSheetResource = ":/qmribbon/themes/ribbon.qss";

} // namespace

// 注意：Q_INIT_RESOURCE 会声明一个 extern 的初始化函数，因此不能写在命名空间里
// （包括匿名命名空间）。静态库里 qrc 生成的初始化函数不会被自动链接进来，
// 必须像这样显式引用一次，否则运行时读不到 :/qmribbon/themes/... 资源。
static void initThemeResources()
{
    static const bool initialized = []() {
        Q_INIT_RESOURCE(qmribbon);
        return true;
    }();
    Q_UNUSED(initialized)
}

namespace {

QByteArray readResource(const char* path)
{
    initThemeResources();

    QFile file(QString::fromLatin1(path));
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }
    return file.readAll();
}

/// 样式表模板只加载一次。
const QString& styleSheetTemplate()
{
    static const QString sheet = QString::fromUtf8(readResource(kStyleSheetResource));
    return sheet;
}

} // namespace

QmRibbonTheme QmRibbonTheme::fromJson(const QByteArray& json, QString* error)
{
    QmRibbonTheme theme;

    QJsonParseError parse_error {};
    const QJsonDocument document = QJsonDocument::fromJson(json, &parse_error);
    if (document.isNull() || !document.isObject()) {
        if (error != nullptr) {
            *error = QStringLiteral("主题配置不是合法的 JSON 对象：%1").arg(parse_error.errorString());
        }
        return theme;
    }

    const QJsonObject root = document.object();
    theme.name_ = root.value(QStringLiteral("name")).toString();
    theme.dark_ = root.value(QStringLiteral("dark")).toBool(false);

    // 动效段：三个键都可以省略，缺项用默认值。
    const QJsonObject animation = root.value(QStringLiteral("animation")).toObject();
    if (!animation.isEmpty()) {
        theme.animation_enabled_ =
            animation.value(QStringLiteral("enabled")).toBool(QmRibbonAnimationUtil::defaultEnabled);

        const QJsonValue duration = animation.value(QStringLiteral("duration"));
        if (!duration.isUndefined()) {
            if (duration.isDouble()) {
                theme.setAnimationDuration(duration.toInt());
            } else if (error != nullptr) {
                *error = QStringLiteral("animation.duration 必须是数字（毫秒）");
            }
        }

        const QJsonValue easing = animation.value(QStringLiteral("easing"));
        if (!easing.isUndefined()) {
            if (easing.isString()) {
                bool ok = true;
                const QEasingCurve curve = QmRibbonAnimationUtil::easingFromName(easing.toString(), &ok);
                if (ok) {
                    theme.animation_easing_ = curve;
                } else if (error != nullptr) {
                    *error = QStringLiteral("animation.easing 不是已知的缓动曲线：%1").arg(easing.toString());
                }
            } else if (error != nullptr) {
                *error = QStringLiteral("animation.easing 必须是字符串，例如 \"OutCubic\"");
            }
        }
    }

    const QJsonObject colors = root.value(QStringLiteral("colors")).toObject();
    for (auto it = colors.begin(); it != colors.end(); ++it) {
        const QColor color(it.value().toString());
        if (color.isValid()) {
            theme.colors_.insert(it.key(), color);
        } else if (error != nullptr) {
            *error = QStringLiteral("颜色值非法：%1 = %2").arg(it.key(), it.value().toString());
        }
    }

    // 图标资源段：可以整个省略（省略时用 asset() 里的内置默认值）。
    const QJsonObject assets = root.value(QStringLiteral("assets")).toObject();
    for (auto it = assets.begin(); it != assets.end(); ++it) {
        const QString path = it.value().toString();
        if (!path.isEmpty()) {
            theme.assets_.insert(it.key(), path);
        } else if (error != nullptr) {
            *error = QStringLiteral("图标资源路径非法：%1").arg(it.key());
        }
    }

    if (theme.name_.isEmpty()) {
        theme.name_ = QStringLiteral("Unnamed");
    }
    if (theme.colors_.isEmpty() && error != nullptr) {
        *error = QStringLiteral("主题配置里没有有效的 colors");
    }

    return theme;
}

QmRibbonTheme QmRibbonTheme::fromFile(const QString& file_path, QString* error)
{
    // 先确保库内置的 qrc 已初始化：这样 `:/qmribbon/themes/*.json` 这类资源路径
    // 也能直接用（内置主题就是从这里读的，静态库下必须显式引用一次初始化函数）。
    initThemeResources();

    QFile file(file_path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error != nullptr) {
            *error = QStringLiteral("无法打开主题文件：%1").arg(file_path);
        }
        return {};
    }
    return fromJson(file.readAll(), error);
}

QStringList QmRibbonTheme::colorKeys()
{
    // 直接从样式表模板里推导需要的配色键，避免两处维护
    //（图标键 `@{chevronDown}` 之类不算配色，见 assetKeys()）。
    static const QStringList keys = []() {
        const QStringList assets = assetKeys();

        QStringList result;
        static const QRegularExpression placeholder(QStringLiteral("@\\{([A-Za-z0-9_]+)\\}"));
        auto it = placeholder.globalMatch(styleSheetTemplate());
        while (it.hasNext()) {
            const QString key = it.next().captured(1);
            if (!result.contains(key) && !assets.contains(key)) {
                result.append(key);
            }
        }
        return result;
    }();
    return keys;
}

QStringList QmRibbonTheme::assetKeys()
{
    return { QStringLiteral("chevronDown"), QStringLiteral("chevronUp") };
}

QString QmRibbonTheme::asset(const QString& key) const
{
    // 主题配置里显式给了就用它，否则按明暗挑内置的那一份。
    if (const QString custom = assets_.value(key); !custom.isEmpty()) {
        return custom;
    }

    const QString suffix = dark_ ? QStringLiteral("-dark.svg") : QStringLiteral("-light.svg");

    if (key == QStringLiteral("chevronDown")) {
        return QStringLiteral(":/qmribbon/images/chevron-down") + suffix;
    }
    if (key == QStringLiteral("chevronUp")) {
        return QStringLiteral(":/qmribbon/images/chevron-up") + suffix;
    }

    return {};
}

bool QmRibbonTheme::isValid() const
{
    return !colors_.isEmpty();
}

QString QmRibbonTheme::name() const
{
    return name_;
}

bool QmRibbonTheme::isDark() const
{
    return dark_;
}

QColor QmRibbonTheme::color(const QString& key) const
{
    return colors_.value(key);
}

void QmRibbonTheme::setColor(const QString& key, const QColor& color)
{
    colors_.insert(key, color);
}

QString QmRibbonTheme::styleSheet() const
{
    return resolveStyleSheet(styleSheetTemplate());
}

QString QmRibbonTheme::resolveStyleSheet(const QString& template_text) const
{
    QString sheet = template_text;
    if (sheet.isEmpty()) {
        return sheet;
    }

    for (auto it = colors_.cbegin(); it != colors_.cend(); ++it) {
        sheet.replace(QStringLiteral("@{%1}").arg(it.key()), it.value().name(QColor::HexRgb));
    }

    // 图标资源：模板里写成 `image: url(@{chevronDown})`，这里替换成明暗对应的 SVG 路径。
    for (const QString& key : assetKeys()) {
        const QString path = asset(key);
        if (!path.isEmpty()) {
            sheet.replace(QStringLiteral("@{%1}").arg(key), path);
        }
    }

    // 主题配置缺项时清掉剩下的占位符，避免样式表里出现非法值。
    static const QRegularExpression unresolved(QStringLiteral("@\\{[A-Za-z0-9_]+\\}"));
    if (sheet.contains(unresolved)) {
        sheet.replace(unresolved, QStringLiteral("transparent"));
    }

    return sheet;
}

QColor QmRibbonTheme::accent() const
{
    return color(QStringLiteral("accent"));
}

QColor QmRibbonTheme::accentHover() const
{
    return color(QStringLiteral("accentHover"));
}

QColor QmRibbonTheme::onAccent() const
{
    return color(QStringLiteral("onAccent"));
}

QColor QmRibbonTheme::danger() const
{
    return color(QStringLiteral("danger"));
}

QColor QmRibbonTheme::windowBackground() const
{
    return color(QStringLiteral("windowBackground"));
}

QColor QmRibbonTheme::contentBackground() const
{
    return color(QStringLiteral("contentBackground"));
}

QColor QmRibbonTheme::text() const
{
    return color(QStringLiteral("text"));
}

QColor QmRibbonTheme::secondaryText() const
{
    return color(QStringLiteral("secondaryText"));
}

QColor QmRibbonTheme::mutedText() const
{
    return color(QStringLiteral("mutedText"));
}

QColor QmRibbonTheme::disabledText() const
{
    return color(QStringLiteral("disabledText"));
}

QColor QmRibbonTheme::border() const
{
    return color(QStringLiteral("border"));
}

QColor QmRibbonTheme::borderStrong() const
{
    return color(QStringLiteral("borderStrong"));
}

QColor QmRibbonTheme::borderHover() const
{
    return color(QStringLiteral("borderHover"));
}

QColor QmRibbonTheme::hover() const
{
    return color(QStringLiteral("hover"));
}

QColor QmRibbonTheme::pressed() const
{
    return color(QStringLiteral("pressed"));
}

QColor QmRibbonTheme::buttonChecked() const
{
    return color(QStringLiteral("buttonChecked"));
}

QColor QmRibbonTheme::buttonCheckedBorder() const
{
    return color(QStringLiteral("buttonCheckedBorder"));
}

QColor QmRibbonTheme::surface() const
{
    return color(QStringLiteral("surface"));
}

QColor QmRibbonTheme::surfaceBorder() const
{
    return color(QStringLiteral("surfaceBorder"));
}

QColor QmRibbonTheme::surfaceHover() const
{
    return color(QStringLiteral("surfaceHover"));
}

QColor QmRibbonTheme::separator() const
{
    return color(QStringLiteral("separator"));
}

QColor QmRibbonTheme::inputBackground() const
{
    return color(QStringLiteral("inputBackground"));
}

QColor QmRibbonTheme::navChecked() const
{
    return color(QStringLiteral("navChecked"));
}

QColor QmRibbonTheme::ribbonShadow() const
{
    return color(QStringLiteral("ribbonShadow"));
}

QPalette QmRibbonTheme::palette() const
{
    QPalette result;

    // 常规状态
    result.setColor(QPalette::Window, windowBackground());
    result.setColor(QPalette::WindowText, text());
    result.setColor(QPalette::Base, inputBackground());
    result.setColor(QPalette::AlternateBase, surface());
    result.setColor(QPalette::Text, text());
    result.setColor(QPalette::Button, windowBackground());
    result.setColor(QPalette::ButtonText, text());
    result.setColor(QPalette::Highlight, accent());
    result.setColor(QPalette::HighlightedText, onAccent());
    result.setColor(QPalette::ToolTipBase, surface());
    result.setColor(QPalette::ToolTipText, text());
    result.setColor(QPalette::PlaceholderText, mutedText());
    result.setColor(QPalette::Link, accent());

    // 有些样式会用到这几个角色；ADS 也明确用到了：
    //   palette(light) → 内容底色，palette(dark) → 分隔线，palette(mid) → 描边。
    result.setColor(QPalette::Light, contentBackground());
    result.setColor(QPalette::Dark, border());
    result.setColor(QPalette::Mid, surfaceBorder());
    result.setColor(QPalette::Midlight, hover());
    result.setColor(QPalette::Shadow, borderStrong());

    // 禁用态
    result.setColor(QPalette::Disabled, QPalette::WindowText, disabledText());
    result.setColor(QPalette::Disabled, QPalette::Text, disabledText());
    result.setColor(QPalette::Disabled, QPalette::ButtonText, disabledText());
    result.setColor(QPalette::Disabled, QPalette::Highlight, hover());
    result.setColor(QPalette::Disabled, QPalette::HighlightedText, mutedText());

    return result;
}

bool QmRibbonTheme::isAnimationEnabled() const
{
    return animation_enabled_;
}

void QmRibbonTheme::setAnimationEnabled(bool enabled)
{
    animation_enabled_ = enabled;
}

int QmRibbonTheme::animationDuration() const
{
    return animation_duration_;
}

void QmRibbonTheme::setAnimationDuration(int milliseconds)
{
    // 0 表示不要动画；上限只是防止配置里误写一个夸张的值。
    animation_duration_ = qBound(0, milliseconds, QmRibbonAnimationUtil::maxDuration);
}

QEasingCurve QmRibbonTheme::animationEasingCurve() const
{
    return animation_easing_;
}

void QmRibbonTheme::setAnimationEasingCurve(const QEasingCurve& curve)
{
    animation_easing_ = curve;
}
