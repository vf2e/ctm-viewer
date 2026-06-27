#include "SettingsDialog.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QFrame>
#include <QGraphicsDropShadowEffect>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVariant>
#include <QVBoxLayout>

namespace {

QPushButton *createThemeCard(const AppTheme::UiThemeEntry &entry, QButtonGroup *group)
{
    auto *card = new QPushButton();
    card->setObjectName(QStringLiteral("themeCard"));
    card->setCheckable(true);
    card->setCursor(Qt::PointingHandCursor);
    card->setProperty("themeId", static_cast<int>(entry.id));

    auto *layout = new QVBoxLayout(card);
    layout->setContentsMargins(14, 14, 14, 14);
    layout->setSpacing(10);

    auto *swatch = new QFrame(card);
    swatch->setObjectName(QStringLiteral("themeSwatch"));
    swatch->setFixedHeight(40);
    swatch->setStyleSheet(QStringLiteral("QFrame#themeSwatch { background: %1; border-radius: 8px; }")
                              .arg(AppTheme::themeSwatchGradient(entry.id)));

    auto *name = new QLabel(entry.name, card);
    name->setObjectName(QStringLiteral("themeCardName"));
    name->setAlignment(Qt::AlignCenter);

    layout->addWidget(swatch);
    layout->addWidget(name);

    group->addButton(card);
    return card;
}

} // namespace

SettingsDialog::SettingsDialog(AppTheme::ThemeId currentTheme, QWidget *parent)
    : QDialog(parent)
    , m_themeGroup(new QButtonGroup(this))
    , m_selectedTheme(currentTheme)
{
    setObjectName(QStringLiteral("settingsDialog"));
    setModal(true);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setFixedSize(420, 400);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(16, 16, 16, 16);

    auto *shell = new QFrame(this);
    shell->setObjectName(QStringLiteral("settingsShell"));

    auto *shadow = new QGraphicsDropShadowEffect(shell);
    shadow->setBlurRadius(14);
    shadow->setOffset(0, 3);
    shadow->setColor(QColor(0, 0, 0, 18));
    shell->setGraphicsEffect(shadow);

    auto *root = new QVBoxLayout(shell);
    root->setContentsMargins(24, 20, 24, 20);
    root->setSpacing(12);

    auto *title = new QLabel(QStringLiteral("设置"));
    title->setObjectName(QStringLiteral("settingsTitle"));

    auto *subtitle = new QLabel(QStringLiteral("界面主题与显示偏好"));
    subtitle->setObjectName(QStringLiteral("settingsSubtitle"));
    subtitle->setWordWrap(true);

    root->addWidget(title);
    root->addWidget(subtitle);

    auto *section = new QLabel(QStringLiteral("界面主题"));
    section->setObjectName(QStringLiteral("settingsSection"));
    root->addWidget(section);

    auto *grid = new QGridLayout();
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(12);

    const QList<AppTheme::UiThemeEntry> themes = AppTheme::availableThemes();
    for (int i = 0; i < themes.size(); ++i) {
        QPushButton *card = createThemeCard(themes.at(i), m_themeGroup);
        if (themes.at(i).id == currentTheme) {
            card->setChecked(true);
        }
        grid->addWidget(card, i / 2, i % 2);
    }

    m_themeGroup->setExclusive(true);
    connect(m_themeGroup, QOverload<QAbstractButton *>::of(&QButtonGroup::buttonClicked), this,
            [this](QAbstractButton *button) {
                m_selectedTheme = static_cast<AppTheme::ThemeId>(button->property("themeId").toInt());
                applyDialogStyle(m_selectedTheme);
            });

    root->addLayout(grid);

    auto *divider = new QFrame(shell);
    divider->setObjectName(QStringLiteral("settingsDivider"));
    divider->setFrameShape(QFrame::HLine);
    root->addWidget(divider);

    auto *actions = new QHBoxLayout();
    actions->addStretch(1);

    auto *okButton = new QPushButton(QStringLiteral("完成"));
    okButton->setObjectName(QStringLiteral("settingsPrimaryBtn"));
    okButton->setCursor(Qt::PointingHandCursor);
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);

    actions->addWidget(okButton);
    root->addLayout(actions);

    outer->addWidget(shell);
    applyDialogStyle(currentTheme);
}

AppTheme::ThemeId SettingsDialog::selectedTheme() const
{
    return m_selectedTheme;
}

void SettingsDialog::applyDialogStyle(AppTheme::ThemeId theme)
{
    setStyleSheet(AppTheme::settingsDialogStylesheet(theme));
}
