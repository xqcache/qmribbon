#pragma once

#include <QAbstractButton>
#include <QColor>

/// Ribbon Tab（Home / Insert / View ...）。
///
/// 使用 QAbstractButton 自绘，以获得 Word 风格的下划线选中态与 Contextual Tab 配色。
class QmRibbonTab : public QAbstractButton {
    Q_OBJECT

    Q_PROPERTY(QString contextualGroup READ contextualGroup WRITE setContextualGroup)
    Q_PROPERTY(QColor contextualColor READ contextualColor WRITE setContextualColor)

public:
    explicit QmRibbonTab(QWidget* parent = nullptr);
    explicit QmRibbonTab(const QString& text, QWidget* parent = nullptr);

    /// Contextual Tab 所属分组名（例如 "Picture Tools"），空表示普通 Tab。
    QString contextualGroup() const;
    void setContextualGroup(const QString& group);

    /// Contextual Tab 的标识色。
    QColor contextualColor() const;
    void setContextualColor(const QColor& color);

    bool isContextual() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QString contextual_group_;
    QColor contextual_color_;
    bool hovered_ { false };
};
