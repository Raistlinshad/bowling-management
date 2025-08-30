#ifndef ENDOFDAYDIALOG_H
#define ENDOFDAYDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QTextEdit>
#include <QGroupBox>
#include <QCheckBox>
#include <QTimer>
#include <QJsonObject>
#include <QVector>

class LaneServer;
class DatabaseManager;

struct EndOfDayStats {
    int totalGamesPlayed = 0;
    int quickGames = 0;
    int leagueGames = 0;
    int totalBowlers = 0;
    double averageScore = 0.0;
    int highGame = 0;
    QString highGameBowler;
    int totalRevenue = 0;
    QVector<int> activeLanes;
    QMap<int, int> gamesPerLane;
};

class EndOfDayDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EndOfDayDialog(LaneServer *laneServer, QWidget *parent = nullptr);

private slots:
    void onGenerateReportClicked();
    void onShutdownAllLanesClicked();
    void onBackupDatabaseClicked();
    void onCloseSystemClicked();
    void onCancelClicked();
    void onLaneShutdownComplete(int laneId);
    void updateProgress();

private:
    void setupUI();
    void generateDailyReport();
    void shutdownAllLanes();
    void backupDatabase();
    void closeSystem();
    EndOfDayStats calculateDailyStats();
    QString formatReportText(const EndOfDayStats &stats);
    
    LaneServer *m_laneServer;
    DatabaseManager *m_dbManager;
    
    // UI Components
    QVBoxLayout *m_mainLayout;
    
    // Statistics section
    QGroupBox *m_statsGroup;
    QTextEdit *m_reportText;
    QPushButton *m_generateReportBtn;
    
    // Shutdown section
    QGroupBox *m_shutdownGroup;
    QCheckBox *m_shutdownLanesCheck;
    QCheckBox *m_backupDbCheck;
    QCheckBox *m_closeSystemCheck;
    QProgressBar *m_progressBar;
    QLabel *m_progressLabel;
    
    // Action buttons
    QFrame *m_buttonFrame;
    QPushButton *m_shutdownAllBtn;
    QPushButton *m_backupBtn;
    QPushButton *m_closeSystemBtn;
    QPushButton *m_cancelBtn;
    
    // State tracking
    QVector<int> m_pendingShutdowns;
    QTimer *m_progressTimer;
    int m_totalShutdownSteps;
    int m_completedSteps;
    bool m_shutdownInProgress;
};

// Implementation
EndOfDayDialog::EndOfDayDialog(LaneServer *laneServer, QWidget *parent)
    : QDialog(parent)
    , m_laneServer(laneServer)
    , m_dbManager(DatabaseManager::instance())
    , m_progressTimer(new QTimer(this))
    , m_totalShutdownSteps(0)
    , m_completedSteps(0)
    , m_shutdownInProgress(false)
{
    setupUI();
    setWindowTitle("End of Day Procedures");
    setModal(true);
    resize(700, 600);
    
    // Center on screen
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // Generate initial report
    generateDailyReport();
    
    connect(m_progressTimer, &QTimer::timeout, this, &EndOfDayDialog::updateProgress);
}

void EndOfDayDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(15);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    
    // Title
    QLabel *titleLabel = new QLabel("END OF DAY PROCEDURES");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel { "
                             "color: #4A90E2; "
                             "font-size: 20px; "
                             "font-weight: bold; "
                             "margin-bottom: 20px; "
                             "}");
    m_mainLayout->addWidget(titleLabel);
    
    // Daily statistics section
    m_statsGroup = new QGroupBox("Daily Report");
    m_statsGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; }");
    
    QVBoxLayout *statsLayout = new QVBoxLayout(m_statsGroup);
    
    m_reportText = new QTextEdit;
    m_reportText->setStyleSheet("QTextEdit { "
                               "background-color: #1a1a1a; "
                               "color: white; "
                               "border: 1px solid #555; "
                               "padding: 10px; "
                               "font-family: 'Courier New', monospace; "
                               "font-size: 11px; "
                               "}");
    m_reportText->setReadOnly(true);
    m_reportText->setFixedHeight(200);
    
    m_generateReportBtn = new QPushButton("Refresh Report");
    m_generateReportBtn->setStyleSheet("QPushButton { "
                                      "background-color: #2563EB; "
                                      "color: white; "
                                      "border: none; "
                                      "padding: 8px 16px; "
                                      "border-radius: 4px; "
                                      "}");
    connect(m_generateReportBtn, &QPushButton::clicked, this, &EndOfDayDialog::onGenerateReportClicked);
    
    statsLayout->addWidget(m_reportText);
    statsLayout->addWidget(m_generateReportBtn);
    
    m_mainLayout->addWidget(m_statsGroup);
    
    // Shutdown procedures section
    m_shutdownGroup = new QGroupBox("Shutdown Procedures");
    m_shutdownGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; }");
    
    QVBoxLayout *shutdownLayout = new QVBoxLayout(m_shutdownGroup);
    
    // Checkboxes for shutdown steps
    m_shutdownLanesCheck = new QCheckBox("Shutdown all active lanes");
    m_shutdownLanesCheck->setStyleSheet("QCheckBox { color: white; }");
    m_shutdownLanesCheck->setChecked(true);
    
    m_backupDbCheck = new QCheckBox("Backup database");
    m_backupDbCheck->setStyleSheet("QCheckBox { color: white; }");
    m_backupDbCheck->setChecked(true);
    
    m_closeSystemCheck = new QCheckBox("Close management system");
    m_closeSystemCheck->setStyleSheet("QCheckBox { color: white; }");
    
    shutdownLayout->addWidget(m_shutdownLanesCheck);
    shutdownLayout->addWidget(m_backupDbCheck);
    shutdownLayout->addWidget(m_closeSystemCheck);
    
    // Progress tracking
    m_progressBar = new QProgressBar;
    m_progressBar->setStyleSheet("QProgressBar { "
                                "border: 1px solid #555; "
                                "background-color: #2a2a2a; "
                                "color: white; "
                                "text-align: center; "
                                "} "
                                "QProgressBar::chunk { "
                                "background-color: #4A90E2; "
                                "}");
    m_progressBar->setVisible(false);
    
    m_progressLabel = new QLabel("Ready to begin shutdown procedures");
    m_progressLabel->setStyleSheet("QLabel { color: white; font-size: 12px; }");
    m_progressLabel->setAlignment(Qt::AlignCenter);
    
    shutdownLayout->addWidget(m_progressBar);
    shutdownLayout->addWidget(m_progressLabel);
    
    m_mainLayout->addWidget(m_shutdownGroup);
    
    // Action buttons
    m_buttonFrame = new QFrame;
    QHBoxLayout *buttonLayout = new QHBoxLayout(m_buttonFrame);
    
    m_shutdownAllBtn = new QPushButton("Shutdown All Lanes");
    m_shutdownAllBtn->setStyleSheet("QPushButton { "
                                   "background-color: #DC2626; "
                                   "color: white; "
                                   "border: none; "
                                   "padding: 10px 20px; "
                                   "border-radius: 5px; "
                                   "font-weight: bold; "
                                   "}");
    connect(m_shutdownAllBtn, &QPushButton::clicked, this, &EndOfDayDialog::onShutdownAllLanesClicked);
    
    m_backupBtn = new QPushButton("Backup Database");
    m_backupBtn->setStyleSheet("QPushButton { "
                              "background-color: #059669; "
                              "color: white; "
                              "border: none; "
                              "padding: 10px 20px; "
                              "border-radius: 5px; "
                              "font-weight: bold; "
                              "}");
    connect(m_backupBtn, &QPushButton::clicked, this, &EndOfDayDialog::onBackupDatabaseClicked);
    
    m_closeSystemBtn = new QPushButton("Close System");
    m_closeSystemBtn->setStyleSheet("QPushButton { "
                                   "background-color: #7C2D12; "
                                   "color: white; "
                                   "border: none; "
                                   "padding: 10px 20px; "
                                   "border-radius: 5px; "
                                   "font-weight: bold; "
                                   "}");
    connect(m_closeSystemBtn, &QPushButton::clicked, this, &EndOfDayDialog::onCloseSystemClicked);
    
    m_cancelBtn = new QPushButton("Cancel");
    m_cancelBtn->setStyleSheet("QPushButton { "
                              "background-color: #6B7280; "
                              "color: white; "
                              "border: none; "
                              "padding: 10px 20px; "
                              "border-radius: 5px; "
                              "}");
    connect(m_cancelBtn, &QPushButton::clicked, this, &EndOfDayDialog::onCancelClicked);
    
    buttonLayout->addWidget(m_shutdownAllBtn);
    buttonLayout->addWidget(m_backupBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_closeSystemBtn);
    buttonLayout->addWidget(m_cancelBtn);
    
    m_mainLayout->addWidget(m_buttonFrame);
}

void EndOfDayDialog::generateDailyReport()
{
    EndOfDayStats stats = calculateDailyStats();
    QString reportText = formatReportText(stats);
    m_reportText->setPlainText(reportText);
}

EndOfDayStats EndOfDayDialog::calculateDailyStats()
{
    EndOfDayStats stats;
    
    // This would query the database for today's statistics
    // For now, using sample data structure
    
    QDate today = QDate::currentDate();
    
    // Query database for today's games
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) as total_games, "
                 "SUM(CASE WHEN game_type = 'quick_game' THEN 1 ELSE 0 END) as quick_games, "
                 "SUM(CASE WHEN game_type = 'league_game' THEN 1 ELSE 0 END) as league_games "
                 "FROM games WHERE DATE(created_at) = ?");
    query.addBindValue(today.toString("yyyy-MM-dd"));
    
    if (query.exec() && query.next()) {
        stats.totalGamesPlayed = query.value("total_games").toInt();
        stats.quickGames = query.value("quick_games").toInt();
        stats.leagueGames = query.value("league_games").toInt();
    }
    
    // Calculate other statistics
    // Sample data for demonstration
    stats.totalBowlers = 45;
    stats.averageScore = 142.5;
    stats.highGame = 287;
    stats.highGameBowler = "John Smith";
    stats.totalRevenue = 1250; // Would calculate from pricing
    
    // Active lanes (get from lane server)
    for (int i = 1; i <= 8; ++i) {
        stats.activeLanes.append(i);
        stats.gamesPerLane[i] = 3 + (i % 4); // Sample data
    }
    
    return stats;
}

QString EndOfDayDialog::formatReportText(const EndOfDayStats &stats)
{
    QString report;
    QDate today = QDate::currentDate();
    QTime now = QTime::currentTime();
    
    report += "=====================================\n";
    report += "       CENTRE BOWLING DAILY REPORT\n";
    report += "=====================================\n";
    report += QString("Date: %1\n").arg(today.toString("dddd, MMMM dd, yyyy"));
    report += QString("Time: %1\n\n").arg(now.toString("hh:mm:ss"));
    
    report += "GAME STATISTICS:\n";
    report += "-------------------------------------\n";
    report += QString("Total Games Played: %1\n").arg(stats.totalGamesPlayed);
    report += QString("  - Quick Games: %1\n").arg(stats.quickGames);
    report += QString("  - League Games: %1\n").arg(stats.leagueGames);
    report += QString("Total Bowlers: %1\n").arg(stats.totalBowlers);
    report += QString("Average Score: %1\n").arg(stats.averageScore, 0, 'f', 1);
    report += QString("High Game: %1 (%2)\n\n").arg(stats.highGame).arg(stats.highGameBowler);
    
    report += "LANE UTILIZATION:\n";
    report += "-------------------------------------\n";
    for (int laneId : stats.activeLanes) {
        int gamesCount = stats.gamesPerLane.value(laneId, 0);
        report += QString("Lane %1: %2 games\n").arg(laneId).arg(gamesCount);
    }
    report += "\n";
    
    report += "REVENUE SUMMARY:\n";
    report += "-------------------------------------\n";
    report += QString("Estimated Revenue: $%1\n").arg(stats.totalRevenue);
    report += QString("Games Revenue: $%1\n").arg(stats.totalGamesPlayed * 15); // $15 per game estimate
    report += QString("Shoe Rentals: $%1\n").arg(stats.totalBowlers * 4); // $4 per rental estimate
    report += "\n";
    
    report += "=====================================\n";
    report += "Report generated at: " + QDateTime::currentDateTime().toString("hh:mm:ss") + "\n";
    
    return report;
}

void EndOfDayDialog::onGenerateReportClicked()
{
    generateDailyReport();
    QMessageBox::information(this, "Report Updated", "Daily report has been refreshed with current data.");
}

void EndOfDayDialog::onShutdownAllLanesClicked()
{
    if (m_shutdownInProgress) {
        QMessageBox::information(this, "Shutdown in Progress", "Lane shutdown is already in progress.");
        return;
    }
    
    int ret = QMessageBox::question(this, "Shutdown All Lanes",
                                   "Are you sure you want to shutdown all active lanes?\n\n"
                                   "This will end all current games and return lanes to advertising mode.",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        shutdownAllLanes();
    }
}

void EndOfDayDialog::onBackupDatabaseClicked()
{
    int ret = QMessageBox::question(this, "Backup Database",
                                   "Create a backup of the current database?\n\n"
                                   "This may take a few minutes depending on database size.",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        backupDatabase();
    }
}

void EndOfDayDialog::onCloseSystemClicked()
{
    int ret = QMessageBox::question(this, "Close System",
                                   "Are you sure you want to close the management system?\n\n"
                                   "This will:\n"
                                   "- Shutdown all lanes (if selected)\n"
                                   "- Backup database (if selected)\n"
                                   "- Close the application",
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        closeSystem();
    }
}

void EndOfDayDialog::onCancelClicked()
{
    if (m_shutdownInProgress) {
        QMessageBox::information(this, "Cannot Cancel", 
                               "Shutdown procedures are in progress and cannot be cancelled.");
        return;
    }
    
    reject();
}

void EndOfDayDialog::shutdownAllLanes()
{
    m_shutdownInProgress = true;
    m_completedSteps = 0;
    
    // Calculate total steps
    EndOfDayStats stats = calculateDailyStats();
    m_totalShutdownSteps = stats.activeLanes.size();
    
    if (m_totalShutdownSteps == 0) {
        QMessageBox::information(this, "No Active Lanes", "All lanes are already shutdown.");
        m_shutdownInProgress = false;
        return;
    }
    
    // Setup progress tracking
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, m_totalShutdownSteps);
    m_progressBar->setValue(0);
    m_progressLabel->setText("Shutting down lanes...");
    
    // Disable buttons during shutdown
    m_shutdownAllBtn->setEnabled(false);
    m_closeSystemBtn->setEnabled(false);
    
    // Start shutdown process
    m_pendingShutdowns = stats.activeLanes;
    
    // Connect to lane server signals for shutdown completion
    connect(m_laneServer, &LaneServer::laneShutdown,
            this, &EndOfDayDialog::onLaneShutdownComplete);
    
    // Send shutdown commands to all active lanes
    for (int laneId : stats.activeLanes) {
        QJsonObject shutdownData;
        shutdownData["lane_id"] = laneId;
        shutdownData["command"] = "shutdown";
        
        // Send via the lane server
        QTimer::singleShot(laneId * 500, [this, shutdownData]() {
            if (m_laneServer) {
                m_laneServer->onLaneCommand(shutdownData);
            }
        });
    }
    
    // Start progress update timer
    m_progressTimer->start(500);
}

void EndOfDayDialog::onLaneShutdownComplete(int laneId)
{
    m_pendingShutdowns.removeOne(laneId);
    m_completedSteps++;
    
    m_progressBar->setValue(m_completedSteps);
    m_progressLabel->setText(QString("Shutting down lanes... (%1/%2)")
                            .arg(m_completedSteps).arg(m_totalShutdownSteps));
    
    if (m_pendingShutdowns.isEmpty()) {
        // All lanes shutdown complete
        m_progressTimer->stop();
        m_progressLabel->setText("All lanes shutdown complete!");
        m_shutdownInProgress = false;
        
        // Re-enable buttons
        m_shutdownAllBtn->setEnabled(true);
        m_closeSystemBtn->setEnabled(true);
        
        QMessageBox::information(this, "Shutdown Complete", 
                               "All active lanes have been shutdown successfully.");
    }
}

void EndOfDayDialog::backupDatabase()
{
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // Indeterminate progress
    m_progressLabel->setText("Creating database backup...");
    
    // Disable backup button
    m_backupBtn->setEnabled(false);
    
    // Perform backup in background
    QTimer::singleShot(100, [this]() {
        QString backupPath = QString("backup_%1.db")
                            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
        
        bool success = m_dbManager->backupDatabase(backupPath);
        
        m_progressBar->setVisible(false);
        m_backupBtn->setEnabled(true);
        
        if (success) {
            m_progressLabel->setText(QString("Backup created: %1").arg(backupPath));
            QMessageBox::information(this, "Backup Complete", 
                                   QString("Database backup created successfully:\n%1").arg(backupPath));
        } else {
            m_progressLabel->setText("Backup failed!");
            QMessageBox::warning(this, "Backup Failed", 
                                "Failed to create database backup. Please check disk space and permissions.");
        }
    });
}

void EndOfDayDialog::closeSystem()
{
    QVector<std::function<void()>> shutdownTasks;
    
    // Add selected shutdown tasks
    if (m_shutdownLanesCheck->isChecked()) {
        shutdownTasks.append([this]() { shutdownAllLanes(); });
    }
    
    if (m_backupDbCheck->isChecked()) {
        shutdownTasks.append([this]() { backupDatabase(); });
    }
    
    if (shutdownTasks.isEmpty()) {
        // No tasks selected, just close
        if (m_closeSystemCheck->isChecked()) {
            QApplication::quit();
        } else {
            accept();
        }
        return;
    }
    
    // Execute tasks sequentially
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, shutdownTasks.size());
    m_progressBar->setValue(0);
    
    // Simplified sequential execution for demo
    m_progressLabel->setText("Executing shutdown procedures...");
    
    QTimer::singleShot(2000, [this]() {
        m_progressBar->setValue(1);
        m_progressLabel->setText("Shutdown procedures complete!");
        
        if (m_closeSystemCheck->isChecked()) {
            QTimer::singleShot(1000, []() {
                QApplication::quit();
            });
        } else {
            accept();
        }
    });
}

void EndOfDayDialog::updateProgress()
{
    // Update progress display during shutdown
    if (m_shutdownInProgress && !m_pendingShutdowns.isEmpty()) {
        // Show current shutdown status
        QString status = QString("Shutting down lanes: %1 remaining")
                        .arg(m_pendingShutdowns.size());
        m_progressLabel->setText(status);
    }
}

#endif // ENDOFDAYDIALOG_H