#pragma once

#include "qmribbon_global.h"

#include <QWidget>

class QmRibbonWelcomeDialog;
class QmRibbon;

class QMRIBBON_API QmRibbonWelcomePage : public QWidget {
    Q_OBJECT
public:
    explicit QmRibbonWelcomePage(QWidget* parent = nullptr);
    ~QmRibbonWelcomePage() noexcept override;

    virtual void setAsFirstShow(bool first_show);

    QmRibbonWelcomeDialog* welcomeDialog() const;
    QmRibbon* ribbon() const;
    void closeBackstage();
};
