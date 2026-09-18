#include "qmribboncombobox.h"

#include "qmribbontheme.h"
#include "qmribbonthememgr.h"

#include <QAbstractItemView>
#include <QCursor>
#include <QFontMetrics>
#include <QPainter>
#include <QPainterPath>
#include <QScrollBar>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QStyle>
#include <QStyledItemDelegate>
#include <QVariant>

namespace {

/// 弹出列表自己的边框宽度（见 ribbon.qss 里 `#RibbonComboBox QAbstractItemView` 的 border）。
constexpr int kPopupFrameWidth = 1;

bool isHeaderIndex(const QModelIndex& index)
{
    return index.data(QmRibbonComboBox::HeaderRole).toBool();
}

/// 条目自己的字体（`Qt::FontRole`），没有就退回列表字体。
QFont itemFont(const QStyleOptionViewItem& option, const QModelIndex& index)
{
    const QVariant value = index.data(Qt::FontRole);
    if (value.isValid() && !value.isNull()) {
        return value.value<QFont>();
    }
    return option.font;
}

/// 由基础字体派生一个小一号（可加粗）的字体。
/// 样式表里给的字号可能是 px 也可能是 pt，两种都要照顾到。
QFont derivedFont(const QFont& base, bool bold)
{
    QFont font = base;
    font.setBold(bold);

    if (font.pixelSize() > 0) {
        font.setPixelSize(qMax(10, font.pixelSize() - 1));
    } else if (font.pointSizeF() > 0.0) {
        font.setPointSizeF(qMax(7.5, font.pointSizeF() - 1.0));
    }

    return font;
}

/// 该项是否被鼠标悬停。
///
/// 优先用 `State_MouseOver`；有的样式不会给列表项设这个状态，
/// 这时按光标位置自己判断一次（弹出列表是顶层窗口，直接读 QCursor 最省事）。
bool isIndexHovered(const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if ((option.state & QStyle::State_MouseOver) != 0) {
        return true;
    }

    const auto* view = qobject_cast<const QAbstractItemView*>(option.widget);
    if (view == nullptr || !view->underMouse()) {
        return false;
    }

    return view->indexAt(view->viewport()->mapFromGlobal(QCursor::pos())) == index;
}

/// 弹出列表的绘制：分组标题、右侧灰色标注、条目自己的字体、悬停 / 选中底色。
class QmRibbonComboBoxDelegate : public QStyledItemDelegate {
public:
    explicit QmRibbonComboBoxDelegate(QObject* parent = nullptr)
        : QStyledItemDelegate(parent)
    {}

    QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        constexpr int padding = QmRibbonMetrics::combo_popup_padding;

        if (isHeaderIndex(index)) {
            const QFontMetrics metrics(derivedFont(option.font, true));
            return { 2 * padding + metrics.horizontalAdvance(index.data(Qt::DisplayRole).toString()),
                     QmRibbonMetrics::combo_header_height };
        }

        const QFontMetrics metrics(itemFont(option, index));
        int width = 2 * padding + metrics.horizontalAdvance(index.data(Qt::DisplayRole).toString());

        const QString annotation = index.data(QmRibbonComboBox::AnnotationRole).toString();
        if (!annotation.isEmpty()) {
            width += QmRibbonMetrics::combo_annotation_gap + QFontMetrics(option.font).horizontalAdvance(annotation);
        }

        if (!index.data(Qt::DecorationRole).isNull()) {
            width += option.decorationSize.width() + QmRibbonMetrics::combo_icon_gap;
        }

        return { width, QmRibbonMetrics::combo_item_height };
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override
    {
        constexpr int padding = QmRibbonMetrics::combo_popup_padding;

        const QmRibbonTheme& theme = QmRibbonThemeMgr::current();
        const QFont base_font = option.font;
        const bool enabled = (option.state & QStyle::State_Enabled) != 0;

        QStyleOptionViewItem opt(option);
        initStyleOption(&opt, index);

        painter->save();
        painter->setClipRect(opt.rect);

        if (isHeaderIndex(index)) {
            paintHeader(painter, opt, base_font, theme);
            painter->restore();
            return;
        }

        // 底色：悬停优先于当前项（Word 里当前项是一块很淡的底色）。
        if (enabled && isIndexHovered(option, index)) {
            painter->fillRect(opt.rect, theme.hover());
        } else if ((opt.state & QStyle::State_Selected) != 0) {
            painter->fillRect(opt.rect, theme.buttonChecked());
            painter->setPen(theme.buttonCheckedBorder());
            painter->drawRect(opt.rect.adjusted(0, 0, -1, -1));
        }

        // 图标（如果有），文字从图标右侧开始。
        int left = opt.rect.left() + padding;
        if ((opt.features & QStyleOptionViewItem::HasDecoration) != 0 && !opt.icon.isNull()) {
            const int icon_size = qMin(opt.decorationSize.width(), opt.rect.height() - 6);
            const QRect icon_rect(left, opt.rect.center().y() - icon_size / 2 + 1, icon_size, icon_size);
            opt.icon.paint(painter, icon_rect, Qt::AlignCenter, enabled ? QIcon::Normal : QIcon::Disabled);
            left = icon_rect.right() + 1 + QmRibbonMetrics::combo_icon_gap;
        }

        // 右侧标注：用列表字体（不跟着条目字体走），颜色是次要文字色。
        const QString annotation = index.data(QmRibbonComboBox::AnnotationRole).toString();
        const int annotation_width = annotation.isEmpty() ? 0 : QFontMetrics(base_font).horizontalAdvance(annotation);

        QRect text_rect = opt.rect.adjusted(left - opt.rect.left(), 0, -(padding + annotation_width), 0);
        if (!annotation.isEmpty()) {
            text_rect.setRight(text_rect.right() - QmRibbonMetrics::combo_annotation_gap);
        }

        const QFont text_font = itemFont(option, index);
        const QString text = QFontMetrics(text_font).elidedText(opt.text, Qt::ElideRight, qMax(text_rect.width(), 0));

        painter->setFont(text_font);
        painter->setPen(enabled ? theme.text() : theme.disabledText());
        painter->drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, text);

        if (!annotation.isEmpty()) {
            painter->setFont(base_font);
            painter->setPen(theme.mutedText());

            const QRect annotation_rect(opt.rect.right() - padding - annotation_width, opt.rect.top(), annotation_width,
                                        opt.rect.height());
            painter->drawText(annotation_rect, Qt::AlignRight | Qt::AlignVCenter, annotation);
        }

        painter->restore();
    }

private:
    static void paintHeader(QPainter* painter, const QStyleOptionViewItem& opt, const QFont& base_font,
                            const QmRibbonTheme& theme)
    {
        constexpr int padding = QmRibbonMetrics::combo_popup_padding;

        const QFont header_font = derivedFont(base_font, true);
        const QFontMetrics metrics(header_font);

        painter->setFont(header_font);
        painter->setPen(theme.secondaryText());

        const QRect text_rect = opt.rect.adjusted(padding, 0, -padding, 0);
        const QString text = metrics.elidedText(opt.text, Qt::ElideRight, qMax(text_rect.width(), 0));
        painter->drawText(text_rect, Qt::AlignLeft | Qt::AlignVCenter, text);

        // 标题下方一条贯穿整行的分隔线（Word 的分组分隔）。
        painter->setPen(theme.separator());
        painter->drawLine(opt.rect.left(), opt.rect.bottom(), opt.rect.right(), opt.rect.bottom());
    }
};

} // namespace

QmRibbonComboBox::QmRibbonComboBox(QWidget* parent)
    : QComboBox(parent)
{
    init();
}

void QmRibbonComboBox::init()
{
    setObjectName(QStringLiteral("RibbonComboBox"));

    // 主题切换时自动重新应用样式表（#RibbonComboBox 在 ribbon.qss 里）。
    QmRibbonThemeMgr::instance().apply(this);

    // 弹出列表全部由内部委托绘制，列表本身只负责滚动与命中测试。
    if (QAbstractItemView* item_view = view()) {
        item_view->setItemDelegate(new QmRibbonComboBoxDelegate(item_view));
        // 每项高度都不一样（分组标题比条目矮），逐像素滚动更顺滑。
        item_view->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        item_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
}

void QmRibbonComboBox::addGroupHeader(const QString& title)
{
    const int index = count();
    const int previous = currentIndex();

    addItem(title);
    setItemData(index, true, HeaderRole);

    // 让标题彻底「点不动」：视图会跳过没有 ItemIsEnabled 的项，
    // 因此它既不会被选中，也不会在键盘导航时被停留。
    if (auto* standard_model = qobject_cast<QStandardItemModel*>(model())) {
        if (QStandardItem* item = standard_model->item(index)) {
            item->setFlags(Qt::NoItemFlags);
        }
    }

    // 列表里第一个加入的恰好是标题时，addItem() 会顺手把它设为当前项，这里撤销。
    if (previous < 0) {
        setCurrentIndex(-1);
    }
}

void QmRibbonComboBox::addEntry(const QString& text, const QString& annotation)
{
    addEntry(text, annotation, QFont());
}

void QmRibbonComboBox::addEntry(const QString& text, const QString& annotation, const QFont& font)
{
    addItem(text);

    const int index = count() - 1;
    setItemAnnotation(index, annotation);
    setItemFont(index, font);

    // 列表以分组标题开头时，标题会被撤销成「无当前项」（见 addGroupHeader），
    // 这里把第一个真正的条目补上。
    if (currentIndex() < 0) {
        setCurrentIndex(index);
    }
}

void QmRibbonComboBox::setItemAnnotation(int index, const QString& annotation)
{
    if (index < 0 || index >= count()) {
        return;
    }

    setItemData(index, annotation, AnnotationRole);
}

QString QmRibbonComboBox::itemAnnotation(int index) const
{
    if (index < 0 || index >= count()) {
        return {};
    }

    return itemData(index, AnnotationRole).toString();
}

void QmRibbonComboBox::setItemFont(int index, const QFont& font)
{
    if (index < 0 || index >= count()) {
        return;
    }

    // 默认构造的 QFont() 表示「不要单独指定字体」。
    setItemData(index, font != QFont() ? QVariant::fromValue(font) : QVariant(), Qt::FontRole);
}

bool QmRibbonComboBox::isGroupHeader(int index) const
{
    if (index < 0 || index >= count()) {
        return false;
    }

    return itemData(index, HeaderRole).toBool();
}

void QmRibbonComboBox::setPopupMinimumWidth(int width)
{
    popup_minimum_width_ = qMax(0, width);
}

int QmRibbonComboBox::popupMinimumWidth() const
{
    return popup_minimum_width_;
}

void QmRibbonComboBox::showPopup()
{
    // 弹出列表允许比输入框本身宽（Word 的字体框就是这样）。
    //
    // QComboBox 只在「原生弹出菜单样式」下才会自己按内容加宽
    //（qcombobox.cpp: `if (usePopup) { ... computeWidthHint() ... }`），
    // 而 Fusion 下 SH_ComboBox_Popup 对**可编辑**组合框返回 false，
    // 于是弹出列表会被压成和输入框一样宽。这里改成显式设置列表视图的最小宽度：
    // 弹出容器在定位时会把几何尺寸撑到布局的最小尺寸
    //（`listRect.setSize(...expandedTo(container->minimumSize()))`），与 usePopup 无关。
    if (QAbstractItemView* item_view = view()) {
        item_view->setMinimumWidth(qMax(qMax(popupContentWidth(), width()), popup_minimum_width_));
    }

    QComboBox::showPopup();
}

int QmRibbonComboBox::popupContentWidth() const
{
    const QAbstractItemView* item_view = view();
    if (item_view == nullptr || item_view->itemDelegate() == nullptr) {
        return 0;
    }

    // 这些属性要和列表真正绘制时拿到的一致（见 Qt 的 QComboBoxListView::initViewItemOption：
    // 它把 option.font 换成了**组合框自己的字体**，装饰尺寸取列表图标尺寸），
    // 否则算出来的宽度和实际渲染会差一点。
    QStyleOptionViewItem option;
    option.initFrom(item_view);
    option.font = font();
    option.fontMetrics = QFontMetrics(option.font);
    option.widget = item_view;

    QSize icon_size = item_view->iconSize();
    if (!icon_size.isValid()) {
        const int pm = style()->pixelMetric(QStyle::PM_SmallIconSize, nullptr, item_view);
        icon_size = QSize(pm, pm);
    }
    option.decorationSize = icon_size;

    // 宽度交给列表委托去算（sizeHint 是它自己声明的），
    // 这样「文字 + 右侧标注 + 内边距」的算法只有一处。
    int width = 0;
    const QAbstractItemModel* item_model = model();
    for (int i = 0, rows = item_model->rowCount(); i < rows; ++i) {
        width = qMax(width, item_view->itemDelegate()->sizeHint(option, item_model->index(i, 0)).width());
    }

    width += 2 * kPopupFrameWidth;

    // 条目多到要滚动时右侧还会出现竖直滚动条，也得给它留出位置。
    if (count() > maxVisibleItems()) {
        width += item_view->verticalScrollBar()->sizeHint().width();
    }

    return width;
}

void QmRibbonComboBox::paintEvent(QPaintEvent* event)
{
    // 边框、底色与当前项文字交给样式表（#RibbonComboBox），这里只补箭头。
    QComboBox::paintEvent(event);

    // 样式表把默认箭头关掉了（::down-arrow { image: none; }），
    // 改成自绘：颜色就能跟着主题的文字色走，也不需要额外的图标资源。
    const int arrow_width = qMin(QmRibbonMetrics::combo_dropdown_width, width());
    if (arrow_width <= 0 || height() <= 0) {
        return;
    }

    const QmRibbonTheme& theme = QmRibbonThemeMgr::current();
    const QColor color = isEnabled() ? theme.text() : theme.disabledText();

    const QRectF area(width() - arrow_width, 0.0, arrow_width, height());

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    QPen pen(color, 1.2);
    pen.setCapStyle(Qt::RoundCap);
    pen.setJoinStyle(Qt::RoundJoin);
    painter.setPen(pen);

    constexpr qreal half_width = 4.0;
    constexpr qreal half_height = 2.2;

    const QPointF center = area.center() + QPointF(0.0, -0.5);
    QPainterPath chevron;
    chevron.moveTo(center.x() - half_width, center.y() - half_height);
    chevron.lineTo(center.x(), center.y() + half_height);
    chevron.lineTo(center.x() + half_width, center.y() - half_height);

    painter.drawPath(chevron);
}
