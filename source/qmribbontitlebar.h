#pragma once

#include <QWidget>

struct QmRibbonTitleBarPrivate;

class QmRibbonTitleBar : public QWidget {
    Q_OBJECT
public:
    explicit QmRibbonTitleBar(QWidget* parent = nullptr);
    ~QmRibbonTitleBar() noexcept override;

private:
    void initUi();
    void connectSignals();

private:
    QmRibbonTitleBarPrivate* d_;
};
