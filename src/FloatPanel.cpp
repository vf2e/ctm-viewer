#include "FloatPanel.h"

#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

FloatPanel::FloatPanel(const QString &title, QWidget *parent)
    : QFrame(parent)
    , m_collapsed(false)
{
    setObjectName(QStringLiteral("floatPanel"));

    auto *shadow = new QGraphicsDropShadowEffect(this);
    shadow->setBlurRadius(24);
    shadow->setOffset(0, 4);
    shadow->setColor(QColor(15, 23, 42, 28));
    setGraphicsEffect(shadow);

    auto *root = new QVBoxLayout(this);
    root->setContentsMargins(14, 12, 14, 14);
    root->setSpacing(10);

    auto *header = new QHBoxLayout();
    header->setSpacing(8);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName(QStringLiteral("floatPanelTitle"));

    m_collapseButton = new QPushButton(QStringLiteral("−"), this);
    m_collapseButton->setObjectName(QStringLiteral("iconBtn"));
    m_collapseButton->setFixedSize(28, 28);
    m_collapseButton->setCursor(Qt::PointingHandCursor);
    connect(m_collapseButton, &QPushButton::clicked, this, [this]() {
        setCollapsed(!m_collapsed);
    });

    header->addWidget(m_titleLabel, 1);
    header->addWidget(m_collapseButton);

    m_body = new QWidget(this);
    m_bodyLayout = new QVBoxLayout(m_body);
    m_bodyLayout->setContentsMargins(0, 0, 0, 0);
    m_bodyLayout->setSpacing(8);

    root->addLayout(header);
    root->addWidget(m_body, 1);
}

QVBoxLayout *FloatPanel::contentLayout() const
{
    return m_bodyLayout;
}

void FloatPanel::setCollapsed(bool collapsed)
{
    m_collapsed = collapsed;
    m_body->setVisible(!collapsed);
    m_collapseButton->setText(collapsed ? QStringLiteral("+") : QStringLiteral("−"));
    emit collapsedChanged(collapsed);
    updateGeometry();
}
