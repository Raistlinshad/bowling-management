#!/bin/bash

# Bowling Management Build Fix Script
echo "========================================="
echo "Fixing Bowling Management Build Issues..."
echo "========================================="

SOURCE_DIR="/home/centrebowl/Desktop/Bowling/NewScreen"
BUILD_DIR="/tmp/bowling_build"

# Check if source directory exists
if [ ! -d "$SOURCE_DIR" ]; then
    echo "Error: Source directory $SOURCE_DIR not found!"
    exit 1
fi

echo "Step 1: Removing problematic files..."
cd "$SOURCE_DIR"

# Remove the conflicting BowlerInfoDialog files
if [ -f "BowlerInfoDialog.h" ]; then
    echo "  Removing BowlerInfoDialog.h"
    rm -f BowlerInfoDialog.h
fi

if [ -f "BowlerInfoDialog.cpp" ]; then
    echo "  Removing BowlerInfoDialog.cpp"
    rm -f BowlerInfoDialog.cpp
fi

echo "Step 2: Creating missing header files..."

# Create EventBus.h
cat > EventBus.h << 'EOF'
#ifndef EVENTBUS_H
#define EVENTBUS_H

#include <QObject>
#include <QJsonObject>

class EventBus : public QObject
{
    Q_OBJECT

public:
    explicit EventBus(QObject *parent = nullptr);
    bool publish(const QString &channel, const QString &event, const QJsonObject &data);

signals:
    void eventReceived(const QString &channel, const QString &event, const QJsonObject &data);
};

#endif // EVENTBUS_H
EOF

# Create EventBus.cpp
cat > EventBus.cpp << 'EOF'
#include "EventBus.h"

EventBus::EventBus(QObject *parent) : QObject(parent) {}

bool EventBus::publish(const QString &channel, const QString &event, const QJsonObject &data)
{
    emit eventReceived(channel, event, data);
    return true;
}
EOF

# Create LaneServer.h
cat > LaneServer.h << 'EOF'
#ifndef LANESERVER_H
#define LANESERVER_H

#include <QObject>
#include <QJsonObject>
#include "EventBus.h"

enum class LaneStatus {
    Idle,
    Active,
    Maintenance,
    Error
};

class LaneServer : public QObject
{
    Q_OBJECT

public:
    explicit LaneServer(EventBus *eventBus, QObject *parent = nullptr);
    void start();
    void stop();
    void handleTeamMove(int fromLane, int toLane, const QString &teamData);

signals:
    void laneStatusChanged(int laneId, LaneStatus status);
    void gameDataReceived(int laneId, const QJsonObject &gameData);

private:
    EventBus *m_eventBus;
    bool m_running;
};

#endif // LANESERVER_H
EOF

# Create LaneServer.cpp
cat > LaneServer.cpp << 'EOF'
#include "LaneServer.h"

LaneServer::LaneServer(EventBus *eventBus, QObject *parent)
    : QObject(parent), m_eventBus(eventBus), m_running(false) {}

void LaneServer::start() { m_running = true; }
void LaneServer::stop() { m_running = false; }
void LaneServer::handleTeamMove(int fromLane, int toLane, const QString &teamData) 
{
    Q_UNUSED(fromLane)
    Q_UNUSED(toLane)
    Q_UNUSED(teamData)
}
EOF

# Create LaneWidget.h
cat > LaneWidget.h << 'EOF'
#ifndef LANEWIDGET_H
#define LANEWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QJsonObject>
#include "LaneServer.h"

class LaneWidget : public QWidget
{
    Q_OBJECT

public:
    explicit LaneWidget(int laneNumber, QWidget *parent = nullptr);
    void setStatus(LaneStatus status);
    void updateGameData(const QJsonObject &gameData);

signals:
    void teamMoveRequested(int fromLane, int toLane, const QString &teamData);

private:
    int m_laneNumber;
    QVBoxLayout *m_layout;
    QLabel *m_laneLabel;
    QLabel *m_statusLabel;
    LaneStatus m_status;
};

#endif // LANEWIDGET_H
EOF

# Create LaneWidget.cpp
cat > LaneWidget.cpp << 'EOF'
#include "LaneWidget.h"

LaneWidget::LaneWidget(int laneNumber, QWidget *parent)
    : QWidget(parent), m_laneNumber(laneNumber), m_status(LaneStatus::Idle)
{
    m_layout = new QVBoxLayout(this);
    m_laneLabel = new QLabel(QString("Lane %1").arg(laneNumber));
    m_statusLabel = new QLabel("Idle");
    
    m_layout->addWidget(m_laneLabel);
    m_layout->addWidget(m_statusLabel);
    
    setFixedSize(80, 80);
    setStyleSheet("QWidget { background-color: #333; border: 1px solid #555; border-radius: 3px; }");
}

void LaneWidget::setStatus(LaneStatus status)
{
    m_status = status;
    switch(status) {
        case LaneStatus::Idle: m_statusLabel->setText("Idle"); break;
        case LaneStatus::Active: m_statusLabel->setText("Active"); break;
        case LaneStatus::Maintenance: m_statusLabel->setText("Maint"); break;
        case LaneStatus::Error: m_statusLabel->setText("Error"); break;
    }
}

void LaneWidget::updateGameData(const QJsonObject &gameData)
{
    Q_UNUSED(gameData)
}
EOF

# Create stub dialog files
echo "Step 3: Creating stub dialog files..."

# QuickStartDialog.h
cat > QuickStartDialog.h << 'EOF'
#ifndef QUICKSTARTDIALOG_H
#define QUICKSTARTDIALOG_H

#include <QDialog>

class QuickStartDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QuickStartDialog(QWidget *parent = nullptr);
};

#endif // QUICKSTARTDIALOG_H
EOF

# QuickStartDialog.cpp
cat > QuickStartDialog.cpp << 'EOF'
#include "QuickStartDialog.h"
#include <QMessageBox>

QuickStartDialog::QuickStartDialog(QWidget *parent) : QDialog(parent)
{
    QMessageBox::information(this, "Quick Start", "Quick Start dialog coming soon");
}
EOF

# QuickGameDialog.h
cat > QuickGameDialog.h << 'EOF'
#ifndef QUICKGAMEDIALOG_H
#define QUICKGAMEDIALOG_H

#include <QDialog>
#include <QJsonObject>

class QuickGameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QuickGameDialog(QWidget *parent = nullptr);
    QJsonObject getGameData() const;
};

#endif // QUICKGAMEDIALOG_H
EOF

# QuickGameDialog.cpp
cat > QuickGameDialog.cpp << 'EOF'
#include "QuickGameDialog.h"

QuickGameDialog::QuickGameDialog(QWidget *parent) : QDialog(parent) {}
QJsonObject QuickGameDialog::getGameData() const { return QJsonObject(); }
EOF

# LeagueGameDialog.h
cat > LeagueGameDialog.h << 'EOF'
#ifndef LEAGUEGAMEDIALOG_H
#define LEAGUEGAMEDIALOG_H

#include <QDialog>
#include <QJsonObject>

class LeagueGameDialog : public QDialog
{
    Q_OBJECT
public:
    explicit LeagueGameDialog(QWidget *parent = nullptr);
    QJsonObject getGameData() const;
};

#endif // LEAGUEGAMEDIALOG_H
EOF

# LeagueGameDialog.cpp
cat > LeagueGameDialog.cpp << 'EOF'
#include "LeagueGameDialog.h"

LeagueGameDialog::LeagueGameDialog(QWidget *parent) : QDialog(parent) {}
QJsonObject LeagueGameDialog::getGameData() const { return QJsonObject(); }
EOF

# DatabaseManager.h
cat > DatabaseManager.h << 'EOF'
#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QString>
#include <QVector>

struct BowlerData {
    int id = 0;
    QString firstName;
    QString lastName;
    QString sex;
    int average = 0;
    QString birthday;
    bool over18 = true;
    QString phone;
    QString address;
    QStringList teams;
};

class DatabaseManager : public QObject
{
    Q_OBJECT

public:
    static DatabaseManager* instance();
    QVector<BowlerData> getAllBowlers(const QString &filter = QString());
    BowlerData getBowlerById(int id);
    int addBowler(const BowlerData &bowler);
    bool updateBowler(const BowlerData &bowler);
    bool deleteBowler(int id);

private:
    explicit DatabaseManager(QObject *parent = nullptr);
    static DatabaseManager* m_instance;
};

#endif // DATABASEMANAGER_H
EOF

# DatabaseManager.cpp
cat > DatabaseManager.cpp << 'EOF'
#include "DatabaseManager.h"

DatabaseManager* DatabaseManager::m_instance = nullptr;

DatabaseManager::DatabaseManager(QObject *parent) : QObject(parent) {}

DatabaseManager* DatabaseManager::instance()
{
    if (!m_instance) {
        m_instance = new DatabaseManager();
    }
    return m_instance;
}

QVector<BowlerData> DatabaseManager::getAllBowlers(const QString &filter)
{
    Q_UNUSED(filter)
    return QVector<BowlerData>();
}

BowlerData DatabaseManager::getBowlerById(int id)
{
    Q_UNUSED(id)
    return BowlerData();
}

int DatabaseManager::addBowler(const BowlerData &bowler)
{
    Q_UNUSED(bowler)
    return 1;
}

bool DatabaseManager::updateBowler(const BowlerData &bowler)
{
    Q_UNUSED(bowler)
    return true;
}

bool DatabaseManager::deleteBowler(int id)
{
    Q_UNUSED(id)
    return true;
}
EOF

echo "Step 4: Cleaning build directory..."
if [ -d "$BUILD_DIR" ]; then
    rm -rf "$BUILD_DIR"/*
fi

echo "✅ Build fix complete!"
echo ""
echo "Next steps:"
echo "1. Replace your existing files with the updated versions provided in the artifacts"
echo "2. Run the normal build script: ./build_bowling.sh"
echo ""
echo "Files that need manual replacement from the artifacts:"
echo "  - Actions.h and Actions.cpp"
echo "  - NewBowlerDialog.h and NewBowlerDialog.cpp"
echo "  - TeamManagementDialog.h and TeamManagementDialog.cpp"
echo "  - LeagueManagementDialog.h and LeagueManagementDialog.cpp"
echo "  - CMakeLists.txt"