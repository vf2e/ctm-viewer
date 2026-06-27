#include "AppTheme.h"

#include <QtTest>

class AppThemeTest : public QObject
{
    Q_OBJECT

private slots:
    void availableThemes_hasFourEntries();
    void viewportColors_returnsValidRgb();
    void stylesheet_isNonEmptyForAllThemes();
    void settingsDialogStylesheet_containsKeySelectors();
    void themeAccent_isValidHexColor();
};

void AppThemeTest::availableThemes_hasFourEntries()
{
    const auto themes = AppTheme::availableThemes();
    QCOMPARE(themes.size(), 4);
    QCOMPARE(themes.first().name, QStringLiteral("晨曦白"));
}

void AppThemeTest::viewportColors_returnsValidRgb()
{
    for (const auto &entry : AppTheme::availableThemes()) {
        const auto colors = AppTheme::viewportColors(entry.id);
        for (int i = 0; i < 3; ++i) {
            QVERIFY(colors.bg1[i] >= 0.0);
            QVERIFY(colors.bg1[i] <= 1.0);
            QVERIFY(colors.bg2[i] >= 0.0);
            QVERIFY(colors.bg2[i] <= 1.0);
        }
    }
}

void AppThemeTest::stylesheet_isNonEmptyForAllThemes()
{
    for (const auto &entry : AppTheme::availableThemes()) {
        const QString css = AppTheme::stylesheet(entry.id);
        QVERIFY2(!css.isEmpty(), qPrintable(entry.name));
        QVERIFY(css.contains(QStringLiteral("#floatPanel")));
        QVERIFY(css.contains(QStringLiteral("QPushButton#primaryBtn")));
    }
}

void AppThemeTest::settingsDialogStylesheet_containsKeySelectors()
{
    for (const auto &entry : AppTheme::availableThemes()) {
        const QString css = AppTheme::settingsDialogStylesheet(entry.id);
        QVERIFY(css.contains(QStringLiteral("QFrame#settingsShell")));
        QVERIFY(css.contains(QStringLiteral("#themeCard")));
        QVERIFY(css.contains(QStringLiteral("#settingsPrimaryBtn")));
    }
}

void AppThemeTest::themeAccent_isValidHexColor()
{
    for (const auto &entry : AppTheme::availableThemes()) {
        const QString accent = AppTheme::themeAccent(entry.id);
        QVERIFY(accent.startsWith(QLatin1Char('#')));
        QCOMPARE(accent.size(), 7);
    }
}

QTEST_APPLESS_MAIN(AppThemeTest)
#include "test_apptheme.moc"
