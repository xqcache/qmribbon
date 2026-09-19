#pragma once

#include "qmribbonexport.h"

#include <QPixmap>
#include <QPoint>
#include <QWidget>

class QPaintEvent;

/// 主题切换遮罩：切主题时盖在窗口上的一层「旧主题截图」，中间挖一个不断扩大的圆，
/// 圆里露出的是它自己画的**新主题截图** —— 于是换肤看起来是从中心「擦」过去的，
/// 而不是硬闪一下。
///
/// 用法（`QmRibbonThemeMgr` 内部就是这么用的）：
///
/// ```cpp
/// auto* mask = new QmRibbonThemeSwitchMask(window);   // window 是顶层窗口
/// mask->setBeforeSnapshot(old_snapshot);              // 切主题前 grab()
/// mask->setAfterSnapshot(window->grab());             // 换完主题再 grab()
/// mask->start();                                      // 铺满窗口、播动画、播完自动 deleteLater()
/// ```
///
/// **两张快照都必须给**，不能只给旧快照、指望「圆里露出下面那层实时窗口」：
/// 遮罩带着 `WA_OpaquePaintEvent`，而 Qt 的重绘管理器会把「被不透明兄弟完全盖住」的窗口的
/// 重绘请求直接丢掉（实测：窗口 `update()` 之后紧跟着 `show()` 遮罩，窗口一次 paintEvent
/// 都不会来，连 `repaint()` 也丢），于是下面的窗口一直停在旧主题上，整段动画就是静止的旧
/// 主题 + 播完瞬间硬切，看起来像动画没播。反过来把遮罩改成不透明以外的画法也不行：那样遮罩
/// 每帧都要让 Qt 重绘下面整窗（实测每帧一次整窗 paint），帧开销直接不可接受。
///
/// 几个约定：
///   - 圆心默认是窗口正中，`setOrigin()` 可以改成任意点（例如某个按钮的中心）；
///   - `progress` 0 = 全是旧主题，1 = 全是新主题，动画由 `QmRibbonAnimationUtil` 提供
///     时长与缓动曲线（主题 `animation` 段），关掉动画时 `start()` 直接收尾；
///   - 遮罩对鼠标透明：过渡期间用户照常能点按钮（包括再切一次主题）；
///   - 窗口尺寸变化时遮罩会跟着贴合，两张快照都按新尺寸缩放绘制。
class QMRIBBON_EXPORT QmRibbonThemeSwitchMask : public QWidget {
    Q_OBJECT

    Q_PROPERTY(qreal progress READ progress WRITE setProgress)

public:
    /// `target` 一般传顶层窗口；遮罩是它的子控件，生命周期也跟着它。
    explicit QmRibbonThemeSwitchMask(QWidget* target);
    ~QmRibbonThemeSwitchMask() noexcept override;

    QWidget* targetWidget() const;

    /// 旧主题截图（切换前）。
    void setBeforeSnapshot(const QPixmap& pixmap);
    QPixmap beforeSnapshot() const;

    /// 新主题截图（切换后）。**可选**：不设时挖空圆里露出的是下面的实时窗口。
    void setAfterSnapshot(const QPixmap& pixmap);
    QPixmap afterSnapshot() const;

    /// 圆心（遮罩坐标），默认 `target->rect().center()`。
    void setOrigin(const QPoint& origin);
    QPoint origin() const;

    /// 0 = 全旧主题，1 = 全新主题。
    qreal progress() const;
    void setProgress(qreal progress);

    /// 铺满 target、置于最上层、按主题动效设置播放；播完 emit finished() 并自动 deleteLater()。
    void start();

signals:
    void finished();

protected:
    void paintEvent(QPaintEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    /// 圆心到最远角的距离：progress == 1 时圆正好盖住整块。
    qreal maxRadius() const;

    struct QmRibbonThemeSwitchMaskPrivate;
    QmRibbonThemeSwitchMaskPrivate* d_ { nullptr };
};
