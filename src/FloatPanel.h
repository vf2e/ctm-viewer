#ifndef FLOATPANEL_H
#define FLOATPANEL_H

#include <QFrame>
#include <QString>

class QHBoxLayout;
class QLabel;
class QPushButton;
class QVBoxLayout;
class QWidget;

class FloatPanel : public QFrame
{
    Q_OBJECT

public:
    enum class Side {
        Left,
        Right,
    };

    explicit FloatPanel(const QString &title,
                        const QString &shortTitle,
                        Side side,
                        QWidget *parent = nullptr);

    QVBoxLayout *contentLayout() const;
    void setCollapsed(bool collapsed);
    bool isCollapsed() const;

signals:
    void collapsedChanged(bool collapsed);

private:
    void updateCollapsedPresentation();

    Side m_side;
    QString m_shortTitle;
    QLabel *m_titleLabel;
    QLabel *m_collapsedTitle;
    QPushButton *m_collapseButton;
    QHBoxLayout *m_headerLayout;
    QWidget *m_body;
    QVBoxLayout *m_bodyLayout;
    bool m_collapsed;
};

#endif
