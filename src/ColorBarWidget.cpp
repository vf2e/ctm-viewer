#include "ColorBarWidget.h"

#include "FieldColorMap.h"

#include <QPainter>
#include <QPainterPath>

ColorBarWidget::ColorBarWidget(QWidget *parent)
    : QWidget(parent)
    , m_minValue(40)
    , m_maxValue(150)
{
    setMinimumWidth(48);
    setMinimumHeight(180);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
}

QSize ColorBarWidget::sizeHint() const
{
    return QSize(48, 320);
}

void ColorBarWidget::setRange(int minValue, int maxValue)
{
    m_minValue = minValue;
    m_maxValue = std::max(minValue + 1, maxValue);
    update();
}

void ColorBarWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int topPad = 22;
    const int bottomPad = 22;
    const QRect barRect(10, topPad, 20, std::max(40, height() - topPad - bottomPad));

    FieldColorMap colorMap;
    colorMap.setRange(m_minValue, m_maxValue);

    for (int y = 0; y < barRect.height(); ++y) {
        const double t = 1.0 - static_cast<double>(y) / std::max(1, barRect.height() - 1);
        const double value = m_minValue + t * (m_maxValue - m_minValue);
        unsigned char rgb[3];
        colorMap.colorForValue(value, rgb);
        painter.setPen(QColor(rgb[0], rgb[1], rgb[2]));
        painter.drawLine(barRect.left(), barRect.top() + y, barRect.right(), barRect.top() + y);
    }

    QPainterPath border;
    border.addRoundedRect(barRect.adjusted(-1, -1, 1, 1), 4, 4);
    painter.setPen(QPen(QColor(255, 255, 255), 2));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(border);

    painter.setPen(QColor(71, 85, 105));
    QFont labelFont = painter.font();
    labelFont.setPointSize(9);
    labelFont.setBold(true);
    painter.setFont(labelFont);
    painter.drawText(QRect(32, topPad - 16, 40, 16),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QString::number(m_maxValue));
    painter.drawText(QRect(32, barRect.bottom() + 2, 40, 16),
                     Qt::AlignLeft | Qt::AlignVCenter,
                     QString::number(m_minValue));
}
