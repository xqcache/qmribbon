#pragma once

#include "qmribbon_global.h"

#include <QWidget>

class QMainWindow;
class QmRibbonPage;
class QmRibbonTabBar;
class QmRibbonTitleBar;

struct QmRibbonPrivate;
class QMRIBBON_API QmRibbon : public QWidget {
    Q_OBJECT
public:
    enum Feature {
        NoRibbonButton = 0x01,
        NoUserInfoButton = 0x02,
        NoQuickAccessToolBar = 0x04,
        NoApplicationButton = 0x08,
        NoApplicationLogo = 0x10,
        NoDefaultStyle = 0x11,
    };
    Q_ENUM(Feature)
    Q_DECLARE_FLAGS(Features, Feature)

    explicit QmRibbon(QWidget* parent = nullptr, Features features = Features());
    ~QmRibbon() noexcept override;

    static QmRibbon* install(QMainWindow* window);

    QmRibbonTitleBar* titleBar() const;
    QmRibbonTabBar* tabBar() const;
    QmRibbonPage* addPage(const QString& title, const QIcon& icon = QIcon());

    void setWindow(QMainWindow* window);
    void setFeature(Feature feature, bool on = true);
    void setFeatures(Features features, bool on = true);

protected:
    bool event(QEvent* event) override;

private:
    void initUi();
    void connectSignals();

private:
    QmRibbonPrivate* d_ { nullptr };
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QmRibbon::Features)
