#include "qmribbontitlebar.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

struct QmRibbonTitleBarPrivate {
    QToolButton* btn_app_icon;
    QWidget* wgt_quick_access;
    QToolButton* btn_account;
    QToolButton* btn_minimized;
    QToolButton* btn_maximized;
    QToolButton* btn_close;
};

QmRibbonTitleBar::QmRibbonTitleBar(QWidget* parent)
    : QWidget(parent)
    , d_(new QmRibbonTitleBarPrivate)
{
    setObjectName("RibbonTitleBar");
    setAttribute(Qt::WA_StyledBackground, true);
    initUi();
}

QmRibbonTitleBar::~QmRibbonTitleBar() noexcept
{
    delete d_;
}

void QmRibbonTitleBar::initUi()
{
    auto* lyt_main = new QHBoxLayout(this);
    lyt_main->setContentsMargins(0, 0, 0, 0);

    d_->btn_app_icon = new QToolButton(this);
    d_->btn_app_icon->setObjectName("RibbonAppIconButton");
    lyt_main->addWidget(d_->btn_app_icon);

    d_->wgt_quick_access = new QWidget(this);
    lyt_main->addWidget(d_->wgt_quick_access);

    lyt_main->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Preferred));

    d_->btn_minimized = new QToolButton(this);
    d_->btn_minimized->setObjectName("RibbonWindowMinButton");
    d_->btn_maximized = new QToolButton(this);
    d_->btn_maximized->setObjectName("RibbonWindowMaxButton");
    d_->btn_close = new QToolButton(this);
    d_->btn_close->setObjectName("RibbonWindowCloseButton");

    d_->btn_account = new QToolButton(this);
    d_->btn_account->setObjectName("RibbonAccountButton");

    lyt_main->addWidget(d_->btn_account);
    lyt_main->addWidget(d_->btn_minimized);
    lyt_main->addWidget(d_->btn_maximized);
    lyt_main->addWidget(d_->btn_close);
}
