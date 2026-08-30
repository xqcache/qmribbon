#pragma once

#include <QWidget>

struct QmRibbonMainWindowPrivate;

class QmRibbonMainWindow : public QWidget {
    Q_OBJECT

public:
    explicit QmRibbonMainWindow(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    ~QmRibbonMainWindow() noexcept override;

    virtual void setCentralWidget(QWidget* widget);
    QWidget* centralWidget() const;

private:
    void initUi();
    void connectSignals();

private:
    QmRibbonMainWindowPrivate* d_ { nullptr };
};