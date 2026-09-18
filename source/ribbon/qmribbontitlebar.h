#pragma once

#include <QWidget>

class QAction;
class QIcon;
class QToolButton;
class QmRibbonQuickAccessBar;

/// Ribbon 标题栏。
///
/// 结构：
///   [AppIcon] [QuickAccessBar]      [WindowTitle]      [Account] [─][□][×]
///
/// 标题栏本身不处理鼠标事件：QmRibbonWindow 会把本控件注册为 QWindowKit 的 titleBar，
/// 其中的可拖动区域、三个窗口按钮以及需要交互的子控件分别由 QWindowKit 声明为
/// HTCAPTION / 系统按钮 / hit-test 可见区域，拖动与双击最大化因此由系统原生完成。
class QmRibbonTitleBar : public QWidget {
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

public:
    explicit QmRibbonTitleBar(QWidget* parent = nullptr);
    ~QmRibbonTitleBar() noexcept override;

    QmRibbonQuickAccessBar* quickAccessBar() const;

    QToolButton* appIconButton() const;
    QToolButton* accountButton() const;
    QToolButton* minimizeButton() const;
    QToolButton* maximizeButton() const;
    QToolButton* closeButton() const;

    void setAppIcon(const QIcon& icon);
    QIcon appIcon() const;

    QString title() const;
    void setTitle(const QString& title);

    /// 在最大化 / 还原之间切换。
    void toggleMaximizeRestore();

signals:
    void titleChanged(const QString& title);

    /// App 图标被点击。
    ///
    /// QmRibbonWindow 会把 appIconButton() 注册为 QWindowKit 的 WindowIcon 系统按钮，
    /// 在 Windows 上点击它由系统弹出窗口系统菜单，因此该信号在 Windows 下不会触发
    /// （其它平台走 Qt 路径时仍然有效）。
    void appIconClicked();

protected:
    // 注意：标题栏的拖动与双击最大化由 QWindowKit 在原生层接管
    // （可拖动区域会被识别为 HTCAPTION），因此这里不再自行处理鼠标事件。
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void updateTitleElide();
    void updateWindowState();

    struct QmRibbonTitleBarPrivate;
    QmRibbonTitleBarPrivate* d_ { nullptr };
};
