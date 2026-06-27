#include "ColorBarWidget.h"

#include "FieldColorMap.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>

ColorBarGradient::ColorBarGradient(QWidget *parent)
    : QWidget(parent)
{
    setMinimumWidth(22);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
}

void ColorBarGradient::setRange(int minValue, int maxValue)
{
    m_minValue = minValue;
    m_maxValue = std::max(minValue + 1, maxValue);
    update();
}

void ColorBarGradient::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event)

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const int pad = 4;
    const QRect barRect(pad, pad, width() - pad * 2, std::max(40, height() - pad * 2));

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
    painter.setPen(QPen(QColor(226, 232, 240), 1));
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(border);
}

ColorBarWidget::ColorBarWidget(QWidget *parent)
    : QWidget(parent)
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(8);

    m_gradient = new ColorBarGradient(this);
    layout->addWidget(m_gradient, 1);

    auto *labelColumn = new QVBoxLayout();
    labelColumn->setContentsMargins(0, 0, 0, 0);
    labelColumn->setSpacing(0);

    m_maxLabel = new QLabel(QStringLiteral("150"), this);
    m_maxLabel->setObjectName(QStringLiteral("valueLabel"));
    m_maxLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    m_minLabel = new QLabel(QStringLiteral("40"), this);
    m_minLabel->setObjectName(QStringLiteral("valueLabel"));
    m_minLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    labelColumn->addWidget(m_maxLabel);
    labelColumn->addStretch(1);
    labelColumn->addWidget(m_minLabel);
    layout->addLayout(labelColumn);

    setMinimumWidth(72);
    setMinimumHeight(180);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
}

QSize ColorBarWidget::sizeHint() const
{
    return QSize(72, 320);
}

void ColorBarWidget::setRange(int minValue, int maxValue)
{
    const int maxClamped = std::max(minValue + 1, maxValue);
    m_maxLabel->setText(QString::number(maxClamped));
    m_minLabel->setText(QString::number(minValue));
    m_gradient->setRange(minValue, maxClamped);
}
