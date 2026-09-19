#pragma once

#include "qmribbonexport.h"

#include <QToolButton>

class QAction;

/// Ribbon 命令按钮。
///
/// 基于 QToolButton 的轻量封装，只负责表达 Ribbon 语义（尺寸与排列方式），
/// 外观统一交给 QmRibbonTheme。
class QMRIBBON_EXPORT QmRibbonButton : public QToolButton {
    Q_OBJECT

public:
    enum class Size {
        Large,  ///< 图标在上、文字在下，占满 Group 内容区高度
        Medium, ///< 与 Large 类似，但更紧凑
        Small   ///< 图标在左、文字在右，可三行堆叠
    };
    Q_ENUM(Size)

    Q_PROPERTY(Size buttonSize READ size WRITE setSize)

    explicit QmRibbonButton(QWidget* parent = nullptr);
    explicit QmRibbonButton(QAction* action, Size size = Size::Small, QWidget* parent = nullptr);

    Size size() const;
    void setSize(Size size);

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

private:
    void init();
    void applySize();

    Size size_ { Size::Small };
};
