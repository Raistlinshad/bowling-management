#ifndef ACTIONS_H
#define ACTIONS_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QJsonObject>
#include "EventBus.h"

class MainWindow;

class Actions : public QObject
{
    Q_OBJECT

public:
    explicit Actions(MainWindow *mainWindow, EventBus *eventBus, QObject *parent = nullptr);
    
public slots:
    void quickStart();
    void leagueStart();
    void newBowler();
    void homeScreen();
    void bowlerInfo();
    void calendar();
    void teams();
    void leagues();
    void party();
    void showStatus();
    void settings();
    void testLaneConnection();
    void runQuickGameDiagnostic();
    void onDatabaseBrowserClicked();

private:
    void showQuickGameDialog();
    void showLeagueGameDialog();
    void showNewBowlerDialog();
    void showTeamManagementDialog();
    void showLeagueManagementDialog();
    void showBowlerManagementDialog();
    void sendGameToLane(int laneId, const QString &gameType, const QJsonObject &gameData);
    
    MainWindow *m_mainWindow;
    EventBus *m_eventBus;
};

#endif // ACTIONS_H
