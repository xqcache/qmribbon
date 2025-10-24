#pragma once

#pragma once

#include "qmribbon_global.h"

#include <QWidget>

class QToolBar;
class QToolButton;

struct QmRibbonTitleBarPrivate;
class QMRIBBON_API QmRibbonTitleBar : public QWidget {
    Q_OBJECT
public:
    explicit QmRibbonTitleBar(QWidget* parent = nullptr);
    ~QmRibbonTitleBar() noexcept override;

    // 左上角快速访问
    QToolBar* quickAccessToolBar() const;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void toggleWindowMaximized();

private:
    void initUi();
    void initWindowButtons();
    void initStyleSheetKey();
    void connectSignals();

private:
    QmRibbonTitleBarPrivate* d_ { nullptr };
};
