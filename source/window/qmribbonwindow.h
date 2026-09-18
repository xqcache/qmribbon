#pragma once

#include <QWidget>

class QStackedWidget;
class QVBoxLayout;
class QmRibbon;

namespace ads {
class CDockManager;
}

/// Ribbon 主窗口。
///
/// 窗口外观采用 Windows / QWindowKit 的系统方案：**不透明窗口 + 系统厚边框**，
/// Win11 下圆角与阴影由 DWM 提供。框架不再自绘阴影，也不使用半透明背景，原因：
///   - 半透明（layered）窗口会被 DWM 排除在圆角处理之外，窗口没有圆角；
///   - layered 窗口按像素 alpha 做命中测试，alpha=0 的阴影留白会让鼠标消息穿透，
///     破坏 QWindowKit 布置在客户区边缘的 Resize 命中；
///   - 关闭系统边框时 QWindowKit 在最大化状态会把客户区四边内缩 8px，
///     造成"最大化后仍能拖动边缘"。
///
/// 无边框窗口的原生行为全部交给 QWindowKit 的 QWK::WidgetWindowAgent：
/// 标题栏拖动、边缘 Resize、Aero Snap、Windows 11 Snap Layout、双击标题栏最大化。
///
///   QmRibbonWindow
///   ├── QmRibbon
///   └── QStackedWidget（MainView / BackstageView）
///
/// 注意：使用本窗口的应用需要在构造 QApplication **之前**设置
/// `QCoreApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings)`，
/// 这是 QWindowKit 的要求。
class QmRibbonWindow : public QWidget {
    Q_OBJECT

public:
    enum class ViewMode {
        Main,      ///< 正常工作区
        Backstage, ///< File Backstage
    };
    Q_ENUM(ViewMode)

    Q_PROPERTY(ViewMode viewMode READ viewMode WRITE setViewMode NOTIFY viewModeChanged)

    explicit QmRibbonWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~QmRibbonWindow() noexcept override;

    QmRibbon* ribbon() const;
    QStackedWidget* viewStack() const;

    /// 设置主工作区内容。窗口接管 widget 的所有权。
    void setCentralWidget(QWidget* widget);
    QWidget* centralWidget() const;

    /// 设置 Backstage 页面；为 nullptr 时 File 按钮不会切换视图。
    void setBackstageWidget(QWidget* widget);
    QWidget* backstageWidget() const;

    ViewMode viewMode() const;
    void setViewMode(ViewMode mode);

    /// MainView 的停靠系统（Qt Advanced Docking System）。
    ///
    /// 首次调用时创建 `ads::CDockManager` 并把它作为主工作区内容（等价于调用
    /// `setCentralWidget()`）；没有调用过的窗口不会创建任何 ADS 对象。
    ads::CDockManager* dockManager();

    /// 把一个**浮层控件**挂到 MainView 之上（典型用途：浮动工具栏）。
    ///
    /// 控件不参与布局：它被 reparent 到 MainView、按 `position` 摆放并置于最上层，
    /// 因此始终浮在停靠区之上；MainView 尺寸变化时会自动夹回可见区域。
    /// 控件仍归调用方所有（窗口只负责摆放，不接管生命周期）。
    void addFloatingWidget(QWidget* widget, const QPoint& position = QPoint(24, 24));
    /// 取消浮层登记（只是不再由窗口摆放，不会删除控件）。
    void removeFloatingWidget(QWidget* widget);
    QList<QWidget*> floatingWidgets() const;

signals:
    void viewModeChanged(ViewMode mode);

protected:
    bool nativeEvent(const QByteArray& event_type, void* message, qintptr* result) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void setupWindowAgent();
    /// 把当前主题的明暗同步给系统窗口边框（DWM）。
    void updateWindowTheme();
    /// 把当前主题的明暗同步给 ADS 停靠区（它自己有两套样式表 / 两套图标）。
    void updateDockManagerTheme();
    /// Backstage 切换动画：从左侧滑入 / 滑出。
    void animateViewSwitch(ViewMode mode);
    /// 把浮层控件挂到 MainView 上（addFloatingWidget 的内部实现）。
    void registerFloatingWidget(QWidget* widget, const QPoint& position);
    /// MainView 尺寸变化时，把浮层夹回可见区域。
    void clampFloatingWidgets();

    struct QmRibbonWindowPrivate;
    QmRibbonWindowPrivate* d_ { nullptr };
};
