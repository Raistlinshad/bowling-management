#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QDateTime>
#include <QKeyEvent>
#include <QDialog>
#include <QMap>
#include "LaneServer.h"
#include "Actions.h"
#include "EnhancedLaneWidget.h"
#include "GameDisplayDialog.h"
#include "BowlerManagementDialog.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void updateTime();
    void onQuickAccessButtonClicked(const QString &buttonText);
    void onLaneStatusChanged(int laneId, LaneStatus status);
    void onGameDataReceived(int laneId, const QJsonObject &gameData);
    void onBowlerManagementClicked();
    
    // Enhanced lane widget slots
    void onLaneClicked(int laneNumber);
    void onLaneHoldToggled(int laneNumber, bool held);
    void onBowlerButtonClicked(int laneNumber, const QString &bowlerName);
    void onGameEditRequested(int laneNumber);
    void onGameResultsRequested(int laneNumber);
    void onLaneShutdownRequested(int laneNumber);
    
    // Game display dialog slots
    void onBallValueChanged(int laneNumber, const QString &bowlerName, int frame, int ball, int newValue);
    void onRevertLastBall(int laneNumber);
    void onGameEdited(int laneNumber, const QJsonObject &updatedData);

private:
    void setupUI();
    void setupTopBar();
    void setupQuickAccessButtons();
    void setupLaneWidgets();
    void setupMainContent();
    void showGameDisplayDialog(int laneNumber);
    void showQuickGameDialog(int laneNumber);
    void sendLaneCommand(int laneNumber, const QString &command, const QJsonObject &data = QJsonObject());
    EnhancedLaneStatus convertLaneStatus(LaneStatus oldStatus);
    
    // UI Components
    QWidget *m_centralWidget;
    QVBoxLayout *m_mainLayout;
    QFrame *m_topBar;
    QLabel *m_logoLabel;
    QLabel *m_dateTimeLabel;
    QLabel *m_statusBar;
    QFrame *m_quickAccessFrame;
    QScrollArea *m_laneScrollArea;
    QWidget *m_laneContainer;
    QHBoxLayout *m_laneLayout;
    QWidget *m_mainContentArea;
    QPushButton *m_bowlerManagementBtn;
    
    // Timer for updates
    QTimer *m_timeUpdateTimer;
    
    // Core components
    LaneServer *m_laneServer;
    Actions *m_actions;
    
    // Enhanced lane widgets
    QVector<EnhancedLaneWidget*> m_laneWidgets;
    int m_totalLanes;
    
    // Game display dialogs (one per lane)
    QMap<int, GameDisplayDialog*> m_gameDialogs;
    
    // Lane data tracking
    QMap<int, QJsonObject> m_laneGameData;
    QMap<int, EnhancedLaneStatus> m_laneStatuses;
    
    QString getCurrentTime() const;
};

#endif // MAINWINDOW_H