#include "qmribbonthemeswitchmask.h"

#include "qmribbonanimationutil.h"

#include <QEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QPropertyAnimation>

#include <cmath>

struct QmRibbonThemeSwitchMask::QmRibbonThemeSwitchMaskPrivate {
    QPointer<QWidget> target;

    QPixmap before;
    QPixmap after;

    /// 圆心（遮罩坐标）。
    QPoint origin;

    qreal progress { 0.0 };

    QPointer<QPropertyAnimation> animation;
};

QmRibbonThemeSwitchMask::QmRibbonThemeSwitchMask(QWidget* target)
    : QWidget(target)
    , d_(new QmRibbonThemeSwitchMaskPrivate)
{
    d_->target = target;

    setObjectName(QStringLiteral("RibbonThemeSwitchMask"));

    // 过渡期间用户照样能点按钮（也就可能再切一次主题），遮罩不吃鼠标事件。
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    // 整块都由我们画（两张全尺寸快照），告诉 Qt 不必操心底下被盖住的重绘。
    setAttribute(Qt::WA_OpaquePaintEvent, true);

    if (target != nullptr) {
        setGeometry(target->rect());
        d_->origin = target->rect().center();
        target->installEventFilter(this);
    }
}

QmRibbonThemeSwitchMask::~QmRibbonThemeSwitchMask() noexcept
{
    if (QWidget* target = d_->target.data()) {
        target->removeEventFilter(this);
    }

    delete d_;
}

QWidget* QmRibbonThemeSwitchMask::targetWidget() const
{
    return d_->target.data();
}

void QmRibbonThemeSwitchMask::setBeforeSnapshot(const QPixmap& pixmap)
{
    d_->before = pixmap;
    update();
}

QPixmap QmRibbonThemeSwitchMask::beforeSnapshot() const
{
    return d_->before;
}

void QmRibbonThemeSwitchMask::setAfterSnapshot(const QPixmap& pixmap)
{
    d_->after = pixmap;
    update();
}

QPixmap QmRibbonThemeSwitchMask::afterSnapshot() const
{
    return d_->after;
}

void QmRibbonThemeSwitchMask::setOrigin(const QPoint& origin)
{
    if (d_->origin == origin) {
        return;
    }

    d_->origin = origin;
    update();
}

QPoint QmRibbonThemeSwitchMask::origin() const
{
    return d_->origin;
}

qreal QmRibbonThemeSwitchMask::progress() const
{
    return d_->progress;
}

void QmRibbonThemeSwitchMask::setProgress(qreal progress)
{
    const qreal clamped = qBound(0.0, progress, 1.0);
    if (qFuzzyCompare(d_->progress, clamped)) {
        return;
    }

    d_->progress = clamped;
    update();
}

void QmRibbonThemeSwitchMask::start()
{
    QWidget* target = d_->target.data();
    if (target == nullptr) {
        deleteLater();
        return;
    }

    auto* animation = new QPropertyAnimation(this, "progress", this);

    // 动画被关掉（主题配置或运行时覆盖）时不该有遮罩：直接收尾，让窗口自己瞬间切完。
    if (!QmRibbonAnimationUtil::prepare(animation)) {
        emit finished();
        deleteLater();
        return;
    }

    d_->animation = animation;

    setGeometry(target->rect());
    show();
    raise();

    animation->setStartValue(0.0);
    animation->setEndValue(1.0);

    connect(animation, &QPropertyAnimation::finished, this, [this]() {
        emit finished();
        deleteLater();
    });

    animation->start();
}

qreal QmRibbonThemeSwitchMask::maxRadius() const
{
    const QRect area = rect();
    const QPointF center(d_->origin);

    // 四个角落取最远的那个（x / y 各自取最大偏移再求斜边）。
    const qreal dx = qMax(center.x() - area.left(), area.right() - center.x());
    const qreal dy = qMax(center.y() - area.top(), area.bottom() - center.y());

    return std::hypot(dx, dy);
}

void QmRibbonThemeSwitchMask::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    // 两张快照缺一不可（原因见头文件）：没有旧快照就没东西可「擦」，没有新快照就没法自给自足。
    if (d_->before.isNull() || d_->after.isNull()) {
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    // 先铺满新主题当底：遮罩是不透明的，圆里露出的就是它自己画的这一张，
    // 不依赖（也依赖不到）下面那层实时窗口 —— 它会被 Qt 的重绘管理器当成「被不透明兄弟
    // 完全盖住」而根本不重绘。
    painter.drawPixmap(rect(), d_->after);

    if (d_->progress >= 1.0) {
        // 圆已经盖住整块：旧主题没有任何要画的地方。
        return;
    }

    // 旧主题只画在「挖空圆」以外：整块矩形加一个圆，奇偶填充规则让圆变成洞。
    // 用 fillPath + 纹理画刷（而不是 setClipPath + drawPixmap）：两者构造完全相同，
    // 但 setClipPath 的裁剪边缘是硬边的（Qt 不对裁剪路径做抗锯齿），fillPath 是平滑的。
    QPainterPath outside;
    outside.setFillRule(Qt::OddEvenFill);
    outside.addRect(QRectF(rect()));

    const qreal radius = maxRadius() * d_->progress;
    if (radius > 0.0) {
        outside.addEllipse(QPointF(d_->origin), radius, radius);
    }

    painter.fillPath(outside, QBrush(d_->before));
}

bool QmRibbonThemeSwitchMask::eventFilter(QObject* watched, QEvent* event)
{
    QWidget* target = d_->target.data();

    if (target != nullptr && watched == target && event->type() == QEvent::Resize) {
        // 过渡途中窗口变大/变小：遮罩跟着贴满，快照按新尺寸缩放画。
        setGeometry(target->rect());
        update();
    }

    return QWidget::eventFilter(watched, event);
}
