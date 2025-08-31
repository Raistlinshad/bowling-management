#include "Actions.h"
#include "MainWindow.h"
#include "QuickGameDialog.h"
#include "LeagueGameDialog.h"
#include "NewBowlerDialog.h"
#include "TeamManagementDialog.h"
#include "LeagueManagementDialog.h"
#include "DatabaseBrowserDialog.h"
#include "CalendarDialog.h"
#include "EndOfDayDialog.h"
#include <QMessageBox>
#include <QApplication>
#include <QDebug>

Actions::Actions(MainWindow *mainWindow, QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
{
}

void Actions::quickStart()
{
    showQuickGameDialog();
}

void Actions::leagueStart()
{
    showLeagueGameDialog();
}

void Actions::newBowler()
{
    showNewBowlerDialog();
}

void Actions::homeScreen()
{
    // Clear main content area and show home screen
    QMessageBox::information(m_mainWindow, "Home", "Returning to home screen");
}

void Actions::bowlerInfo()
{
    showBowlerManagementDialog();
}

void Actions::calendar()
{
    CalendarDialog dialog(m_mainWindow, m_mainWindow);
    dialog.exec();
}

void Actions::teams()
{
    showTeamManagementDialog();
}

void Actions::leagues()
{
    showLeagueManagementDialog();
}

void Actions::party()
{
    QMessageBox::information(m_mainWindow, "Party", "Party mode feature coming soon");
}

void Actions::showStatus()
{
    // Show system status dialog
    QDialog statusDialog(m_mainWindow);
    statusDialog.setWindowTitle("System Status");
    statusDialog.setModal(true);
    statusDialog.resize(600, 400);
    
    QVBoxLayout *layout = new QVBoxLayout(&statusDialog);
    
    // System info
    QGroupBox *systemGroup = new QGroupBox("System Information");
    QVBoxLayout *systemLayout = new QVBoxLayout(systemGroup);
    
    QLabel *versionLabel = new QLabel("Centre Bowling Management v1.0");
    QLabel *uptimeLabel = new QLabel("System Uptime: " + getSystemUptime());
    QLabel *dbStatusLabel = new QLabel("Database: Connected");
    QLabel *serverStatusLabel = new QLabel("Lane Server: Running on port 50005");
    
    systemLayout->addWidget(versionLabel);
    systemLayout->addWidget(uptimeLabel);
    systemLayout->addWidget(dbStatusLabel);
    systemLayout->addWidget(serverStatusLabel);
    
    layout->addWidget(systemGroup);
    
    // Lane status
    QGroupBox *laneGroup = new QGroupBox("Lane Status");
    QGridLayout *laneLayout = new QGridLayout(laneGroup);
    
    // Sample lane status (would get from actual lane server)
    for (int i = 1; i <= 8; ++i) {
        QLabel *laneLabel = new QLabel(QString("Lane %1:").arg(i));
        QLabel *statusLabel = new QLabel("Connected");
        statusLabel->setStyleSheet("QLabel { color: green; }");
        
        laneLayout->addWidget(laneLabel, (i-1)/4, ((i-1)%4)*2);
        laneLayout->addWidget(statusLabel, (i-1)/4, ((i-1)%4)*2 + 1);
    }
    
    layout->addWidget(laneGroup);
    
    QPushButton *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, &statusDialog, &QDialog::accept);
    layout->addWidget(closeBtn);
    
    statusDialog.exec();
}

void Actions::settings()
{
    QMessageBox::information(m_mainWindow, "Settings", "Settings feature coming soon");
}

void Actions::testLaneConnection()
{
    QMessageBox::information(m_mainWindow, "Test Lane", "Lane connection test feature coming soon");
}

void Actions::runQuickGameDiagnostic()
{
    QMessageBox::information(m_mainWindow, "QG Diagnostic", "Quick game diagnostic feature coming soon");
}

void Actions::endOfDay()
{
    // Get lane server from main window
    LaneServer *laneServer = m_mainWindow->findChild<LaneServer*>();
    if (!laneServer) {
        QMessageBox::warning(m_mainWindow, "Error", "Lane server not available.");
        return;
    }
    
    EndOfDayDialog dialog(laneServer, m_mainWindow);
    int result = dialog.exec();
    
    if (result == QDialog::Accepted) {
        qDebug() << "End of day procedures completed";
    }
}

void Actions::showQuickGameDialog()
{
    QuickGameDialog dialog(m_mainWindow);
    if (dialog.exec() == QDialog::Accepted) {
        QJsonObject gameData = dialog.getGameData();
        int laneId = gameData["lane_id"].toInt();
        
        sendGameToLane(laneId, "quick_game", gameData);
    }
}

void Actions::showLeagueGameDialog()
{
    LeagueGameDialog dialog(m_mainWindow);
    if (dialog.exec() == QDialog::Accepted) {
        QJsonObject gameData = dialog.getGameData();
        int laneId = gameData["lane_id"].toInt();
        
        sendGameToLane(laneId, "league_game", gameData);
    }
}

void Actions::showNewBowlerDialog()
{
    NewBowlerDialog dialog(m_mainWindow);
    if (dialog.exec() == QDialog::Accepted) {
        BowlerInfo bowlerInfo = dialog.getBowlerInfo();
        
        QMessageBox::information(m_mainWindow, "Success", 
                               QString("Bowler '%1 %2' added successfully")
                               .arg(bowlerInfo.firstName).arg(bowlerInfo.lastName));
    }
}

void Actions::showTeamManagementDialog()
{
    TeamManagementDialog dialog(m_mainWindow);
    dialog.exec();
}

void Actions::showLeagueManagementDialog()
{
    LeagueManagementDialog dialog(m_mainWindow);
    dialog.exec();
}

void Actions::showBowlerManagementDialog()
{
    QMessageBox msgBox(m_mainWindow);
    msgBox.setWindowTitle("Bowler Management");
    msgBox.setText("What would you like to do?");
    msgBox.setStyleSheet("QMessageBox { background-color: #2a2a2a; color: white; }");
    
    QPushButton *addBowlerBtn = msgBox.addButton("Add New Bowler", QMessageBox::ActionRole);
    QPushButton *searchBowlerBtn = msgBox.addButton("Search Bowlers", QMessageBox::ActionRole);
    QPushButton *viewAllBtn = msgBox.addButton("View All Bowlers", QMessageBox::ActionRole);
    QPushButton *endDayBtn = msgBox.addButton("End of Day", QMessageBox::ActionRole);
    QPushButton *cancelBtn = msgBox.addButton(QMessageBox::Cancel);
    
    msgBox.exec();
    
    if (msgBox.clickedButton() == addBowlerBtn) {
        showNewBowlerDialog();
    } else if (msgBox.clickedButton() == searchBowlerBtn) {
        QMessageBox::information(m_mainWindow, "Search Bowlers", "Bowler search feature coming soon");
    } else if (msgBox.clickedButton() == viewAllBtn) {
        QMessageBox::information(m_mainWindow, "View All Bowlers", "View all bowlers feature coming soon");
    } else if (msgBox.clickedButton() == endDayBtn) {
        endOfDay();
    }
}

void Actions::sendGameToLane(int laneId, const QString &gameType, const QJsonObject &gameData)
{
    QJsonObject commandData;
    commandData["lane_id"] = laneId;
    commandData["type"] = gameType;
    commandData["data"] = gameData;
    
    // Get LaneServer from mainWindow and call directly
    LaneServer *laneServer = m_mainWindow->findChild<LaneServer*>();
    if (laneServer) {
        laneServer->onLaneCommand(commandData);
        QMessageBox::information(m_mainWindow, "Game Started", 
                               QString("Game started on lane %1").arg(laneId));
    } else {
        QMessageBox::warning(m_mainWindow, "Error", 
                           QString("Failed to start game on lane %1").arg(laneId));
    }
}

void Actions::onDatabaseBrowserClicked()
{
    DatabaseBrowserDialog *dialog = new DatabaseBrowserDialog(m_mainWindow);
    dialog->exec();
    dialog->deleteLater();
}

QString Actions::getSystemUptime() const
{
    // Calculate uptime since application start
    static QDateTime startTime = QDateTime::currentDateTime();
    QDateTime now = QDateTime::currentDateTime();
    qint64 seconds = startTime.secsTo(now);
    
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    
    return QString("%1h %2m %3s").arg(hours).arg(minutes).arg(secs);
}