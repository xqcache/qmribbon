#pragma once

#include "qmribbon_global.h"
#include "qmribbonsection.h"

#include <QWidget>

class QmRibbonSection;

struct QmRibbonPagePrivate;
class QMRIBBON_API QmRibbonPage : public QWidget {
    Q_OBJECT
public:
    explicit QmRibbonPage(QWidget* parent = nullptr);
    ~QmRibbonPage() noexcept override;

    QSize sizeHint() const override;

    QmRibbonSection* addSection(const QString& title, const QIcon& icon = QIcon(), QmRibbonSection::Features features = QmRibbonSection::Features());

signals:
    void sectionCreated(QmRibbonSection* section, QPrivateSignal);

private:
    QmRibbonPagePrivate* d_ { nullptr };
};
