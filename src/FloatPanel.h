#ifndef FLOATPANEL_H
#define FLOATPANEL_H

#include <QFrame>
#include <QString>

class QLabel;
class QPushButton;
class QVBoxLayout;
class QWidget;

class FloatPanel : public QFrame
{
    Q_OBJECT

public:
    explicit FloatPanel(const QString &title, QWidget *parent = nullptr);

    QVBoxLayout *contentLayout() const;
    void setCollapsed(bool collapsed);

signals:
    void collapsedChanged(bool collapsed);

private:
    QLabel *m_titleLabel;
    QPushButton *m_collapseButton;
    QWidget *m_body;
    QVBoxLayout *m_bodyLayout;
    bool m_collapsed;
};

#endif
