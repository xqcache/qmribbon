#pragma once

#include "qmribbon_global.h"

#include <QWidget>

struct QmRibbonSectionPrivate;
class QMRIBBON_API QmRibbonSection : public QWidget {
    Q_OBJECT
public:
    enum Feature {
        NoTitle = 0x01,
        NoExpandButton = 0x02,
        Expanding = 0x04,
    };
    Q_ENUM(Feature)
    Q_DECLARE_FLAGS(Features, Feature)

    explicit QmRibbonSection(QWidget* parent = nullptr);
    ~QmRibbonSection() noexcept override;

    QSize sizeHint() const override;

    void setTitle(const QString& title);
    void setIcon(const QIcon& icon);
    void setWidget(QWidget* widget);

    const QIcon& icon() const;
    QString title() const;

    void setTitleVisible(bool visible);
    void setExpandButtonVisible(bool visible);

    Features features() const;
    void setFeature(Feature feature, bool on = true);
    void setFeatures(Features features, bool on = true);

signals:
    void expandButtonClicked();

protected:
    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    void initUi();
    bool isWidthSufficient() const;

private:
    Q_PROPERTY(Features features READ features WRITE setFeatures)
    QmRibbonSectionPrivate* d_ { nullptr };
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QmRibbonSection::Features)
