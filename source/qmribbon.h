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
    enum Feature {
        NoRibbonButton = 0x01,
        NoUserInfoButton = 0x02,
        NoQuickAccessToolBar = 0x04,
    };
    Q_ENUM(Feature)
    Q_DECLARE_FLAGS(Features, Feature)

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

Q_DECLARE_OPERATORS_FOR_FLAGS(QmRibbon::Features)
