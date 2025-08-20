#ifndef LANEWIDGET_H
#define LANEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QJsonObject>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDrag>
#include <QMouseEvent>

// Forward declaration - used enum from LaneServer.h
enum class LaneStatus;

class DraggableTeamButton : public QPushButton
{
    Q_OBJECT

public:
    explicit DraggableTeamButton(const QString &teamName, QWidget *parent = nullptr);

signals:
    void dragStarted(const QString &teamData);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    QPoint m_dragStartPosition;
    QString m_teamData;
};

class LaneWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LaneWidget(int laneNumber, QWidget *parent = nullptr);
    
    void setStatus(LaneStatus status);
    void updateGameData(const QJsonObject &gameData);
    
    int getLaneNumber() const { return m_laneNumber; }

signals:
    void teamMoveRequested(int fromLane, int toLane, const QString &teamData);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onTeamButtonDragStarted(const QString &teamData);

private:
    void setupUI();
    void updateBackgroundColor();
    
    int m_laneNumber;
    LaneStatus m_status;
    QVBoxLayout *m_layout;
    QLabel *m_laneLabel;
    QLabel *m_statusLabel;
    DraggableTeamButton *m_teamButton;
    QJsonObject m_currentGameData;
};

#endif // LANEWIDGET_H