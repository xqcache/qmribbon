#pragma once

#include <QWidget>

class QmRibbonGroup;

/// Ribbon Page。
///
/// 一个 Ribbon Tab 对应一个 Page，内部横向排列若干 QmRibbonGroup，
/// 并负责绘制 Group 之间的分隔线。
class QmRibbonPage : public QWidget {
    Q_OBJECT

    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

public:
    explicit QmRibbonPage(QWidget* parent = nullptr);
    explicit QmRibbonPage(const QString& title, QWidget* parent = nullptr);
    ~QmRibbonPage() noexcept override;

    QmRibbonGroup* addGroup(const QString& title);
    QmRibbonGroup* insertGroup(int index, const QString& title);

    QList<QmRibbonGroup*> groups() const;
    int groupCount() const;

    void setTitle(const QString& text);
    QString title() const;

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

signals:
    void titleChanged(const QString& text);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    struct QmRibbonPagePrivate;
    QmRibbonPagePrivate* d_ { nullptr };
};
