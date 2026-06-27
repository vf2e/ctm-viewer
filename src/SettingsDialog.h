#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "AppTheme.h"

#include <QDialog>

class QButtonGroup;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(AppTheme::ThemeId currentTheme, QWidget *parent = nullptr);

    AppTheme::ThemeId selectedTheme() const;

private:
    void applyDialogStyle(AppTheme::ThemeId theme);

    QButtonGroup *m_themeGroup;
    AppTheme::ThemeId m_selectedTheme;
};

#endif
