#pragma once

#include "qmribbon_global.h"

#include <QWidget>

class QmRibbonSection;

struct QmRibbonPagePrivate;
class QMRIBBON_API QmRibbonPage : public QWidget {
    Q_OBJECT
public:
    explicit QmRibbonPage(QWidget* parent = nullptr);
    QmRibbonPage(const QString& title, QWidget* parent = nullptr);
    QmRibbonPage(const QString& title, const QIcon& icon, QWidget* parent = nullptr);
    ~QmRibbonPage() noexcept override;

    void setTitle(const QString& title);
    void setIcon(const QIcon& icon);

    QmRibbonSection* addSection(const QString& title, const QIcon& icon = QIcon());

private:
    QmRibbonPagePrivate* d_ { nullptr };
};