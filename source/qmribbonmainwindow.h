#pragma once

#include "qmribbon.h"
#include "qmribbon_global.h"

#include <QMainWindow>

class QmRibbonBackstageView;

struct QmRibbonMainWindowPrivate;

class QMRIBBON_API QmRibbonMainWindow : public QMainWindow {
    Q_OBJECT
    Q_PROPERTY(bool viewAnimationEnabled READ isViewAnimationEnabled WRITE setViewAnimationEnabled)
public:
    enum class ViewMode {
        MainView,
        BackstageView
    };

    explicit QmRibbonMainWindow(QWidget* parent = nullptr, QmRibbon::Features features = QmRibbon::Features(),
        Qt::WindowFlags flags = Qt::WindowFlags());
    ~QmRibbonMainWindow() noexcept override;

    void setBackstageView(QmRibbonBackstageView* view);
    QmRibbonBackstageView* backstageView() const;

    void showBackstageView();
    void showMainView();

    void setViewAnimationEnabled(bool enabled);
    bool isViewAnimationEnabled() const;

    void setMainView(QWidget* widget);
    QWidget* mainView() const;

    QmRibbon* ribbon() const;

protected:
    void showEvent(QShowEvent* event) override;

private:
    void initUi(QmRibbon::Features features);
    void connectSignals();
    void transitionToView(QWidget* target, ViewMode mode, int direction);
    void setCurrentView(QWidget* target, ViewMode mode);
    void animateRibbonVisibility(bool visible);
    void finishRibbonAnimation();
    void cancelViewTransition();
    void finishViewTransition();

private:
    QmRibbonMainWindowPrivate* d_ { nullptr };
};
