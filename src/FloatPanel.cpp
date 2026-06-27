#include "FloatPanel.h"

#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QVBoxLayout>

namespace {

QString verticalLabelText(const QString &text)
{
    QString vertical;
    vertical.reserve(text.size() * 2);
    for (int i = 0; i < text.size(); ++i) {
        if (i > 0) {
            vertical.append(QLatin1Char('\n'));
        }
        vertical.append(text.at(i));
    }
    return vertical;
}

} // namespace

FloatPanel::FloatPanel(const QString &title,
                       const QString &shortTitle,
                       Side side,
                       QWidget *parent)
    : QFrame(parent)
    , m_side(side)
    , m_shortTitle(shortTitle)
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

    auto *headerWidget = new QWidget(this);
    m_headerLayout = new QHBoxLayout(headerWidget);
    m_headerLayout->setContentsMargins(0, 0, 0, 0);
    m_headerLayout->setSpacing(8);

    m_titleLabel = new QLabel(title, this);
    m_titleLabel->setObjectName(QStringLiteral("floatPanelTitle"));

    m_collapsedTitle = new QLabel(verticalLabelText(shortTitle), this);
    m_collapsedTitle->setObjectName(QStringLiteral("collapsedRailTitle"));
    m_collapsedTitle->setAlignment(Qt::AlignCenter);
    m_collapsedTitle->setVisible(false);

    m_collapseButton = new QPushButton(this);
    m_collapseButton->setObjectName(QStringLiteral("railBtn"));
    m_collapseButton->setFixedSize(30, 30);
    m_collapseButton->setCursor(Qt::PointingHandCursor);
    connect(m_collapseButton, &QPushButton::clicked, this, [this]() {
        setCollapsed(!m_collapsed);
    });

    if (m_side == Side::Left) {
        m_headerLayout->addWidget(m_titleLabel, 1);
        m_headerLayout->addWidget(m_collapseButton);
    } else {
        m_headerLayout->addWidget(m_collapseButton);
        m_headerLayout->addWidget(m_titleLabel, 1);
    }

    m_body = new QWidget(this);
    m_bodyLayout = new QVBoxLayout(m_body);
    m_bodyLayout->setContentsMargins(0, 0, 0, 0);
    m_bodyLayout->setSpacing(8);

    root->addWidget(headerWidget);
    root->addWidget(m_collapsedTitle, 0, Qt::AlignHCenter);
    root->addWidget(m_body, 1);

    updateCollapsedPresentation();
}

QVBoxLayout *FloatPanel::contentLayout() const
{
    return m_bodyLayout;
}

bool FloatPanel::isCollapsed() const
{
    return m_collapsed;
}

void FloatPanel::updateCollapsedPresentation()
{
    m_body->setVisible(!m_collapsed);
    m_titleLabel->setVisible(!m_collapsed);
    m_collapsedTitle->setVisible(m_collapsed);

    if (m_collapsed) {
        setObjectName(QStringLiteral("floatPanelCollapsed"));
        layout()->setContentsMargins(6, 10, 6, 10);
        if (m_side == Side::Left) {
            m_headerLayout->setAlignment(m_collapseButton, Qt::AlignHCenter);
        } else {
            m_headerLayout->setAlignment(m_collapseButton, Qt::AlignHCenter);
        }
        m_collapseButton->setText(m_side == Side::Left ? QStringLiteral("›") : QStringLiteral("‹"));
        m_collapseButton->setToolTip(m_side == Side::Left
                                         ? QStringLiteral("展开面板")
                                         : QStringLiteral("展开色标"));
    } else {
        setObjectName(QStringLiteral("floatPanel"));
        layout()->setContentsMargins(14, 12, 14, 14);
        m_collapseButton->setText(m_side == Side::Left ? QStringLiteral("‹") : QStringLiteral("›"));
        m_collapseButton->setToolTip(m_side == Side::Left
                                         ? QStringLiteral("收起面板")
                                         : QStringLiteral("收起色标"));
    }

    style()->unpolish(this);
    style()->polish(this);
    updateGeometry();
}

void FloatPanel::setCollapsed(bool collapsed)
{
    if (m_collapsed == collapsed) {
        return;
    }

    m_collapsed = collapsed;
    updateCollapsedPresentation();
    emit collapsedChanged(collapsed);
}
