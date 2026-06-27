#ifndef APPSTYLE_H
#define APPSTYLE_H

#include <QString>

namespace AppStyle {

inline QString stylesheet()
{
    return QStringLiteral(R"(
QMainWindow {
    background: #FAFBFE;
}
QMenuBar {
    background: #FCFCFD;
    border-bottom: 1px solid #E8ECF2;
    padding: 2px 8px;
    font-size: 12px;
    color: #475569;
}
QMenuBar::item {
    padding: 5px 10px;
    border-radius: 6px;
}
QMenuBar::item:selected {
    background: #F1F5F9;
    color: #2563EB;
}
QMenu {
    background: #FFFFFF;
    border: 1px solid #E8ECF2;
    border-radius: 8px;
    padding: 6px;
}
QStatusBar {
    background: #FCFCFD;
    border-top: 1px solid #E8ECF2;
    color: #64748B;
    font-size: 12px;
}
#floatPanel {
    background: rgba(255, 255, 255, 0.94);
    border: 1px solid #E8ECF2;
    border-radius: 14px;
}
#floatPanelTitle {
    color: #0F172A;
    font-size: 13px;
    font-weight: bold;
}
#groupTitle {
    color: #64748B;
    font-size: 11px;
    font-weight: bold;
    padding-top: 2px;
}
#fieldLabel {
    color: #64748B;
    font-size: 12px;
    min-width: 28px;
}
#valueLabel {
    color: #334155;
    font-size: 12px;
    font-weight: 600;
    min-width: 36px;
}
#unitLabel {
    color: #94A3B8;
    font-size: 11px;
}
#colorBarTitle {
    color: #475569;
    font-size: 11px;
    font-weight: bold;
}
QPushButton#primaryBtn {
    background: #3B82F6;
    color: #FFFFFF;
    border: none;
    border-radius: 8px;
    padding: 0 14px;
    font-size: 13px;
    font-weight: bold;
    min-height: 34px;
}
QPushButton#primaryBtn:hover { background: #2563EB; }
QPushButton#secondaryBtn {
    background: #FFFFFF;
    color: #475569;
    border: 1px solid #D8DEE8;
    border-radius: 8px;
    padding: 0 14px;
    font-size: 13px;
    min-height: 34px;
}
QPushButton#secondaryBtn:hover {
    background: #F8FAFC;
    border-color: #93C5FD;
}
QPushButton#iconBtn {
    background: #F8FAFC;
    color: #64748B;
    border: 1px solid #E2E8F0;
    border-radius: 8px;
    font-size: 14px;
    font-weight: bold;
}
QPushButton#iconBtn:hover {
    background: #EFF6FF;
    color: #2563EB;
}
QCheckBox {
    color: #334155;
    font-size: 12px;
    spacing: 6px;
}
QCheckBox::indicator {
    width: 16px;
    height: 16px;
    border-radius: 4px;
    border: 1px solid #CBD5E1;
    background: #FFFFFF;
}
QCheckBox::indicator:checked {
    background: #3B82F6;
    border-color: #3B82F6;
}
QSlider::groove:horizontal {
    height: 5px;
    background: #E8ECF2;
    border-radius: 3px;
}
QSlider::sub-page:horizontal {
    background: #B8C0CC;
    border-radius: 3px;
}
QSlider#primarySlider::sub-page:horizontal {
    background: #60A5FA;
}
QSlider#accentSlider::sub-page:horizontal {
    background: #38BDF8;
}
QSlider::handle:horizontal {
    width: 16px;
    height: 16px;
    margin: -6px 0;
    border-radius: 8px;
    background: #FFFFFF;
    border: 2px solid #B8C0CC;
}
QSlider#primarySlider::handle:horizontal {
    border-color: #3B82F6;
}
QSlider#accentSlider::handle:horizontal {
    border-color: #38BDF8;
}
)");
}

} // namespace AppStyle

#endif
