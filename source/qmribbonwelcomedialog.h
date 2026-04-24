#pragma once

#include "qmframelessdialog.h"
#include "qmribbon_global.h"
#include "qmribbonwelcomepage.h"

#include <QDialog>

class QmRibbon;
class QmRibbonWelcomePage;

struct QmRibbonWelcomeDialogPrivate;

class QMRIBBON_API QmRibbonWelcomeDialog : public QmFramelessDialog {
    Q_OBJECT
public:
    explicit QmRibbonWelcomeDialog(QmRibbon* ribbon);
    ~QmRibbonWelcomeDialog() noexcept override;

    int execOverMainWindow(bool first_show = false);
    void setPage(QmRibbonWelcomePage* page);

protected:
    void showEvent(QShowEvent* event) override;
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void initUi();
    void connectSignals();

private:
    QmRibbonWelcomeDialogPrivate* d_ { nullptr };
};
