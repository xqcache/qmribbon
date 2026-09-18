#include "qmribbontitlebar.h"

#include "qmribbonquickaccessbar.h"
#include "qmribbontheme.h"
#include "qmribbonthememgr.h"

#include <QEvent>
#include <QFontMetrics>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPointer>
#include <QToolButton>

namespace {

/// 窗口控制按钮（最小化 / 最大化 / 还原 / 关闭）。
///
/// 完全自绘，因此悬浮 / 按下状态以及关闭按钮的高对比配色都不依赖样式表。
class WindowButton : public QToolButton {
public:
    enum class Glyph {
        Minimize,
        Maximize,
        Restore,
        Close,
    };

    explicit WindowButton(Glyph glyph, QWidget* parent = nullptr)
        : QToolButton(parent)
        , glyph_(glyph)
    {
        setObjectName(glyphName(glyph));
        setAutoRaise(true);
        setFocusPolicy(Qt::NoFocus);
        setCursor(Qt::ArrowCursor);
        setFixedSize(QmRibbonMetrics::window_button_width, QmRibbonMetrics::title_bar_height);
    }

    Glyph glyph() const
    {
        return glyph_;
    }

    void setGlyph(Glyph glyph)
    {
        if (glyph_ == glyph) {
            return;
        }
        glyph_ = glyph;
        setObjectName(glyphName(glyph));
        update();
    }

    static QString glyphName(Glyph glyph)
    {
        switch (glyph) {
        case Glyph::Minimize:
            return QStringLiteral("RibbonWindowMinButton");
        case Glyph::Maximize:
        case Glyph::Restore:
            return QStringLiteral("RibbonWindowMaxButton");
        case Glyph::Close:
            return QStringLiteral("RibbonWindowCloseButton");
        }
        return QString();
    }

protected:
    void paintEvent(QPaintEvent* event) override
    {
        Q_UNUSED(event)

        const bool is_close = (glyph_ == Glyph::Close);
        const bool hovered = underMouse();

        const QmRibbonTheme& theme = QmRibbonThemeMgr::current();

        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);

        QColor background = Qt::transparent;
        if (isDown()) {
            background = is_close ? theme.danger().darker(115) : theme.pressed();
        } else if (hovered) {
            background = is_close ? theme.danger() : theme.hover();
        }

        if (background.alpha() > 0) {
            painter.fillRect(rect(), background);
        }

        painter.setPen(QPen(is_close && hovered ? QColor(Qt::white) : theme.text(), 1.0));

        const QPointF center = QRectF(rect()).center();
        constexpr qreal half = 5.0;

        switch (glyph_) {
        case Glyph::Minimize:
            painter.drawLine(QPointF(center.x() - half, center.y()), QPointF(center.x() + half, center.y()));
            break;

        case Glyph::Maximize:
            painter.drawRect(QRectF(center.x() - half, center.y() - half, 2 * half, 2 * half));
            break;

        case Glyph::Restore:
            painter.drawRect(QRectF(center.x() - half, center.y() - half + 2, 2 * half - 2, 2 * half - 2));
            painter.drawLine(QPointF(center.x() - half + 2, center.y() - half + 2),
                             QPointF(center.x() - half + 2, center.y() - half));
            painter.drawLine(QPointF(center.x() - half + 2, center.y() - half),
                             QPointF(center.x() + half, center.y() - half));
            painter.drawLine(QPointF(center.x() + half, center.y() - half),
                             QPointF(center.x() + half, center.y() + half - 2));
            break;

        case Glyph::Close:
            painter.drawLine(QPointF(center.x() - half, center.y() - half),
                             QPointF(center.x() + half, center.y() + half));
            painter.drawLine(QPointF(center.x() - half, center.y() + half),
                             QPointF(center.x() + half, center.y() - half));
            break;
        }
    }

private:
    Glyph glyph_;
};

} // namespace

struct QmRibbonTitleBar::QmRibbonTitleBarPrivate {
    QmRibbonQuickAccessBar* quick_access { nullptr };
    QToolButton* app_icon { nullptr };
    QToolButton* account { nullptr };
    QLabel* title_label { nullptr };

    WindowButton* minimize { nullptr };
    WindowButton* maximize { nullptr };
    WindowButton* close { nullptr };

    QString title;
    QPointer<QWidget> watched_window;
};

QmRibbonTitleBar::QmRibbonTitleBar(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonTitleBarPrivate)
{
    setObjectName(QStringLiteral("RibbonTitleBar"));
    setAttribute(Qt::WA_StyledBackground, true);
    setFixedHeight(QmRibbonMetrics::title_bar_height);

    auto* layout = new QGridLayout(this);
    layout->setContentsMargins(4, 0, 0, 0);
    layout->setHorizontalSpacing(6);
    layout->setVerticalSpacing(0);

    // ---------- 左侧：App 图标 + Quick Access ----------
    d_->app_icon = new QToolButton(this);
    d_->app_icon->setObjectName(QStringLiteral("RibbonAppIconButton"));
    d_->app_icon->setAutoRaise(true);
    d_->app_icon->setFocusPolicy(Qt::NoFocus);
    d_->app_icon->setCursor(Qt::ArrowCursor);
    d_->app_icon->setIconSize(QSize(QmRibbonMetrics::icon_size, QmRibbonMetrics::icon_size));
    d_->app_icon->setFixedSize(QmRibbonMetrics::quick_access_button_size, QmRibbonMetrics::quick_access_button_size);
    d_->app_icon->hide();

    d_->quick_access = new QmRibbonQuickAccessBar(this);

    // ---------- 中间：标题 ----------
    d_->title_label = new QLabel(this);
    d_->title_label->setObjectName(QStringLiteral("RibbonWindowTitle"));
    d_->title_label->setAlignment(Qt::AlignCenter);
    d_->title_label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    d_->title_label->setMinimumWidth(0);

    // ---------- 右侧：Account + 窗口按钮 ----------
    d_->account = new QToolButton(this);
    d_->account->setObjectName(QStringLiteral("RibbonAccountButton"));
    d_->account->setAutoRaise(true);
    d_->account->setFocusPolicy(Qt::NoFocus);
    d_->account->setCursor(Qt::ArrowCursor);
    d_->account->setIconSize(QSize(QmRibbonMetrics::icon_size, QmRibbonMetrics::icon_size));
    d_->account->setFixedSize(QmRibbonMetrics::quick_access_button_size, QmRibbonMetrics::quick_access_button_size);
    d_->account->hide();

    auto* window_buttons = new QWidget(this);
    window_buttons->setObjectName(QStringLiteral("RibbonWindowButtonGroup"));
    window_buttons->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    auto* window_buttons_layout = new QHBoxLayout(window_buttons);
    window_buttons_layout->setContentsMargins(0, 0, 0, 0);
    window_buttons_layout->setSpacing(0);

    d_->minimize = new WindowButton(WindowButton::Glyph::Minimize, window_buttons);
    d_->maximize = new WindowButton(WindowButton::Glyph::Maximize, window_buttons);
    d_->close = new WindowButton(WindowButton::Glyph::Close, window_buttons);

    window_buttons_layout->addWidget(d_->minimize);
    window_buttons_layout->addWidget(d_->maximize);
    window_buttons_layout->addWidget(d_->close);

    layout->addWidget(d_->app_icon, 0, 0);
    layout->addWidget(d_->quick_access, 0, 1);
    layout->addWidget(d_->title_label, 0, 2);
    layout->addWidget(d_->account, 0, 3);
    layout->addWidget(window_buttons, 0, 4);
    layout->setColumnStretch(2, 1);

    connect(d_->app_icon, &QToolButton::clicked, this, &QmRibbonTitleBar::appIconClicked);

    connect(d_->minimize, &QToolButton::clicked, this, [this]() {
        if (QWidget* top = window()) {
            top->showMinimized();
        }
    });

    connect(d_->maximize, &QToolButton::clicked, this, &QmRibbonTitleBar::toggleMaximizeRestore);

    connect(d_->close, &QToolButton::clicked, this, [this]() {
        if (QWidget* top = window()) {
            top->close();
        }
    });
}

QmRibbonTitleBar::~QmRibbonTitleBar() noexcept
{
    delete d_;
}

QmRibbonQuickAccessBar* QmRibbonTitleBar::quickAccessBar() const
{
    return d_->quick_access;
}

QToolButton* QmRibbonTitleBar::appIconButton() const
{
    return d_->app_icon;
}

QToolButton* QmRibbonTitleBar::accountButton() const
{
    return d_->account;
}

QToolButton* QmRibbonTitleBar::minimizeButton() const
{
    return d_->minimize;
}

QToolButton* QmRibbonTitleBar::maximizeButton() const
{
    return d_->maximize;
}

QToolButton* QmRibbonTitleBar::closeButton() const
{
    return d_->close;
}

void QmRibbonTitleBar::setAppIcon(const QIcon& icon)
{
    d_->app_icon->setIcon(icon);
    d_->app_icon->setVisible(!icon.isNull());
}

QIcon QmRibbonTitleBar::appIcon() const
{
    return d_->app_icon->icon();
}

QString QmRibbonTitleBar::title() const
{
    return d_->title;
}

void QmRibbonTitleBar::setTitle(const QString& title)
{
    if (d_->title == title) {
        return;
    }

    d_->title = title;
    updateTitleElide();

    emit titleChanged(title);
}

void QmRibbonTitleBar::updateTitleElide()
{
    if (d_->title.isEmpty()) {
        d_->title_label->clear();
        return;
    }

    const QFontMetrics metrics(d_->title_label->font());
    const int available = qMax(24, d_->title_label->width());
    d_->title_label->setText(metrics.elidedText(d_->title, Qt::ElideRight, available));
}

void QmRibbonTitleBar::updateWindowState()
{
    const bool maximized = window() != nullptr && window()->isMaximized();
    d_->maximize->setGlyph(maximized ? WindowButton::Glyph::Restore : WindowButton::Glyph::Maximize);
    d_->maximize->setToolTip(maximized ? tr("Restore") : tr("Maximize"));
}

void QmRibbonTitleBar::toggleMaximizeRestore()
{
    QWidget* top = window();
    if (top == nullptr) {
        return;
    }

    if (top->isMaximized()) {
        top->showNormal();
    } else {
        top->showMaximized();
    }
}

void QmRibbonTitleBar::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    updateTitleElide();
}

void QmRibbonTitleBar::showEvent(QShowEvent* event)
{
    QWidget::showEvent(event);

    QWidget* top = window();
    if (top == nullptr || top == d_->watched_window) {
        return;
    }

    if (d_->watched_window != nullptr) {
        d_->watched_window->removeEventFilter(this);
    }

    d_->watched_window = top;
    top->installEventFilter(this);

    setTitle(top->windowTitle());
    updateWindowState();
}

bool QmRibbonTitleBar::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == d_->watched_window) {
        switch (event->type()) {
        case QEvent::WindowStateChange:
            updateWindowState();
            break;

        case QEvent::WindowTitleChange:
            if (QWidget* top = window()) {
                setTitle(top->windowTitle());
            }
            break;

        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}
