#include "qmribbontabbar.h"

#include <QButtonGroup>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QSpacerItem>
#include <QTabBar>
#include <QToolButton>

struct QmRibbonTabBarPrivate {
    QToolButton* app_button { nullptr };
    QButtonGroup* tab_button_group { nullptr };
};

QmRibbonTabBar::QmRibbonTabBar(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonTabBarPrivate)
{
    setAttribute(Qt::WA_StyledBackground, true);
    initUi();
}

QmRibbonTabBar::~QmRibbonTabBar() noexcept
{
    delete d_;
}

void QmRibbonTabBar::initUi()
{
    d_->app_button = new QToolButton(this);
    d_->app_button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    d_->app_button->setProperty("Style", "RibbonApplicationButton");
    d_->tab_button_group = new QButtonGroup(this);

    auto* lyt_main = new QHBoxLayout(this);
    lyt_main->setContentsMargins(0, 1, 0, 1);
    lyt_main->setSpacing(0);
    lyt_main->addWidget(d_->app_button);
    lyt_main->addSpacerItem(new QSpacerItem(20, 0, QSizePolicy::Expanding));

    connect(d_->tab_button_group, &QButtonGroup::buttonClicked, this, [this](QAbstractButton* button) {
        int button_index = d_->tab_button_group->buttons().indexOf(button);
        if (button_index >= 0) {
            emit tabActivated(button_index, QPrivateSignal());
        }
    });
}

QToolButton* QmRibbonTabBar::addTab(const QString& title, const QIcon& icon)
{
    auto* tab_button = new QToolButton(this);
    tab_button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    tab_button->setProperty("Style", "RibbonTabButton");
    tab_button->setCheckable(true);
    tab_button->setIcon(icon);
    tab_button->setText(title);
    tab_button->installEventFilter(this);
    static_cast<QBoxLayout*>(layout())->insertWidget(static_cast<QBoxLayout*>(layout())->count() - 1, tab_button);
    d_->tab_button_group->addButton(tab_button);

    if (d_->tab_button_group->buttons().size() == 1) {
        tab_button->setChecked(true);
    }
    return tab_button;
}

QToolButton* QmRibbonTabBar::applicationButton() const
{
    return d_->app_button;
}

void QmRibbonTabBar::setApplicationButtonVisible(bool visible)
{
    d_->app_button->setVisible(visible);
}

bool QmRibbonTabBar::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type()) {
    case QEvent::MouseButtonDblClick: {
        QMouseEvent* mouse_evt = static_cast<QMouseEvent*>(event);
        if (mouse_evt->buttons() == Qt::LeftButton && mouse_evt->modifiers() == Qt::NoModifier) {
            emit requestToggleFloating(QPrivateSignal());
        }
    } break;
    default:
        break;
    }
    return QWidget::eventFilter(watched, event);
}

void QmRibbonTabBar::activeTab(int index)
{
    const auto& buttons = d_->tab_button_group->buttons();
    if (index < 0 || index >= buttons.size()) {
        return;
    }
    buttons[index]->setChecked(true);
}

void QmRibbonTabBar::changeEvent(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange) {
    }
    QWidget::changeEvent(event);
}
