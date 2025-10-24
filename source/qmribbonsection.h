#pragma once

#include <QWidget>

class QmRibbonSection : public QWidget {
    Q_OBJECT
public:
    explicit QmRibbonSection(QWidget* parent = nullptr);

    void setTitle(const QString& title);
    void setIcon(const QIcon& icon);

    const QIcon& icon() const;
    QString title() const;
};