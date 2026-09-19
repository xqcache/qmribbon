#pragma once

#include "qmribbonexport.h"

#include <QWidget>

class QStackedWidget;
class QmRibbonPage;
class QmRibbonQuickAccessBar;
class QmRibbonTabBar;
class QmRibbonTitleBar;

/// Ribbon 整体容器。
///
/// 结构：
///   QmRibbon
///   ├── QmRibbonTitleBar
///   ├── QmRibbonTabBar
///   └── QStackedWidget（Ribbon Page Stack）
class QMRIBBON_EXPORT QmRibbon : public QWidget {
    Q_OBJECT

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)

public:
    /// Ribbon 显示模式。
    enum class DisplayMode {
        Expanded, ///< 展开：显示 Tab 栏与页面
        TabsOnly  ///< 折叠：只显示 TitleBar 与 Tab 栏
    };
    Q_ENUM(DisplayMode)

    explicit QmRibbon(QWidget* parent = nullptr);
    ~QmRibbon() noexcept override;

    QmRibbonTitleBar* titleBar() const;
    QmRibbonTabBar* tabBar() const;
    QmRibbonQuickAccessBar* quickAccessBar() const;
    QStackedWidget* pageStack() const;

    /// 添加一个 Ribbon Page，同时创建对应的 Tab。
    QmRibbonPage* addPage(const QString& title);

    int pageCount() const;
    QmRibbonPage* pageAt(int index) const;
    QmRibbonPage* currentPage() const;

    int currentIndex() const;
    void setCurrentIndex(int index);
    void setCurrentPage(QmRibbonPage* page);

    DisplayMode displayMode() const;
    void setDisplayMode(DisplayMode mode);
    bool isCollapsed() const;

    /// 只保留 TitleBar（隐藏 Tab 栏与 Page），用于 Backstage 模式。
    bool isTitleBarOnly() const;
    void setTitleBarOnly(bool title_bar_only);

signals:
    void currentIndexChanged(int index);
    void currentPageChanged(QmRibbonPage* page);
    void displayModeChanged(DisplayMode mode);
    /// File 按钮被点击（用于进入 Backstage View）。
    void fileButtonClicked();

protected:
    /// 绘制 Ribbon 底色与下边沿的阴影带（分隔 Ribbon 与 MainView）。
    void paintEvent(QPaintEvent* event) override;

private:
    /// 根据 display_mode / title_bar_only 统一刷新子控件可见性。
    void updateVisibility();
    /// 应用当前的显示模式；animate 为 true 时播放折叠 / 展开动画。
    void applyDisplayMode(bool animate);
    /// 展开状态下页面区域应有的高度。
    int expandedPageHeight() const;

    struct QmRibbonPrivate;
    QmRibbonPrivate* d_ { nullptr };
};
