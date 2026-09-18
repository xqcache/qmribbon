#include "qmribbonpage.h"

#include "qmribbongroup.h"
#include "qmribbontheme.h"
#include "qmribbonthememgr.h"

#include <QHBoxLayout>
#include <QPainter>

struct QmRibbonPage::QmRibbonPagePrivate {
    QString title;
    QHBoxLayout* layout { nullptr };
    QList<QmRibbonGroup*> groups;
};

QmRibbonPage::QmRibbonPage(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonPagePrivate)
{
    setObjectName(QStringLiteral("RibbonPage"));
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setFixedHeight(QmRibbonMetrics::group_total_height + 2 * QmRibbonMetrics::page_margin);

    d_->layout = new QHBoxLayout(this);
    d_->layout->setContentsMargins(QmRibbonMetrics::page_margin, QmRibbonMetrics::page_margin,
                                   QmRibbonMetrics::page_margin, QmRibbonMetrics::page_margin);
    d_->layout->setSpacing(QmRibbonMetrics::group_spacing);
    d_->layout->addStretch(1);
}

QmRibbonPage::QmRibbonPage(const QString& title, QWidget* parent)
    : QmRibbonPage(parent)
{
    setTitle(title);
}

QmRibbonPage::~QmRibbonPage() noexcept
{
    delete d_;
}

QmRibbonGroup* QmRibbonPage::addGroup(const QString& title)
{
    return insertGroup(static_cast<int>(d_->groups.size()), title);
}

QmRibbonGroup* QmRibbonPage::insertGroup(int index, const QString& title)
{
    index = qBound(0, index, static_cast<int>(d_->groups.size()));

    auto* group = new QmRibbonGroup(title, this);
    d_->groups.insert(index, group);

    // 末尾固定保留一个 stretch，Group 因此可以整体左对齐。
    d_->layout->insertWidget(index, group, 0, Qt::AlignTop | Qt::AlignLeft);

    updateGeometry();
    return group;
}

QList<QmRibbonGroup*> QmRibbonPage::groups() const
{
    return d_->groups;
}

int QmRibbonPage::groupCount() const
{
    return static_cast<int>(d_->groups.size());
}

void QmRibbonPage::setTitle(const QString& text)
{
    if (d_->title == text) {
        return;
    }

    d_->title = text;
    emit titleChanged(text);
}

QString QmRibbonPage::title() const
{
    return d_->title;
}

QSize QmRibbonPage::sizeHint() const
{
    QSize hint = QWidget::sizeHint();
    hint.setHeight(QmRibbonMetrics::group_total_height + 2 * QmRibbonMetrics::page_margin);
    return hint;
}

QSize QmRibbonPage::minimumSizeHint() const
{
    return sizeHint();
}

void QmRibbonPage::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    const QmRibbonTheme& theme = QmRibbonThemeMgr::current();

    QPainter painter(this);
    painter.fillRect(rect(), theme.contentBackground());

    // Group 之间的分隔线，只覆盖内容区，不穿过 Group 标题。
    painter.setPen(theme.border());

    const int top = QmRibbonMetrics::page_margin + 2;
    const int bottom = height() - QmRibbonMetrics::group_title_height - QmRibbonMetrics::page_margin;

    for (int i = 0; i + 1 < d_->groups.size(); ++i) {
        const QRect geometry = d_->groups.at(i)->geometry();
        if (!geometry.isValid()) {
            continue;
        }

        const int x = geometry.right() + QmRibbonMetrics::group_spacing / 2;
        painter.drawLine(x, top, x, bottom);
    }
}
