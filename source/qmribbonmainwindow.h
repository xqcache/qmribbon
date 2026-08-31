#pragma once

#include <QWidget>

class QmRibbon;

struct QmRibbonMainWindowPrivate;

class QmRibbonMainWindow : public QWidget {
    Q_OBJECT

public:
    explicit QmRibbonMainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~QmRibbonMainWindow() noexcept override;

    virtual void setCentralWidget(QWidget* widget);
    QWidget* centralWidget() const;

    QmRibbon* ribbon() const;

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void initUi();
    void connectSignals();

private:
    QmRibbonMainWindowPrivate* d_ { nullptr };
};