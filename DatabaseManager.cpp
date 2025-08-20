#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDateTime>
#include <QStandardPaths>
#include <QDir>

DatabaseManager* DatabaseManager::m_instance = nullptr;

DatabaseManager* DatabaseManager::instance()
{
    if (!m_instance) {
        m_instance = new DatabaseManager();
    }
    return m_instance;
}

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent)
{
    initializeDatabase();
}

DatabaseManager::~DatabaseManager()
{
    closeDatabase();
}

bool DatabaseManager::initializeDatabase()
{
    // Create database in application data directory
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataDir);
    QString dbPath = dataDir + "/bowling.db";
    
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    m_database.setDatabaseName(dbPath);
    
    if (!m_database.open()) {
        qCritical() << "Failed to open database:" << m_database.lastError().text();
        return false;
    }
    
    qDebug() << "Database opened successfully at:" << dbPath;
    
    return createTables();
}

void DatabaseManager::closeDatabase()
{
    if (m_database.isOpen()) {
        m_database.close();
    }
}

bool DatabaseManager::createTables()
{
    QSqlQuery query(m_database);
    
    // Create bowlers table
    QString createBowlersTable = R"(
        CREATE TABLE IF NOT EXISTS bowlers (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            first_name TEXT NOT NULL,
            last_name TEXT NOT NULL,
            sex TEXT DEFAULT 'Male',
            avg INTEGER DEFAULT 0,
            address TEXT,
            phone TEXT,
            birthday TEXT,
            over_18 INTEGER DEFAULT 1,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createBowlersTable)) {
        qCritical() << "Failed to create bowlers table:" << query.lastError().text();
        return false;
    }
    
    // Create teams table
    QString createTeamsTable = R"(
        CREATE TABLE IF NOT EXISTS teams (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT UNIQUE NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createTeamsTable)) {
        qCritical() << "Failed to create teams table:" << query.lastError().text();
        return false;
    }
    
    // Create team_bowlers junction table
    QString createTeamBowlersTable = R"(
        CREATE TABLE IF NOT EXISTS team_bowlers (
            team_id INTEGER,
            bowler_id INTEGER,
            PRIMARY KEY (team_id, bowler_id),
            FOREIGN KEY (team_id) REFERENCES teams(id) ON DELETE CASCADE,
            FOREIGN KEY (bowler_id) REFERENCES bowlers(id) ON DELETE CASCADE
        )
    )";
    
    if (!query.exec(createTeamBowlersTable)) {
        qCritical() << "Failed to create team_bowlers table:" << query.lastError().text();
        return false;
    }
    
    // Create leagues table
    QString createLeaguesTable = R"(
        CREATE TABLE IF NOT EXISTS leagues (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            name TEXT UNIQUE NOT NULL,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
        )
    )";
    
    if (!query.exec(createLeaguesTable)) {
        qCritical() << "Failed to create leagues table:" << query.lastError().text();
        return false;
    }
    
    // Create league_teams junction table
    QString createLeagueTeamsTable = R"(
        CREATE TABLE IF NOT EXISTS league_teams (
            league_id INTEGER,
            team_id INTEGER,
            PRIMARY KEY (league_id, team_id),
            FOREIGN KEY (league_id) REFERENCES leagues(id) ON DELETE CASCADE,
            FOREIGN KEY (team_id) REFERENCES teams(id) ON DELETE CASCADE
        )
    )";
    
    if (!query.exec(createLeagueTeamsTable)) {
        qCritical() << "Failed to create league_teams table:" << query.lastError().text();
        return false;
    }
    
    // CREATE CALENDAR EVENTS TABLE - This was missing!
    if (!createCalendarEventsTable()) {
        return false;
    }
    
    qDebug() << "All database tables created successfully";
    return true;
}
QVector<BowlerData> DatabaseManager::getAllBowlers(const QString &searchFilter)
{
    QVector<BowlerData> bowlers;
    QSqlQuery query(m_database);
    
    QString sql = "SELECT id, first_name, last_name, sex, avg, address, phone, birthday, over_18, created_at FROM bowlers";
    
    if (!searchFilter.isEmpty()) {
        sql += " WHERE LOWER(first_name) LIKE ? OR LOWER(last_name) LIKE ?";
    }
    
    sql += " ORDER BY last_name, first_name";
    
    query.prepare(sql);
    
    if (!searchFilter.isEmpty()) {
        QString filter = "%" + searchFilter.toLower() + "%";
        query.addBindValue(filter);
        query.addBindValue(filter);
    }
    
    if (!query.exec()) {
        qCritical() << "Failed to get bowlers:" << query.lastError().text();
        return bowlers;
    }
    
    while (query.next()) {
        BowlerData bowler;
        bowler.id = query.value("id").toInt();
        bowler.firstName = query.value("first_name").toString();
        bowler.lastName = query.value("last_name").toString();
        bowler.sex = query.value("sex").toString();
        bowler.average = query.value("avg").toInt();
        bowler.address = query.value("address").toString();
        bowler.phone = query.value("phone").toString();
        bowler.birthday = query.value("birthday").toString();
        bowler.over18 = query.value("over_18").toBool();
        bowler.createdAt = query.value("created_at").toString();
        
        // Get teams for this bowler
        bowler.teams = getBowlerTeams(bowler.id);
        
        bowlers.append(bowler);
    }
    
    return bowlers;
}

BowlerData DatabaseManager::getBowlerById(int id)
{
    BowlerData bowler;
    QSqlQuery query(m_database);
    
    query.prepare("SELECT id, first_name, last_name, sex, avg, address, phone, birthday, over_18, created_at FROM bowlers WHERE id = ?");
    query.addBindValue(id);
    
    if (!query.exec()) {
        qCritical() << "Failed to get bowler by ID:" << query.lastError().text();
        return bowler;
    }
    
    if (query.next()) {
        bowler.id = query.value("id").toInt();
        bowler.firstName = query.value("first_name").toString();
        bowler.lastName = query.value("last_name").toString();
        bowler.sex = query.value("sex").toString();
        bowler.average = query.value("avg").toInt();
        bowler.address = query.value("address").toString();
        bowler.phone = query.value("phone").toString();
        bowler.birthday = query.value("birthday").toString();
        bowler.over18 = query.value("over_18").toBool();
        bowler.createdAt = query.value("created_at").toString();
        
        // Get teams for this bowler
        bowler.teams = getBowlerTeams(bowler.id);
    }
    
    return bowler;
}

int DatabaseManager::addBowler(const BowlerData &bowler)
{
    QSqlQuery query(m_database);
    
    query.prepare(R"(
        INSERT INTO bowlers (first_name, last_name, sex, avg, address, phone, birthday, over_18)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?)
    )");
    
    query.addBindValue(bowler.firstName);
    query.addBindValue(bowler.lastName);
    query.addBindValue(bowler.sex);
    query.addBindValue(bowler.average);
    query.addBindValue(bowler.address);
    query.addBindValue(bowler.phone);
    query.addBindValue(bowler.birthday);
    query.addBindValue(bowler.over18 ? 1 : 0);
    
    if (!query.exec()) {
        qCritical() << "Failed to add bowler:" << query.lastError().text();
        return -1;
    }
    
    int newId = query.lastInsertId().toInt();
    qDebug() << "Added bowler with ID:" << newId;
    return newId;
}

bool DatabaseManager::updateBowler(const BowlerData &bowler)
{
    QSqlQuery query(m_database);
    
    query.prepare(R"(
        UPDATE bowlers 
        SET first_name = ?, last_name = ?, sex = ?, avg = ?, address = ?, phone = ?, birthday = ?, over_18 = ?
        WHERE id = ?
    )");
    
    query.addBindValue(bowler.firstName);
    query.addBindValue(bowler.lastName);
    query.addBindValue(bowler.sex);
    query.addBindValue(bowler.average);
    query.addBindValue(bowler.address);
    query.addBindValue(bowler.phone);
    query.addBindValue(bowler.birthday);
    query.addBindValue(bowler.over18 ? 1 : 0);
    query.addBindValue(bowler.id);
    
    if (!query.exec()) {
        qCritical() << "Failed to update bowler:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "Updated bowler with ID:" << bowler.id;
    return true;
}

bool DatabaseManager::deleteBowler(int id)
{
    QSqlQuery query(m_database);
    
    // First remove from all teams
    query.prepare("DELETE FROM team_bowlers WHERE bowler_id = ?");
    query.addBindValue(id);
    
    if (!query.exec()) {
        qCritical() << "Failed to remove bowler from teams:" << query.lastError().text();
        return false;
    }
    
    // Then delete the bowler
    query.prepare("DELETE FROM bowlers WHERE id = ?");
    query.addBindValue(id);
    
    if (!query.exec()) {
        qCritical() << "Failed to delete bowler:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "Deleted bowler with ID:" << id;
    return true;
}

QStringList DatabaseManager::getBowlerTeams(int bowlerId)
{
    QStringList teams;
    QSqlQuery query(m_database);
    
    query.prepare(R"(
        SELECT t.name 
        FROM teams t
        JOIN team_bowlers tb ON t.id = tb.team_id
        WHERE tb.bowler_id = ?
        ORDER BY t.name
    )");
    
    query.addBindValue(bowlerId);
    
    if (!query.exec()) {
        qCritical() << "Failed to get bowler teams:" << query.lastError().text();
        return teams;
    }
    
    while (query.next()) {
        teams.append(query.value("name").toString());
    }
    
    return teams;
}

QVector<TeamData> DatabaseManager::getAllTeams()
{
    QVector<TeamData> teams;
    QSqlQuery query(m_database);
    
    if (!query.exec("SELECT id, name, created_at FROM teams ORDER BY name")) {
        qCritical() << "Failed to get teams:" << query.lastError().text();
        return teams;
    }
    
    while (query.next()) {
        TeamData team;
        team.id = query.value("id").toInt();
        team.name = query.value("name").toString();
        team.createdAt = query.value("created_at").toString();
        
        // Get bowler count for this team
        QSqlQuery countQuery(m_database);
        countQuery.prepare("SELECT COUNT(*) FROM team_bowlers WHERE team_id = ?");
        countQuery.addBindValue(team.id);
        
        if (countQuery.exec() && countQuery.next()) {
            team.bowlerCount = countQuery.value(0).toInt();
        }
        
        teams.append(team);
    }
    
    return teams;
}

TeamData DatabaseManager::getTeamById(int id)
{
    TeamData team;
    QSqlQuery query(m_database);
    
    query.prepare("SELECT id, name, created_at FROM teams WHERE id = ?");
    query.addBindValue(id);
    
    if (!query.exec()) {
        qCritical() << "Failed to get team by ID:" << query.lastError().text();
        return team;
    }
    
    if (query.next()) {
        team.id = query.value("id").toInt();
        team.name = query.value("name").toString();
        team.createdAt = query.value("created_at").toString();
    }
    
    return team;
}

int DatabaseManager::addTeam(const QString &name)
{
    QSqlQuery query(m_database);
    
    query.prepare("INSERT INTO teams (name) VALUES (?)");
    query.addBindValue(name);
    
    if (!query.exec()) {
        qCritical() << "Failed to add team:" << query.lastError().text();
        return -1;
    }
    
    int newId = query.lastInsertId().toInt();
    qDebug() << "Added team with ID:" << newId;
    return newId;
}

bool DatabaseManager::updateTeam(const TeamData &team)
{
    QSqlQuery query(m_database);
    
    query.prepare("UPDATE teams SET name = ? WHERE id = ?");
    query.addBindValue(team.name);
    query.addBindValue(team.id);
    
    if (!query.exec()) {
        qCritical() << "Failed to update team:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "Updated team with ID:" << team.id;
    return true;
}

bool DatabaseManager::deleteTeam(int id)
{
    QSqlQuery query(m_database);
    
    // Team_bowlers entries will be automatically deleted due to CASCADE
    query.prepare("DELETE FROM teams WHERE id = ?");
    query.addBindValue(id);
    
    if (!query.exec()) {
        qCritical() << "Failed to delete team:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "Deleted team with ID:" << id;
    return true;
}

QVector<LeagueData> DatabaseManager::getAllLeagues()
{
    QVector<LeagueData> leagues;
    QSqlQuery query(m_database);
    
    if (!query.exec("SELECT id, name, created_at FROM leagues ORDER BY name")) {
        qCritical() << "Failed to get leagues:" << query.lastError().text();
        return leagues;
    }
    
    while (query.next()) {
        LeagueData league;
        league.id = query.value("id").toInt();
        league.name = query.value("name").toString();
        league.createdAt = query.value("created_at").toString();
        
        // Get team count for this league
        QSqlQuery countQuery(m_database);
        countQuery.prepare("SELECT COUNT(*) FROM league_teams WHERE league_id = ?");
        countQuery.addBindValue(league.id);
        
        if (countQuery.exec() && countQuery.next()) {
            league.teamCount = countQuery.value(0).toInt();
        }
        
        leagues.append(league);
    }
    
    return leagues;
}

LeagueData DatabaseManager::getLeagueById(int id)
{
    LeagueData league;
    QSqlQuery query(m_database);
    
    query.prepare("SELECT id, name, created_at FROM leagues WHERE id = ?");
    query.addBindValue(id);
    
    if (!query.exec()) {
        qCritical() << "Failed to get league by ID:" << query.lastError().text();
        return league;
    }
    
    if (query.next()) {
        league.id = query.value("id").toInt();
        league.name = query.value("name").toString();
        league.createdAt = query.value("created_at").toString();
    }
    
    return league;
}

int DatabaseManager::addLeague(const QString &name)
{
    QSqlQuery query(m_database);
    
    query.prepare("INSERT INTO leagues (name) VALUES (?)");
    query.addBindValue(name);
    
    if (!query.exec()) {
        qCritical() << "Failed to add league:" << query.lastError().text();
        return -1;
    }
    
    int newId = query.lastInsertId().toInt();
    qDebug() << "Added league with ID:" << newId;
    return newId;
}

bool DatabaseManager::updateLeague(const LeagueData &league)
{
    QSqlQuery query(m_database);
    
    query.prepare("UPDATE leagues SET name = ? WHERE id = ?");
    query.addBindValue(league.name);
    query.addBindValue(league.id);
    
    if (!query.exec()) {
        qCritical() << "Failed to update league:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "Updated league with ID:" << league.id;
    return true;
}

bool DatabaseManager::deleteLeague(int id)
{
    QSqlQuery query(m_database);
    
    // League_teams entries will be automatically deleted due to CASCADE
    query.prepare("DELETE FROM leagues WHERE id = ?");
    query.addBindValue(id);
    
    if (!query.exec()) {
        qCritical() << "Failed to delete league:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "Deleted league with ID:" << id;
    return true;
}


bool DatabaseManager::createCalendarEventsTable()
{
    QSqlQuery query(m_database);
    
    QString createCalendarEventsTable = R"(
        CREATE TABLE IF NOT EXISTS calendar_events (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            date TEXT NOT NULL,
            start_time TEXT NOT NULL,
            end_time TEXT NOT NULL,
            lane_id INTEGER NOT NULL,
            event_type TEXT NOT NULL DEFAULT 'Open Bowling',
            title TEXT NOT NULL,
            description TEXT,
            contact_name TEXT NOT NULL,
            contact_phone TEXT NOT NULL,
            contact_email TEXT,
            bowler_count INTEGER DEFAULT 1,
            additional_details TEXT,
            league_id INTEGER DEFAULT 0,
            team_id INTEGER DEFAULT 0,
            created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            FOREIGN KEY (league_id) REFERENCES leagues(id) ON DELETE SET NULL,
            FOREIGN KEY (team_id) REFERENCES teams(id) ON DELETE SET NULL
        )
    )";
    
    if (!query.exec(createCalendarEventsTable)) {
        qCritical() << "Failed to create calendar_events table:" << query.lastError().text();
        return false;
    }
    
    // Create indexes for better performance
    query.exec("CREATE INDEX IF NOT EXISTS idx_calendar_events_date ON calendar_events(date)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_calendar_events_lane_date ON calendar_events(lane_id, date)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_calendar_events_date_time ON calendar_events(date, start_time, end_time)");
    
    qDebug() << "Calendar events table created successfully";
    return true;
}

QVector<CalendarEventData> DatabaseManager::getAllCalendarEvents()
{
    QVector<CalendarEventData> events;
    QSqlQuery query(m_database);
    
    QString sql = R"(
        SELECT id, date, start_time, end_time, lane_id, event_type, title, description,
               contact_name, contact_phone, contact_email, bowler_count, additional_details,
               league_id, team_id, created_at, updated_at
        FROM calendar_events
        ORDER BY date, start_time
    )";
    
    if (!query.exec(sql)) {
        qCritical() << "Failed to get all calendar events:" << query.lastError().text();
        return events;
    }
    
    while (query.next()) {
        CalendarEventData event;
        event.id = query.value("id").toInt();
        event.date = QDate::fromString(query.value("date").toString(), Qt::ISODate);
        event.startTime = QTime::fromString(query.value("start_time").toString(), "hh:mm:ss");
        event.endTime = QTime::fromString(query.value("end_time").toString(), "hh:mm:ss");
        event.laneId = query.value("lane_id").toInt();
        event.eventType = query.value("event_type").toString();
        event.title = query.value("title").toString();
        event.description = query.value("description").toString();
        event.contactName = query.value("contact_name").toString();
        event.contactPhone = query.value("contact_phone").toString();
        event.contactEmail = query.value("contact_email").toString();
        event.bowlerCount = query.value("bowler_count").toInt();
        event.additionalDetails = query.value("additional_details").toString();
        event.leagueId = query.value("league_id").toInt();
        event.teamId = query.value("team_id").toInt();
        event.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
        event.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
        
        events.append(event);
    }
    
    return events;
}

QVector<CalendarEventData> DatabaseManager::getCalendarEventsForDate(const QDate &date)
{
    QVector<CalendarEventData> events;
    QSqlQuery query(m_database);
    
    QString sql = R"(
        SELECT id, date, start_time, end_time, lane_id, event_type, title, description,
               contact_name, contact_phone, contact_email, bowler_count, additional_details,
               league_id, team_id, created_at, updated_at
        FROM calendar_events
        WHERE date = ?
        ORDER BY start_time, lane_id
    )";
    
    query.prepare(sql);
    query.addBindValue(date.toString(Qt::ISODate));
    
    if (!query.exec()) {
        qCritical() << "Failed to get calendar events for date:" << query.lastError().text();
        return events;
    }
    
    while (query.next()) {
        CalendarEventData event;
        event.id = query.value("id").toInt();
        event.date = QDate::fromString(query.value("date").toString(), Qt::ISODate);
        event.startTime = QTime::fromString(query.value("start_time").toString(), "hh:mm:ss");
        event.endTime = QTime::fromString(query.value("end_time").toString(), "hh:mm:ss");
        event.laneId = query.value("lane_id").toInt();
        event.eventType = query.value("event_type").toString();
        event.title = query.value("title").toString();
        event.description = query.value("description").toString();
        event.contactName = query.value("contact_name").toString();
        event.contactPhone = query.value("contact_phone").toString();
        event.contactEmail = query.value("contact_email").toString();
        event.bowlerCount = query.value("bowler_count").toInt();
        event.additionalDetails = query.value("additional_details").toString();
        event.leagueId = query.value("league_id").toInt();
        event.teamId = query.value("team_id").toInt();
        event.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
        event.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
        
        events.append(event);
    }
    
    return events;
}

QVector<CalendarEventData> DatabaseManager::getCalendarEventsForMonth(int year, int month)
{
    QVector<CalendarEventData> events;
    QSqlQuery query(m_database);
    
    QDate startDate(year, month, 1);
    QDate endDate = startDate.addMonths(1).addDays(-1);
    
    QString sql = R"(
        SELECT id, date, start_time, end_time, lane_id, event_type, title, description,
               contact_name, contact_phone, contact_email, bowler_count, additional_details,
               league_id, team_id, created_at, updated_at
        FROM calendar_events
        WHERE date >= ? AND date <= ?
        ORDER BY date, start_time, lane_id
    )";
    
    query.prepare(sql);
    query.addBindValue(startDate.toString(Qt::ISODate));
    query.addBindValue(endDate.toString(Qt::ISODate));
    
    if (!query.exec()) {
        qCritical() << "Failed to get calendar events for month:" << query.lastError().text();
        return events;
    }
    
    while (query.next()) {
        CalendarEventData event;
        event.id = query.value("id").toInt();
        event.date = QDate::fromString(query.value("date").toString(), Qt::ISODate);
        event.startTime = QTime::fromString(query.value("start_time").toString(), "hh:mm:ss");
        event.endTime = QTime::fromString(query.value("end_time").toString(), "hh:mm:ss");
        event.laneId = query.value("lane_id").toInt();
        event.eventType = query.value("event_type").toString();
        event.title = query.value("title").toString();
        event.description = query.value("description").toString();
        event.contactName = query.value("contact_name").toString();
        event.contactPhone = query.value("contact_phone").toString();
        event.contactEmail = query.value("contact_email").toString();
        event.bowlerCount = query.value("bowler_count").toInt();
        event.additionalDetails = query.value("additional_details").toString();
        event.leagueId = query.value("league_id").toInt();
        event.teamId = query.value("team_id").toInt();
        event.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
        event.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
        
        events.append(event);
    }
    
    return events;
}

CalendarEventData DatabaseManager::getCalendarEventById(int id)
{
    CalendarEventData event;
    QSqlQuery query(m_database);
    
    QString sql = R"(
        SELECT id, date, start_time, end_time, lane_id, event_type, title, description,
               contact_name, contact_phone, contact_email, bowler_count, additional_details,
               league_id, team_id, created_at, updated_at
        FROM calendar_events
        WHERE id = ?
    )";
    
    query.prepare(sql);
    query.addBindValue(id);
    
    if (!query.exec()) {
        qCritical() << "Failed to get calendar event by ID:" << query.lastError().text();
        return event;
    }
    
    if (query.next()) {
        event.id = query.value("id").toInt();
        event.date = QDate::fromString(query.value("date").toString(), Qt::ISODate);
        event.startTime = QTime::fromString(query.value("start_time").toString(), "hh:mm:ss");
        event.endTime = QTime::fromString(query.value("end_time").toString(), "hh:mm:ss");
        event.laneId = query.value("lane_id").toInt();
        event.eventType = query.value("event_type").toString();
        event.title = query.value("title").toString();
        event.description = query.value("description").toString();
        event.contactName = query.value("contact_name").toString();
        event.contactPhone = query.value("contact_phone").toString();
        event.contactEmail = query.value("contact_email").toString();
        event.bowlerCount = query.value("bowler_count").toInt();
        event.additionalDetails = query.value("additional_details").toString();
        event.leagueId = query.value("league_id").toInt();
        event.teamId = query.value("team_id").toInt();
        event.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
        event.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
    }
    
    return event;
}

int DatabaseManager::addCalendarEvent(const CalendarEventData &event)
{
    QSqlQuery query(m_database);
    
    QString sql = R"(
        INSERT INTO calendar_events (
            date, start_time, end_time, lane_id, event_type, title, description,
            contact_name, contact_phone, contact_email, bowler_count, additional_details,
            league_id, team_id
        ) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
    )";
    
    query.prepare(sql);
    query.addBindValue(event.date.toString(Qt::ISODate));
    query.addBindValue(event.startTime.toString("hh:mm:ss"));
    query.addBindValue(event.endTime.toString("hh:mm:ss"));
    query.addBindValue(event.laneId);
    query.addBindValue(event.eventType);
    query.addBindValue(event.title);
    query.addBindValue(event.description);
    query.addBindValue(event.contactName);
    query.addBindValue(event.contactPhone);
    query.addBindValue(event.contactEmail);
    query.addBindValue(event.bowlerCount);
    query.addBindValue(event.additionalDetails);
    query.addBindValue(event.leagueId);
    query.addBindValue(event.teamId);
    
    if (!query.exec()) {
        qCritical() << "Failed to add calendar event:" << query.lastError().text();
        return -1;
    }
    
    int newId = query.lastInsertId().toInt();
    qDebug() << "Added calendar event with ID:" << newId;
    return newId;
}

bool DatabaseManager::updateCalendarEvent(const CalendarEventData &event)
{
    QSqlQuery query(m_database);
    
    QString sql = R"(
        UPDATE calendar_events SET
            date = ?, start_time = ?, end_time = ?, lane_id = ?, event_type = ?,
            title = ?, description = ?, contact_name = ?, contact_phone = ?,
            contact_email = ?, bowler_count = ?, additional_details = ?,
            league_id = ?, team_id = ?, updated_at = CURRENT_TIMESTAMP
        WHERE id = ?
    )";
    
    query.prepare(sql);
    query.addBindValue(event.date.toString(Qt::ISODate));
    query.addBindValue(event.startTime.toString("hh:mm:ss"));
    query.addBindValue(event.endTime.toString("hh:mm:ss"));
    query.addBindValue(event.laneId);
    query.addBindValue(event.eventType);
    query.addBindValue(event.title);
    query.addBindValue(event.description);
    query.addBindValue(event.contactName);
    query.addBindValue(event.contactPhone);
    query.addBindValue(event.contactEmail);
    query.addBindValue(event.bowlerCount);
    query.addBindValue(event.additionalDetails);
    query.addBindValue(event.leagueId);
    query.addBindValue(event.teamId);
    query.addBindValue(event.id);
    
    if (!query.exec()) {
        qCritical() << "Failed to update calendar event:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "Updated calendar event with ID:" << event.id;
    return true;
}

bool DatabaseManager::deleteCalendarEvent(int id)
{
    QSqlQuery query(m_database);
    
    query.prepare("DELETE FROM calendar_events WHERE id = ?");
    query.addBindValue(id);
    
    if (!query.exec()) {
        qCritical() << "Failed to delete calendar event:" << query.lastError().text();
        return false;
    }
    
    qDebug() << "Deleted calendar event with ID:" << id;
    return true;
}

bool DatabaseManager::isLaneAvailable(const QDate &date, const QTime &startTime, const QTime &endTime, 
                                     int laneId, int excludeEventId)
{
    QSqlQuery query(m_database);
    
    QString sql = R"(
        SELECT COUNT(*) FROM calendar_events
        WHERE date = ? AND lane_id = ? AND id != ?
        AND (
            (start_time < ? AND end_time > ?) OR
            (start_time < ? AND end_time > ?) OR
            (start_time >= ? AND end_time <= ?)
        )
    )";
    
    query.prepare(sql);
    query.addBindValue(date.toString(Qt::ISODate));
    query.addBindValue(laneId);
    query.addBindValue(excludeEventId);
    query.addBindValue(endTime.toString("hh:mm:ss"));
    query.addBindValue(startTime.toString("hh:mm:ss"));
    query.addBindValue(endTime.toString("hh:mm:ss"));
    query.addBindValue(startTime.toString("hh:mm:ss"));
    query.addBindValue(startTime.toString("hh:mm:ss"));
    query.addBindValue(endTime.toString("hh:mm:ss"));
    
    if (!query.exec()) {
        qCritical() << "Failed to check lane availability:" << query.lastError().text();
        return false;
    }
    
    if (query.next()) {
        int conflictCount = query.value(0).toInt();
        return conflictCount == 0;
    }
    
    return false;
}

QVector<CalendarEventData> DatabaseManager::getConflictingEvents(const QDate &date, const QTime &startTime, 
                                                                const QTime &endTime, int laneId, int excludeEventId)
{
    QVector<CalendarEventData> conflicts;
    QSqlQuery query(m_database);
    
    QString sql = R"(
        SELECT id, date, start_time, end_time, lane_id, event_type, title, description,
               contact_name, contact_phone, contact_email, bowler_count, additional_details,
               league_id, team_id, created_at, updated_at
        FROM calendar_events
        WHERE date = ? AND lane_id = ? AND id != ?
        AND (
            (start_time < ? AND end_time > ?) OR
            (start_time < ? AND end_time > ?) OR
            (start_time >= ? AND end_time <= ?)
        )
        ORDER BY start_time
    )";
    
    query.prepare(sql);
    query.addBindValue(date.toString(Qt::ISODate));
    query.addBindValue(laneId);
    query.addBindValue(excludeEventId);
    query.addBindValue(endTime.toString("hh:mm:ss"));
    query.addBindValue(startTime.toString("hh:mm:ss"));
    query.addBindValue(endTime.toString("hh:mm:ss"));
    query.addBindValue(startTime.toString("hh:mm:ss"));
    query.addBindValue(startTime.toString("hh:mm:ss"));
    query.addBindValue(endTime.toString("hh:mm:ss"));
    
    if (!query.exec()) {
        qCritical() << "Failed to get conflicting events:" << query.lastError().text();
        return conflicts;
    }
    
    while (query.next()) {
        CalendarEventData event;
        event.id = query.value("id").toInt();
        event.date = QDate::fromString(query.value("date").toString(), Qt::ISODate);
        event.startTime = QTime::fromString(query.value("start_time").toString(), "hh:mm:ss");
        event.endTime = QTime::fromString(query.value("end_time").toString(), "hh:mm:ss");
        event.laneId = query.value("lane_id").toInt();
        event.eventType = query.value("event_type").toString();
        event.title = query.value("title").toString();
        event.description = query.value("description").toString();
        event.contactName = query.value("contact_name").toString();
        event.contactPhone = query.value("contact_phone").toString();
        event.contactEmail = query.value("contact_email").toString();
        event.bowlerCount = query.value("bowler_count").toInt();
        event.additionalDetails = query.value("additional_details").toString();
        event.leagueId = query.value("league_id").toInt();
        event.teamId = query.value("team_id").toInt();
        event.createdAt = QDateTime::fromString(query.value("created_at").toString(), Qt::ISODate);
        event.updatedAt = QDateTime::fromString(query.value("updated_at").toString(), Qt::ISODate);
        
        conflicts.append(event);
    }
    
    return conflicts;
}

QVector<int> DatabaseManager::addLeagueSchedule(int leagueId, const QString &leagueName, 
                                               const QDate &startDate, const QTime &startTime,
                                               int durationMinutes, int frequencyDays, int numberOfWeeks,
                                               const QVector<int> &laneIds, const QString &contactName,
                                               const QString &contactPhone, const QString &contactEmail,
                                               const QString &additionalDetails)
{
    QVector<int> createdEventIds;
    QTime endTime = startTime.addSecs(durationMinutes * 60);
    
    m_database.transaction(); // Start transaction for all events
    
    try {
        for (int week = 0; week < numberOfWeeks; ++week) {
            QDate eventDate = startDate.addDays(week * frequencyDays);
            
            for (int laneId : laneIds) {
                // Check for conflicts
                if (!isLaneAvailable(eventDate, startTime, endTime, laneId)) {
                    qWarning() << "Conflict detected for" << leagueName << "on" << eventDate.toString() 
                              << "lane" << laneId << "- skipping";
                    continue;
                }
                
                // Create event
                CalendarEventData event;
                event.date = eventDate;
                event.startTime = startTime;
                event.endTime = endTime;
                event.laneId = laneId;
                event.eventType = "League";
                event.title = QString("%1 - Week %2 - Lane %3").arg(leagueName).arg(week + 1).arg(laneId);
                event.description = QString("Week %1 of %2").arg(week + 1).arg(numberOfWeeks);
                event.contactName = contactName;
                event.contactPhone = contactPhone;
                event.contactEmail = contactEmail;
                event.bowlerCount = 4; // Default for league
                event.additionalDetails = additionalDetails;
                event.leagueId = leagueId;
                event.teamId = 0;
                
                int eventId = addCalendarEvent(event);
                if (eventId > 0) {
                    createdEventIds.append(eventId);
                }
            }
        }
        
        m_database.commit(); // Commit all events
        qDebug() << "Successfully created" << createdEventIds.size() << "league events for" << leagueName;
        
    } catch (const std::exception &e) {
        m_database.rollback(); // Rollback on error
        qCritical() << "Error creating league schedule:" << e.what();
        createdEventIds.clear();
    }
    
    return createdEventIds;
}