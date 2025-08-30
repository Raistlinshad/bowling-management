#include "LeagueManager.h"
#include "LaneServer.h"
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QJsonDocument>
#include <QtMath>
#include <QRandomGenerator>

LeagueManager::LeagueManager(LaneServer *laneServer, QObject *parent)
    : QObject(parent)
    , m_laneServer(laneServer)
    , m_dbManager(DatabaseManager::instance())
    , m_updateTimer(new QTimer(this))
{
    initializeDatabase();
    
    // Setup periodic updates (every 5 minutes)
    m_updateTimer->setInterval(300000);
    connect(m_updateTimer, &QTimer::timeout, this, &LeagueManager::onPeriodicUpdate);
    m_updateTimer->start();
    
    qDebug() << "LeagueManager initialized";
}

LeagueManager::~LeagueManager()
{
    m_updateTimer->stop();
}

void LeagueManager::initializeDatabase()
{
    // Create league management tables
    QSqlQuery query;
    
    // League configurations table
    query.exec("CREATE TABLE IF NOT EXISTS league_configs ("
              "league_id INTEGER PRIMARY KEY, "
              "name TEXT NOT NULL, "
              "start_date TEXT, "
              "end_date TEXT, "
              "number_of_weeks INTEGER, "
              "lane_ids TEXT, "
              "status TEXT DEFAULT 'scheduled', "
              "config_json TEXT, "
              "created_at DATETIME DEFAULT CURRENT_TIMESTAMP"
              ")");
    
    // League teams table
    query.exec("CREATE TABLE IF NOT EXISTS league_teams ("
              "team_id INTEGER PRIMARY KEY, "
              "league_id INTEGER, "
              "name TEXT NOT NULL, "
              "bowler_ids TEXT, "
              "division_id INTEGER DEFAULT 0, "
              "wins INTEGER DEFAULT 0, "
              "losses INTEGER DEFAULT 0, "
              "ties INTEGER DEFAULT 0, "
              "total_points INTEGER DEFAULT 0, "
              "team_average REAL DEFAULT 0.0, "
              "FOREIGN KEY(league_id) REFERENCES league_configs(league_id)"
              ")");
    
    // Bowler season data table
    query.exec("CREATE TABLE IF NOT EXISTS bowler_season_data ("
              "bowler_id INTEGER, "
              "league_id INTEGER, "
              "team_id INTEGER, "
              "current_average REAL DEFAULT 0.0, "
              "current_handicap REAL DEFAULT 0.0, "
              "games_played INTEGER DEFAULT 0, "
              "total_pins INTEGER DEFAULT 0, "
              "balls_thrown INTEGER DEFAULT 0, "
              "strikes INTEGER DEFAULT 0, "
              "spares INTEGER DEFAULT 0, "
              "high_game INTEGER DEFAULT 0, "
              "high_series INTEGER DEFAULT 0, "
              "prebowl_games TEXT, "
              "last_updated DATETIME DEFAULT CURRENT_TIMESTAMP, "
              "PRIMARY KEY(bowler_id, league_id)"
              ")");
    
    // League events table
    query.exec("CREATE TABLE IF NOT EXISTS league_events ("
              "event_id INTEGER PRIMARY KEY, "
              "league_id INTEGER, "
              "week_number INTEGER, "
              "scheduled_time DATETIME, "
              "lane_ids TEXT, "
              "matchups_json TEXT, "
              "event_completed BOOLEAN DEFAULT 0, "
              "FOREIGN KEY(league_id) REFERENCES league_configs(league_id)"
              ")");
    
    // Pre-bowl games table
    query.exec("CREATE TABLE IF NOT EXISTS prebowl_games ("
              "prebowl_id INTEGER PRIMARY KEY, "
              "bowler_id INTEGER, "
              "league_id INTEGER, "
              "game_data TEXT, "
              "times_used INTEGER DEFAULT 0, "
              "max_uses INTEGER DEFAULT 1, "
              "created_at DATETIME DEFAULT CURRENT_TIMESTAMP, "
              "FOREIGN KEY(bowler_id) REFERENCES bowlers(id), "
              "FOREIGN KEY(league_id) REFERENCES league_configs(league_id)"
              ")");
    
    if (query.lastError().isValid()) {
        qWarning() << "Database initialization error:" << query.lastError().text();
    }
}

int LeagueManager::createLeague(const LeagueConfig &config)
{
    if (!validateLeagueConfig(config)) {
        qWarning() << "Invalid league configuration";
        return -1;
    }
    
    QSqlQuery query;
    query.prepare("INSERT INTO league_configs (name, start_date, end_date, number_of_weeks, "
                 "lane_ids, status, config_json) VALUES (?, ?, ?, ?, ?, ?, ?)");
    
    // Convert lane IDs to JSON string
    QJsonArray laneArray;
    for (int laneId : config.laneIds) {
        laneArray.append(laneId);
    }
    
    // Convert config to JSON
    QJsonObject configJson;
    configJson["avg_calc_type"] = static_cast<int>(config.avgCalc.type);
    configJson["avg_calc_interval"] = config.avgCalc.updateInterval;
    configJson["avg_delay_games"] = config.avgCalc.delayGames;
    
    configJson["hdcp_calc_type"] = static_cast<int>(config.hdcpCalc.type);
    configJson["hdcp_high_value"] = config.hdcpCalc.highValue;
    configJson["hdcp_percentage"] = config.hdcpCalc.percentage;
    configJson["hdcp_deduction"] = config.hdcpCalc.deduction;
    configJson["hdcp_delay_games"] = config.hdcpCalc.delayGames;
    
    configJson["absent_type"] = static_cast<int>(config.absentHandling.type);
    configJson["absent_percentage"] = config.absentHandling.percentage;
    configJson["absent_fixed_value"] = config.absentHandling.fixedValue;
    
    configJson["prebowl_enabled"] = config.preBowlRules.enabled;
    configJson["prebowl_carry_over"] = config.preBowlRules.carryToNextSeason;
    configJson["prebowl_random_use"] = config.preBowlRules.randomUseWhenAbsent;
    configJson["prebowl_use_by"] = static_cast<int>(config.preBowlRules.useBy);
    configJson["prebowl_max_uses"] = config.preBowlRules.maxUsesPerGame;
    
    configJson["divisions_count"] = config.divisions.count;
    configJson["divisions_reorder"] = config.divisions.reorderMidSeason;
    configJson["divisions_order_by"] = static_cast<int>(config.divisions.orderBy);
    
    configJson["playoffs_type"] = static_cast<int>(config.playoffs.type);
    configJson["playoffs_division_only"] = config.playoffs.divisionOnly;
    
    configJson["points_type"] = static_cast<int>(config.pointSystem.type);
    configJson["points_win"] = config.pointSystem.winPoints;
    configJson["points_loss"] = config.pointSystem.lossPoints;
    configJson["points_tie"] = config.pointSystem.tiePoints;
    configJson["points_heads_up"] = config.pointSystem.includeHeadsUp;
    configJson["points_heads_up_hdcp"] = config.pointSystem.headsUpWithHandicap;
    configJson["points_stacked_ties"] = config.pointSystem.stackedTiePoints;
    
    query.addBindValue(config.name);
    query.addBindValue(config.startDate.toString("yyyy-MM-dd"));
    query.addBindValue(config.endDate.toString("yyyy-MM-dd"));
    query.addBindValue(config.numberOfWeeks);
    query.addBindValue(QJsonDocument(laneArray).toJson(QJsonDocument::Compact));
    query.addBindValue(config.status);
    query.addBindValue(QJsonDocument(configJson).toJson(QJsonDocument::Compact));
    
    if (!query.exec()) {
        qWarning() << "Failed to create league:" << query.lastError().text();
        return -1;
    }
    
    int leagueId = query.lastInsertId().toInt();
    
    // Cache the configuration
    LeagueConfig newConfig = config;
    newConfig.leagueId = leagueId;
    m_leagueConfigs[leagueId] = newConfig;
    
    emit leagueCreated(leagueId, config.name);
    
    qDebug() << "Created league" << config.name << "with ID" << leagueId;
    return leagueId;
}

bool LeagueManager::generateLeagueSchedule(int leagueId)
{
    if (!m_leagueConfigs.contains(leagueId)) {
        qWarning() << "League not found:" << leagueId;
        return false;
    }
    
    const LeagueConfig &config = m_leagueConfigs[leagueId];
    QVector<TeamData> teams = getLeagueTeams(leagueId);
    
    if (teams.size() < 2) {
        qWarning() << "Not enough teams to generate schedule for league" << leagueId;
        return false;
    }
    
    // Generate round-robin schedule
    QVector<QPair<int, int>> matchupPairs = generateRoundRobinPairs(
        QVector<int>(teams.begin(), teams.end(), [](const TeamData &t) { return t.teamId; }));
    
    // Calculate games needed for complete round-robin
    int gamesPerWeek = config.laneIds.size();
    int totalMatchups = matchupPairs.size();
    int weeksNeeded = qCeil(static_cast<double>(totalMatchups) / gamesPerWeek);
    
    // Generate events for each week
    QVector<LeagueEvent> events;
    int currentMatchupIndex = 0;
    
    for (int week = 1; week <= config.numberOfWeeks && week <= weeksNeeded; ++week) {
        LeagueEvent event;
        event.leagueId = leagueId;
        event.weekNumber = week;
        event.scheduledTime = QDateTime(config.startDate.addDays((week - 1) * 7), QTime(19, 0));
        event.laneIds = config.laneIds;
        
        // Assign matchups to lanes for this week
        for (int lane = 0; lane < config.laneIds.size() && currentMatchupIndex < totalMatchups; ++lane) {
            if (currentMatchupIndex < matchupPairs.size()) {
                LeagueEvent::Matchup matchup;
                matchup.team1Id = matchupPairs[currentMatchupIndex].first;
                matchup.team2Id = matchupPairs[currentMatchupIndex].second;
                matchup.laneId = config.laneIds[lane];
                
                event.matchups.append(matchup);
                currentMatchupIndex++;
            }
        }
        
        events.append(event);
    }
    
    // Save events to database
    for (const LeagueEvent &event : events) {
        saveLeagueEvent(event);
    }
    
    m_leagueEvents[leagueId] = events;
    
    qDebug() << "Generated schedule for league" << leagueId << ":" << events.size() << "events";
    return true;
}

QVector<QPair<int, int>> LeagueManager::generateRoundRobinPairs(const QVector<int> &teamIds) const
{
    QVector<QPair<int, int>> pairs;
    int teamCount = teamIds.size();
    
    // Standard round-robin algorithm
    for (int i = 0; i < teamCount; ++i) {
        for (int j = i + 1; j < teamCount; ++j) {
            pairs.append(QPair<int, int>(teamIds[i], teamIds[j]));
        }
    }
    
    return pairs;
}

void LeagueManager::processLeagueGame(int leagueId, int eventId, int laneId, const QJsonObject &gameData)
{
    qDebug() << "Processing league game for league" << leagueId << "event" << eventId << "lane" << laneId;
    
    // Find the corresponding event and matchup
    if (!m_leagueEvents.contains(leagueId)) {
        qWarning() << "No events found for league" << leagueId;
        return;
    }
    
    QVector<LeagueEvent> &events = m_leagueEvents[leagueId];
    LeagueEvent *currentEvent = nullptr;
    LeagueEvent::Matchup *currentMatchup = nullptr;
    
    // Find the event and matchup
    for (LeagueEvent &event : events) {
        if (event.eventId == eventId) {
            currentEvent = &event;
            for (LeagueEvent::Matchup &matchup : event.matchups) {
                if (matchup.laneId == laneId) {
                    currentMatchup = &matchup;
                    break;
                }
            }
            break;
        }
    }
    
    if (!currentEvent || !currentMatchup) {
        qWarning() << "Could not find event/matchup for league" << leagueId << "event" << eventId << "lane" << laneId;
        return;
    }
    
    // Process individual bowler games
    QJsonArray bowlers = gameData["bowlers"].toArray();
    for (const QJsonValue &bowlerValue : bowlers) {
        QJsonObject bowlerGame = bowlerValue.toObject();
        int bowlerId = bowlerGame["bowler_id"].toInt();
        
        if (bowlerId > 0) {
            processBowlerGame(bowlerId, leagueId, bowlerGame);
        }
    }
    
    // Update matchup results
    currentMatchup->team1Score = gameData["team1_total"].toInt();
    currentMatchup->team2Score = gameData["team2_total"].toInt();
    currentMatchup->completed = true;
    
    // Calculate points based on league point system
    calculateHeadsUpPoints(*currentMatchup, currentMatchup->team1Points, currentMatchup->team2Points);
    
    // Check if event is complete
    bool eventComplete = true;
    for (const LeagueEvent::Matchup &matchup : currentEvent->matchups) {
        if (!matchup.completed) {
            eventComplete = false;
            break;
        }
    }
    
    if (eventComplete) {
        currentEvent->eventCompleted = true;
        calculateEventPoints(eventId);
        emit eventCompleted(eventId, leagueId);
    }
    
    // Update team and league standings
    updateTeamStatistics(currentMatchup->team1Id);
    updateTeamStatistics(currentMatchup->team2Id);
    updateLeagueStandings(leagueId);
    
    // Save updated event
    saveLeagueEvent(*currentEvent);
}

void LeagueManager::processBowlerGame(int bowlerId, int leagueId, const QJsonObject &gameData)
{
    BowlerSeasonData bowlerData = loadBowlerSeasonData(bowlerId, leagueId);
    
    // Update game statistics
    int gameScore = gameData["total_score"].toInt();
    int strikes = gameData["strikes"].toInt(0);
    int spares = gameData["spares"].toInt(0);
    
    bowlerData.gamesPlayed++;
    bowlerData.totalPins += gameScore;
    bowlerData.strikes += strikes;
    bowlerData.spares += spares;
    
    if (gameScore > bowlerData.highGame) {
        bowlerData.highGame = gameScore;
    }
    
    // Update series tracking (assuming 3-game series)
    if (bowlerData.gamesPlayed % 3 == 0) {
        int seriesScore = gameData["series_total"].toInt(gameScore * 3); // Fallback calculation
        if (seriesScore > bowlerData.highSeries) {
            bowlerData.highSeries = seriesScore;
        }
    }
    
    // Update average and handicap
    updateBowlerStatistics(bowlerId, leagueId);
    
    // Save updated data
    saveBowlerSeasonData(bowlerData);
    
    emit bowlerStatisticsUpdated(bowlerId, leagueId);
}

double LeagueManager::calculateBowlerAverage(int bowlerId, int leagueId) const
{
    const LeagueConfig &config = m_leagueConfigs[leagueId];
    BowlerSeasonData bowlerData = loadBowlerSeasonData(bowlerId, leagueId);
    
    if (bowlerData.gamesPlayed < config.avgCalc.delayGames) {
        return 0.0; // Not enough games played yet
    }
    
    double average = 0.0;
    
    switch (config.avgCalc.type) {
    case LeagueConfig::AverageCalculation::TotalPinsPerGame:
        if (bowlerData.gamesPlayed > 0) {
            average = static_cast<double>(bowlerData.totalPins) / bowlerData.gamesPlayed;
        }
        break;
        
    case LeagueConfig::AverageCalculation::TotalPinsPerBall:
        if (bowlerData.ballsThrown > 0) {
            average = static_cast<double>(bowlerData.totalPins) / bowlerData.ballsThrown;
        }
        break;
        
    case LeagueConfig::AverageCalculation::PeriodicUpdate:
        // Update only every N games
        if (bowlerData.gamesPlayed % config.avgCalc.updateInterval == 0) {
            if (bowlerData.gamesPlayed > 0) {
                average = static_cast<double>(bowlerData.totalPins) / bowlerData.gamesPlayed;
            }
        } else {
            // Return current stored average
            average = bowlerData.currentAverage;
        }
        break;
    }
    
    return qRound(average * 100.0) / 100.0; // Round to 2 decimal places
}

double LeagueManager::calculateBowlerHandicap(int bowlerId, int leagueId) const
{
    const LeagueConfig &config = m_leagueConfigs[leagueId];
    BowlerSeasonData bowlerData = loadBowlerSeasonData(bowlerId, leagueId);
    
    if (bowlerData.gamesPlayed < config.hdcpCalc.delayGames) {
        return 0.0; // Not enough games for handicap calculation
    }
    
    double average = calculateBowlerAverage(bowlerId, leagueId);
    if (average <= 0.0) {
        return 0.0;
    }
    
    double handicap = 0.0;
    
    switch (config.hdcpCalc.type) {
    case LeagueConfig::HandicapCalculation::PercentageBased:
        handicap = (config.hdcpCalc.highValue - average) * config.hdcpCalc.percentage;
        break;
        
    case LeagueConfig::HandicapCalculation::StraightDifference:
        handicap = config.hdcpCalc.highValue - average;
        break;
        
    case LeagueConfig::HandicapCalculation::WithDeduction:
        handicap = config.hdcpCalc.highValue - average - config.hdcpCalc.deduction;
        break;
    }
    
    return qMax(0.0, qRound(handicap * 100.0) / 100.0); // No negative handicaps
}

int LeagueManager::calculateAbsentScore(int bowlerId, int leagueId) const
{
    const LeagueConfig &config = m_leagueConfigs[leagueId];
    double bowlerAverage = calculateBowlerAverage(bowlerId, leagueId);
    
    int absentScore = 0;
    
    switch (config.absentHandling.type) {
    case LeagueConfig::AbsentHandling::PercentageOfAverage:
        absentScore = static_cast<int>(bowlerAverage * config.absentHandling.percentage);
        break;
        
    case LeagueConfig::AbsentHandling::FixedValue:
        absentScore = config.absentHandling.fixedValue;
        break;
        
    case LeagueConfig::AbsentHandling::UseAverage:
        absentScore = static_cast<int>(bowlerAverage);
        break;
    }
    
    return qMax(0, absentScore);
}

void LeagueManager::handleAbsentBowler(int bowlerId, int leagueId, int eventId)
{
    const LeagueConfig &config = m_leagueConfigs[leagueId];
    
    // Check if we should use a pre-bowl game instead
    if (config.preBowlRules.enabled && config.preBowlRules.randomUseWhenAbsent) {
        QVector<int> availablePreBowls = getAvailablePreBowls(bowlerId, leagueId);
        
        if (!availablePreBowls.isEmpty()) {
            // Randomly select a pre-bowl game
            int randomIndex = QRandomGenerator::global()->bounded(availablePreBowls.size());
            int preBowlGameId = availablePreBowls[randomIndex];
            
            usePreBowlGame(bowlerId, leagueId, preBowlGameId);
            
            qDebug() << "Used pre-bowl game" << preBowlGameId << "for absent bowler" << bowlerId;
            return;
        }
    }
    
    // Use absent score calculation
    int absentScore = calculateAbsentScore(bowlerId, leagueId);
    
    // Create absent game data
    QJsonObject absentGameData;
    absentGameData["bowler_id"] = bowlerId;
    absentGameData["total_score"] = absentScore;
    absentGameData["absent"] = true;
    absentGameData["strikes"] = 0;
    absentGameData["spares"] = 0;
    
    // Process as regular game
    processBowlerGame(bowlerId, leagueId, absentGameData);
    
    qDebug() << "Processed absent score" << absentScore << "for bowler" << bowlerId;
}

int LeagueManager::recordPreBowlGame(int bowlerId, int leagueId, const QJsonObject &gameData)
{
    const LeagueConfig &config = m_leagueConfigs[leagueId];
    
    if (!config.preBowlRules.enabled) {
        qWarning() << "Pre-bowl games not enabled for league" << leagueId;
        return -1;
    }
    
    QSqlQuery query;
    query.prepare("INSERT INTO prebowl_games (bowler_id, league_id, game_data, max_uses) "
                 "VALUES (?, ?, ?, ?)");
    
    query.addBindValue(bowlerId);
    query.addBindValue(leagueId);
    query.addBindValue(QJsonDocument(gameData).toJson(QJsonDocument::Compact));
    query.addBindValue(config.preBowlRules.maxUsesPerGame);
    
    if (!query.exec()) {
        qWarning() << "Failed to record pre-bowl game:" << query.lastError().text();
        return -1;
    }
    
    int preBowlId = query.lastInsertId().toInt();
    
    qDebug() << "Recorded pre-bowl game" << preBowlId << "for bowler" << bowlerId << "league" << leagueId;
    return preBowlId;
}

QVector<int> LeagueManager::getAvailablePreBowls(int bowlerId, int leagueId) const
{
    QVector<int> availablePreBowls;
    
    QSqlQuery query;
    query.prepare("SELECT prebowl_id FROM prebowl_games "
                 "WHERE bowler_id = ? AND league_id = ? AND times_used < max_uses");
    query.addBindValue(bowlerId);
    query.addBindValue(leagueId);
    
    if (query.exec()) {
        while (query.next()) {
            availablePreBowls.append(query.value("prebowl_id").toInt());
        }
    }
    
    return availablePreBowls;
}

void LeagueManager::usePreBowlGame(int bowlerId, int leagueId, int preBowlGameId)
{
    // Get pre-bowl game data
    QSqlQuery query;
    query.prepare("SELECT game_data FROM prebowl_games WHERE prebowl_id = ?");
    query.addBindValue(preBowlGameId);
    
    if (!query.exec() || !query.next()) {
        qWarning() << "Pre-bowl game not found:" << preBowlGameId;
        return;
    }
    
    QString gameDataStr = query.value("game_data").toString();
    QJsonDocument gameDoc = QJsonDocument::fromJson(gameDataStr.toUtf8());
    QJsonObject gameData = gameDoc.object();
    
    // Process the pre-bowl game
    processBowlerGame(bowlerId, leagueId, gameData);
    
    // Update usage count
    query.prepare("UPDATE prebowl_games SET times_used = times_used + 1 "
                 "WHERE prebowl_id = ?");
    query.addBindValue(preBowlGameId);
    query.exec();
    
    qDebug() << "Used pre-bowl game" << preBowlGameId << "for bowler" << bowlerId;
}

void LeagueManager::calculateHeadsUpPoints(const LeagueEvent::Matchup &matchup, 
                                          int &team1Points, int &team2Points)
{
    team1Points = 0;
    team2Points = 0;
    
    if (matchup.team1Score > matchup.team2Score) {
        team1Points = 2; // Win
        team2Points = 0; // Loss
    } else if (matchup.team2Score > matchup.team1Score) {
        team1Points = 0; // Loss
        team2Points = 2; // Win
    } else {
        team1Points = 1; // Tie
        team2Points = 1; // Tie
    }
}

void LeagueManager::calculateEventPoints(int eventId)
{
    // Find the event
    LeagueEvent *event = nullptr;
    int leagueId = 0;
    
    for (auto it = m_leagueEvents.begin(); it != m_leagueEvents.end(); ++it) {
        for (LeagueEvent &evt : it.value()) {
            if (evt.eventId == eventId) {
                event = &evt;
                leagueId = it.key();
                break;
            }
        }
        if (event) break;
    }
    
    if (!event) {
        qWarning() << "Event not found:" << eventId;
        return;
    }
    
    const LeagueConfig &config = m_leagueConfigs[leagueId];
    
    // Calculate points based on point system
    switch (config.pointSystem.type) {
    case LeagueConfig::PointSystem::WinLossTie:
        // Already calculated in calculateHeadsUpPoints
        break;
        
    case LeagueConfig::PointSystem::TeamVsTeam:
        {
            QMap<int, int> teamPoints;
            calculateTeamVsPoints(eventId, teamPoints);
            
            // Update matchup points
            for (LeagueEvent::Matchup &matchup : event->matchups) {
                matchup.team1Points = teamPoints.value(matchup.team1Id, 0);
                matchup.team2Points = teamPoints.value(matchup.team2Id, 0);
            }
        }
        break;
        
    case LeagueConfig::PointSystem::Custom:
        // Implement custom point calculation logic
        break;
    }
    
    // Update team statistics with points
    for (const LeagueEvent::Matchup &matchup : event->matchups) {
        updateTeamPoints(matchup.team1Id, matchup.team1Points);
        updateTeamPoints(matchup.team2Id, matchup.team2Points);
    }
}

void LeagueManager::calculateTeamVsPoints(int eventId, QMap<int, int> &teamPoints)
{
    // Get all teams and their scores for this event
    QMap<int, int> teamScores;
    
    // Find the event and collect team scores
    for (const auto &events : m_leagueEvents) {
        for (const LeagueEvent &event : events) {
            if (event.eventId == eventId) {
                for (const LeagueEvent::Matchup &matchup : event.matchups) {
                    teamScores[matchup.team1Id] = matchup.team1Score;
                    teamScores[matchup.team2Id] = matchup.team2Score;
                }
                break;
            }
        }
    }
    
    // Sort teams by score (highest first)
    QVector<QPair<int, int>> sortedTeams; // (teamId, score)
    for (auto it = teamScores.begin(); it != teamScores.end(); ++it) {
        sortedTeams.append(QPair<int, int>(it.key(), it.value()));
    }
    
    std::sort(sortedTeams.begin(), sortedTeams.end(), 
              [](const QPair<int, int> &a, const QPair<int, int> &b) {
                  return a.second > b.second; // Sort by score descending
              });
    
    // Assign points based on ranking
    int totalTeams = sortedTeams.size();
    for (int i = 0; i < sortedTeams.size(); ++i) {
        int teamId = sortedTeams[i].first;
        int points = totalTeams - i; // Highest score gets most points
        teamPoints[teamId] = points;
    }
}

void LeagueManager::updateTeamPoints(int teamId, int points)
{
    QSqlQuery query;
    query.prepare("UPDATE league_teams SET total_points = total_points + ? WHERE team_id = ?");
    query.addBindValue(points);
    query.addBindValue(teamId);
    
    if (!query.exec()) {
        qWarning() << "Failed to update team points:" << query.lastError().text();
    }
}

void LeagueManager::updateBowlerStatistics(int bowlerId, int leagueId)
{
    BowlerSeasonData bowlerData = loadBowlerSeasonData(bowlerId, leagueId);
    
    // Recalculate average and handicap
    bowlerData.currentAverage = calculateBowlerAverage(bowlerId, leagueId);
    bowlerData.currentHandicap = calculateBowlerHandicap(bowlerId, leagueId);
    bowlerData.lastUpdated = QDateTime::currentDateTime();
    
    // Save updated statistics
    saveBowlerSeasonData(bowlerData);
}

void LeagueManager::updateTeamStatistics(int teamId)
{
    TeamData teamData = loadTeamData(teamId);
    
    // Calculate team average from all bowler averages
    double totalAverage = 0.0;
    int activeBowlers = 0;
    
    for (int bowlerId : teamData.bowlerIds) {
        double bowlerAvg = calculateBowlerAverage(bowlerId, teamData.leagueId);
        if (bowlerAvg > 0.0) {
            totalAverage += bowlerAvg;
            activeBowlers++;
        }
    }
    
    if (activeBowlers > 0) {
        teamData.teamAverage = totalAverage / activeBowlers;
    }
    
    // Save updated team data
    saveTeamData(teamData);
    
    emit teamStatisticsUpdated(teamId);
}

void LeagueManager::updateLeagueStandings(int leagueId)
{
    // This triggers a recalculation of all standings
    emit standingsUpdated(leagueId);
}

// Message handling from lanes
void LeagueManager::handleLeagueGameStart(int laneId, const QJsonObject &gameData)
{
    qDebug() << "League game starting on lane" << laneId;
    
    int leagueId = gameData["league_id"].toInt();
    int eventId = gameData["event_id"].toInt();
    
    if (leagueId <= 0 || eventId <= 0) {
        qWarning() << "Invalid league game data:" << gameData;
        return;
    }
    
    // Send league configuration to lane
    QJsonObject configData;
    if (m_leagueConfigs.contains(leagueId)) {
        const LeagueConfig &config = m_leagueConfigs[leagueId];
        
        configData["league_name"] = config.name;
        configData["handicap_enabled"] = (config.hdcpCalc.type != LeagueConfig::HandicapCalculation::PercentageBased || 
                                         config.hdcpCalc.highValue > 0);
        configData["prebowl_enabled"] = config.preBowlRules.enabled;
        configData["point_system"] = static_cast<int>(config.pointSystem.type);
    }
    
    emit sendToLane(laneId, "league_config", configData);
}

void LeagueManager::handleLeagueGameComplete(int laneId, const QJsonObject &gameData)
{
    qDebug() << "League game completed on lane" << laneId;
    
    int leagueId = gameData["league_id"].toInt();
    int eventId = gameData["event_id"].toInt();
    
    if (leagueId > 0 && eventId > 0) {
        processLeagueGame(leagueId, eventId, laneId, gameData);
    }
}

void LeagueManager::handleDisplayModeChange(int laneId, const QJsonObject &displayData)
{
    qDebug() << "Display mode change for lane" << laneId << ":" << displayData;
    
    // Forward display mode changes to the specific lane
    emit sendToLane(laneId, "display_mode_update", displayData);
}

// Database operations
void LeagueManager::saveLeagueEvent(const LeagueEvent &event)
{
    QJsonObject matchupsJson;
    QJsonArray matchupsArray;
    
    for (const LeagueEvent::Matchup &matchup : event.matchups) {
        QJsonObject matchupObj;
        matchupObj["team1_id"] = matchup.team1Id;
        matchupObj["team2_id"] = matchup.team2Id;
        matchupObj["lane_id"] = matchup.laneId;
        matchupObj["completed"] = matchup.completed;
        matchupObj["team1_score"] = matchup.team1Score;
        matchupObj["team2_score"] = matchup.team2Score;
        matchupObj["team1_points"] = matchup.team1Points;
        matchupObj["team2_points"] = matchup.team2Points;
        
        QJsonArray gameIdsArray;
        for (int gameId : matchup.gameIds) {
            gameIdsArray.append(gameId);
        }
        matchupObj["game_ids"] = gameIdsArray;
        
        matchupsArray.append(matchupObj);
    }
    matchupsJson["matchups"] = matchupsArray;
    
    QJsonArray laneIdsArray;
    for (int laneId : event.laneIds) {
        laneIdsArray.append(laneId);
    }
    
    QSqlQuery query;
    if (event.eventId > 0) {
        // Update existing event
        query.prepare("UPDATE league_events SET matchups_json = ?, event_completed = ? "
                     "WHERE event_id = ?");
        query.addBindValue(QJsonDocument(matchupsJson).toJson(QJsonDocument::Compact));
        query.addBindValue(event.eventCompleted);
        query.addBindValue(event.eventId);
    } else {
        // Insert new event
        query.prepare("INSERT INTO league_events (league_id, week_number, scheduled_time, "
                     "lane_ids, matchups_json, event_completed) VALUES (?, ?, ?, ?, ?, ?)");
        query.addBindValue(event.leagueId);
        query.addBindValue(event.weekNumber);
        query.addBindValue(event.scheduledTime.toString(Qt::ISODate));
        query.addBindValue(QJsonDocument(laneIdsArray).toJson(QJsonDocument::Compact));
        query.addBindValue(QJsonDocument(matchupsJson).toJson(QJsonDocument::Compact));
        query.addBindValue(event.eventCompleted);
    }
    
    if (!query.exec()) {
        qWarning() << "Failed to save league event:" << query.lastError().text();
    }
}

BowlerSeasonData LeagueManager::loadBowlerSeasonData(int bowlerId, int leagueId) const
{
    BowlerSeasonData data;
    data.bowlerId = bowlerId;
    data.leagueId = leagueId;
    
    QSqlQuery query;
    query.prepare("SELECT * FROM bowler_season_data WHERE bowler_id = ? AND league_id = ?");
    query.addBindValue(bowlerId);
    query.addBindValue(leagueId);
    
    if (query.exec() && query.next()) {
        data.teamId = query.value("team_id").toInt();
        data.currentAverage = query.value("current_average").toDouble();
        data.currentHandicap = query.value("current_handicap").toDouble();
        data.gamesPlayed = query.value("games_played").toInt();
        data.totalPins = query.value("total_pins").toInt();
        data.ballsThrown = query.value("balls_thrown").toInt();
        data.strikes = query.value("strikes").toInt();
        data.spares = query.value("spares").toInt();
        data.highGame = query.value("high_game").toInt();
        data.highSeries = query.value("high_series").toInt();
        data.lastUpdated = QDateTime::fromString(query.value("last_updated").toString(), Qt::ISODate);
        
        // Parse pre-bowl games
        QString preBowlStr = query.value("prebowl_games").toString();
        if (!preBowlStr.isEmpty()) {
            QJsonDocument preBowlDoc = QJsonDocument::fromJson(preBowlStr.toUtf8());
            QJsonArray preBowlArray = preBowlDoc.array();
            for (const QJsonValue &value : preBowlArray) {
                data.preBowlGameIds.append(value.toInt());
            }
        }
    }
    
    return data;
}

void LeagueManager::saveBowlerSeasonData(const BowlerSeasonData &data)
{
    // Convert pre-bowl games to JSON
    QJsonArray preBowlArray;
    for (int gameId : data.preBowlGameIds) {
        preBowlArray.append(gameId);
    }
    
    QSqlQuery query;
    query.prepare("INSERT OR REPLACE INTO bowler_season_data "
                 "(bowler_id, league_id, team_id, current_average, current_handicap, "
                 "games_played, total_pins, balls_thrown, strikes, spares, high_game, "
                 "high_series, prebowl_games, last_updated) "
                 "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
    
    query.addBindValue(data.bowlerId);
    query.addBindValue(data.leagueId);
    query.addBindValue(data.teamId);
    query.addBindValue(data.currentAverage);
    query.addBindValue(data.currentHandicap);
    query.addBindValue(data.gamesPlayed);
    query.addBindValue(data.totalPins);
    query.addBindValue(data.ballsThrown);
    query.addBindValue(data.strikes);
    query.addBindValue(data.spares);
    query.addBindValue(data.highGame);
    query.addBindValue(data.highSeries);
    query.addBindValue(QJsonDocument(preBowlArray).toJson(QJsonDocument::Compact));
    query.addBindValue(data.lastUpdated.toString(Qt::ISODate));
    
    if (!query.exec()) {
        qWarning() << "Failed to save bowler season data:" << query.lastError().text();
    }
}

bool LeagueManager::validateLeagueConfig(const LeagueConfig &config) const
{
    if (config.name.isEmpty()) return false;
    if (config.numberOfWeeks <= 0 || config.numberOfWeeks > 52) return false;
    if (config.laneIds.isEmpty()) return false;
    if (config.startDate >= config.endDate) return false;
    
    return true;
}

void LeagueManager::onPeriodicUpdate()
{
    // Perform periodic maintenance tasks
    qDebug() << "League Manager periodic update";
    
    // Update any pending calculations
    // Check for completed events
    // Clean up old data if needed
}

QJsonObject LeagueManager::getLeagueStandings(int leagueId, int divisionId) const
{
    QJsonObject standings;
    QVector<TeamData> teams = getLeagueTeams(leagueId);
    
    // Filter by division if specified
    if (divisionId > 0) {
        teams.erase(std::remove_if(teams.begin(), teams.end(),
                                  [divisionId](const TeamData &team) {
                                      return team.divisionId != divisionId;
                                  }), teams.end());
    }
    
    // Sort by total points (descending)
    std::sort(teams.begin(), teams.end(),
              [](const TeamData &a, const TeamData &b) {
                  return a.totalPoints > b.totalPoints;
              });
    
    QJsonArray teamsArray;
    for (int i = 0; i < teams.size(); ++i) {
        const TeamData &team = teams[i];
        QJsonObject teamObj;
        teamObj["rank"] = i + 1;
        teamObj["team_name"] = team.name;
        teamObj["wins"] = team.wins;
        teamObj["losses"] = team.losses;
        teamObj["ties"] = team.ties;
        teamObj["total_points"] = team.totalPoints;
        teamObj["team_average"] = team.teamAverage;
        
        teamsArray.append(teamObj);
    }
    
    standings["teams"] = teamsArray;
    standings["division_id"] = divisionId;
    standings["last_updated"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    return standings;
}