#pragma once

#include <QWidget>

struct QmRibbonPageContainerPrivate;
class QmRibbonPageContainer : public QWidget {
    Q_OBJECT
public:
    explicit QmRibbonPageContainer(QWidget* parent = nullptr);
    ~QmRibbonPageContainer() noexcept override;

    void setFloating(bool floating);
    bool isFloating() const;

    int addWidget(QWidget* widget);

    QWidget* currentWidget() const;
    int currentIndex() const;
    void setCurrentIndex(int index);

protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

private:
    void initUi();
    void showWithAnimation();
    void hideWithAnimation();

private:
    QmRibbonPageContainerPrivate* d_ { nullptr };
};
