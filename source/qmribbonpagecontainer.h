#pragma once

#include <QStackedWidget>

class QmRibbonPageContainer : public QStackedWidget {
    Q_OBJECT
public:
    explicit QmRibbonPageContainer(QWidget* parent = nullptr);

    void setFloating(bool floating);
    bool isFloating() const;

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void showWithAnimation();
    void hideWithAnimation();
};
