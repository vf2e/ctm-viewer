#ifndef APPTHEME_H
#define APPTHEME_H

#include <QString>
#include <QList>

namespace AppTheme {

struct ViewportColors {
    double bg1[3];
    double bg2[3];
};

enum class ThemeId {
    Light,
    Dark,
    Ocean,
    Mint,
};

struct UiThemeEntry {
    ThemeId id;
    QString name;
};

inline QList<UiThemeEntry> availableThemes()
{
    return {
        {ThemeId::Light, QStringLiteral("晨曦白")},
        {ThemeId::Dark, QStringLiteral("深空夜")},
        {ThemeId::Ocean, QStringLiteral("海洋蓝")},
        {ThemeId::Mint, QStringLiteral("薄荷绿")},
    };
}

inline ViewportColors viewportColors(ThemeId theme)
{
    switch (theme) {
    case ThemeId::Dark:
        return {{0.10, 0.12, 0.16}, {0.06, 0.07, 0.10}};
    case ThemeId::Ocean:
        return {{0.93, 0.97, 1.00}, {0.84, 0.92, 0.98}};
    case ThemeId::Mint:
        return {{0.96, 0.99, 0.97}, {0.90, 0.97, 0.93}};
    case ThemeId::Light:
    default:
        return {{0.98, 0.99, 1.00}, {0.94, 0.96, 0.99}};
    }
}

inline QString stylesheet(ThemeId theme)
{
    switch (theme) {
    case ThemeId::Dark:
        return QStringLiteral(R"(
QMainWindow { background: #0F1419; }
QWidget { color: #E2E8F0; }
QMenuBar {
    background: #151B23;
    border-bottom: 1px solid #243044;
    padding: 2px 8px;
    font-size: 12px;
    color: #94A3B8;
}
QMenuBar::item { padding: 5px 10px; border-radius: 6px; }
QMenuBar::item:selected { background: #1E293B; color: #60A5FA; }
QMenu {
    background: #1A2332;
    border: 1px solid #334155;
    border-radius: 8px;
    padding: 6px;
    color: #E2E8F0;
}
QMenu::item:selected { background: #2563EB; }
QStatusBar {
    background: #151B23;
    border-top: 1px solid #243044;
    color: #94A3B8;
    font-size: 12px;
}
#floatPanel {
    background: rgba(26, 35, 50, 0.96);
    border: 1px solid #334155;
    border-radius: 14px;
}
#floatPanelCollapsed {
    background: rgba(26, 35, 50, 0.96);
    border: 1px solid #334155;
    border-radius: 12px;
}
#floatPanelTitle { color: #F1F5F9; font-size: 13px; font-weight: bold; }
#collapsedRailTitle {
    color: #94A3B8;
    font-size: 11px;
    font-weight: bold;
    line-height: 1.1;
    padding-top: 4px;
}
#railBtn {
    background: #1E293B;
    color: #94A3B8;
    border: 1px solid #475569;
    border-radius: 8px;
    font-size: 16px;
    font-weight: bold;
}
#railBtn:hover { background: #243044; color: #BFDBFE; border-color: #60A5FA; }
#groupTitle { color: #94A3B8; font-size: 11px; font-weight: bold; padding-top: 2px; }
#fieldLabel { color: #94A3B8; font-size: 12px; min-width: 28px; }
#valueLabel { color: #E2E8F0; font-size: 12px; font-weight: 600; min-width: 36px; }
#unitLabel { color: #64748B; font-size: 11px; }
#sectionDivider { background: #334155; max-height: 1px; min-height: 1px; }
QPushButton#primaryBtn {
    background: #3B82F6; color: #FFFFFF; border: none; border-radius: 8px;
    padding: 0 14px; font-size: 13px; font-weight: bold; min-height: 34px;
}
QPushButton#primaryBtn:hover { background: #2563EB; }
QPushButton#roamBtn {
    background: #1E293B; color: #94A3B8; border: 1px solid #475569; border-radius: 8px;
    padding: 0 14px; font-size: 13px; min-height: 34px;
}
QPushButton#roamBtn:checked {
    background: #7C3AED; color: #FFFFFF; border-color: #7C3AED;
}
QPushButton#roamBtn:hover { border-color: #60A5FA; }
QPushButton#secondaryBtn {
    background: #1E293B; color: #CBD5E1; border: 1px solid #475569;
    border-radius: 8px; padding: 0 14px; font-size: 13px; min-height: 34px;
}
QPushButton#secondaryBtn:hover { background: #243044; border-color: #60A5FA; }
QPushButton#iconBtn {
    background: #1E293B; color: #94A3B8; border: 1px solid #475569;
    border-radius: 8px; font-size: 14px; font-weight: bold;
}
QPushButton#iconBtn:hover { background: #243044; color: #60A5FA; }
QPushButton#toggleBtn {
    background: #1E293B; color: #94A3B8; border: 1px solid #475569;
    border-radius: 8px; padding: 8px 12px; font-size: 12px; min-height: 32px; text-align: left;
}
QPushButton#toggleBtn:checked {
    background: #1E3A5F; color: #BFDBFE; border-color: #3B82F6; font-weight: 600;
}
QPushButton#toggleBtn:hover { background: #243044; border-color: #60A5FA; }
QSpinBox#flatField {
    background: #1E293B; color: #E2E8F0; border: 1px solid #475569;
    border-radius: 8px; padding: 0 12px; min-height: 38px; max-height: 38px;
    font-size: 14px;
}
QSpinBox#flatField:focus { border-color: #93C5FD; background: #151B23; }
QWidget#modalOverlay { background-color: rgba(15, 23, 42, 0.35); }
QSlider::groove:horizontal { height: 5px; background: #334155; border-radius: 3px; }
QSlider::sub-page:horizontal { background: #475569; border-radius: 3px; }
QSlider#primarySlider::sub-page:horizontal { background: #60A5FA; }
QSlider#accentSlider::sub-page:horizontal { background: #38BDF8; }
QSlider::handle:horizontal {
    width: 16px; height: 16px; margin: -6px 0; border-radius: 8px;
    background: #1E293B; border: 2px solid #64748B;
}
QSlider#primarySlider::handle:horizontal { border-color: #3B82F6; }
QSlider#accentSlider::handle:horizontal { border-color: #38BDF8; }
)");

    case ThemeId::Ocean:
        return QStringLiteral(R"(
QMainWindow { background: #EEF6FC; }
QMenuBar {
    background: #F5FAFF; border-bottom: 1px solid #C8DDF0;
    padding: 2px 8px; font-size: 12px; color: #3B6B8C;
}
QMenuBar::item { padding: 5px 10px; border-radius: 6px; }
QMenuBar::item:selected { background: #D6EBFA; color: #0369A1; }
QMenu { background: #FFFFFF; border: 1px solid #C8DDF0; border-radius: 8px; padding: 6px; }
QStatusBar {
    background: #F5FAFF; border-top: 1px solid #C8DDF0;
    color: #5B8BAE; font-size: 12px;
}
#floatPanel {
    background: rgba(255, 255, 255, 0.95);
    border: 1px solid #B8D4EA; border-radius: 14px;
}
#floatPanelCollapsed {
    background: rgba(255, 255, 255, 0.95);
    border: 1px solid #B8D4EA; border-radius: 12px;
}
#floatPanelTitle { color: #0C4A6E; font-size: 13px; font-weight: bold; }
#collapsedRailTitle {
    color: #5B8BAE;
    font-size: 11px;
    font-weight: bold;
    line-height: 1.1;
    padding-top: 4px;
}
#railBtn {
    background: #F0F9FF;
    color: #5B8BAE;
    border: 1px solid #B8D4EA;
    border-radius: 8px;
    font-size: 16px;
    font-weight: bold;
}
#railBtn:hover { background: #E0F2FE; color: #0284C7; border-color: #38BDF8; }
#groupTitle { color: #5B8BAE; font-size: 11px; font-weight: bold; padding-top: 2px; }
#fieldLabel { color: #5B8BAE; font-size: 12px; min-width: 28px; }
#valueLabel { color: #0C4A6E; font-size: 12px; font-weight: 600; min-width: 36px; }
#unitLabel { color: #7BA3C0; font-size: 11px; }
#sectionDivider { background: #C8DDF0; max-height: 1px; min-height: 1px; }
QPushButton#primaryBtn {
    background: #0284C7; color: #FFFFFF; border: none; border-radius: 8px;
    padding: 0 14px; font-size: 13px; font-weight: bold; min-height: 34px;
}
QPushButton#primaryBtn:hover { background: #0369A1; }
QPushButton#roamBtn {
    background: #FFFFFF; color: #5B8BAE; border: 1px solid #B8D4EA;
    border-radius: 8px; padding: 0 14px; font-size: 13px; min-height: 34px;
}
QPushButton#roamBtn:checked { background: #0EA5E9; color: #FFFFFF; border-color: #0EA5E9; }
QPushButton#secondaryBtn {
    background: #FFFFFF; color: #3B6B8C; border: 1px solid #B8D4EA;
    border-radius: 8px; padding: 0 14px; font-size: 13px; min-height: 34px;
}
QPushButton#secondaryBtn:hover { background: #F0F9FF; border-color: #38BDF8; }
QPushButton#iconBtn {
    background: #F0F9FF; color: #5B8BAE; border: 1px solid #B8D4EA;
    border-radius: 8px; font-size: 14px; font-weight: bold;
}
QPushButton#iconBtn:hover { background: #E0F2FE; color: #0284C7; }
QPushButton#toggleBtn {
    background: #FFFFFF; color: #5B8BAE; border: 1px solid #B8D4EA;
    border-radius: 8px; padding: 8px 12px; font-size: 12px; min-height: 32px; text-align: left;
}
QPushButton#toggleBtn:checked {
    background: #E0F2FE; color: #0369A1; border-color: #38BDF8; font-weight: 600;
}
QPushButton#toggleBtn:hover { background: #F0F9FF; border-color: #0EA5E9; }
QSpinBox#flatField {
    background: #FFFFFF; color: #0C4A6E; border: 1px solid #CBD5E1;
    border-radius: 8px; padding: 0 12px; min-height: 38px; max-height: 38px;
    font-size: 14px;
}
QSpinBox#flatField:focus { border-color: #93C5FD; background: #FFFFFF; }
QWidget#modalOverlay { background-color: rgba(15, 23, 42, 0.35); }
QSlider::groove:horizontal { height: 5px; background: #D6EBFA; border-radius: 3px; }
QSlider::sub-page:horizontal { background: #93C5FD; border-radius: 3px; }
QSlider#primarySlider::sub-page:horizontal { background: #38BDF8; }
QSlider#accentSlider::sub-page:horizontal { background: #0EA5E9; }
QSlider::handle:horizontal {
    width: 16px; height: 16px; margin: -6px 0; border-radius: 8px;
    background: #FFFFFF; border: 2px solid #93C5FD;
}
QSlider#primarySlider::handle:horizontal { border-color: #0284C7; }
QSlider#accentSlider::handle:horizontal { border-color: #0EA5E9; }
)");

    case ThemeId::Mint:
        return QStringLiteral(R"(
QMainWindow { background: #F2FAF5; }
QMenuBar {
    background: #F8FDF9; border-bottom: 1px solid #C6E7D4;
    padding: 2px 8px; font-size: 12px; color: #4D7A62;
}
QMenuBar::item { padding: 5px 10px; border-radius: 6px; }
QMenuBar::item:selected { background: #DCFCE7; color: #15803D; }
QMenu { background: #FFFFFF; border: 1px solid #C6E7D4; border-radius: 8px; padding: 6px; }
QStatusBar {
    background: #F8FDF9; border-top: 1px solid #C6E7D4;
    color: #5F8F72; font-size: 12px;
}
#floatPanel {
    background: rgba(255, 255, 255, 0.95);
    border: 1px solid #BBE4CC; border-radius: 14px;
}
#floatPanelCollapsed {
    background: rgba(255, 255, 255, 0.95);
    border: 1px solid #BBE4CC; border-radius: 12px;
}
#floatPanelTitle { color: #14532D; font-size: 13px; font-weight: bold; }
#collapsedRailTitle {
    color: #5F8F72;
    font-size: 11px;
    font-weight: bold;
    line-height: 1.1;
    padding-top: 4px;
}
#railBtn {
    background: #F0FDF4;
    color: #5F8F72;
    border: 1px solid #BBE4CC;
    border-radius: 8px;
    font-size: 16px;
    font-weight: bold;
}
#railBtn:hover { background: #DCFCE7; color: #16A34A; border-color: #4ADE80; }
#groupTitle { color: #5F8F72; font-size: 11px; font-weight: bold; padding-top: 2px; }
#fieldLabel { color: #5F8F72; font-size: 12px; min-width: 28px; }
#valueLabel { color: #14532D; font-size: 12px; font-weight: 600; min-width: 36px; }
#unitLabel { color: #7BAF8E; font-size: 11px; }
#sectionDivider { background: #C6E7D4; max-height: 1px; min-height: 1px; }
QPushButton#primaryBtn {
    background: #16A34A; color: #FFFFFF; border: none; border-radius: 8px;
    padding: 0 14px; font-size: 13px; font-weight: bold; min-height: 34px;
}
QPushButton#primaryBtn:hover { background: #15803D; }
QPushButton#roamBtn {
    background: #FFFFFF; color: #5F8F72; border: 1px solid #BBE4CC;
    border-radius: 8px; padding: 0 14px; font-size: 13px; min-height: 34px;
}
QPushButton#roamBtn:checked { background: #059669; color: #FFFFFF; border-color: #059669; }
QPushButton#secondaryBtn {
    background: #FFFFFF; color: #4D7A62; border: 1px solid #BBE4CC;
    border-radius: 8px; padding: 0 14px; font-size: 13px; min-height: 34px;
}
QPushButton#secondaryBtn:hover { background: #F0FDF4; border-color: #4ADE80; }
QPushButton#iconBtn {
    background: #F0FDF4; color: #5F8F72; border: 1px solid #BBE4CC;
    border-radius: 8px; font-size: 14px; font-weight: bold;
}
QPushButton#iconBtn:hover { background: #DCFCE7; color: #16A34A; }
QPushButton#toggleBtn {
    background: #FFFFFF; color: #5F8F72; border: 1px solid #BBE4CC;
    border-radius: 8px; padding: 8px 12px; font-size: 12px; min-height: 32px; text-align: left;
}
QPushButton#toggleBtn:checked {
    background: #DCFCE7; color: #15803D; border-color: #4ADE80; font-weight: 600;
}
QPushButton#toggleBtn:hover { background: #F0FDF4; border-color: #34D399; }
QSpinBox#flatField {
    background: #FFFFFF; color: #14532D; border: 1px solid #CBD5E1;
    border-radius: 8px; padding: 0 12px; min-height: 38px; max-height: 38px;
    font-size: 14px;
}
QSpinBox#flatField:focus { border-color: #93C5FD; background: #FFFFFF; }
QWidget#modalOverlay { background-color: rgba(15, 23, 42, 0.35); }
QSlider::groove:horizontal { height: 5px; background: #DCFCE7; border-radius: 3px; }
QSlider::sub-page:horizontal { background: #86EFAC; border-radius: 3px; }
QSlider#primarySlider::sub-page:horizontal { background: #4ADE80; }
QSlider#accentSlider::sub-page:horizontal { background: #34D399; }
QSlider::handle:horizontal {
    width: 16px; height: 16px; margin: -6px 0; border-radius: 8px;
    background: #FFFFFF; border: 2px solid #86EFAC;
}
QSlider#primarySlider::handle:horizontal { border-color: #16A34A; }
QSlider#accentSlider::handle:horizontal { border-color: #059669; }
)");

    case ThemeId::Light:
    default:
        return QStringLiteral(R"(
QMainWindow { background: #FAFBFE; }
QMenuBar {
    background: #FCFCFD; border-bottom: 1px solid #E8ECF2;
    padding: 2px 8px; font-size: 12px; color: #475569;
}
QMenuBar::item { padding: 5px 10px; border-radius: 6px; }
QMenuBar::item:selected { background: #F1F5F9; color: #2563EB; }
QMenu { background: #FFFFFF; border: 1px solid #E8ECF2; border-radius: 8px; padding: 6px; }
QStatusBar {
    background: #FCFCFD; border-top: 1px solid #E8ECF2;
    color: #64748B; font-size: 12px;
}
#floatPanel {
    background: rgba(255, 255, 255, 0.94);
    border: 1px solid #E8ECF2; border-radius: 14px;
}
#floatPanelCollapsed {
    background: rgba(255, 255, 255, 0.94);
    border: 1px solid #E8ECF2; border-radius: 12px;
}
#floatPanelTitle { color: #0F172A; font-size: 13px; font-weight: bold; }
#collapsedRailTitle {
    color: #64748B;
    font-size: 11px;
    font-weight: bold;
    line-height: 1.1;
    padding-top: 4px;
}
#railBtn {
    background: #F8FAFC;
    color: #64748B;
    border: 1px solid #E2E8F0;
    border-radius: 8px;
    font-size: 16px;
    font-weight: bold;
}
#railBtn:hover { background: #EFF6FF; color: #2563EB; border-color: #93C5FD; }
#groupTitle { color: #64748B; font-size: 11px; font-weight: bold; padding-top: 2px; }
#fieldLabel { color: #64748B; font-size: 12px; min-width: 28px; }
#valueLabel { color: #334155; font-size: 12px; font-weight: 600; min-width: 36px; }
#unitLabel { color: #94A3B8; font-size: 11px; }
#sectionDivider { background: #E8ECF2; max-height: 1px; min-height: 1px; }
QPushButton#primaryBtn {
    background: #3B82F6; color: #FFFFFF; border: none; border-radius: 8px;
    padding: 0 14px; font-size: 13px; font-weight: bold; min-height: 34px;
}
QPushButton#primaryBtn:hover { background: #2563EB; }
QPushButton#roamBtn {
    background: #FFFFFF; color: #64748B; border: 1px solid #D8DEE8;
    border-radius: 8px; padding: 0 14px; font-size: 13px; min-height: 34px;
}
QPushButton#roamBtn:checked {
    background: #8B5CF6; color: #FFFFFF; border-color: #8B5CF6;
}
QPushButton#roamBtn:hover { border-color: #A78BFA; }
QPushButton#secondaryBtn {
    background: #FFFFFF; color: #475569; border: 1px solid #D8DEE8;
    border-radius: 8px; padding: 0 14px; font-size: 13px; min-height: 34px;
}
QPushButton#secondaryBtn:hover { background: #F8FAFC; border-color: #93C5FD; }
QPushButton#iconBtn {
    background: #F8FAFC; color: #64748B; border: 1px solid #E2E8F0;
    border-radius: 8px; font-size: 14px; font-weight: bold;
}
QPushButton#iconBtn:hover { background: #EFF6FF; color: #2563EB; }
QPushButton#toggleBtn {
    background: #FFFFFF; color: #64748B; border: 1px solid #D8DEE8;
    border-radius: 8px; padding: 8px 12px; font-size: 12px; min-height: 32px; text-align: left;
}
QPushButton#toggleBtn:checked {
    background: #EFF6FF; color: #1D4ED8; border-color: #93C5FD; font-weight: 600;
}
QPushButton#toggleBtn:hover { background: #F8FAFC; border-color: #60A5FA; }
QSpinBox#flatField {
    background: #F8FAFC; color: #334155; border: 1px solid #CBD5E1;
    border-radius: 8px; padding: 0 12px; min-height: 38px; max-height: 38px;
    font-size: 14px;
}
QSpinBox#flatField:focus { border-color: #93C5FD; background: #FFFFFF; }
QWidget#modalOverlay { background-color: rgba(15, 23, 42, 0.35); }
QSlider::groove:horizontal { height: 5px; background: #E8ECF2; border-radius: 3px; }
QSlider::sub-page:horizontal { background: #B8C0CC; border-radius: 3px; }
QSlider#primarySlider::sub-page:horizontal { background: #60A5FA; }
QSlider#accentSlider::sub-page:horizontal { background: #38BDF8; }
QSlider::handle:horizontal {
    width: 16px; height: 16px; margin: -6px 0; border-radius: 8px;
    background: #FFFFFF; border: 2px solid #B8C0CC;
}
QSlider#primarySlider::handle:horizontal { border-color: #3B82F6; }
QSlider#accentSlider::handle:horizontal { border-color: #38BDF8; }
)");
    }
}

inline QString themeAccent(ThemeId theme)
{
    switch (theme) {
    case ThemeId::Dark:
        return QStringLiteral("#60A5FA");
    case ThemeId::Ocean:
        return QStringLiteral("#0EA5E9");
    case ThemeId::Mint:
        return QStringLiteral("#22C55E");
    case ThemeId::Light:
    default:
        return QStringLiteral("#3B82F6");
    }
}

inline QString themeSwatchGradient(ThemeId theme)
{
    switch (theme) {
    case ThemeId::Dark:
        return QStringLiteral("qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #334155, stop:1 #0F172A)");
    case ThemeId::Ocean:
        return QStringLiteral("qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #7DD3FC, stop:1 #0284C7)");
    case ThemeId::Mint:
        return QStringLiteral("qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #86EFAC, stop:1 #16A34A)");
    case ThemeId::Light:
    default:
        return QStringLiteral("qlineargradient(x1:0,y1:0,x2:1,y2:1, stop:0 #93C5FD, stop:1 #3B82F6)");
    }
}

inline QString settingsDialogStylesheet(ThemeId theme)
{
    struct Palette {
        QString shellBg;
        QString border;
        QString title;
        QString subtitle;
        QString section;
        QString cardBg;
        QString cardBorder;
        QString cardHover;
        QString cardCheckedBg;
        QString cardCheckedBorder;
        QString cardCheckedText;
        QString cardText;
        QString divider;
        QString primary;
        QString primaryHover;
    };

    Palette p;
    switch (theme) {
    case ThemeId::Dark:
        p = {QStringLiteral("#1A2332"), QStringLiteral("#334155"), QStringLiteral("#F8FAFC"),
             QStringLiteral("#94A3B8"), QStringLiteral("#64748B"), QStringLiteral("#151B23"),
             QStringLiteral("#334155"), QStringLiteral("#243044"), QStringLiteral("#1E3A5F"),
             QStringLiteral("#93C5FD"), QStringLiteral("#BFDBFE"), QStringLiteral("#E2E8F0"),
             QStringLiteral("#334155"), QStringLiteral("#3B82F6"), QStringLiteral("#2563EB")};
        break;
    case ThemeId::Ocean:
        p = {QStringLiteral("#FFFFFF"), QStringLiteral("#E2E8F0"), QStringLiteral("#0C4A6E"),
             QStringLiteral("#64748B"), QStringLiteral("#64748B"), QStringLiteral("#FFFFFF"),
             QStringLiteral("#E2E8F0"), QStringLiteral("#F8FAFC"), QStringLiteral("#DBEAFE"),
             QStringLiteral("#93C5FD"), QStringLiteral("#1D4ED8"), QStringLiteral("#0C4A6E"),
             QStringLiteral("#E2E8F0"), QStringLiteral("#0284C7"), QStringLiteral("#0369A1")};
        break;
    case ThemeId::Mint:
        p = {QStringLiteral("#FFFFFF"), QStringLiteral("#E2E8F0"), QStringLiteral("#14532D"),
             QStringLiteral("#64748B"), QStringLiteral("#64748B"), QStringLiteral("#FFFFFF"),
             QStringLiteral("#E2E8F0"), QStringLiteral("#F8FAFC"), QStringLiteral("#DCFCE7"),
             QStringLiteral("#86EFAC"), QStringLiteral("#15803D"), QStringLiteral("#14532D"),
             QStringLiteral("#E2E8F0"), QStringLiteral("#16A34A"), QStringLiteral("#15803D")};
        break;
    case ThemeId::Light:
    default:
        p = {QStringLiteral("#FFFFFF"), QStringLiteral("#E2E8F0"), QStringLiteral("#0F172A"),
             QStringLiteral("#64748B"), QStringLiteral("#64748B"), QStringLiteral("#FFFFFF"),
             QStringLiteral("#E2E8F0"), QStringLiteral("#F8FAFC"), QStringLiteral("#DBEAFE"),
             QStringLiteral("#93C5FD"), QStringLiteral("#1D4ED8"), QStringLiteral("#334155"),
             QStringLiteral("#E2E8F0"), QStringLiteral("#3B82F6"), QStringLiteral("#2563EB")};
        break;
    }

    return QStringLiteral(
               "QDialog#settingsDialog { background: transparent; }"
               "QFrame#settingsShell {"
               "  background: %1; border: 1px solid %2; border-radius: 12px;"
               "}"
               "#settingsTitle { color: %3; font-size: 18px; font-weight: 700; }"
               "#settingsSubtitle { color: %4; font-size: 13px; }"
               "#settingsSection { color: %5; font-size: 12px; font-weight: 700; }"
               "#settingsDivider { background: %13; max-height: 1px; min-height: 1px; }"
               "QPushButton#themeCard {"
               "  background: %6; border: 1px solid %7; border-radius: 8px; padding: 0; min-height: 88px;"
               "}"
               "QPushButton#themeCard:hover { background: %8; }"
               "QPushButton#themeCard:checked {"
               "  background: %9; border: 1px solid %10; color: %11;"
               "}"
               "#themeCardName { color: %12; font-size: 13px; font-weight: 600; }"
               "QPushButton#themeCard:checked #themeCardName { color: %11; }"
               "QPushButton#settingsPrimaryBtn {"
               "  background: %14; color: #FFFFFF; border: none; border-radius: 8px;"
               "  min-height: 38px; max-height: 38px; min-width: 104px; font-size: 14px; font-weight: 700;"
               "}"
               "QPushButton#settingsPrimaryBtn:hover { background: %15; }")
        .arg(p.shellBg, p.border, p.title, p.subtitle, p.section, p.cardBg, p.cardBorder, p.cardHover,
             p.cardCheckedBg, p.cardCheckedBorder, p.cardCheckedText, p.cardText, p.divider, p.primary,
             p.primaryHover);
}

} // namespace AppTheme

#endif
