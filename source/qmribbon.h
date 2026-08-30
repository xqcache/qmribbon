#pragma once

#include <QWidget>

class QmRibbonPage;

struct QmRibbonPrivate;

class QmRibbon : public QWidget {
    Q_OBJECT
public:
    explicit QmRibbon(QWidget* parent = nullptr);
    ~QmRibbon() noexcept override;

    QmRibbonPage *addPage(const QString &title);


private:
    void initUi();
    void connectSignals();

private:
    QmRibbonPrivate* d_ { nullptr };
};
