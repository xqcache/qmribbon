#include "launcherpopup.h"

#include "qmribbonthememgr.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFont>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QScreen>
#include <QSpinBox>
#include <QVBoxLayout>

namespace {

/// 预览用的示例文本（中英文 + 数字，方便看字形）。
const char* const kPreviewText = "字体预览 AaBbGg 0123";

/// 按当前选择刷新预览。
void updatePreview(QLabel* preview, const QComboBox* family, const QSpinBox* size, const QCheckBox* bold)
{
    QFont font(family->currentText());
    font.setPointSize(size->value());
    font.setBold(bold != nullptr && bold->isChecked());
    preview->setFont(font);
    preview->setText(QString::fromLatin1(kPreviewText));
}

} // namespace

void showFontLauncherPopup(QWidget* anchor)
{
    if (anchor == nullptr) {
        return;
    }

    // Qt::Popup：点面板外面（或按 Esc）就关掉，和 Office 的这类浮出面板一致。
    QDialog popup(anchor->window(), Qt::Popup);
    popup.setObjectName(QStringLiteral("RibbonLauncherPopup"));

    // 业务自己的窗口登记一次主题，面板就跟着主题走（配色 + 样式表）。
    QmRibbonThemeMgr::instance().apply(&popup);

    auto* layout = new QVBoxLayout(&popup);
    layout->setContentsMargins(14, 12, 14, 12);
    layout->setSpacing(10);

    auto* title = new QLabel(QObject::tr("字体"), &popup);
    title->setObjectName(QStringLiteral("RibbonLauncherPopupTitle"));
    layout->addWidget(title);

    auto* hint = new QLabel(QObject::tr("Dialog Box Launcher（↘）的模拟弹窗"), &popup);
    hint->setObjectName(QStringLiteral("RibbonLauncherPopupHint"));
    layout->addWidget(hint);

    auto* form = new QFormLayout;
    form->setContentsMargins(0, 0, 0, 0);
    form->setSpacing(8);

    auto* family = new QComboBox(&popup);
    family->addItems({ QStringLiteral("等线"), QStringLiteral("微软雅黑"), QStringLiteral("宋体"),
                       QStringLiteral("Arial") });
    form->addRow(QObject::tr("中文字体"), family);

    auto* style = new QComboBox(&popup);
    style->addItems({ QObject::tr("常规"), QObject::tr("倾斜"), QObject::tr("加粗"), QObject::tr("加粗 倾斜") });
    form->addRow(QObject::tr("字形"), style);

    auto* size = new QSpinBox(&popup);
    size->setRange(8, 72);
    size->setValue(11);
    size->setSuffix(QStringLiteral(" pt"));
    form->addRow(QObject::tr("字号"), size);

    layout->addLayout(form);

    auto* bold = new QCheckBox(QObject::tr("加粗"), &popup);
    auto* underline = new QCheckBox(QObject::tr("下划线"), &popup);
    underline->setChecked(true);
    layout->addWidget(bold);
    layout->addWidget(underline);

    auto* preview = new QLabel(&popup);
    preview->setObjectName(QStringLiteral("RibbonLauncherPopupPreview"));
    preview->setAlignment(Qt::AlignCenter);
    layout->addWidget(preview);

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &popup);
    buttons->button(QDialogButtonBox::Ok)->setText(QObject::tr("确定"));
    buttons->button(QDialogButtonBox::Cancel)->setText(QObject::tr("取消"));
    layout->addWidget(buttons);

    QObject::connect(buttons, &QDialogButtonBox::accepted, &popup, &QDialog::accept);
    QObject::connect(buttons, &QDialogButtonBox::rejected, &popup, &QDialog::reject);

    // 模拟「改设置即时看效果」：字体 / 字号 / 加粗一变就刷新预览。
    const auto refresh = [preview, family, size, bold]() { updatePreview(preview, family, size, bold); };
    QObject::connect(family, &QComboBox::currentTextChanged, &popup, refresh);
    QObject::connect(size, &QSpinBox::valueChanged, &popup, refresh);
    QObject::connect(bold, &QCheckBox::toggled, &popup, refresh);
    refresh();

    // 贴着 launcher 按钮弹出；放不下就翻到按钮上方、并且不越出屏幕。
    popup.adjustSize();
    QPoint position = anchor->mapToGlobal(QPoint(0, anchor->height() + 2));

    if (QScreen* screen = anchor->screen()) {
        const QRect available = screen->availableGeometry();
        if (position.x() + popup.width() > available.right()) {
            position.setX(available.right() - popup.width());
        }
        if (position.y() + popup.height() > available.bottom()) {
            position = anchor->mapToGlobal(QPoint(0, -popup.height() - 2));
        }
        position.setX(qMax(position.x(), available.left()));
        position.setY(qMax(position.y(), available.top()));
    }

    popup.move(position);
    popup.exec();
}
