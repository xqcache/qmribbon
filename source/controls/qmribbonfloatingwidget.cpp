#include "qmribbonfloatingwidget.h"

#include "qmribbontheme.h"
#include "qmribbonthememgr.h"

#include <QAction>
#include <QEvent>
#include <QGridLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPen>
#include <QPointer>
#include <QStyle>
#include <QToolButton>

#include <utility>

struct QmRibbonFloatingWidget::QmRibbonFloatingWidgetPrivate {
    QGridLayout* layout { nullptr };
    /// 面板里的条目（按加入顺序，`relayout()` 按预设排列摆放它们）。
    QList<QPointer<QWidget>> items;

    LayoutMode mode { LayoutMode::Horizontal };
    int line_count { 2 };

    /// 已经监听了 resize 的父控件（避免重复安装 / 换父控件后失效）。
    QPointer<QWidget> filtered_parent;

    QString title;

    bool movable { true };
    bool pinned { false };
    bool snap_to_edges { true };

    /// 正在拖动，以及按下时鼠标相对面板左上角的偏移（全局坐标）。
    bool dragging { false };
    QPoint drag_offset;
};

QmRibbonFloatingWidget::QmRibbonFloatingWidget(QWidget* parent)
    : QFrame(parent)
    , d_(new QmRibbonFloatingWidgetPrivate)
{
    setObjectName(QStringLiteral("RibbonFloatingWidget"));
    // 面板底色 / 边框走样式表（#RibbonFloatingWidget），需要 WA_StyledBackground。
    setAttribute(Qt::WA_StyledBackground, true);
    QmRibbonThemeMgr::instance().apply(this);

    d_->layout = new QGridLayout(this);
    // 上边留出拖动条的位置。
    d_->layout->setContentsMargins(6, dragBarHeight() + 4, 6, 6);
    d_->layout->setSpacing(4);

    // 父控件（MainView）尺寸变化时把自己夹回可见区域，这样即使没经过
    // QmRibbonWindow::addFloatingWidget() 也不会跑到窗口外面去。
    watchParentResize();
}

QmRibbonFloatingWidget::~QmRibbonFloatingWidget() noexcept
{
    delete d_;
}

QToolButton* QmRibbonFloatingWidget::addAction(QAction* action)
{
    if (action == nullptr) {
        return nullptr;
    }

    auto* button = new QToolButton;
    // 文本 / 图标 / 勾选状态 / 提示 / 菜单全部跟随 action，点击即触发它。
    button->setDefaultAction(action);
    button->setToolButtonStyle(action->icon().isNull() ? Qt::ToolButtonTextOnly : Qt::ToolButtonTextBesideIcon);
    button->setIconSize(QSize(QmRibbonMetrics::icon_size, QmRibbonMetrics::icon_size));
    button->setAutoRaise(true); // 平铺：基础态只显示文字/图标，悬停才有底色
    button->setCursor(Qt::PointingHandCursor);

    addWidget(button);
    return button;
}

void QmRibbonFloatingWidget::addWidget(QWidget* widget)
{
    if (widget == nullptr) {
        return;
    }

    widget->setParent(this);
    d_->items.append(QPointer<QWidget>(widget));

    relayout();
}

int QmRibbonFloatingWidget::count() const
{
    int result = 0;
    for (const QPointer<QWidget>& item : d_->items) {
        if (item != nullptr) {
            ++result;
        }
    }
    return result;
}

void QmRibbonFloatingWidget::clear()
{
    for (const QPointer<QWidget>& item : std::as_const(d_->items)) {
        if (item != nullptr) {
            // 先藏起来再排队删除：不然它会以「已经不在布局里」的状态停留一帧。
            item->hide();
            item->deleteLater();
        }
    }

    d_->items.clear();
    relayout();
}

QmRibbonFloatingWidget::LayoutMode QmRibbonFloatingWidget::layoutMode() const
{
    return d_->mode;
}

void QmRibbonFloatingWidget::setLayoutMode(LayoutMode mode, int line_count)
{
    d_->mode = mode;
    d_->line_count = qMax(1, line_count);

    relayout();
}

int QmRibbonFloatingWidget::lineCount() const
{
    return d_->line_count;
}

void QmRibbonFloatingWidget::setLineCount(int line_count)
{
    setLayoutMode(d_->mode, line_count);
}

void QmRibbonFloatingWidget::relayout()
{
    // 先把条目从布局里摘下来（只删布局项，控件留着），顺便丢掉已经被销毁的。
    while (QLayoutItem* item = d_->layout->takeAt(0)) {
        delete item;
    }

    QList<QWidget*> items;
    items.reserve(d_->items.size());
    for (const QPointer<QWidget>& entry : std::as_const(d_->items)) {
        if (entry != nullptr) {
            items.append(entry.data());
        }
    }
    d_->items.clear();
    for (QWidget* widget : std::as_const(items)) {
        d_->items.append(QPointer<QWidget>(widget));
    }

    // 复位上一次留下的伸缩（setRowStretch 会累积）。
    for (int i = 0; i < d_->layout->rowCount(); ++i) {
        d_->layout->setRowStretch(i, 0);
    }
    for (int i = 0; i < d_->layout->columnCount(); ++i) {
        d_->layout->setColumnStretch(i, 0);
    }

    const int total = items.size();
    const int lines = qMax(1, d_->line_count);

    int last_row = -1;
    int last_column = -1;

    for (int i = 0; i < total; ++i) {
        QWidget* widget = items.at(i);

        int row = 0;
        int column = 0;

        switch (d_->mode) {
        case LayoutMode::Horizontal:
            row = 0;
            column = i;
            break;

        case LayoutMode::Vertical:
            row = i;
            column = 0;
            break;

        case LayoutMode::Rows: {
            // 横向排、共 lines 行：先排满一行再换行。
            const int per_row = (total + lines - 1) / lines;
            row = i / per_row;
            column = i % per_row;
            break;
        }

        case LayoutMode::Columns: {
            // 纵向排、共 lines 列：先排满一列再换列。
            const int per_column = (total + lines - 1) / lines;
            row = i % per_column;
            column = i / per_column;
            break;
        }
        }

        d_->layout->addWidget(widget, row, column);
        widget->show();

        last_row = qMax(last_row, row);
        last_column = qMax(last_column, column);
    }

    // 右下角留一个伸缩格：面板被拉大时条目也只贴左上角，不会被摊开。
    d_->layout->setRowStretch(last_row + 1, 1);
    d_->layout->setColumnStretch(last_column + 1, 1);

    // 条目变了，尺寸也跟着内容走（需要固定尺寸的话，在加完条目之后自己 resize()）。
    adjustSize();
}

void QmRibbonFloatingWidget::setTitle(const QString& title)
{
    if (d_->title == title) {
        return;
    }

    d_->title = title;
    update();
}

QString QmRibbonFloatingWidget::title() const
{
    return d_->title;
}

bool QmRibbonFloatingWidget::isMovable() const
{
    return d_->movable;
}

void QmRibbonFloatingWidget::setMovable(bool movable)
{
    d_->movable = movable;
    unsetCursor();
}

bool QmRibbonFloatingWidget::isPinned() const
{
    return d_->pinned;
}

void QmRibbonFloatingWidget::setPinned(bool pinned)
{
    if (d_->pinned == pinned) {
        return;
    }

    d_->pinned = pinned;
    d_->dragging = false;
    unsetCursor();

    // `pinned` 是 Q_PROPERTY，样式表里的 `#RibbonFloatingWidget[pinned="true"]` 能直接读到它；
    // 改完重新 polish 一次，让这条规则立刻生效。
    style()->unpolish(this);
    style()->polish(this);

    update();

    emit pinnedChanged(pinned);
}

bool QmRibbonFloatingWidget::snapToEdges() const
{
    return d_->snap_to_edges;
}

void QmRibbonFloatingWidget::setSnapToEdges(bool snap)
{
    d_->snap_to_edges = snap;
}

int QmRibbonFloatingWidget::dragBarHeight() const
{
    return QmRibbonMetrics::floating_drag_bar_height;
}

void QmRibbonFloatingWidget::clampToParent()
{
    QWidget* p = parentWidget();
    if (p == nullptr) {
        return;
    }

    const QRect bounds = p->rect();
    QPoint pos = this->pos();

    pos.setX(qBound(bounds.left(), pos.x(), qMax(bounds.left(), bounds.right() - width() + 1)));
    pos.setY(qBound(bounds.top(), pos.y(), qMax(bounds.top(), bounds.bottom() - height() + 1)));

    if (pos != this->pos()) {
        move(pos);
    }
}

void QmRibbonFloatingWidget::snapToNearestEdge()
{
    QWidget* p = parentWidget();
    if (p == nullptr) {
        return;
    }

    clampToParent();

    const QRect bounds = p->rect();
    QPoint pos = this->pos();

    // 左 / 右、上 / 下各自独立判断：靠近哪条边就贴哪条边，角落里就同时贴两条。
    if (pos.x() - bounds.left() <= QmRibbonMetrics::floating_snap_distance) {
        pos.setX(bounds.left());
    } else if (bounds.right() - (pos.x() + width() - 1) <= QmRibbonMetrics::floating_snap_distance) {
        pos.setX(bounds.right() - width() + 1);
    }

    if (pos.y() - bounds.top() <= QmRibbonMetrics::floating_snap_distance) {
        pos.setY(bounds.top());
    } else if (bounds.bottom() - (pos.y() + height() - 1) <= QmRibbonMetrics::floating_snap_distance) {
        pos.setY(bounds.bottom() - height() + 1);
    }

    if (pos != this->pos()) {
        move(pos);
    }
}

bool QmRibbonFloatingWidget::isDragArea(const QPoint& pos) const
{
    if (pos.y() < dragBarHeight()) {
        return true;
    }

    // 面板里没有子控件的空白区域也能拖：点在按钮上时交给按钮。
    return childAt(pos) == nullptr;
}

bool QmRibbonFloatingWidget::canDrag() const
{
    return d_->movable && !d_->pinned;
}

void QmRibbonFloatingWidget::watchParentResize()
{
    QWidget* p = parentWidget();
    if (d_->filtered_parent == p) {
        return;
    }

    if (d_->filtered_parent != nullptr) {
        d_->filtered_parent->removeEventFilter(this);
    }

    d_->filtered_parent = p;

    if (p != nullptr) {
        p->installEventFilter(this);
    }
}

void QmRibbonFloatingWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)

    // 面板底色与边框由样式表画（WA_StyledBackground），这里只补拖动条。
    const QmRibbonTheme& theme = QmRibbonThemeMgr::current();

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRect bar(1, 1, width() - 2, dragBarHeight());

    // 拖动条底色 + 一条分隔线，示意「这里可以拖」。
    painter.fillRect(bar, theme.surface());
    painter.setPen(theme.separator());
    painter.drawLine(bar.left(), bar.bottom(), bar.right(), bar.bottom());

    // 左侧抓手：两列小圆点；固定住的时候画淡一点，表示现在拖不动。
    const QColor grip_color = canDrag() ? theme.mutedText() : theme.disabledText();
    painter.setPen(Qt::NoPen);
    painter.setBrush(grip_color);

    const int grip_x = bar.left() + 7;
    const int grip_y = bar.center().y();
    for (int column = 0; column < 2; ++column) {
        for (int row = -1; row <= 1; ++row) {
            painter.drawEllipse(QPointF(grip_x + column * 4.0, grip_y + row * 4.0), 1.0, 1.0);
        }
    }

    // 标题
    int title_right = bar.right() - 6;
    if (d_->pinned) {
        // 固定标记：一枚小图钉（就画在拖动条右端）。
        const QPointF center(bar.right() - 9.0, bar.center().y());
        QPainterPath pin;
        pin.addEllipse(QRectF(center.x() - 2.5, center.y() - 3.5, 5.0, 5.0));
        painter.setPen(Qt::NoPen);
        painter.setBrush(theme.accent());
        painter.drawPath(pin);

        painter.setPen(QPen(theme.accent(), 1.4, Qt::SolidLine, Qt::RoundCap));
        painter.drawLine(QPointF(center.x(), center.y() + 1.5), QPointF(center.x(), center.y() + 4.0));
        painter.drawLine(QPointF(center.x() - 3.0, center.y() + 4.0), QPointF(center.x() + 3.0, center.y() + 4.0));

        title_right -= 14;
    }

    if (!d_->title.isEmpty()) {
        painter.setPen(theme.secondaryText());
        QFont title_font = font();
        title_font.setBold(true);
        painter.setFont(title_font);

        const QRect text_rect(bar.left() + 20, bar.top(), qMax(title_right - bar.left() - 20, 0), bar.height());
        painter.drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, d_->title);
    }
}

void QmRibbonFloatingWidget::mousePressEvent(QMouseEvent* event)
{
    if (event->button() != Qt::LeftButton || !canDrag() || !isDragArea(event->position().toPoint())) {
        QFrame::mousePressEvent(event);
        return;
    }

    d_->dragging = true;

    // 鼠标相对面板左上角的偏移。注意必须用 mapToGlobal(QPoint(0, 0)) 拿面板的**全局**左上角：
    // 子控件的 frameGeometry()/geometry() 都是**父坐标**，和 globalPosition() 混用会差出
    // 一个「父控件在屏幕上的位置」（MainView 的 y 偏移就是 Ribbon 的高度），
    // 结果面板一按下就被推到上边界外、被夹住 —— 表现就是「只能横向移动」。
    d_->drag_offset = event->globalPosition().toPoint() - mapToGlobal(QPoint(0, 0));

    setCursor(Qt::ClosedHandCursor);
    event->accept();
}

void QmRibbonFloatingWidget::mouseMoveEvent(QMouseEvent* event)
{
    QWidget* p = parentWidget();

    if (!d_->dragging || p == nullptr) {
        // 悬停在拖动条上时给个「可以拖」的手型光标。
        if (canDrag() && event->position().y() < dragBarHeight()) {
            setCursor(Qt::OpenHandCursor);
        } else {
            unsetCursor();
        }

        QFrame::mouseMoveEvent(event);
        return;
    }

    // target 是面板左上角的目标**全局**坐标，mapFromGlobal() 再换成父控件坐标。
    const QPoint target = event->globalPosition().toPoint() - d_->drag_offset;
    move(p->mapFromGlobal(target));
    clampToParent();

    event->accept();
}

void QmRibbonFloatingWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if (!d_->dragging) {
        QFrame::mouseReleaseEvent(event);
        return;
    }

    d_->dragging = false;
    unsetCursor();

    if (d_->snap_to_edges) {
        snapToNearestEdge();
    }

    emit moved(pos());
    event->accept();
}

void QmRibbonFloatingWidget::mouseDoubleClickEvent(QMouseEvent* event)
{
    // 双击拖动条 = 在「固定 / 可拖动」之间切换（和停靠系统里的双击标题栏一个意思）。
    if (event->button() == Qt::LeftButton && event->position().y() < dragBarHeight()) {
        setPinned(!d_->pinned);
        event->accept();
        return;
    }

    QFrame::mouseDoubleClickEvent(event);
}

bool QmRibbonFloatingWidget::event(QEvent* event)
{
    if (event->type() == QEvent::Leave) {
        unsetCursor();
    } else if (event->type() == QEvent::ParentChange) {
        // 被 addFloatingWidget() 之类换过父控件后，改成监听新父控件的 resize。
        watchParentResize();
    }

    return QFrame::event(event);
}

bool QmRibbonFloatingWidget::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == parentWidget() && event->type() == QEvent::Resize) {
        clampToParent();
    }

    return QFrame::eventFilter(watched, event);
}
