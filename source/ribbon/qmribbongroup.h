#pragma once

#include "qmribbonbutton.h"

#include <QWidget>

class QAction;
class QToolButton;

/// Ribbon Group（Clipboard / Font / Paragraph ...）。
///
/// Group 内部按 Word 的方式组织内容区：
///   - Large / Medium 按钮各自独占一列，占满内容区高度；
///   - Small 按钮以及通过 addWidget() 加入的控件按「列」堆叠，每列最多三行。
///
/// 业务层只需要描述 Ribbon 语义，不需要操作布局。
class QmRibbonGroup : public QWidget {
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

public:
    explicit QmRibbonGroup(QWidget* parent = nullptr);
    explicit QmRibbonGroup(const QString& title, QWidget* parent = nullptr);
    ~QmRibbonGroup() noexcept override;

    QString title() const;
    void setTitle(const QString& text);

    /// 添加 Large 按钮（图标在上、文字在下）。
    QmRibbonButton* addLargeAction(QAction* action);
    /// 添加 Small 按钮（图标在左、文字在右），每三行换一列。
    QmRibbonButton* addAction(QAction* action);
    /// 添加自定义尺寸的按钮。
    QmRibbonButton* addButton(QAction* action, QmRibbonButton::Size size);

    /// 添加任意控件（QComboBox / QLineEdit / QCheckBox / 自定义面板）。
    ///
    /// 控件高度**以固定高度优先**（`setFixedHeight()`），否则用 `sizeHint()`；
    /// 一列最多三行，且总高度不超过 Group 的内容区 —— 放不下会自动另起一列，
    /// 不会把控件裁掉半截。想让原生控件正好占一行，用 `setFixedHeight(22)` 压到行高即可。
    void addWidget(QWidget* widget);

    /// 已加入该 Group 的按钮。
    QList<QmRibbonButton*> buttons() const;
    QmRibbonButton* buttonForAction(QAction* action) const;

    /// Group 右下角的 Dialog Box Launcher（↘）。
    void setLauncherAction(QAction* action);
    QAction* launcherAction() const;
    QToolButton* launcherButton() const;

    bool isEmpty() const;

signals:
    void titleChanged(const QString& text);

private:
    /// 把 item 追加到当前列：列满三行，或与当前列的内容类型不一致时自动另起一列。
    /// is_widget 为 true 表示 item 是通过 addWidget() 加入的任意控件。
    void appendToColumn(QWidget* item, bool is_widget);

    struct QmRibbonGroupPrivate;
    QmRibbonGroupPrivate* d_ { nullptr };
};
