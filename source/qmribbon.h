#pragma once

#include "qmribbon_global.h"

#include <QWidget>

class QToolButton;
class QMainWindow;
class QmRibbonPage;
class QmRibbonWelcomeDialog;
class QmRibbonWelcomePage;
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
        NoDefaultTitleBar = 0x20,
        NoDefaultStyle = 0x40,
    };
    Q_ENUM(Feature)
    Q_DECLARE_FLAGS(Features, Feature)

    explicit QmRibbon(QWidget* parent = nullptr, Features features = Features());
    ~QmRibbon() noexcept override;

    static QmRibbon* install(QMainWindow* window, Features features = Features());

    QmRibbonTitleBar* titleBar() const;
    QmRibbonTabBar* tabBar() const;
    QmRibbonPage* addPage(const QString& title, const QIcon& icon = QIcon());

    QToolButton* pageButton(QmRibbonPage* page) const;
    QmRibbonPage* findPage(const QString& object_name) const;
    QToolButton* findPageButton(const QString& object_name) const;

    void setMainWindow(QMainWindow* window);
    void setFeature(Feature feature, bool on = true);
    void setFeatures(Features features, bool on = true);

    QMainWindow* mainWindow() const;

signals:
    void mainWindowChanged(QMainWindow* win);
    void enterBackstageView();

protected:
    bool event(QEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;

private:
    void initUi();
    void connectSignals();
    void applySystemTheme();

private slots:
    void onContainerFloatingRequested();

private:
    QmRibbonPrivate* d_ { nullptr };
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QmRibbon::Features)
