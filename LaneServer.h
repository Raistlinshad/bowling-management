#ifndef LANESERVER_H
#define LANESERVER_H

#include <QObject>
#include <QJsonObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QDateTime>
#include <QMap>
#include "LeagueManager.h"


enum class LaneStatus {
    Idle,
    Active,
    Maintenance,
    Error,
    Gaming,
    LeagueGame,
    Ready,
    Unknown,
    Offline
};

struct LaneConnection {
    QTcpSocket *socket = nullptr;
    int laneId = -1;
    QDateTime lastSeen;
    LaneStatus status = LaneStatus::Idle;
    QJsonObject gameData;
};

class LaneServer : public QObject
{
    Q_OBJECT

public:
    explicit LaneServer(QObject *parent = nullptr);
    ~LaneServer();
    void start(quint16 port = 50005);
    void stop();
    void handleTeamMove(int fromLane, int toLane, const QString &teamData);
    void onLaneCommand(const QJsonObject &data);
    LeagueManager* getLeagueManager() const { return m_leagueManager; }

signals:
    void laneStatusChanged(int laneId, LaneStatus status);
    void gameDataReceived(int laneId, const QJsonObject &gameData);
    void laneShutdown(int laneId);
    void gameStarted(int laneId, const QString &gameType, const QJsonObject &gameData);
    void ballUpdated(int laneId, const QString &bowlerName, int frame, int ball, int newValue);
    void lastBallReverted(int laneId);
    void ballThrown(int laneId, const QJsonObject &data);
    void frameCompleted(int laneId, const QJsonObject &data);
    void gameCompleted(int laneId, const QString &gameType, const QJsonObject &data);
    void displayModeChanged(int laneId, const QString &frameMode, const QString &totalDisplay);

private slots:
    void onNewConnection();
    void onClientDisconnected(); 
    void onClientDataReady();
    void checkConnections();
    void onLeagueCreated(int leagueId, const QString &leagueName);
    void onLeagueEventCompleted(int eventId, int leagueId);

private:
    void processMessage(QTcpSocket *socket, const QJsonObject &message);
    void handleRegistration(QTcpSocket *socket, const QJsonObject &message);
    void handleHeartbeat(QTcpSocket *socket, const QJsonObject &message);
    void handleGameData(QTcpSocket *socket, const QJsonObject &message);
    void updateLaneStatus(int laneId, LaneStatus status);
    void sendToLane(int laneId, const QString &command, const QJsonObject &data);

    QTcpServer *m_server;
    QTimer *m_connectionTimer;
    QMap<QTcpSocket*, LaneConnection> m_connections;
    QMap<int, QTcpSocket*> m_laneToSocket;
    bool m_running;
    
    static const int HEARTBEAT_TIMEOUT = 30000; // 30 seconds

    LeagueManager *m_leagueManager;
    
    // League-specific message handlers
    void handleLeagueGameMessage(int laneId, const QJsonObject &data);
    void handleQuickGameMessage(int laneId, const QJsonObject &data);
    void handleDisplayModeChange(int laneId, const QJsonObject &data);
    
    // Game state tracking
    QMap<int, QString> m_laneGameTypes; // laneId -> "quick_game" or "league_game"
    QMap<int, QJsonObject> m_laneGameData; // laneId -> game configuration

    void handleGameComplete(int laneId, const QJsonObject &data);
    void handleQuickGameComplete(int laneId, const QJsonObject &data);
    void handleBallThrown(int laneId, const QJsonObject &data);
    void handleFrameComplete(int laneId, const QJsonObject &data);
    void handleStatusUpdate(int laneId, const QJsonObject &data);
    void handleShutdownAcknowledged(int laneId, const QJsonObject &data);
    void handleHoldAcknowledged(int laneId, const QJsonObject &data);
    void handleBallUpdateAcknowledged(int laneId, const QJsonObject &data);
    void handleRevertAcknowledged(int laneId, const QJsonObject &data);
    
    void sendMessageToLane(int laneId, const QJsonObject &message);
    void processClientMessage(QTcpSocket *socket, const QJsonObject &message);
    void setLaneStatus(int laneId, LaneStatus status);
    void broadcastToManagementClients(const QJsonObject &message);
    QTcpSocket* getSocketForLane(int laneId);
    int getLaneIdFromSocket(QTcpSocket *socket);
    
    void sendHoldCommand(int laneId, bool hold);
    void sendBallUpdateCommand(int laneId, const QJsonObject &updateData);
    void sendRevertCommand(int laneId);
    void sendShutdownCommand(int laneId);
    
    // Add missing member variables:
    QMap<QTcpSocket*, int> m_laneClients;
    QMap<int, LaneStatus> m_laneStatuses;

};

#endif // LANESERVER_H