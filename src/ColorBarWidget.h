#ifndef COLORBARWIDGET_H
#define COLORBARWIDGET_H

#include <QWidget>

class QLabel;

class ColorBarGradient : public QWidget
{
    Q_OBJECT

public:
    explicit ColorBarGradient(QWidget *parent = nullptr);

    void setRange(int minValue, int maxValue);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    int m_minValue = 40;
    int m_maxValue = 150;
};

class ColorBarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ColorBarWidget(QWidget *parent = nullptr);

    void setRange(int minValue, int maxValue);

    QSize sizeHint() const override;

private:
    ColorBarGradient *m_gradient;
    QLabel *m_maxLabel;
    QLabel *m_minLabel;
};

#endif
