#pragma once

#include "qmframelessdialog.h"
#include "qmribbon_global.h"

#include <QDialog>

class QmRibbon;

struct QmRibbonAppPagePrivate;

class QMRIBBON_API QmRibbonAppPage : public QmFramelessDialog {
    Q_OBJECT
public:
    explicit QmRibbonAppPage(QmRibbon* ribbon);
    ~QmRibbonAppPage() noexcept override;

    QToolButton* backButton() const;

protected:
    void showEvent(QShowEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void initUi();
    void connectSignals();

private:
    QmRibbonAppPagePrivate* d_ { nullptr };
};
