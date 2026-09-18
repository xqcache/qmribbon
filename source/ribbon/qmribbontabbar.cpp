#include "qmribbontabbar.h"

#include "qmribbontab.h"
#include "qmribbontheme.h"
#include "qmribbonthememgr.h"

#include <QAction>
#include <QButtonGroup>
#include <QEvent>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>
#include <QToolButton>

namespace {

QIcon makeChevronIcon(bool up)
{
    constexpr int size = 16;
    constexpr qreal ratio = 2.0;

    QPixmap pixmap(QSize(size, size) * static_cast<int>(ratio));
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(QmRibbonThemeMgr::current().text(), 1.4);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);

    const qreal cx = size / 2.0;
    const qreal cy = size / 2.0;
    const qreal dx = 4.0;
    const qreal dy = 2.5;

    if (up) {
        painter.drawPolyline(QPolygonF { { cx - dx, cy + dy }, { cx, cy - dy }, { cx + dx, cy + dy } });
    } else {
        painter.drawPolyline(QPolygonF { { cx - dx, cy - dy }, { cx, cy + dy }, { cx + dx, cy - dy } });
    }

    return QIcon(pixmap);
}

} // namespace

struct QmRibbonTabBar::QmRibbonTabBarPrivate {
    QHBoxLayout* layout { nullptr };
    QToolButton* file_button { nullptr };
    QToolButton* collapse_button { nullptr };
    QButtonGroup* button_group { nullptr };
    QList<QmRibbonTab*> tabs;
    QList<QToolButton*> right_buttons;
    int current { -1 };
    /// 当前是否处于折叠状态（由 setCollapsed() 同步）。
    bool collapsed { false };
    /// 本次鼠标手势「按下那一刻」是否折叠；双击时据此决定目标模式。
    bool collapsed_at_press { false };
};

QmRibbonTabBar::QmRibbonTabBar(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonTabBarPrivate)
{
    setObjectName(QStringLiteral("RibbonTabBar"));
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedHeight(QmRibbonMetrics::tab_bar_height);

    d_->layout = new QHBoxLayout(this);
    d_->layout->setContentsMargins(0, 0, 6, 0);
    d_->layout->setSpacing(0);

    d_->file_button = new QToolButton(this);
    d_->file_button->setObjectName(QStringLiteral("RibbonFileButton"));
    d_->file_button->setText(tr("File"));
    d_->file_button->setAutoRaise(true);
    d_->file_button->setFocusPolicy(Qt::NoFocus);
    d_->file_button->setCursor(Qt::ArrowCursor);
    d_->file_button->setFixedHeight(QmRibbonMetrics::tab_bar_height);
    d_->layout->addWidget(d_->file_button);

    d_->button_group = new QButtonGroup(this);
    d_->button_group->setExclusive(true);

    d_->layout->addStretch(1);

    d_->collapse_button = new QToolButton(this);
    d_->collapse_button->setObjectName(QStringLiteral("RibbonCollapseButton"));
    d_->collapse_button->setAutoRaise(true);
    d_->collapse_button->setFocusPolicy(Qt::NoFocus);
    d_->collapse_button->setCursor(Qt::ArrowCursor);
    d_->collapse_button->setIcon(makeChevronIcon(true));
    d_->collapse_button->setIconSize(QSize(QmRibbonMetrics::icon_size, QmRibbonMetrics::icon_size));
    d_->collapse_button->setToolTip(tr("Collapse the Ribbon"));
    d_->collapse_button->setFixedSize(QmRibbonMetrics::quick_access_button_size,
                                      QmRibbonMetrics::quick_access_button_size);
    d_->layout->addWidget(d_->collapse_button);

    connect(d_->file_button, &QToolButton::clicked, this, &QmRibbonTabBar::fileButtonClicked);
    connect(d_->collapse_button, &QToolButton::clicked, this, &QmRibbonTabBar::collapseButtonClicked);
}

QmRibbonTabBar::~QmRibbonTabBar() noexcept
{
    delete d_;
}

QmRibbonTab* QmRibbonTabBar::addTab(const QString& title)
{
    return insertTab(d_->tabs.size(), title);
}

QmRibbonTab* QmRibbonTabBar::insertTab(int index, const QString& title)
{
    index = qBound(0, index, d_->tabs.size());

    auto* tab = new QmRibbonTab(title, this);
    d_->tabs.insert(index, tab);
    d_->button_group->addButton(tab, index);

    // 双击 Tab 也要能折叠 / 展开 Ribbon，因此监听它的鼠标事件。
    tab->installEventFilter(this);

    // 布局顺序：[File][Tab...][stretch][右侧动作][Collapse]
    d_->layout->insertWidget(1 + index, tab);

    connect(tab, &QAbstractButton::clicked, this, [this, tab]() {
        const int tab_index = d_->tabs.indexOf(tab);
        if (tab_index < 0) {
            return;
        }

        if (d_->current != tab_index) {
            d_->current = tab_index;
            emit currentIndexChanged(tab_index);
        }
        emit tabClicked(tab_index);
    });

    connect(tab, &QObject::destroyed, this, [this, tab]() { d_->tabs.removeAll(tab); });

    if (d_->current < 0) {
        setCurrentIndex(index);
    }

    updateGeometry();
    return tab;
}

void QmRibbonTabBar::removeTab(int index)
{
    if (index < 0 || index >= d_->tabs.size()) {
        return;
    }

    QmRibbonTab* tab = d_->tabs.takeAt(index);
    d_->button_group->removeButton(tab);
    d_->layout->removeWidget(tab);
    tab->deleteLater();

    if (d_->current == index) {
        d_->current = -1;
        setCurrentIndex(qBound(0, index, d_->tabs.size() - 1));
        if (d_->tabs.isEmpty()) {
            emit currentIndexChanged(-1);
        }
    } else if (d_->current > index) {
        --d_->current;
    }

    updateGeometry();
}

void QmRibbonTabBar::clear()
{
    while (!d_->tabs.isEmpty()) {
        removeTab(d_->tabs.size() - 1);
    }
}

int QmRibbonTabBar::count() const
{
    return static_cast<int>(d_->tabs.size());
}

int QmRibbonTabBar::currentIndex() const
{
    return d_->current;
}

int QmRibbonTabBar::indexOf(QmRibbonTab* tab) const
{
    return static_cast<int>(d_->tabs.indexOf(tab));
}

QmRibbonTab* QmRibbonTabBar::tabAt(int index) const
{
    if (index < 0 || index >= d_->tabs.size()) {
        return nullptr;
    }
    return d_->tabs.at(index);
}

QmRibbonTab* QmRibbonTabBar::currentTab() const
{
    return tabAt(d_->current);
}

void QmRibbonTabBar::setCurrentIndex(int index)
{
    if (index < -1 || index >= d_->tabs.size()) {
        return;
    }

    if (index >= 0 && !d_->tabs.at(index)->isChecked()) {
        d_->tabs.at(index)->setChecked(true);
    }

    if (d_->current == index) {
        return;
    }

    d_->current = index;
    emit currentIndexChanged(index);
}

QToolButton* QmRibbonTabBar::fileButton() const
{
    return d_->file_button;
}

void QmRibbonTabBar::setFileButtonVisible(bool visible)
{
    d_->file_button->setVisible(visible);
}

QToolButton* QmRibbonTabBar::collapseButton() const
{
    return d_->collapse_button;
}

void QmRibbonTabBar::setCollapseButtonVisible(bool visible)
{
    d_->collapse_button->setVisible(visible);
}

void QmRibbonTabBar::setCollapsed(bool collapsed)
{
    d_->collapsed = collapsed;
    d_->collapse_button->setIcon(makeChevronIcon(!collapsed));
    d_->collapse_button->setToolTip(collapsed ? tr("Pin the Ribbon") : tr("Collapse the Ribbon"));
}

QToolButton* QmRibbonTabBar::addRightAction(QAction* action)
{
    if (action == nullptr) {
        return nullptr;
    }

    auto* button = new QToolButton(this);
    button->setObjectName(QStringLiteral("RibbonHelpButton"));
    button->setDefaultAction(action);
    button->setAutoRaise(true);
    button->setFocusPolicy(Qt::NoFocus);
    button->setCursor(Qt::ArrowCursor);
    button->setIconSize(QSize(QmRibbonMetrics::icon_size, QmRibbonMetrics::icon_size));
    button->setFixedSize(QmRibbonMetrics::quick_access_button_size, QmRibbonMetrics::quick_access_button_size);

    d_->right_buttons.append(button);

    // 插到 stretch 之后、Collapse 之前。
    const int collapse_index = d_->layout->indexOf(d_->collapse_button);
    d_->layout->insertWidget(collapse_index, button);

    return button;
}

void QmRibbonTabBar::mousePressEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        d_->collapsed_at_press = d_->collapsed;
    }

    QWidget::mousePressEvent(event);
}

void QmRibbonTabBar::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton) {
        emit barDoubleClicked(d_->collapsed_at_press);
        event->accept();
        return;
    }

    QWidget::mouseDoubleClickEvent(event);
}

bool QmRibbonTabBar::eventFilter(QObject* watched, QEvent* event)
{
    if (qobject_cast<QmRibbonTab*>(watched) == nullptr) {
        return QWidget::eventFilter(watched, event);
    }

    // 过滤器会收到该控件的所有事件，先筛类型再转型。
    const QEvent::Type type = event->type();
    if (type != QEvent::MouseButtonPress && type != QEvent::MouseButtonDblClick) {
        return QWidget::eventFilter(watched, event);
    }

    auto* mouse_event = static_cast<QMouseEvent*>(event);
    if (mouse_event->button() != Qt::LeftButton) {
        return QWidget::eventFilter(watched, event);
    }

    // 记住手势开始时的状态：双击的第一下会先触发 Tab 的 clicked
    //（折叠状态下会顺带把 Ribbon 展开），所以双击必须按这个状态判断。
    if (type == QEvent::MouseButtonPress) {
        d_->collapsed_at_press = d_->collapsed;
        return QWidget::eventFilter(watched, event);
    }

    // 双击落在 Tab 上时，QAbstractButton 会把事件吃掉，这里代为转发。
    emit barDoubleClicked(d_->collapsed_at_press);
    return true;
}
