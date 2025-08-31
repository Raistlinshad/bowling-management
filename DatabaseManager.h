#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QDate>
#include <QTime>
#include <QDateTime>

struct BowlerData {
    int id;
    QString firstName;
    QString lastName;
    QString sex;
    int average;
    QString address;
    QString phone;
    QString birthday;
    bool over18;
    QStringList teams;
    QString createdAt;
};

struct TeamData {
    int id;
    QString name;
    QString createdAt;
    QStringList bowlers;
    int bowlerCount;
};

struct LeagueData {
    int id;
    QString name;
    QString createdAt;
    QStringList teams;
    int teamCount;
};

struct CalendarEventData {
    int id = 0;
    QDate date;
    QTime startTime;
    QTime endTime;
    int laneId = 0;
    QString eventType;
    QString title;
    QString description;
    QString contactName;
    QString contactPhone;
    QString contactEmail;
    int bowlerCount = 0;
    QString additionalDetails;
    QDateTime createdAt;
    QDateTime updatedAt;
    
    // Optional foreign key relationships
    int leagueId = 0;    // 0 if not league-related
    int teamId = 0;      // 0 if not team-related
    
    // Default constructor
    CalendarEventData() = default;
};

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager* instance();
    
    bool initializeDatabase();
    bool backupDatabase(const QString& backupPath);
    void closeDatabase();
    
    // Bowler operations
    QVector<BowlerData> getAllBowlers(const QString &searchFilter = "");
    BowlerData getBowlerById(int id);
    int addBowler(const BowlerData &bowler);
    bool updateBowler(const BowlerData &bowler);
    bool deleteBowler(int id);
    
    // Team operations
    QVector<TeamData> getAllTeams();
    TeamData getTeamById(int id);
    int addTeam(const QString &name);
    bool updateTeam(const TeamData &team);
    bool deleteTeam(int id);
    QStringList getBowlerTeams(int bowlerId);
    
    // League operations
    QVector<LeagueData> getAllLeagues();
    LeagueData getLeagueById(int id);
    int addLeague(const QString &name);
    bool updateLeague(const LeagueData &league);
    bool deleteLeague(int id);

    // Calendar event methods
    QVector<CalendarEventData> getAllCalendarEvents();
    QVector<CalendarEventData> getCalendarEventsForDate(const QDate &date);
    QVector<CalendarEventData> getCalendarEventsForDateRange(const QDate &startDate, const QDate &endDate);
    QVector<CalendarEventData> getCalendarEventsForMonth(int year, int month);
    CalendarEventData getCalendarEventById(int id);
    int addCalendarEvent(const CalendarEventData &event);
    bool updateCalendarEvent(const CalendarEventData &event);
    bool deleteCalendarEvent(int id);
    
    // Availability checking
    bool isLaneAvailable(const QDate &date, const QTime &startTime, const QTime &endTime, 
                        int laneId, int excludeEventId = -1);
    QVector<CalendarEventData> getConflictingEvents(const QDate &date, const QTime &startTime, 
                                                   const QTime &endTime, int laneId, int excludeEventId = -1);
    
    // League schedule integration
    QVector<int> addLeagueSchedule(int leagueId, const QString &leagueName, 
                                  const QDate &startDate, const QTime &startTime,
                                  int durationMinutes, int frequencyDays, int numberOfWeeks,
                                  const QVector<int> &laneIds, const QString &contactName,
                                  const QString &contactPhone, const QString &contactEmail = "",
                                  const QString &additionalDetails = "");

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();
    
    bool createTables();
    bool createCalendarEventsTable();
    
    static DatabaseManager* m_instance;
    QSqlDatabase m_database;
};

#endif // DATABASEMANAGER_H