#include "qmribbongroup.h"

#include "qmribbontheme.h"
#include "qmribbonthememgr.h"

#include <QAction>
#include <QGridLayout>
#include <QHash>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPixmap>
#include <QSet>
#include <QToolButton>
#include <QVBoxLayout>

namespace {

/// Group 右下角的 Dialog Box Launcher 图标（↘）。
QIcon makeLauncherIcon()
{
    constexpr int size = 16;
    constexpr qreal ratio = 2.0;

    QPixmap pixmap(QSize(size, size) * static_cast<int>(ratio));
    pixmap.setDevicePixelRatio(ratio);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(QmRibbonThemeMgr::current().secondaryText(), 1.2);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);

    painter.drawLine(QPointF(4, 4), QPointF(11, 11));
    painter.drawLine(QPointF(11, 11), QPointF(5.5, 11));
    painter.drawLine(QPointF(11, 11), QPointF(11, 5.5));

    return QIcon(pixmap);
}

/// 当前列中内容的类型：类型变化时另起一列。
enum class ColumnKind {
    None,
    Button,
    Widget,
};

/// 控件要在 Group 里占多高：**固定高度优先**。
///
/// 业务经常用 `setFixedHeight()` 把原生控件压到 Ribbon 的一行高（22px），
/// 但 `sizeHint()` 往往更大（例如带主题样式的 QSpinBox 是 25、QComboBox 是 24），
/// 直接拿 sizeHint() 当行高会把整列撑过 Group 的内容区，最后一个控件被裁掉半截。
int itemHeight(const QWidget* item)
{
    return qBound(item->minimumHeight(), item->sizeHint().height(), item->maximumHeight());
}

} // namespace

struct QmRibbonGroup::QmRibbonGroupPrivate {
    QString title;

    QWidget* content { nullptr };
    QHBoxLayout* content_layout { nullptr };

    QLabel* title_label { nullptr };
    QToolButton* launcher { nullptr };
    QAction* launcher_action { nullptr };

    // 正在填充的列：每列最多三行，行满或类型变化时另起一列。
    QWidget* current_column { nullptr };
    QGridLayout* current_grid { nullptr };
    int current_row { 0 };
    /// 当前列已经占掉的高度（含行间距），用来判断还能不能放得下。
    int current_column_height { 0 };
    ColumnKind current_kind { ColumnKind::None };

    QList<QmRibbonButton*> buttons;
    QHash<QAction*, QmRibbonButton*> action_buttons;
    QSet<QAction*> connected_launcher_actions;
};

QmRibbonGroup::QmRibbonGroup(QWidget* parent)
    : QmRibbonGroup(QString(), parent)
{}

QmRibbonGroup::QmRibbonGroup(const QString& title, QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonGroupPrivate)
{
    setObjectName(QStringLiteral("RibbonGroup"));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    setFixedHeight(QmRibbonMetrics::group_total_height);

    // ---------- 内容区 ----------
    // 上下 margin 必须为 0：内容区 + 标题区的高度之和正好等于 Group 固定高度。
    auto* layout_main = new QVBoxLayout(this);
    layout_main->setContentsMargins(2, 0, 2, 0);
    layout_main->setSpacing(0);

    d_->content = new QWidget(this);
    d_->content->setObjectName(QStringLiteral("RibbonGroupContent"));
    d_->content->setFixedHeight(QmRibbonMetrics::group_content_height);

    d_->content_layout = new QHBoxLayout(d_->content);
    // 上下 margin 必须为 0：内容区高度 70 正好等于 3 行 Small 按钮（22*3 + 2*2）。
    d_->content_layout->setContentsMargins(1, 0, 1, 0);
    d_->content_layout->setSpacing(4);

    layout_main->addWidget(d_->content);

    // ---------- 标题区 ----------
    auto* footer = new QWidget(this);
    footer->setObjectName(QStringLiteral("RibbonGroupFooter"));
    footer->setFixedHeight(QmRibbonMetrics::group_title_height);

    auto* footer_layout = new QHBoxLayout(footer);
    footer_layout->setContentsMargins(0, 0, 1, 1);
    footer_layout->setSpacing(0);

    d_->title_label = new QLabel(footer);
    d_->title_label->setObjectName(QStringLiteral("RibbonGroupTitle"));
    d_->title_label->setAlignment(Qt::AlignCenter);

    d_->launcher = new QToolButton(footer);
    d_->launcher->setObjectName(QStringLiteral("RibbonLauncherButton"));
    d_->launcher->setAutoRaise(true);
    d_->launcher->setFocusPolicy(Qt::NoFocus);
    d_->launcher->setCursor(Qt::ArrowCursor);
    d_->launcher->setIcon(makeLauncherIcon());
    d_->launcher->setIconSize(QSize(QmRibbonMetrics::launcher_size, QmRibbonMetrics::launcher_size));
    d_->launcher->setFixedSize(QmRibbonMetrics::launcher_size, QmRibbonMetrics::launcher_size);
    d_->launcher->setToolTip(tr("Dialog Box Launcher"));
    d_->launcher->hide();

    connect(d_->launcher, &QToolButton::clicked, this, [this]() {
        if (d_->launcher_action != nullptr) {
            d_->launcher_action->trigger();
        }
    });

    footer_layout->addStretch(1);
    footer_layout->addWidget(d_->title_label);
    footer_layout->addStretch(1);
    footer_layout->addWidget(d_->launcher, 0, Qt::AlignBottom | Qt::AlignRight);

    layout_main->addWidget(footer);

    setTitle(title);
}

QmRibbonGroup::~QmRibbonGroup() noexcept
{
    delete d_;
}

QString QmRibbonGroup::title() const
{
    return d_->title;
}

void QmRibbonGroup::setTitle(const QString& text)
{
    if (d_->title == text) {
        return;
    }

    d_->title = text;
    d_->title_label->setText(text);

    emit titleChanged(text);
    updateGeometry();
}

void QmRibbonGroup::appendToColumn(QWidget* item, bool is_widget)
{
    const ColumnKind kind = is_widget ? ColumnKind::Widget : ColumnKind::Button;
    const int item_height = itemHeight(item);

    // 一列最多 small_column_rows 行，而且**总高度不能超过 Group 的内容区**：
    // 内容区高度是固定的（70px），放不下就另起一列，别把控件裁掉半截。
    const bool column_full =
        d_->current_column != nullptr && d_->current_row > 0 &&
        d_->current_column_height + QmRibbonMetrics::small_button_spacing + item_height >
            QmRibbonMetrics::group_content_height;

    const bool need_new_column = d_->current_column == nullptr ||
                                 d_->current_row >= QmRibbonMetrics::small_column_rows || d_->current_kind != kind ||
                                 column_full;

    if (need_new_column) {
        d_->current_column = new QWidget(d_->content);
        d_->current_column->setObjectName(QStringLiteral("RibbonGroupColumn"));
        d_->current_column->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

        d_->current_grid = new QGridLayout(d_->current_column);
        d_->current_grid->setContentsMargins(0, 0, 0, 0);
        d_->current_grid->setHorizontalSpacing(0);
        d_->current_grid->setVerticalSpacing(QmRibbonMetrics::small_button_spacing);
        // 第 small_column_rows 行是空的伸缩行，把内容顶到列的上方。
        d_->current_grid->setRowStretch(QmRibbonMetrics::small_column_rows, 1);

        d_->current_kind = kind;
        d_->current_row = 0;
        d_->current_column_height = 0;

        d_->content_layout->addWidget(d_->current_column, 0, Qt::AlignTop | Qt::AlignLeft);
    }

    d_->current_grid->addWidget(item, d_->current_row, 0);
    d_->current_grid->setRowMinimumHeight(d_->current_row, item_height);

    if (d_->current_row > 0) {
        d_->current_column_height += QmRibbonMetrics::small_button_spacing;
    }
    d_->current_column_height += item_height;

    ++d_->current_row;
}

QmRibbonButton* QmRibbonGroup::addButton(QAction* action, QmRibbonButton::Size size)
{
    if (action == nullptr) {
        return nullptr;
    }

    auto* button = new QmRibbonButton(action, size, d_->content);
    d_->buttons.append(button);

    if (d_->action_buttons.value(action) == nullptr) {
        d_->action_buttons.insert(action, button);
    }

    switch (size) {
    case QmRibbonButton::Size::Small:
        appendToColumn(button, false);
        break;

    case QmRibbonButton::Size::Large:
    case QmRibbonButton::Size::Medium:
        // Large / Medium 独占一列：结束当前列，后续 Small 会另起新列。
        d_->current_column = nullptr;
        d_->current_grid = nullptr;
        d_->current_row = 0;
        d_->current_kind = ColumnKind::None;

        d_->content_layout->addWidget(button, 0, Qt::AlignTop | Qt::AlignLeft);
        break;
    }

    updateGeometry();
    return button;
}

QmRibbonButton* QmRibbonGroup::addLargeAction(QAction* action)
{
    return addButton(action, QmRibbonButton::Size::Large);
}

QmRibbonButton* QmRibbonGroup::addAction(QAction* action)
{
    return addButton(action, QmRibbonButton::Size::Small);
}

void QmRibbonGroup::addWidget(QWidget* widget)
{
    if (widget == nullptr) {
        return;
    }

    appendToColumn(widget, true);
    updateGeometry();
}

QList<QmRibbonButton*> QmRibbonGroup::buttons() const
{
    return d_->buttons;
}

QmRibbonButton* QmRibbonGroup::buttonForAction(QAction* action) const
{
    return d_->action_buttons.value(action, nullptr);
}

void QmRibbonGroup::setLauncherAction(QAction* action)
{
    d_->launcher_action = action;

    const bool visible = action != nullptr;
    d_->launcher->setVisible(visible);
    d_->launcher->setEnabled(visible && action->isEnabled());

    if (visible) {
        d_->launcher->setToolTip(action->text());
    }

    if (visible && !d_->connected_launcher_actions.contains(action)) {
        d_->connected_launcher_actions.insert(action);
        connect(action, &QAction::changed, d_->launcher, [this, action]() {
            if (d_->launcher_action == action) {
                d_->launcher->setEnabled(action->isEnabled());
                d_->launcher->setToolTip(action->text());
            }
        });
    }
}

QAction* QmRibbonGroup::launcherAction() const
{
    return d_->launcher_action;
}

QToolButton* QmRibbonGroup::launcherButton() const
{
    return d_->launcher;
}

bool QmRibbonGroup::isEmpty() const
{
    return d_->content_layout->count() == 0;
}
