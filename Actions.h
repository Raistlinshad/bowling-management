#ifndef ACTIONS_H
#define ACTIONS_H

#include <QObject>
#include <QWidget>
#include <QDialog>
#include <QJsonObject>
#include <QDateTime>

class MainWindow;

class Actions : public QObject
{
    Q_OBJECT

public:
    explicit Actions(MainWindow *mainWindow,QObject *parent = nullptr);
    
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
    void endOfDay();
    void onDatabaseBrowserClicked();

private:
    void showQuickGameDialog();
    void showLeagueGameDialog();
    void showNewBowlerDialog();
    void showTeamManagementDialog();
    void showLeagueManagementDialog();
    void showBowlerManagementDialog();
    void sendGameToLane(int laneId, const QString &gameType, const QJsonObject &gameData);
    QString getSystemUptime() const;
    
    MainWindow *m_mainWindow;
};

#endif // ACTIONS_H