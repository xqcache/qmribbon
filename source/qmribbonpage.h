#pragma once

#include <QWidget>

class QmRibbonGroup;

struct QmRibbonPagePrivate;

class QmRibbonPage : public QWidget {
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)
public:
    explicit QmRibbonPage(QWidget* parent = nullptr);
    ~QmRibbonPage() noexcept override;

    QmRibbonGroup* addGroup(const QString& title, const QIcon& icon = QIcon());

    void setTitle(const QString& text);
    QString title() const;

signals:
    void titleChanged(const QString& text);

private:
    QmRibbonPagePrivate* d_ { nullptr };
};
