#ifndef LEAGUEMANAGER_H
#define LEAGUEMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>
#include <QVector>
#include <QMap>
#include <QTimer>
#include "DatabaseManager.h"

// Forward declarations
class LaneServer;

// League configuration structures
struct LeagueConfig {
    int leagueId;
    QString name;
    QDate startDate;
    QDate endDate;
    int numberOfWeeks;
    QVector<int> laneIds;
    QString status; // "scheduled", "active", "completed", "cancelled"
    
    // League rules
    struct AverageCalculation {
        enum Type {
            TotalPinsPerGame,      // Total pins / games played
            TotalPinsPerBall,      // Total pins / balls thrown  
            PeriodicUpdate         // Update every N games (default 3)
        } type = TotalPinsPerGame;
        
        int updateInterval = 3;    // For PeriodicUpdate
        int delayGames = 0;        // Delay before counting averages
    } avgCalc;
    
    struct HandicapCalculation {
        enum Type {
            PercentageBased,       // (High - AVG) * percentage
            StraightDifference,    // High - AVG
            WithDeduction          // High - AVG - deduction
        } type = PercentageBased;
        
        int highValue = 225;       // Handicap high value
        double percentage = 0.8;   // For PercentageBased (80%)
        int deduction = 10;        // For WithDeduction
        int delayGames = 3;        // Delay HDCP calculation
    } hdcpCalc;
    
    struct AbsentHandling {
        enum Type {
            PercentageOfAverage,   // % of bowler's average
            FixedValue,           // Set fixed value
            UseAverage            // Use bowler's current average
        } type = PercentageOfAverage;
        
        double percentage = 0.9;   // 90% of average
        int fixedValue = 100;      // Fixed absent score
    } absentHandling;
    
    struct PreBowlRules {
        bool enabled = false;
        bool carryToNextSeason = false;
        bool randomUseWhenAbsent = false;
        enum UseBy {
            ByGame,               // Use one game at a time
            ByThreeGameSet        // Use full 3-game sets
        } useBy = ByGame;
        
        int maxUsesPerGame = 1;    // How many times each pre-bowl can be used
    } preBowlRules;
    
    struct Divisions {
        int count = 1;             // Number of divisions
        bool reorderMidSeason = false;
        enum OrderBy {
            Manual,               // League coordinator sets
            SystemRanked          // System orders by performance
        } orderBy = Manual;
        
        QMap<int, QString> divisionNames; // Division ID -> Name
        QMap<int, QVector<int>> teamAssignments; // Division ID -> Team IDs
    } divisions;
    
    struct Playoffs {
        enum Type {
            None,
            RoundRobin,           // All teams or division only
            RoundRobinWithPlacement,
            PlacementOnly,        // 1vs8, 2vs7, etc
            RoundRobinPlusPlacement
        } type = None;
        
        bool divisionOnly = false; // For RoundRobin
        QVector<QPair<int, int>> placementPairs; // Custom placement matchups
    } playoffs;
    
    struct PointSystem {
        enum Type {
            WinLossTie,          // Individual and team W/L/T
            TeamVsTeam,          // Overall team vs team points
            Custom               // Custom combination
        } type = WinLossTie;
        
        // Point values
        int winPoints = 2;
        int lossPoints = 0;
        int tiePoints = 1;
        
        // Heads-up options
        bool includeHeadsUp = true;
        bool headsUpWithHandicap = true;
        
        // Team vs team options
        bool stackedTiePoints = true; // vs linear distribution
        
        // Custom options
        bool trackHeadsUp = true;
        bool trackTeamVs = true;
        bool trackDivisionVs = false;
        bool trackLeagueVs = false;
        bool trackScratch = false;
        bool trackHandicap = true;
    } pointSystem;
};

struct TeamData {
    int teamId;
    int leagueId;
    QString name;
    QVector<int> bowlerIds;
    int divisionId = 0;
    
    // Season statistics
    int wins = 0;
    int losses = 0;
    int ties = 0;
    int totalPoints = 0;
    double teamAverage = 0.0;
    
    // Track individual matchups
    QMap<int, int> headsUpWins;    // vs other team IDs
    QMap<int, int> headsUpLosses;
    QMap<int, int> headsUpTies;
};

struct BowlerSeasonData {
    int bowlerId;
    int leagueId;
    int teamId;
    
    // Current season stats
    double currentAverage = 0.0;
    double currentHandicap = 0.0;
    int gamesPlayed = 0;
    int totalPins = 0;
    int ballsThrown = 0;
    
    // Performance tracking
    int strikes = 0;
    int spares = 0;
    int highGame = 0;
    int highSeries = 0;
    
    // Pre-bowl management
    QVector<int> preBowlGameIds;   // Available pre-bowled games
    QMap<int, int> preBowlUses;    // Game ID -> times used
    
    QDateTime lastUpdated;
};

struct LeagueEvent {
    int eventId;
    int leagueId;
    int weekNumber;
    QDateTime scheduledTime;
    QVector<int> laneIds;
    
    // Matchups for this event
    struct Matchup {
        int team1Id;
        int team2Id;
        int laneId;
        bool completed = false;
        
        // Results
        int team1Score = 0;
        int team2Score = 0;
        int team1Points = 0;
        int team2Points = 0;
        
        QVector<int> gameIds; // Individual game results
    };
    
    QVector<Matchup> matchups;
    bool eventCompleted = false;
};

class LeagueManager : public QObject
{
    Q_OBJECT

public:
    explicit LeagueManager(LaneServer *laneServer, QObject *parent = nullptr);
    ~LeagueManager();
    
    // League creation and management
    int createLeague(const LeagueConfig &config);
    bool updateLeague(int leagueId, const LeagueConfig &config);
    bool deleteLeague(int leagueId);
    LeagueConfig getLeagueConfig(int leagueId) const;
    
    // Team management
    int addTeam(int leagueId, const QString &teamName, const QVector<int> &bowlerIds);
    bool updateTeam(int teamId, const QString &teamName, const QVector<int> &bowlerIds);
    bool removeTeam(int teamId);
    bool assignTeamToDivision(int teamId, int divisionId);
    QVector<TeamData> getLeagueTeams(int leagueId) const;
    
    // Schedule generation
    bool generateLeagueSchedule(int leagueId);
    bool generateRoundRobinSchedule(int leagueId);
    bool generateCustomSchedule(int leagueId, const QJsonObject &scheduleRules);
    QVector<LeagueEvent> getLeagueSchedule(int leagueId) const;
    
    // Game processing
    void processLeagueGame(int leagueId, int eventId, int laneId, const QJsonObject &gameData);
    void processBowlerGame(int bowlerId, int leagueId, const QJsonObject &gameData);
    void handleAbsentBowler(int bowlerId, int leagueId, int eventId);
    void usePreBowlGame(int bowlerId, int leagueId, int preBowlGameId);
    
    // Statistics and calculations
    void updateBowlerStatistics(int bowlerId, int leagueId);
    void updateTeamStatistics(int teamId);
    void updateLeagueStandings(int leagueId);
    
    double calculateBowlerAverage(int bowlerId, int leagueId) const;
    double calculateBowlerHandicap(int bowlerId, int leagueId) const;
    int calculateAbsentScore(int bowlerId, int leagueId) const;
    
    // Point system management
    void calculateEventPoints(int eventId);
    void calculateHeadsUpPoints(const LeagueEvent::Matchup &matchup, int &team1Points, int &team2Points);
    void calculateTeamVsPoints(int eventId, QMap<int, int> &teamPoints);
    
    // Pre-bowl management
    int recordPreBowlGame(int bowlerId, int leagueId, const QJsonObject &gameData);
    QVector<int> getAvailablePreBowls(int bowlerId, int leagueId) const;
    bool deletePreBowlGame(int preBowlGameId);
    void clearUnusedPreBowls(int leagueId); // End of season cleanup
    
    // Division management
    bool createDivisions(int leagueId, int divisionCount, const QStringList &divisionNames);
    bool reorderDivisions(int leagueId);
    void assignTeamsToDivisions(int leagueId, const QMap<int, QVector<int>> &assignments);
    
    // Playoff management
    bool setupPlayoffs(int leagueId);
    QVector<LeagueEvent> generatePlayoffSchedule(int leagueId);
    bool advancePlayoffTeams(int leagueId, const QVector<int> &winnerTeamIds);
    
    // Reporting and queries
    QJsonObject getLeagueStandings(int leagueId, int divisionId = 0) const;
    QJsonObject getBowlerStatistics(int bowlerId, int leagueId) const;
    QJsonObject getTeamStatistics(int teamId) const;
    QJsonObject getLeagueSummary(int leagueId) const;
    
    // Event handling from client
    void handleLeagueGameStart(int laneId, const QJsonObject &gameData);
    void handleLeagueGameComplete(int laneId, const QJsonObject &gameData);
    void handleDisplayModeChange(int laneId, const QJsonObject &displayData);

signals:
    void leagueCreated(int leagueId, const QString &leagueName);
    void leagueUpdated(int leagueId);
    void leagueCompleted(int leagueId);
    void eventCompleted(int eventId, int leagueId);
    void standingsUpdated(int leagueId);
    void bowlerStatisticsUpdated(int bowlerId, int leagueId);
    void teamStatisticsUpdated(int teamId);
    
    // Send updates to lanes
    void sendToLane(int laneId, const QString &messageType, const QJsonObject &data);

private slots:
    void onPeriodicUpdate();
    void onEventScheduled();

private:
    // Helper methods
    void initializeDatabase();
    bool validateLeagueConfig(const LeagueConfig &config) const;
    bool validateTeamAssignment(int leagueId, const QVector<int> &bowlerIds) const;
    
    QVector<QPair<int, int>> generateRoundRobinPairs(const QVector<int> &teamIds) const;
    void assignLanesToMatchups(int leagueId, LeagueEvent &event) const;
    
    void saveLeagueConfig(const LeagueConfig &config);
    void saveBowlerSeasonData(const BowlerSeasonData &data);
    void saveTeamData(const TeamData &data);
    void saveLeagueEvent(const LeagueEvent &event);
    
    LeagueConfig loadLeagueConfig(int leagueId) const;
    BowlerSeasonData loadBowlerSeasonData(int bowlerId, int leagueId) const;
    TeamData loadTeamData(int teamId) const;
    LeagueEvent loadLeagueEvent(int eventId) const;
    
    // Data members
    LaneServer *m_laneServer;
    DatabaseManager *m_dbManager;
    
    QMap<int, LeagueConfig> m_leagueConfigs;
    QMap<int, QVector<TeamData>> m_leagueTeams;
    QMap<QPair<int, int>, BowlerSeasonData> m_bowlerSeasonData; // (bowlerId, leagueId) -> data
    QMap<int, QVector<LeagueEvent>> m_leagueEvents;
    
    QTimer *m_updateTimer;
    
    // Constants
    static const int DEFAULT_HIGH_HANDICAP = 225;
    static const double DEFAULT_HANDICAP_PERCENTAGE = 0.8;
    static const int MAX_TEAMS_PER_DIVISION = 12;
    static const int MIN_TEAMS_FOR_PLAYOFFS = 4;
};

#endif // LEAGUEMANAGER_H