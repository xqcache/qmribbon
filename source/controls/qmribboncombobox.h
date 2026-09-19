#pragma once

#include "qmribbonexport.h"

#include <QComboBox>
#include <QFont>

/// Ribbon 组合框（下拉框）。
///
/// 继承 QComboBox，只做两件 QComboBox 本身没有的事：
///   1. **外观**：直角、1px 细边框、右侧带分隔线的下拉按钮区，箭头由控件自绘
///      （这样箭头颜色跟着主题的文字色走，不需要额外的图标资源）；
///   2. **Word 风格的弹出列表**：支持分组标题、右侧灰色标注，以及「用项自己的字体渲染」
///      —— 也就是 Word「字体」/「字号」那两个框的样子。
///
/// 配色与尺寸走主题：`#RibbonComboBox` 在 `resources/themes/ribbon.qss` 里，
/// 列表项由内部委托用 `QmRibbonThemeMgr::current()` 自绘。
///
/// 字体列表的例子：
/// ```cpp
/// auto* combo = new QmRibbonComboBox;
/// combo->setEditable(true);
///
/// combo->addGroupHeader(QStringLiteral("主题字体"));
/// combo->addEntry(QStringLiteral("等线 Light"), QStringLiteral("(标题)"), QFont(QStringLiteral("等线 Light")));
/// combo->addEntry(QStringLiteral("等线"), QStringLiteral("(正文)"), QFont(QStringLiteral("等线")));
///
/// combo->addGroupHeader(QStringLiteral("所有字体"));
/// combo->addEntry(QStringLiteral("Arial"), QString(), QFont(QStringLiteral("Arial")));
/// ```
///
/// 分组标题只是「不可选中的一行」：鼠标与键盘都会跳过它，
/// 点击它不会改变 `currentIndex()`（用 `isGroupHeader()` 可以判断某项是不是标题）。
class QMRIBBON_EXPORT QmRibbonComboBox : public QComboBox {
    Q_OBJECT

public:
    /// 自定义项数据角色（用于 `setItemData()` / 内部委托读取）。
    enum ItemRole {
        /// QString：显示在右侧的灰色标注，例如 `"(正文)"`。
        AnnotationRole = Qt::UserRole + 1,
        /// bool：该项是分组标题。
        HeaderRole = Qt::UserRole + 2,
    };

    explicit QmRibbonComboBox(QWidget* parent = nullptr);

    /// 追加一个分组标题：小号加粗文字 + 一条下边分隔线，不可选中、不可点击。
    void addGroupHeader(const QString& title);

    /// 追加一个条目；`annotation` 可以留空（右侧灰色小字，如 `"(正文)"`）。
    void addEntry(const QString& text, const QString& annotation = QString());

    /// 追加一个条目，并用 `font` 渲染这一项（字体列表就是这么做的）。
    void addEntry(const QString& text, const QString& annotation, const QFont& font);

    void setItemAnnotation(int index, const QString& annotation);
    QString itemAnnotation(int index) const;

    /// 用 `font` 渲染该项；传默认构造的 QFont() 表示清除。
    void setItemFont(int index, const QFont& font);

    bool isGroupHeader(int index) const;

    /// 弹出列表的最小宽度；0（默认）表示「完全按内容算」。
    ///
    /// 内容宽度 = 最宽的一条（文字 + 右侧标注 + 内边距），并且**不会比组合框本身窄**：
    /// 弹出列表比输入框宽是正常的 —— Word 的字体框就是这样。
    void setPopupMinimumWidth(int width);
    int popupMinimumWidth() const;

    /// 重写只为了在弹出前按内容重算列表宽度（见 `setPopupMinimumWidth()`）。
    void showPopup() override;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void init();

    /// 弹出列表的「内容宽度」：取所有条目里最宽的那条。
    /// 宽度复用列表委托的 `sizeHint()` 计算，避免「文字 + 标注 + 内边距」两处维护。
    int popupContentWidth() const;

    /// 弹出列表的最小宽度，0 表示只按内容算。
    int popup_minimum_width_ { 0 };
};
