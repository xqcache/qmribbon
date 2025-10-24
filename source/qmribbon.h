#pragma once

#include "qmribbon_global.h"

#include <QWidget>

class QMainWindow;
class QmRibbonPage;
class QmRibbonTitleBar;

struct QmRibbonPrivate;
class QMRIBBON_API QmRibbon : public QWidget {
    Q_OBJECT
public:
    explicit QmRibbon(QWidget* parent = nullptr);
    ~QmRibbon() noexcept override;

    void setWindow(QMainWindow* window);
    static QmRibbon* install(QMainWindow* window);

    QmRibbonTitleBar* titleBar() const;

    QmRibbonPage* addPage(const QString& title, const QIcon& icon = QIcon());

private:
    void initUi();
    bool event(QEvent* event) override;

    void updateWidgetGeometry();

private:
    QmRibbonPrivate* d_ { nullptr };
};