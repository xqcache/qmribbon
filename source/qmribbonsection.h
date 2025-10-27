#pragma once

#include "qmribbon_global.h"

#include <QWidget>

struct QmRibbonSectionPrivate;
class QMRIBBON_API QmRibbonSection : public QWidget {
    Q_OBJECT
public:
    explicit QmRibbonSection(QWidget* parent = nullptr);
    ~QmRibbonSection() noexcept override;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

    void setTitle(const QString& title);
    void setIcon(const QIcon& icon);

    void setWidget(QWidget* widget);

    const QIcon& icon() const;
    QString title() const;

signals:
    void expandButtonClicked();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void initUi();

    bool isWidthSufficient() const;

private:
    QmRibbonSectionPrivate* d_ { nullptr };
};