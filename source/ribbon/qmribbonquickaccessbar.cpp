#include "qmribbonquickaccessbar.h"

#include "qmribbontheme.h"

#include <QAction>
#include <QFrame>
#include <QHBoxLayout>
#include <QHash>
#include <QToolButton>

#include <utility>

struct QmRibbonQuickAccessBar::QmRibbonQuickAccessBarPrivate {
    QHBoxLayout* layout { nullptr };
    QHash<QAction*, QToolButton*> buttons;
    QSize icon_size { QmRibbonMetrics::icon_size, QmRibbonMetrics::icon_size };
};

QmRibbonQuickAccessBar::QmRibbonQuickAccessBar(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonQuickAccessBarPrivate)
{
    setObjectName(QStringLiteral("RibbonQuickAccessBar"));
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    d_->layout = new QHBoxLayout(this);
    d_->layout->setContentsMargins(0, 0, 0, 0);
    d_->layout->setSpacing(0);
}

QmRibbonQuickAccessBar::~QmRibbonQuickAccessBar() noexcept
{
    delete d_;
}

QToolButton* QmRibbonQuickAccessBar::addAction(QAction* action)
{
    if (action == nullptr) {
        return nullptr;
    }

    auto* button = new QToolButton(this);
    button->setObjectName(QStringLiteral("RibbonQuickAccessButton"));
    button->setDefaultAction(action);
    button->setAutoRaise(true);
    button->setFocusPolicy(Qt::NoFocus);
    button->setCursor(Qt::ArrowCursor);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setIconSize(d_->icon_size);
    button->setFixedSize(QmRibbonMetrics::quick_access_button_size, QmRibbonMetrics::quick_access_button_size);

    d_->buttons.insert(action, button);
    d_->layout->addWidget(button);

    connect(action, &QObject::destroyed, this, [this, action]() {
        if (QToolButton* owned = d_->buttons.take(action)) {
            d_->layout->removeWidget(owned);
            owned->deleteLater();
        }
    });

    return button;
}

QToolButton* QmRibbonQuickAccessBar::insertAction(QAction* before, QAction* action)
{
    if (before == nullptr || !d_->buttons.contains(before)) {
        return addAction(action);
    }

    auto* button = new QToolButton(this);
    button->setObjectName(QStringLiteral("RibbonQuickAccessButton"));
    button->setDefaultAction(action);
    button->setAutoRaise(true);
    button->setFocusPolicy(Qt::NoFocus);
    button->setCursor(Qt::ArrowCursor);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setIconSize(d_->icon_size);
    button->setFixedSize(QmRibbonMetrics::quick_access_button_size, QmRibbonMetrics::quick_access_button_size);

    d_->buttons.insert(action, button);
    d_->layout->insertWidget(d_->layout->indexOf(d_->buttons.value(before)), button);

    connect(action, &QObject::destroyed, this, [this, action]() {
        if (QToolButton* owned = d_->buttons.take(action)) {
            d_->layout->removeWidget(owned);
            owned->deleteLater();
        }
    });

    return button;
}

void QmRibbonQuickAccessBar::addSeparator()
{
    auto* separator = new QFrame(this);
    separator->setObjectName(QStringLiteral("RibbonQuickAccessSeparator"));
    separator->setFrameShape(QFrame::VLine);
    separator->setFrameShadow(QFrame::Plain);
    separator->setFixedWidth(1);
    separator->setFixedHeight(QmRibbonMetrics::icon_size);
    separator->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

    d_->layout->addSpacing(2);
    d_->layout->addWidget(separator);
    d_->layout->addSpacing(2);
}

void QmRibbonQuickAccessBar::removeAction(QAction* action)
{
    QToolButton* button = d_->buttons.take(action);
    if (button == nullptr) {
        return;
    }

    d_->layout->removeWidget(button);
    button->deleteLater();
}

void QmRibbonQuickAccessBar::clear()
{
    const QList<QAction*> owned = d_->buttons.keys();
    for (QAction* action : owned) {
        removeAction(action);
    }
}

QList<QAction*> QmRibbonQuickAccessBar::actions() const
{
    return d_->buttons.keys();
}

QToolButton* QmRibbonQuickAccessBar::buttonForAction(QAction* action) const
{
    return d_->buttons.value(action, nullptr);
}

QSize QmRibbonQuickAccessBar::iconSize() const
{
    return d_->icon_size;
}

void QmRibbonQuickAccessBar::setIconSize(const QSize& size)
{
    d_->icon_size = size;
    for (QToolButton* button : std::as_const(d_->buttons)) {
        button->setIconSize(size);
    }
}
