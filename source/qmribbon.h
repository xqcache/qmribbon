#pragma once

#include "qmribbon_global.h"

#include <QWidget>

class QToolButton;
class QMainWindow;
class QmRibbonPage;
class QmRibbonWelcomeDialog;
class QmRibbonTabBar;
class QmRibbonTitleBar;

struct QmRibbonPrivate;
class QMRIBBON_API QmRibbon : public QWidget {
    Q_OBJECT
public:
    constexpr static auto kRibbonWindowPropName = "QmRibbon-Window";
    constexpr static auto kRibbonPropName = "QmRibbon";

    enum Feature {
        NoRibbonButton = 0x01,
        NoUserInfoButton = 0x02,
        NoQuickAccessToolBar = 0x04,
        NoApplicationButton = 0x08,
        NoApplicationLogo = 0x10,
        NoDefaultStyle = 0x20,
    };
    Q_ENUM(Feature)
    Q_DECLARE_FLAGS(Features, Feature)

    explicit QmRibbon(QWidget* parent = nullptr, Features features = Features());
    ~QmRibbon() noexcept override;

    static QmRibbon* install(QMainWindow* window, Features features = Features());

    QmRibbonTitleBar* titleBar() const;
    QmRibbonTabBar* tabBar() const;
    QmRibbonPage* addPage(const QString& title, const QIcon& icon = QIcon());

    QmRibbonWelcomeDialog* welcomeDialog() const;

    QToolButton* pageButton(QmRibbonPage* page) const;
    QmRibbonPage* findPage(const QString& object_name) const;
    QToolButton* findPageButton(const QString& object_name) const;

    void setMainWindow(QMainWindow* window);
    void setFeature(Feature feature, bool on = true);
    void setFeatures(Features features, bool on = true);

    QMainWindow* mainWindow() const;

signals:
    void mainWindowChanged(QMainWindow* win);

protected:
    bool event(QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void initUi();
    void connectSignals();

private slots:
    void onContainerFloatingRequested();

private:
    QmRibbonPrivate* d_ { nullptr };
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QmRibbon::Features)
