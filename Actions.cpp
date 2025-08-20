#include "Actions.h"
#include "MainWindow.h"
#include "QuickGameDialog.h"
#include "LeagueGameDialog.h"
#include "NewBowlerDialog.h"
#include "TeamManagementDialog.h"
#include "LeagueManagementDialog.h"
#include "DatabaseBrowserDialog.h"
#include "CalendarDialog.h"
#include <QMessageBox>
#include <QApplication>
#include <QDebug>

Actions::Actions(MainWindow *mainWindow, EventBus *eventBus, QObject *parent)
    : QObject(parent)
    , m_mainWindow(mainWindow)
    , m_eventBus(eventBus)
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
    QMessageBox::information(m_mainWindow, "Status", "Status display feature coming soon");
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
        // Get bowler info instead of JSON data
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
    QPushButton *cancelBtn = msgBox.addButton(QMessageBox::Cancel);
    
    msgBox.exec();
    
    if (msgBox.clickedButton() == addBowlerBtn) {
        showNewBowlerDialog();
    } else if (msgBox.clickedButton() == searchBowlerBtn) {
        QMessageBox::information(m_mainWindow, "Search Bowlers", "Bowler search feature coming soon");
    } else if (msgBox.clickedButton() == viewAllBtn) {
        QMessageBox::information(m_mainWindow, "View All Bowlers", "View all bowlers feature coming soon");
    }
}

void Actions::sendGameToLane(int laneId, const QString &gameType, const QJsonObject &gameData)
{
    QJsonObject commandData;
    commandData["lane_id"] = laneId;
    commandData["type"] = gameType;
    commandData["data"] = gameData;
    
    bool success = m_eventBus->publish("server", "lane_command", commandData);
    
    if (success) {
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