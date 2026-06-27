#ifndef COLORBARWIDGET_H
#define COLORBARWIDGET_H

#include <QWidget>

class ColorBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ColorBarWidget(QWidget *parent = nullptr);

    void setRange(int minValue, int maxValue);

protected:
    void paintEvent(QPaintEvent *event) override;
    QSize sizeHint() const override;

private:
    int m_minValue;
    int m_maxValue;
};

#endif
