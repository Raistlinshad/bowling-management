#include "LaneServer.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QHostAddress>
#include <QDebug>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

LaneServer::LaneServer(QObject *parent)
    : QObject(parent)
    , m_leagueManager(new LeagueManager(this, this))
    , m_server(new QTcpServer(this))
    , m_connectionTimer(new QTimer(this))
{
    connect(m_server, &QTcpServer::newConnection, this, &LaneServer::onNewConnection);
    connect(m_connectionTimer, &QTimer::timeout, this, &LaneServer::checkConnections);
    connect(m_leagueManager, &LeagueManager::sendToLane,
            this, [this](int laneId, const QString& command, const QJsonObject& data) {
                QJsonObject message;
                message["type"] = command;
                message["data"] = data;
                sendMessageToLane(laneId, message);
            });
    connect(m_leagueManager, &LeagueManager::leagueCreated,
            this, &LaneServer::onLeagueCreated);
    connect(m_leagueManager, &LeagueManager::eventCompleted,
            this, &LaneServer::onLeagueEventCompleted);
    
    qDebug() << "LaneServer initialized with LeagueManager support";
        
    m_connectionTimer->start(10000); // Check every 10 seconds
}

LaneServer::~LaneServer()
{
    stop();
}

void LaneServer::start(quint16 port)
{
    if (m_server->listen(QHostAddress::Any, port)) {
        qDebug() << "Lane server started on port" << port;
    } else {
        qDebug() << "Failed to start server:" << m_server->errorString();
    }
}

void LaneServer::onNewConnection()
{
    QTcpSocket *socket = m_server->nextPendingConnection();
    if (!socket) return;
    
    qDebug() << "New connection from" << socket->peerAddress().toString();
    
    // Connect socket signals
    connect(socket, &QTcpSocket::disconnected, this, &LaneServer::onClientDisconnected);
    connect(socket, &QTcpSocket::readyRead, this, &LaneServer::onClientDataReady);
    
    // Initialize connection data
    LaneConnection connection;
    connection.socket = socket;
    connection.laneId = -1; // Will be set during registration
    connection.lastSeen = QDateTime::currentDateTime();
    connection.status = LaneStatus::Idle;
    
    m_connections[socket] = connection;
}

void LaneServer::onClientDisconnected()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    if (m_connections.contains(socket)) {
        LaneConnection connection = m_connections[socket];
        if (connection.laneId > 0) {
            updateLaneStatus(connection.laneId, LaneStatus::Idle);
            m_laneToSocket.remove(connection.laneId);
            qDebug() << "Lane" << connection.laneId << "disconnected";
        }
        m_connections.remove(socket);
    }
    
    socket->deleteLater();
}

void LaneServer::stop()
{
    if (m_server->isListening()) {
        m_server->close();
        qDebug() << "Server stopped";
    }
    
    // Clean up all connections
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        QTcpSocket *socket = it.key();
        if (socket) {
            socket->disconnectFromHost();
            socket->deleteLater();
        }
    }
    m_connections.clear();
    m_laneToSocket.clear();
}

void LaneServer::onClientDataReady()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    while (socket->canReadLine()) {
        QByteArray data = socket->readLine();
        
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(data, &error);
        
        if (error.error != QJsonParseError::NoError) {
            qWarning() << "JSON parse error:" << error.errorString();
            continue;
        }
        
        if (doc.isObject()) {
            processMessage(socket, doc.object());
        }
    }
}

void LaneServer::handleRegistration(QTcpSocket *socket, const QJsonObject &message)
{
    int laneId = message["lane_id"].toInt();
    
    if (laneId <= 0) {
        qWarning() << "Invalid lane ID in registration";
        return;
    }
    
    // Update connection info
    if (m_connections.contains(socket)) {
        m_connections[socket].laneId = laneId;
        m_connections[socket].lastSeen = QDateTime::currentDateTime();
        m_connections[socket].status = LaneStatus::Active;
        
        m_laneToSocket[laneId] = socket;
        
        // Send registration response
        QJsonObject response;
        response["type"] = "registration_response";
        response["status"] = "success";
        response["lane_id"] = laneId;
        response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        QJsonDocument doc(response);
        socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
        
        updateLaneStatus(laneId, LaneStatus::Active);
        qDebug() << "Lane" << laneId << "registered successfully";
    }
}

void LaneServer::handleHeartbeat(QTcpSocket *socket, const QJsonObject &message)
{
    if (m_connections.contains(socket)) {
        m_connections[socket].lastSeen = QDateTime::currentDateTime();
        
        // Send heartbeat response
        QJsonObject response;
        response["type"] = "heartbeat_response";
        response["status"] = "ok";
        response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        QJsonDocument doc(response);
        socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
    }
}

void LaneServer::handleGameData(QTcpSocket *socket, const QJsonObject &message)
{
    if (m_connections.contains(socket)) {
        LaneConnection &connection = m_connections[socket];
        connection.gameData = message["data"].toObject();
        connection.lastSeen = QDateTime::currentDateTime();
        
        // Update status to game running if we have game data
        if (connection.status != LaneStatus::Active) {
            connection.status = LaneStatus::Active;
            updateLaneStatus(connection.laneId, LaneStatus::Active);
        }
        
        emit gameDataReceived(connection.laneId, connection.gameData);
    }
}

void LaneServer::checkConnections()
{
    QDateTime now = QDateTime::currentDateTime();
    
    for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
        LaneConnection &connection = it.value();
        
        if (connection.laneId > 0) {
            qint64 secondsSinceLastSeen = connection.lastSeen.secsTo(now);
            
            if (secondsSinceLastSeen > HEARTBEAT_TIMEOUT / 1000) {
                // Connection is stale
                if (connection.status != LaneStatus::Idle) {
                    updateLaneStatus(connection.laneId, LaneStatus::Idle);
                    connection.status = LaneStatus::Idle;
                    qDebug() << "Lane" << connection.laneId << "connection timeout";
                }
            }
        }
    }
}

void LaneServer::updateLaneStatus(int laneId, LaneStatus status)
{
    emit laneStatusChanged(laneId, status);
}

void LaneServer::sendToLane(int laneId, const QString &command, const QJsonObject &data)
{
    if (!m_laneToSocket.contains(laneId)) {
        qWarning() << "Lane" << laneId << "not connected";
        return;
    }
    
    QTcpSocket *socket = m_laneToSocket[laneId];
    if (!socket || socket->state() != QTcpSocket::ConnectedState) {
        qWarning() << "Lane" << laneId << "socket not valid";
        return;
    }
    
    QJsonObject message;
    message["type"] = command;
    message["data"] = data;
    message["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QJsonDocument doc(message);
    socket->write(doc.toJson(QJsonDocument::Compact) + "\n");
    
    qDebug() << "Sent" << command << "to lane" << laneId;
}

void LaneServer::handleTeamMove(int fromLane, int toLane, const QString &teamData)
{
    qDebug() << "Team move request: from lane" << fromLane << "to lane" << toLane;
    
    // Send team data to target lane
    QJsonObject moveData;
    moveData["type"] = "team_move";
    moveData["from_lane"] = fromLane;
    moveData["to_lane"] = toLane;
    moveData["team_data"] = teamData;
    
    sendToLane(toLane, "team_move", moveData);
    
    // Optionally send confirmation back to source lane
    QJsonObject confirmData;
    confirmData["type"] = "team_move_confirm";
    confirmData["target_lane"] = toLane;
    confirmData["status"] = "sent";
    
    sendToLane(fromLane, "team_move_confirm", confirmData);
}


void LaneServer::handleDisplayModeChange(int laneId, const QJsonObject &data)
{
    qDebug() << "Display mode change for lane" << laneId << ":" << data;
    
    QString frameMode = data["frame_mode"].toString();
    QString totalDisplay = data["total_display"].toString();
    
    // Forward to league manager if this is a league game
    if (m_laneGameTypes.value(laneId) == "league_game" && m_leagueManager) {
        m_leagueManager->handleDisplayModeChange(laneId, data);
    }
    
    // Create response
    QJsonObject response;
    response["type"] = "display_mode_updated";
    response["lane_id"] = laneId;
    response["frame_mode"] = frameMode;
    response["total_display"] = totalDisplay;
    response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Send acknowledgment to lane
    sendMessageToLane(laneId, response);
    
    emit displayModeChanged(laneId, frameMode, totalDisplay);
}

void LaneServer::handleGameComplete(int laneId, const QJsonObject &data)
{
    qDebug() << "Game completed on lane" << laneId;
    
    QString gameType = m_laneGameTypes.value(laneId, "unknown");
    
    if (gameType == "league_game" && m_leagueManager) {
        // Process league game completion
        m_leagueManager->handleLeagueGameComplete(laneId, data);
    } else if (gameType == "quick_game") {
        // Process quick game completion
        handleQuickGameComplete(laneId, data);
    }
    
    // Clean up game state
    m_laneGameTypes.remove(laneId);
    m_laneGameData.remove(laneId);
    
    // Update lane status
    setLaneStatus(laneId, LaneStatus::Ready);
    
    emit gameCompleted(laneId, gameType, data);
}

void LaneServer::handleQuickGameComplete(int laneId, const QJsonObject &data)
{
    qDebug() << "Quick game completed on lane" << laneId;
    
    // Process quick game results
    QJsonArray bowlers = data["bowlers"].toArray();
    for (const QJsonValue &bowlerValue : bowlers) {
        QJsonObject bowlerData = bowlerValue.toObject();
        
        // Update bowler statistics in database if needed
        QString bowlerName = bowlerData["name"].toString();
        int totalScore = bowlerData["total_score"].toInt();
        int highGame = bowlerData["high_game"].toInt();
        
        qDebug() << "Bowler" << bowlerName << "scored" << totalScore << "high game:" << highGame;
    }
    
    // Send completion acknowledgment
    QJsonObject response;
    response["type"] = "game_complete_ack";
    response["lane_id"] = laneId;
    response["game_type"] = "quick_game";
    response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    sendMessageToLane(laneId, response);
}

void LaneServer::handleBallThrown(int laneId, const QJsonObject &data)
{
    // Extract ball data
    QJsonArray pins = data["pins"].toArray();
    int ballValue = data["value"].toInt();
    QString bowlerName = data["bowler"].toString();
    int frame = data["frame"].toInt();
    int ball = data["ball"].toInt();
    
    qDebug() << "Ball thrown on lane" << laneId << "by" << bowlerName 
             << "frame" << frame << "ball" << ball << "value" << ballValue;
    
    // Process ball data based on game type
    QString gameType = m_laneGameTypes.value(laneId, "unknown");
    
    if (gameType == "league_game") {
        // Additional league-specific ball processing
        QJsonObject ballData = data;
        ballData["game_type"] = "league_game";
        
        // Could send to league manager for real-time statistics
        emit ballThrown(laneId, ballData);
    } else {
        // Standard quick game ball processing
        emit ballThrown(laneId, data);
    }
    
    // Send acknowledgment
    QJsonObject response;
    response["type"] = "ball_ack";
    response["lane_id"] = laneId;
    response["frame"] = frame;
    response["ball"] = ball;
    response["value"] = ballValue;
    
    sendMessageToLane(laneId, response);
}

void LaneServer::handleFrameComplete(int laneId, const QJsonObject &data)
{
    QString bowlerName = data["bowler"].toString();
    int frame = data["frame"].toInt();
    int frameScore = data["frame_score"].toInt();
    int runningTotal = data["running_total"].toInt();
    bool isStrike = data["is_strike"].toBool();
    bool isSpare = data["is_spare"].toBool();
    
    qDebug() << "Frame" << frame << "completed on lane" << laneId 
             << "by" << bowlerName << "score:" << frameScore << "total:" << runningTotal;
    
    // Process frame completion based on game type
    QString gameType = m_laneGameTypes.value(laneId, "unknown");
    
    if (gameType == "league_game") {
        // League-specific frame processing
        QJsonObject frameData = data;
        frameData["game_type"] = "league_game";
        
        // Could trigger handicap calculations, team score updates, etc.
        emit frameCompleted(laneId, frameData);
    } else {
        // Standard frame processing
        emit frameCompleted(laneId, data);
    }
    
    // Special effects for strikes/spares
    if (isStrike) {
        QJsonObject effectData;
        effectData["type"] = "strike_effect";
        effectData["bowler"] = bowlerName;
        effectData["frame"] = frame;
        sendMessageToLane(laneId, effectData);
    } else if (isSpare) {
        QJsonObject effectData;
        effectData["type"] = "spare_effect";
        effectData["bowler"] = bowlerName;
        effectData["frame"] = frame;
        sendMessageToLane(laneId, effectData);
    }
}

void LaneServer::handleStatusUpdate(int laneId, const QJsonObject &data)
{
    QString status = data["status"].toString();
    QString message = data["message"].toString();
    
    qDebug() << "Status update from lane" << laneId << ":" << status << message;
    
    // Map status strings to LaneStatus enum
    LaneStatus laneStatus = LaneStatus::Unknown;
    if (status == "ready") {
        laneStatus = LaneStatus::Ready;
    } else if (status == "gaming" || status == "game_running") {
        laneStatus = LaneStatus::Gaming;
    } else if (status == "league_game_running") {
        laneStatus = LaneStatus::LeagueGame;
    } else if (status == "maintenance") {
        laneStatus = LaneStatus::Maintenance;
    } else if (status == "error") {
        laneStatus = LaneStatus::Error;
    } else if (status == "offline") {
        laneStatus = LaneStatus::Offline;
    }
    
    setLaneStatus(laneId, laneStatus);
    emit laneStatusChanged(laneId, laneStatus);
}

// League manager event handlers
void LaneServer::onLeagueCreated(int leagueId, const QString &leagueName)
{
    qDebug() << "League created:" << leagueName << "ID:" << leagueId;
    
    // Broadcast to all connected management clients
    QJsonObject notification;
    notification["type"] = "league_created";
    notification["league_id"] = leagueId;
    notification["league_name"] = leagueName;
    notification["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    broadcastToManagementClients(notification);
}

void LaneServer::onLeagueEventCompleted(int eventId, int leagueId)
{
    qDebug() << "League event completed - Event:" << eventId << "League:" << leagueId;
    
    // Get updated standings and broadcast
    if (m_leagueManager) {
        QJsonObject standings = m_leagueManager->getLeagueStandings(leagueId);
        
        QJsonObject notification;
        notification["type"] = "league_standings_updated";
        notification["league_id"] = leagueId;
        notification["event_id"] = eventId;
        notification["standings"] = standings;
        notification["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        broadcastToManagementClients(notification);
    }
}

// Utility methods
void LaneServer::sendMessageToLane(int laneId, const QJsonObject &message)
{
    QTcpSocket *socket = getSocketForLane(laneId);
    if (!socket) {
        qWarning() << "No socket found for lane" << laneId;
        return;
    }
    
    QJsonDocument doc(message);
    QByteArray data = doc.toJson(QJsonDocument::Compact) + "\n";
    
    socket->write(data);
    socket->flush();
    
    qDebug() << "Sent message to lane" << laneId << ":" << message["type"].toString();
}

void LaneServer::broadcastToManagementClients(const QJsonObject &message)
{
    // Broadcast to all connected management/display clients
    // Implementation depends on your architecture
    qDebug() << "Broadcasting to management clients:" << message["type"].toString();
    
    // This could send to a separate management client connection,
    // or to a web interface, or update a display board, etc.
}

QTcpSocket* LaneServer::getSocketForLane(int laneId)
{
    // Find the socket associated with the given lane ID
    for (auto it = m_laneClients.begin(); it != m_laneClients.end(); ++it) {
        if (it.value() == laneId) {
            return it.key();
        }
    }
    return nullptr;
}

int LaneServer::getLaneIdFromSocket(QTcpSocket *socket)
{
    return m_laneClients.value(socket, 0);
}

void LaneServer::setLaneStatus(int laneId, LaneStatus status)
{
    m_laneStatuses[laneId] = status;
    
    qDebug() << "Lane" << laneId << "status changed to:" << static_cast<int>(status);
}

void LaneServer::processMessage(QTcpSocket *socket, const QJsonObject &message)
{
    QString type = message["type"].toString();
    QJsonObject data = message["data"].toObject();
    int laneId = getLaneIdFromSocket(socket);
    
    qDebug() << "Processing message from lane" << laneId << "type:" << type;
    
    if (type == "registration") {
        handleRegistration(socket, message);
    } else if (type == "heartbeat") {
        handleHeartbeat(socket, message);
    } else if (type == "game_data") {
        handleGameData(socket, message);
    } else if (type == "quick_game_update") {
        handleQuickGameMessage(laneId, data);
    } else if (type == "league_game_update") {
        handleLeagueGameMessage(laneId, data);
    } else if (type == "game_complete") {
        handleGameComplete(laneId, data);
    } else if (type == "display_mode_change") {
        handleDisplayModeChange(laneId, data);
    } else if (type == "ball_thrown") {
        handleBallThrown(laneId, data);
    } else if (type == "frame_complete") {
        handleFrameComplete(laneId, data);
    } else if (type == "status_update") {
        handleStatusUpdate(laneId, data);
    } else if (type == "hold_acknowledged") {
        handleHoldAcknowledged(laneId, data);
    } else if (type == "ball_update_acknowledged") {
        handleBallUpdateAcknowledged(laneId, data);
    } else if (type == "revert_acknowledged") {
        handleRevertAcknowledged(laneId, data);
    } else if (type == "shutdown_acknowledged") {
        handleShutdownAcknowledged(laneId, data);
    } else {
        qWarning() << "Unknown message type from lane" << laneId << ":" << type;
    }
}

void LaneServer::handleHoldAcknowledged(int laneId, const QJsonObject &data)
{
    bool isHeld = data["held"].toBool();
    qDebug() << "Lane" << laneId << "hold state changed to:" << isHeld;
    
    // Update game data to reflect hold state
    if (m_laneGameData.contains(laneId)) {
        QJsonObject gameData = m_laneGameData[laneId];
        gameData["held"] = isHeld;
        m_laneGameData[laneId] = gameData;
        
        // Emit updated game data
        emit gameDataReceived(laneId, gameData);
    }
    
    // Update status
    if (isHeld) {
        setLaneStatus(laneId, LaneStatus::Error); // Repurpose Error as Hold for now
    } else {
        setLaneStatus(laneId, LaneStatus::Active);
    }
}

void LaneServer::handleBallUpdateAcknowledged(int laneId, const QJsonObject &data)
{
    QString bowlerName = data["bowler_name"].toString();
    int frame = data["frame"].toInt();
    int ball = data["ball"].toInt();
    int newValue = data["new_value"].toInt();
    
    qDebug() << "Lane" << laneId << "acknowledged ball update for" << bowlerName 
             << "frame" << frame << "ball" << ball << "new value:" << newValue;
    
    // Update stored game data with new ball value
    if (m_laneGameData.contains(laneId)) {
        QJsonObject gameData = m_laneGameData[laneId];
        QJsonArray bowlers = gameData["bowlers"].toArray();
        
        for (int i = 0; i < bowlers.size(); ++i) {
            QJsonObject bowlerData = bowlers[i].toObject();
            if (bowlerData["name"].toString() == bowlerName) {
                QJsonArray frames = bowlerData["frames"].toArray();
                
                if (frame - 1 < frames.size()) {
                    QJsonArray frameData = frames[frame - 1].toArray();
                    
                    // Ensure frame array is large enough
                    while (frameData.size() < ball) {
                        frameData.append(-1);
                    }
                    
                    // Update the specific ball
                    frameData[ball - 1] = newValue;
                    frames[frame - 1] = frameData;
                    bowlerData["frames"] = frames;
                    
                    // Recalculate totals (simplified - real implementation would be more complex)
                    int newTotal = bowlerData["total_score"].toInt();
                    bowlerData["total_score"] = newTotal;
                    
                    bowlers[i] = bowlerData;
                    break;
                }
            }
        }
        
        gameData["bowlers"] = bowlers;
        m_laneGameData[laneId] = gameData;
        
        // Emit updated game data
        emit gameDataReceived(laneId, gameData);
    }
}

void LaneServer::handleRevertAcknowledged(int laneId, const QJsonObject &data)
{
    qDebug() << "Lane" << laneId << "acknowledged revert last ball";
    
    // Lane will send updated game data after reverting
    // Just log the acknowledgment for now
    QString revertedBowler = data["bowler_name"].toString();
    int revertedFrame = data["frame"].toInt();
    int revertedBall = data["ball"].toInt();
    
    qDebug() << "Reverted ball for" << revertedBowler << "frame" << revertedFrame << "ball" << revertedBall;
}

void LaneServer::handleShutdownAcknowledged(int laneId, const QJsonObject &data)
{
    qDebug() << "Lane" << laneId << "acknowledged shutdown command";
    
    // Clear game state
    m_laneGameTypes.remove(laneId);
    m_laneGameData.remove(laneId);
    
    // Set status back to ready/connected
    setLaneStatus(laneId, LaneStatus::Idle);
    
    // Notify that lane is now available
    emit laneStatusChanged(laneId, LaneStatus::Idle);
}

// Enhanced command handlers for new functionality
void LaneServer::processClientMessage(QTcpSocket *socket, const QJsonObject &message)
{
    QString type = message["type"].toString();
    QJsonObject data = message["data"].toObject();
    int laneId = getLaneIdFromSocket(socket);
    
    qDebug() << "Processing message from lane" << laneId << "type:" << type;
    
    if (type == "quick_game") {
        handleQuickGameMessage(laneId, data);
    } else if (type == "league_game") {
        handleLeagueGameMessage(laneId, data);
    } else if (type == "game_complete") {
        handleGameComplete(laneId, data);
    } else if (type == "display_mode_change") {
        handleDisplayModeChange(laneId, data);
    } else if (type == "ball_thrown") {
        handleBallThrown(laneId, data);
    } else if (type == "frame_complete") {
        handleFrameComplete(laneId, data);
    } else if (type == "status_update") {
        handleStatusUpdate(laneId, data);
    } else if (type == "hold_acknowledged") {
        handleHoldAcknowledged(laneId, data);
    } else if (type == "ball_update_acknowledged") {
        handleBallUpdateAcknowledged(laneId, data);
    } else if (type == "revert_acknowledged") {
        handleRevertAcknowledged(laneId, data);
    } else if (type == "shutdown_acknowledged") {
        handleShutdownAcknowledged(laneId, data);
    } else {
        qWarning() << "Unknown message type from lane" << laneId << ":" << type;
    }
}

void LaneServer::onLaneCommand(const QJsonObject &data)
{
    int laneId = data["lane_id"].toInt();
    QString command = data["command"].toString();
    QString type = data["type"].toString();
    QJsonObject commandData = data["data"].toObject();
    
    qDebug() << "Processing lane command:" << command << "type:" << type << "for lane" << laneId;
    
    if (!command.isEmpty()) {
        // New command-based system
        if (command == "hold_toggle") {
            sendHoldCommand(laneId, commandData["hold"].toBool());
        } else if (command == "update_ball") {
            sendBallUpdateCommand(laneId, commandData);
        } else if (command == "revert_last_ball") {
            sendRevertCommand(laneId);
        } else if (command == "shutdown") {
            sendShutdownCommand(laneId);
        } else {
            qWarning() << "Unknown command:" << command;
        }
    } else if (!type.isEmpty()) {
        // Legacy type-based system
        sendToLane(laneId, type, commandData);
    }
}

void LaneServer::sendHoldCommand(int laneId, bool hold)
{
    QJsonObject holdData;
    holdData["hold"] = hold;
    holdData["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    sendToLane(laneId, "hold_toggle", holdData);
    
    qDebug() << "Sent hold command to lane" << laneId << "hold:" << hold;
}

void LaneServer::sendBallUpdateCommand(int laneId, const QJsonObject &updateData)
{
    QJsonObject ballUpdate;
    ballUpdate["bowler_name"] = updateData["bowler_name"];
    ballUpdate["frame"] = updateData["frame"];
    ballUpdate["ball"] = updateData["ball"];
    ballUpdate["new_value"] = updateData["new_value"];
    ballUpdate["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    sendToLane(laneId, "update_ball", ballUpdate);
    
    qDebug() << "Sent ball update to lane" << laneId << ":" << updateData;
}

void LaneServer::sendRevertCommand(int laneId)
{
    QJsonObject revertData;
    revertData["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    sendToLane(laneId, "revert_last_ball", revertData);
    
    qDebug() << "Sent revert last ball command to lane" << laneId;
}

void LaneServer::sendShutdownCommand(int laneId)
{
    QJsonObject shutdownData;
    shutdownData["reason"] = "operator_request";
    shutdownData["return_to"] = "advertising";
    shutdownData["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    sendToLane(laneId, "shutdown_lane", shutdownData);
    
    qDebug() << "Sent shutdown command to lane" << laneId;
}

// Enhanced game data tracking with hold states
void LaneServer::handleQuickGameMessage(int laneId, const QJsonObject &data)
{
    qDebug() << "Starting Quick Game on lane" << laneId;
    
    // Store game type and enhanced data
    m_laneGameTypes[laneId] = "quick_game";
    
    // Create enhanced game data with 5-pin bowling structure
    QJsonObject enhancedData = data;
    enhancedData["held"] = false;
    enhancedData["completed"] = false;
    enhancedData["current_bowler"] = 0;
    enhancedData["current_frame"] = 1;
    enhancedData["current_ball"] = 1;
    
    // Initialize bowler frame data for 5-pin bowling
    QJsonArray bowlers = data["bowlers"].toArray();
    QJsonArray enhancedBowlers;
    
    for (int i = 0; i < bowlers.size(); ++i) {
        QJsonObject bowler = bowlers[i].toObject();
        
        // Initialize frame structure (10 frames, up to 3 balls each for 5-pin)
        QJsonArray frames;
        QJsonArray frameTotals;
        QJsonArray runningTotals;
        
        for (int f = 0; f < 10; ++f) {
            QJsonArray frameData;
            frameData.append(-1); // Ball 1 (not thrown yet)
            frameData.append(-1); // Ball 2
            frameData.append(-1); // Ball 3
            frames.append(frameData);
            frameTotals.append(0);
            runningTotals.append(0);
        }
        
        bowler["frames"] = frames;
        bowler["frame_totals"] = frameTotals;
        bowler["running_totals"] = runningTotals;
        bowler["current_frame"] = 1;
        bowler["current_ball"] = 1;
        bowler["total_score"] = 0;
        bowler["is_active"] = (i == 0); // First bowler is active
        
        enhancedBowlers.append(bowler);
    }
    
    enhancedData["bowlers"] = enhancedBowlers;
    m_laneGameData[laneId] = enhancedData;
    
    // Create response data
    QJsonObject response;
    response["type"] = "quick_game_start";
    response["lane_id"] = laneId;
    response["game_data"] = enhancedData;
    response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Send to lane
    sendMessageToLane(laneId, response);
    
    // Update lane status
    setLaneStatus(laneId, LaneStatus::Active);
    emit gameStarted(laneId, "quick_game", enhancedData);
    emit gameDataReceived(laneId, enhancedData);
}

void LaneServer::handleLeagueGameMessage(int laneId, const QJsonObject &data)
{
    qDebug() << "Starting League Game on lane" << laneId;
    
    // Store game type and enhanced data
    m_laneGameTypes[laneId] = "league_game";
    
    QJsonObject enhancedData = data;
    enhancedData["held"] = false;
    enhancedData["completed"] = false;
    enhancedData["current_bowler"] = 0;
    
    // Initialize team/bowler data similar to quick game but with league specifics
    QJsonArray teams = data["teams"].toArray();
    QJsonArray enhancedBowlers;
    
    for (const QJsonValue &teamValue : teams) {
        QJsonObject team = teamValue.toObject();
        QJsonArray teamBowlers = team["bowlers"].toArray();
        
        for (const QJsonValue &bowlerValue : teamBowlers) {
            QJsonObject bowler = bowlerValue.toObject();
            
            // Add league-specific data
            bowler["team_name"] = team["name"];
            bowler["average"] = bowler.contains("average") ? bowler["average"].toDouble() : 150.0;
            bowler["handicap"] = bowler.contains("handicap") ? bowler["handicap"].toDouble() : 0.0;
            
            // Initialize frame structure
            QJsonArray frames;
            QJsonArray frameTotals;
            QJsonArray runningTotals;
            
            for (int f = 0; f < 10; ++f) {
                QJsonArray frameData;
                frameData.append(-1);
                frameData.append(-1); 
                frameData.append(-1);
                frames.append(frameData);
                frameTotals.append(0);
                runningTotals.append(0);
            }
            
            bowler["frames"] = frames;
            bowler["frame_totals"] = frameTotals;
            bowler["running_totals"] = runningTotals;
            bowler["current_frame"] = 1;
            bowler["current_ball"] = 1;
            bowler["total_score"] = 0;
            bowler["is_active"] = (enhancedBowlers.size() == 0);
            
            enhancedBowlers.append(bowler);
        }
    }
    
    enhancedData["bowlers"] = enhancedBowlers;
    enhancedData["team_name"] = teams.size() > 0 ? teams[0].toObject()["name"].toString() : "";
    
    m_laneGameData[laneId] = enhancedData;
    
    int leagueId = data["league_id"].toInt();
    int eventId = data["event_id"].toInt();
    
    // Notify league manager
    if (m_leagueManager) {
        m_leagueManager->handleLeagueGameStart(laneId, enhancedData);
    }
    
    // Create response data with league-specific configuration
    QJsonObject response;
    response["type"] = "league_game_start";
    response["lane_id"] = laneId;
    response["league_id"] = leagueId;
    response["event_id"] = eventId;
    response["game_data"] = enhancedData;
    response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    // Send to lane
    sendMessageToLane(laneId, response);
    
    // Update lane status to special league status
    setLaneStatus(laneId, LaneStatus::Active);
    emit gameStarted(laneId, "league_game", enhancedData);
    emit gameDataReceived(laneId, enhancedData);
}
