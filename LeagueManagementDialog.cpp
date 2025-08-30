#include "LeagueManagementDialog.h"
#include "MainWindow.h"
#include <QDateTime>

LeagueManagementDialog::LeagueManagementDialog(MainWindow *mainWindow, QWidget *parent)
    : QDialog(parent)
    , m_mainWindow(mainWindow)
    , m_leagueManager(nullptr)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
{
    setupUI();
    setWindowTitle("League Management");
    setModal(true);
    resize(1200, 800);
    
    // Center the dialog
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // Initialize sample data with enhanced features
    initializeEnhancedLeagueData();
    
    // Load initial data
    updateLeaguesList();
}

void LeagueManagementDialog::initializeEnhancedLeagueData()
{
    // Create sample enhanced league data
    EnhancedLeagueInfo league1;
    league1.basicInfo.id = 1;
    league1.basicInfo.name = "Monday Night League";
    league1.basicInfo.teamCount = 8;
    league1.startDate = "2024-09-01";
    league1.endDate = "2024-12-15";
    league1.numberOfWeeks = 16;
    league1.status = "Active";
    league1.createdAt = "2024-08-15";
    league1.totalTeams = 8;
    league1.activeBowlers = 32;
    league1.averageScore = 167.5;
    league1.completedWeeks = 8;
    
    // Initialize advanced config with defaults
    league1.advancedConfig.leagueId = 1;
    league1.advancedConfig.name = league1.basicInfo.name;
    league1.advancedConfig.avgCalc.type = LeagueConfig::AverageCalculation::TotalPinsPerGame;
    league1.advancedConfig.hdcpCalc.type = LeagueConfig::HandicapCalculation::PercentageBased;
    league1.advancedConfig.hdcpCalc.highValue = 225;
    league1.advancedConfig.hdcpCalc.percentage = 0.8;
    league1.advancedConfig.pointSystem.type = LeagueConfig::PointSystem::WinLossTie;
    league1.advancedConfig.preBowlRules.enabled = true;
    league1.advancedConfig.divisions.count = 2;
    
    m_enhancedLeagues.append(league1);
    
    EnhancedLeagueInfo league2;
    league2.basicInfo.id = 2;
    league2.basicInfo.name = "Friday Mixed Doubles";
    league2.basicInfo.teamCount = 12;
    league2.startDate = "2024-09-06";
    league2.endDate = "2024-12-20";
    league2.numberOfWeeks = 16;
    league2.status = "Active";
    league2.createdAt = "2024-08-20";
    league2.totalTeams = 12;
    league2.activeBowlers = 48;
    league2.averageScore = 152.3;
    league2.completedWeeks = 7;
    
    league2.advancedConfig.leagueId = 2;
    league2.advancedConfig.name = league2.basicInfo.name;
    league2.advancedConfig.avgCalc.type = LeagueConfig::AverageCalculation::PeriodicUpdate;
    league2.advancedConfig.avgCalc.updateInterval = 3;
    league2.advancedConfig.hdcpCalc.type = LeagueConfig::HandicapCalculation::StraightDifference;
    league2.advancedConfig.hdcpCalc.highValue = 200;
    league2.advancedConfig.pointSystem.type = LeagueConfig::PointSystem::TeamVsTeam;
    league2.advancedConfig.preBowlRules.enabled = false;
    league2.advancedConfig.divisions.count = 1;
    
    m_enhancedLeagues.append(league2);
}

void LeagueManagementDialog::setupUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    
    m_splitter = new QSplitter(Qt::Horizontal);
    m_mainLayout->addWidget(m_splitter);
    
    setupLeftPane();
    setupMiddlePane();
    setupRightPane();
    
    // Set splitter proportions (2:4:2 ratio)
    m_splitter->setStretchFactor(0, 2);
    m_splitter->setStretchFactor(1, 4);
    m_splitter->setStretchFactor(2, 2);
}

void LeagueManagementDialog::setupLeftPane()
{
    m_leftFrame = new QFrame;
    m_leftFrame->setFrameStyle(QFrame::StyledPanel);
    m_leftFrame->setStyleSheet("QFrame { background-color: #2a2a2a; border: 1px solid #333; }");
    
    m_leftLayout = new QVBoxLayout(m_leftFrame);
    m_leftLayout->setContentsMargins(10, 10, 10, 10);
    
    // Title
    QLabel *leaguesLabel = new QLabel("LEAGUES");
    leaguesLabel->setAlignment(Qt::AlignCenter);
    leaguesLabel->setStyleSheet("QLabel { "
                               "color: white; "
                               "font-size: 14px; "
                               "font-weight: bold; "
                               "margin-bottom: 10px; "
                               "}");
    m_leftLayout->addWidget(leaguesLabel);
    
    // Search section
    QLabel *searchLabel = new QLabel("Search Leagues:");
    searchLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
    m_leftLayout->addWidget(searchLabel);
    
    m_leagueSearchEdit = new QLineEdit;
    m_leagueSearchEdit->setPlaceholderText("Enter league name...");
    m_leagueSearchEdit->setStyleSheet("QLineEdit { "
                                     "background-color: #333; "
                                     "color: white; "
                                     "border: 1px solid #555; "
                                     "padding: 5px; "
                                     "border-radius: 3px; "
                                     "}");
    connect(m_leagueSearchEdit, &QLineEdit::textChanged, this, &LeagueManagementDialog::onSearchTextChanged);
    m_leftLayout->addWidget(m_leagueSearchEdit);
    
    // Leagues list
    m_leaguesList = new QListWidget;
    m_leaguesList->setStyleSheet("QListWidget { "
                                "background-color: #1a1a1a; "
                                "color: white; "
                                "border: 1px solid #333; "
                                "selection-background-color: #4A90E2; "
                                "font-size: 12px; "
                                "}");
    connect(m_leaguesList, &QListWidget::currentRowChanged, 
            this, &LeagueManagementDialog::onLeagueSelectionChanged);
    m_leftLayout->addWidget(m_leaguesList);
    
    m_splitter->addWidget(m_leftFrame);
}

void LeagueManagementDialog::setupMiddlePane()
{
    m_middleFrame = new QFrame;
    m_middleFrame->setFrameStyle(QFrame::StyledPanel);
    m_middleFrame->setStyleSheet("QFrame { background-color: #2a2a2a; border: 1px solid #333; }");
    
    m_middleLayout = new QVBoxLayout(m_middleFrame);
    m_middleLayout->setContentsMargins(10, 10, 10, 10);
    
    // Title
    QLabel *contentLabel = new QLabel("LEAGUE INFORMATION");
    contentLabel->setAlignment(Qt::AlignCenter);
    contentLabel->setStyleSheet("QLabel { "
                               "color: white; "
                               "font-size: 14px; "
                               "font-weight: bold; "
                               "margin-bottom: 10px; "
                               "}");
    m_middleLayout->addWidget(contentLabel);
    
    // Tab widget for different views
    m_middleTabWidget = new QTabWidget;
    m_middleTabWidget->setStyleSheet(
        "QTabWidget::pane { background-color: #1a1a1a; border: 1px solid #333; } "
        "QTabBar::tab { background-color: #333; color: white; padding: 8px 16px; margin-right: 2px; } "
        "QTabBar::tab:selected { background-color: #4A90E2; } "
        "QTabBar::tab:hover { background-color: #555; }"
    );
    
    // Teams tab
    QWidget *teamsTabWidget = new QWidget;
    QVBoxLayout *teamsLayout = new QVBoxLayout(teamsTabWidget);
    
    m_teamsTree = new QTreeWidget;
    m_teamsTree->setStyleSheet("QTreeWidget { "
                              "background-color: #1a1a1a; "
                              "color: white; "
                              "border: 1px solid #333; "
                              "selection-background-color: #4A90E2; "
                              "font-size: 11px; "
                              "} "
                              "QHeaderView::section { "
                              "background-color: #333; "
                              "color: white; "
                              "border: 1px solid #555; "
                              "padding: 4px; "
                              "}");
    
    QStringList teamHeaders;
    teamHeaders << "Team Name" << "Division" << "Wins" << "Losses" << "Points" << "Average";
    m_teamsTree->setHeaderLabels(teamHeaders);
    m_teamsTree->header()->setStretchLastSection(false);
    m_teamsTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    
    teamsLayout->addWidget(m_teamsTree);
    m_middleTabWidget->addTab(teamsTabWidget, "Teams");
    
    // Schedule tab
    QWidget *scheduleTabWidget = new QWidget;
    QVBoxLayout *scheduleLayout = new QVBoxLayout(scheduleTabWidget);
    
    m_scheduleTree = new QTreeWidget;
    m_scheduleTree->setStyleSheet(m_teamsTree->styleSheet());
    
    QStringList scheduleHeaders;
    scheduleHeaders << "Week" << "Date" << "Matchups" << "Status";
    m_scheduleTree->setHeaderLabels(scheduleHeaders);
    m_scheduleTree->header()->setStretchLastSection(true);
    
    scheduleLayout->addWidget(m_scheduleTree);
    m_middleTabWidget->addTab(scheduleTabWidget, "Schedule");
    
    // Standings tab
    QWidget *standingsTabWidget = new QWidget;
    QVBoxLayout *standingsLayout = new QVBoxLayout(standingsTabWidget);
    
    m_standingsTree = new QTreeWidget;
    m_standingsTree->setStyleSheet(m_teamsTree->styleSheet());
    
    QStringList standingsHeaders;
    standingsHeaders << "Rank" << "Team" << "W" << "L" << "T" << "Pts" << "Avg" << "Series High";
    m_standingsTree->setHeaderLabels(standingsHeaders);
    
    standingsLayout->addWidget(m_standingsTree);
    m_middleTabWidget->addTab(standingsTabWidget, "Standings");
    
    m_middleLayout->addWidget(m_middleTabWidget);
    
    m_splitter->addWidget(m_middleFrame);
}

void LeagueManagementDialog::setupRightPane()
{
    m_rightFrame = new QFrame;
    m_rightFrame->setFrameStyle(QFrame::StyledPanel);
    m_rightFrame->setStyleSheet("QFrame { background-color: #2a2a2a; border: 1px solid #333; }");
    
    m_rightLayout = new QVBoxLayout(m_rightFrame);
    m_rightLayout->setContentsMargins(10, 10, 10, 10);
    
    // Actions group
    m_actionsGroup = new QGroupBox("LEAGUE ACTIONS");
    m_actionsGroup->setStyleSheet("QGroupBox { "
                                 "color: white; "
                                 "font-weight: bold; "
                                 "border: 1px solid #555; "
                                 "margin-top: 10px; "
                                 "} "
                                 "QGroupBox::title { "
                                 "subcontrol-origin: margin; "
                                 "left: 10px; "
                                 "padding: 0 5px 0 5px; "
                                 "}");
    
    QVBoxLayout *actionsLayout = new QVBoxLayout(m_actionsGroup);
    actionsLayout->setSpacing(6);
    
    QString buttonStyle = "QPushButton { "
                         "background-color: #333; "
                         "color: white; "
                         "border: 1px solid #555; "
                         "padding: 8px; "
                         "text-align: left; "
                         "border-radius: 3px; "
                         "font-size: 11px; "
                         "} "
                         "QPushButton:hover { background-color: #4A90E2; } "
                         "QPushButton:pressed { background-color: #357ABD; } "
                         "QPushButton:disabled { background-color: #666; color: #999; }";
    
    m_createLeagueBtn = new QPushButton("Create New League");
    m_createLeagueBtn->setStyleSheet(buttonStyle);
    connect(m_createLeagueBtn, &QPushButton::clicked, this, &LeagueManagementDialog::onCreateLeagueClicked);
    actionsLayout->addWidget(m_createLeagueBtn);
    
    m_editLeagueBtn = new QPushButton("Edit League Settings");
    m_editLeagueBtn->setStyleSheet(buttonStyle);
    m_editLeagueBtn->setEnabled(false);
    connect(m_editLeagueBtn, &QPushButton::clicked, this, &LeagueManagementDialog::onEditLeagueClicked);
    actionsLayout->addWidget(m_editLeagueBtn);
    
    m_deleteLeagueBtn = new QPushButton("Delete League");
    m_deleteLeagueBtn->setStyleSheet(buttonStyle);
    m_deleteLeagueBtn->setEnabled(false);
    connect(m_deleteLeagueBtn, &QPushButton::clicked, this, &LeagueManagementDialog::onDeleteLeagueClicked);
    actionsLayout->addWidget(m_deleteLeagueBtn);
    
    // Add separator
    QFrame *separator1 = new QFrame;
    separator1->setFrameStyle(QFrame::HLine | QFrame::Sunken);
    separator1->setStyleSheet("color: #555;");
    actionsLayout->addWidget(separator1);
    
    m_viewStandingsBtn = new QPushButton("View Detailed Standings");
    m_viewStandingsBtn->setStyleSheet(buttonStyle);
    m_viewStandingsBtn->setEnabled(false);
    connect(m_viewStandingsBtn, &QPushButton::clicked, this, &LeagueManagementDialog::onViewStandingsClicked);
    actionsLayout->addWidget(m_viewStandingsBtn);
    
    m_scheduleBtn = new QPushButton("Manage Schedule");
    m_scheduleBtn->setStyleSheet(buttonStyle);
    m_scheduleBtn->setEnabled(false);
    connect(m_scheduleBtn, &QPushButton::clicked, this, &LeagueManagementDialog::onScheduleClicked);
    actionsLayout->addWidget(m_scheduleBtn);
    
    m_manageTeamsBtn = new QPushButton("Manage Teams");
    m_manageTeamsBtn->setStyleSheet(buttonStyle);
    m_manageTeamsBtn->setEnabled(false);
    connect(m_manageTeamsBtn, &QPushButton::clicked, this, &LeagueManagementDialog::onManageTeamsClicked);
    actionsLayout->addWidget(m_manageTeamsBtn);
    
    m_manageBowlersBtn = new QPushButton("Manage Bowlers");
    m_manageBowlersBtn->setStyleSheet(buttonStyle);
    m_manageBowlersBtn->setEnabled(false);
    connect(m_manageBowlersBtn, &QPushButton::clicked, this, &LeagueManagementDialog::onManageBowlersClicked);
    actionsLayout->addWidget(m_manageBowlersBtn);
    
    m_statisticsBtn = new QPushButton("View Statistics");
    m_statisticsBtn->setStyleSheet(buttonStyle);
    m_statisticsBtn->setEnabled(false);
    connect(m_statisticsBtn, &QPushButton::clicked, this, &LeagueManagementDialog::onViewStatisticsClicked);
    actionsLayout->addWidget(m_statisticsBtn);
    
    m_rightLayout->addWidget(m_actionsGroup);
    
    // Details group
    m_detailsGroup = new QGroupBox("League Details");
    m_detailsGroup->setStyleSheet("QGroupBox { "
                                 "color: white; "
                                 "font-weight: bold; "
                                 "border: 1px solid #555; "
                                 "margin-top: 10px; "
                                 "} "
                                 "QGroupBox::title { "
                                 "subcontrol-origin: margin; "
                                 "left: 10px; "
                                 "padding: 0 5px 0 5px; "
                                 "}");
    
    QVBoxLayout *detailsLayout = new QVBoxLayout(m_detailsGroup);
    
    m_detailsText = new QTextEdit;
    m_detailsText->setStyleSheet("QTextEdit { "
                                "color: white; "
                                "padding: 15px; "
                                "background-color: #1a1a1a; "
                                "border: 1px solid #333; "
                                "border-radius: 3px; "
                                "font-size: 11px; "
                                "}");
    m_detailsText->setReadOnly(true);
    m_detailsText->setText("Select a league to view details");
    m_detailsText->setMinimumHeight(200);
    detailsLayout->addWidget(m_detailsText);
    
    m_rightLayout->addWidget(m_detailsGroup);
    
    // Close button
    m_closeBtn = new QPushButton("Close");
    m_closeBtn->setStyleSheet("QPushButton { "
                             "background-color: #666; "
                             "color: white; "
                             "border: none; "
                             "padding: 10px 20px; "
                             "border-radius: 5px; "
                             "font-size: 12px; "
                             "} "
                             "QPushButton:hover { background-color: #777; } "
                             "QPushButton:pressed { background-color: #555; }");
    connect(m_closeBtn, &QPushButton::clicked, this, &LeagueManagementDialog::onCloseClicked);
    
    m_rightLayout->addStretch();
    m_rightLayout->addWidget(m_closeBtn);
    
    m_splitter->addWidget(m_rightFrame);
}

void LeagueManagementDialog::onLeagueSelectionChanged()
{
    int currentRow = m_leaguesList->currentRow();
    bool hasSelection = currentRow >= 0;
    
    m_editLeagueBtn->setEnabled(hasSelection);
    m_deleteLeagueBtn->setEnabled(hasSelection);
    m_viewStandingsBtn->setEnabled(hasSelection);
    m_scheduleBtn->setEnabled(hasSelection);
    m_manageTeamsBtn->setEnabled(hasSelection);
    m_manageBowlersBtn->setEnabled(hasSelection);
    m_statisticsBtn->setEnabled(hasSelection);
    
    updateLeagueTeams();
    updateLeagueDetails();
}

void LeagueManagementDialog::onCreateLeagueClicked()
{
    showCreateLeagueDialog();
}

void LeagueManagementDialog::onEditLeagueClicked()
{
    showEditLeagueDialog();
}

void LeagueManagementDialog::onDeleteLeagueClicked()
{
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    int ret = QMessageBox::question(this, "Confirm Deletion",
                                   QString("Are you sure you want to delete league '%1'?\n\n"
                                          "This will remove all associated data including teams and scores.")
                                   .arg(league.basicInfo.name),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // Remove from sample data
        for (int i = 0; i < m_enhancedLeagues.size(); ++i) {
            if (m_enhancedLeagues[i].basicInfo.id == league.basicInfo.id) {
                m_enhancedLeagues.removeAt(i);
                break;
            }
        }
        
        QMessageBox::information(this, "Success", 
                               QString("League '%1' deleted successfully.").arg(league.basicInfo.name));
        updateLeaguesList();
    }
}

void LeagueManagementDialog::onViewStandingsClicked()
{
    showStandingsDialog();
}

void LeagueManagementDialog::onScheduleClicked()
{
    showScheduleDialog();
}

void LeagueManagementDialog::onManageTeamsClicked()
{
    showTeamManagementDialog();
}

void LeagueManagementDialog::onManageBowlersClicked()
{
    showBowlerManagementDialog();
}

void LeagueManagementDialog::onViewStatisticsClicked()
{
    showStatisticsDialog();
}

void LeagueManagementDialog::onPreBowlManagementClicked()
{
    // Implementation for pre-bowl management
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    QMessageBox::information(this, "Pre-Bowl Management", 
                           QString("Pre-bowl game management for '%1' coming soon.")
                           .arg(league.basicInfo.name));
}

void LeagueManagementDialog::onDivisionManagementClicked()
{
    // Implementation for division management
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    QMessageBox::information(this, "Division Management", 
                           QString("Division management for '%1' coming soon.")
                           .arg(league.basicInfo.name));
}

void LeagueManagementDialog::onPlayoffSetupClicked()
{
    // Implementation for playoff setup
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    QMessageBox::information(this, "Playoff Setup", 
                           QString("Playoff setup for '%1' coming soon.")
                           .arg(league.basicInfo.name));
}

void LeagueManagementDialog::onSearchTextChanged(const QString &text)
{
    Q_UNUSED(text)
    updateLeaguesList();
}

void LeagueManagementDialog::onCloseClicked()
{
    close();
}

void LeagueManagementDialog::updateLeaguesList()
{
    QString searchFilter = m_leagueSearchEdit->text();
    
    m_leaguesList->clear();
    m_leagueIdMap.clear();
    
    int index = 0;
    for (const EnhancedLeagueInfo &league : m_enhancedLeagues) {
        // Apply search filter
        if (!searchFilter.isEmpty() && 
            !league.basicInfo.name.contains(searchFilter, Qt::CaseInsensitive)) {
            continue;
        }
        
        QString statusIcon;
        if (league.status == "Active") {
            statusIcon = "● ";
        } else if (league.status == "Scheduled") {
            statusIcon = "○ ";
        } else if (league.status == "Completed") {
            statusIcon = "◉ ";
        } else {
            statusIcon = "◯ ";
        }
        
        QString listItem = QString("%1%2 (%3 teams, %4)")
                          .arg(statusIcon)
                          .arg(league.basicInfo.name)
                          .arg(league.totalTeams)
                          .arg(league.status);
        
        m_leaguesList->addItem(listItem);
        m_leagueIdMap[index] = league.basicInfo.id;
        index++;
    }
}

void LeagueManagementDialog::updateLeagueTeams()
{
    m_teamsTree->clear();
    
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    // Sample team data for demonstration
    QStringList teamNames = {"Strike Force", "Pin Busters", "Spare Me", "Gutter Balls", 
                            "Rolling Thunder", "Split Happens", "Ten Pin Terrors", "Lane Legends"};
    
    for (int i = 0; i < qMin(league.basicInfo.teamCount, teamNames.size()); ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_teamsTree);
        item->setText(0, teamNames[i]);
        
        // Calculate division
        int divisionNum = (i / 4) + 1; // 4 teams per division
        item->setText(1, QString("Division %1").arg(divisionNum));
        
        item->setText(2, QString::number(10 + (i * 2))); // Wins
        item->setText(3, QString::number(6 - (i * 2)));  // Losses
        item->setText(4, QString::number(20 + (i * 4))); // Points
        item->setText(5, QString::number(150 + (i * 5))); // Average
    }
}

void LeagueManagementDialog::updateLeagueDetails()
{
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) {
        m_detailsText->setText("Select a league to view details");
        return;
    }
    
    QString details = QString("League: %1\n\n").arg(league.basicInfo.name);
    details += QString("Status: %1\n").arg(league.status);
    details += QString("Start Date: %1\n").arg(league.startDate);
    details += QString("End Date: %1\n").arg(league.endDate);
    details += QString("Number of Weeks: %1\n").arg(league.numberOfWeeks);
    details += QString("Teams: %1\n").arg(league.basicInfo.teamCount);
    details += QString("Active Bowlers: %1\n").arg(league.activeBowlers);
    details += QString("League Average: %1\n").arg(league.averageScore, 0, 'f', 1);
    details += QString("Completed Weeks: %1 of %2\n\n").arg(league.completedWeeks).arg(league.numberOfWeeks);
    
    // Advanced configuration details
    details += QString("Advanced Configuration:\n");
    
    // Average calculation
    details += QString("• Average Calculation: ");
    switch (league.advancedConfig.avgCalc.type) {
    case LeagueConfig::AverageCalculation::TotalPinsPerGame:
        details += "Total Pins ÷ Games Played";
        break;
    case LeagueConfig::AverageCalculation::TotalPinsPerBall:
        details += "Total Pins ÷ Balls Thrown";
        break;
    case LeagueConfig::AverageCalculation::PeriodicUpdate:
        details += QString("Periodic Update (every %1 games)").arg(league.advancedConfig.avgCalc.updateInterval);
        break;
    }
    details += "\n";
    
    // Handicap calculation
    details += QString("• Handicap System: ");
    switch (league.advancedConfig.hdcpCalc.type) {
    case LeagueConfig::HandicapCalculation::PercentageBased:
        details += QString("(%1 - Avg) × %2%").arg(league.advancedConfig.hdcpCalc.highValue).arg(league.advancedConfig.hdcpCalc.percentage * 100, 0, 'f', 0);
        break;
    case LeagueConfig::HandicapCalculation::StraightDifference:
        details += QString("%1 - Average").arg(league.advancedConfig.hdcpCalc.highValue);
        break;
    case LeagueConfig::HandicapCalculation::WithDeduction:
        details += QString("(%1 - Avg) - %2").arg(league.advancedConfig.hdcpCalc.highValue).arg(league.advancedConfig.hdcpCalc.deduction);
        break;
    }
    details += "\n";
    
    // Point system
    details += QString("• Point System: ");
    switch (league.advancedConfig.pointSystem.type) {
    case LeagueConfig::PointSystem::WinLossTie:
        details += "Win/Loss/Tie";
        break;
    case LeagueConfig::PointSystem::TeamVsTeam:
        details += "Team vs Team";
        break;
    case LeagueConfig::PointSystem::Custom:
        details += "Custom";
        break;
    }
    details += "\n";
    
    // Divisions
    details += QString("• Divisions: %1\n").arg(league.advancedConfig.divisions.count);
    
    // Pre-bowl
    if (league.advancedConfig.preBowlRules.enabled) {
        details += "• Pre-Bowl Games: Enabled\n";
    } else {
        details += "• Pre-Bowl Games: Disabled\n";
    }
    
    m_detailsText->setText(details);
}

void LeagueManagementDialog::showCreateLeagueDialog()
{
    EnhancedLeagueSetupDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        LeagueConfig newConfig = dialog.getLeagueConfiguration();
        
        // Create enhanced league info
        EnhancedLeagueInfo enhancedLeague;
        enhancedLeague.basicInfo.id = m_enhancedLeagues.size() + 1;
        enhancedLeague.basicInfo.name = newConfig.name;
        enhancedLeague.basicInfo.teamCount = 0;
        enhancedLeague.advancedConfig = newConfig;
        enhancedLeague.startDate = newConfig.startDate.toString("yyyy-MM-dd");
        enhancedLeague.endDate = newConfig.endDate.toString("yyyy-MM-dd");
        enhancedLeague.numberOfWeeks = newConfig.numberOfWeeks;
        enhancedLeague.status = "Scheduled";
        enhancedLeague.createdAt = QDateTime::currentDateTime().toString("yyyy-MM-dd");
        enhancedLeague.totalTeams = 0;
        enhancedLeague.activeBowlers = 0;
        enhancedLeague.averageScore = 0.0;
        enhancedLeague.completedWeeks = 0;
        
        m_enhancedLeagues.append(enhancedLeague);
        
        QMessageBox::information(this, "Success", 
                               QString("League '%1' created successfully.").arg(newConfig.name));
        updateLeaguesList();
    }
}

void LeagueManagementDialog::showEditLeagueDialog()
{
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    EnhancedLeagueSetupDialog dialog(league, this);
    dialog.setWindowTitle("Edit League");
    
    if (dialog.exec() == QDialog::Accepted) {
        LeagueConfig updatedConfig = dialog.getLeagueConfiguration();
        
        // Update in enhanced data
        for (int i = 0; i < m_enhancedLeagues.size(); ++i) {
            if (m_enhancedLeagues[i].basicInfo.id == league.basicInfo.id) {
                m_enhancedLeagues[i].advancedConfig = updatedConfig;
                m_enhancedLeagues[i].basicInfo.name = updatedConfig.name;
                break;
            }
        }
        
        QMessageBox::information(this, "Success", 
                               QString("League '%1' updated successfully.").arg(updatedConfig.name));
        updateLeaguesList();
    }
}

void LeagueManagementDialog::showStandingsDialog()
{
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    QMessageBox::information(this, "League Standings", 
                           QString("Detailed standings for '%1' coming soon.\n\n"
                                  "This will show:\n"
                                  "- Team rankings\n"
                                  "- Win/Loss records\n"
                                  "- Points standings\n"
                                  "- Individual statistics")
                           .arg(league.basicInfo.name));
}

void LeagueManagementDialog::showScheduleDialog()
{
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    QMessageBox::information(this, "League Schedule", 
                           QString("Schedule view for '%1' coming soon.\n\n"
                                  "This will show:\n"
                                  "- Weekly matchups\n"
                                  "- Lane assignments\n"
                                  "- Game times\n"
                                  "- Make-up games")
                           .arg(league.basicInfo.name));
}

void LeagueManagementDialog::showTeamManagementDialog()
{
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    QMessageBox::information(this, "Team Management", 
                           QString("Team management for '%1' coming soon.\n\n"
                                  "This will allow:\n"
                                  "- Adding/removing teams\n"
                                  "- Assigning bowlers to teams\n"
                                  "- Managing team divisions\n"
                                  "- Team statistics")
                           .arg(league.basicInfo.name));
}

void LeagueManagementDialog::showBowlerManagementDialog()
{
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    QMessageBox::information(this, "Bowler Management", 
                           QString("Bowler management for '%1' coming soon.\n\n"
                                  "This will allow:\n"
                                  "- Adding/removing bowlers\n"
                                  "- Managing averages and handicaps\n"
                                  "- Pre-bowl game management\n"
                                  "- Individual statistics")
                           .arg(league.basicInfo.name));
}

void LeagueManagementDialog::showStatisticsDialog()
{
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    QMessageBox::information(this, "League Statistics", 
                           QString("Statistics view for '%1' coming soon.\n\n"
                                  "This will show:\n"
                                  "- League-wide statistics\n"
                                  "- High games and series\n"
                                  "- Achievement tracking\n"
                                  "- Performance trends")
                           .arg(league.basicInfo.name));
}

void LeagueManagementDialog::showPreBowlDialog()
{
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    QMessageBox::information(this, "Pre-Bowl Management", 
                           QString("Pre-bowl management for '%1' coming soon.")
                           .arg(league.basicInfo.name));
}

void LeagueManagementDialog::showDivisionDialog()
{
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    QMessageBox::information(this, "Division Management", 
                           QString("Division management for '%1' coming soon.")
                           .arg(league.basicInfo.name));
}

void LeagueManagementDialog::showPlayoffDialog()
{
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    QMessageBox::information(this, "Playoff Setup", 
                           QString("Playoff setup for '%1' coming soon.")
                           .arg(league.basicInfo.name));
}

EnhancedLeagueInfo LeagueManagementDialog::getSelectedLeagueInfo()
{
    int currentRow = m_leaguesList->currentRow();
    if (currentRow < 0 || !m_leagueIdMap.contains(currentRow)) {
        return EnhancedLeagueInfo(); // Return empty league
    }
    
    int leagueId = m_leagueIdMap[currentRow];
    for (const EnhancedLeagueInfo &league : m_enhancedLeagues) {
        if (league.basicInfo.id == leagueId) {
            return league;
        }
    }
    
    return EnhancedLeagueInfo(); // Return empty league if not found
}

// Enhanced League Setup Dialog Implementation
EnhancedLeagueSetupDialog::EnhancedLeagueSetupDialog(QWidget *parent)
    : QDialog(parent)
    , m_editMode(false)
    , m_editingLeagueId(0)
    , m_totalLanes(8)
{
    setupUI();
    setWindowTitle("Create New League");
    setModal(true);
    resize(900, 700);
    
    // Center the dialog
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // Load available bowlers
    DatabaseManager *db = DatabaseManager::instance();
    m_availableBowlers = db->getAllBowlers();
    populateTeamsList();
    
    // Connect signals
    connect(m_createBtn, &QPushButton::clicked, this, &EnhancedLeagueSetupDialog::onCreateLeagueClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &EnhancedLeagueSetupDialog::onCancelClicked);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &EnhancedLeagueSetupDialog::onTabChanged);
    
    updateSummary();
}

EnhancedLeagueSetupDialog::EnhancedLeagueSetupDialog(const EnhancedLeagueInfo &league, QWidget *parent)
    : QDialog(parent)
    , m_editMode(true)
    , m_editingLeagueId(league.basicInfo.id)
    , m_totalLanes(8)
{
    setupUI();
    populateFromExisting(league);
    setWindowTitle("Edit League");
    setModal(true);
    resize(900, 700);
    
    // Center the dialog
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // Load available bowlers
    DatabaseManager *db = DatabaseManager::instance();
    m_availableBowlers = db->getAllBowlers();
    populateTeamsList();
    
    connect(m_createBtn, &QPushButton::clicked, this, &EnhancedLeagueSetupDialog::onCreateLeagueClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &EnhancedLeagueSetupDialog::onCancelClicked);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &EnhancedLeagueSetupDialog::onTabChanged);
    
    updateSummary();
}

void EnhancedLeagueSetupDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Create tab widget for comprehensive league setup
    m_tabWidget = new QTabWidget;
    m_tabWidget->setStyleSheet(
        "QTabWidget::pane { background-color: #f5f5f5; border: 1px solid #ccc; } "
        "QTabBar::tab { background-color: #e0e0e0; padding: 8px 16px; margin-right: 2px; } "
        "QTabBar::tab:selected { background-color: #4A90E2; color: white; } "
        "QTabBar::tab:hover { background-color: #d0d0d0; }"
    );
    m_mainLayout->addWidget(m_tabWidget);
    
    setupBasicInfoTab();
    setupScheduleTab();
    setupAverageCalculationTab();
    setupHandicapCalculationTab();
    setupAbsentHandlingTab();
    setupPreBowlTab();
    setupPointSystemTab();
    setupTeamsTab();
    setupSummaryTab();
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_cancelBtn = new QPushButton("Cancel");
    m_createBtn = new QPushButton(m_editMode ? "Update" : "Create");
    m_createBtn->setStyleSheet("QPushButton { "
                              "background-color: #4A90E2; "
                              "color: white; "
                              "border: none; "
                              "padding: 10px 20px; "
                              "border-radius: 3px; "
                              "} "
                              "QPushButton:hover { background-color: #357ABD; }");
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelBtn);
    buttonLayout->addWidget(m_createBtn);
    
    m_mainLayout->addLayout(buttonLayout);
}

// Simplified tab setup methods with essential controls only
void EnhancedLeagueSetupDialog::setupBasicInfoTab()
{
    m_basicInfoTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_basicInfoTab);
    
    QGroupBox *infoGroup = new QGroupBox("League Information");
    QGridLayout *infoLayout = new QGridLayout(infoGroup);
    
    infoLayout->addWidget(new QLabel("League Name:"), 0, 0);
    m_leagueNameEdit = new QLineEdit;
    infoLayout->addWidget(m_leagueNameEdit, 0, 1);
    
    infoLayout->addWidget(new QLabel("Contact Name:"), 1, 0);
    m_contactNameEdit = new QLineEdit;
    infoLayout->addWidget(m_contactNameEdit, 1, 1);
    
    infoLayout->addWidget(new QLabel("Contact Phone:"), 2, 0);
    m_contactPhoneEdit = new QLineEdit;
    infoLayout->addWidget(m_contactPhoneEdit, 2, 1);
    
    infoLayout->addWidget(new QLabel("Contact Email:"), 3, 0);
    m_contactEmailEdit = new QLineEdit;
    infoLayout->addWidget(m_contactEmailEdit, 3, 1);
    
    layout->addWidget(infoGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(m_basicInfoTab, "Basic Info");
}

void EnhancedLeagueSetupDialog::setupScheduleTab()
{
    m_scheduleTab = new QWidget;
    QGridLayout *layout = new QGridLayout(m_scheduleTab);
    
    layout->addWidget(new QLabel("Start Date:"), 0, 0);
    m_startDateEdit = new QDateEdit;
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setDate(QDate::currentDate().addDays(7));
    layout->addWidget(m_startDateEdit, 0, 1);
    
    layout->addWidget(new QLabel("Number of Weeks:"), 1, 0);
    m_weeksSpinner = new QSpinBox;
    m_weeksSpinner->setRange(1, 52);
    m_weeksSpinner->setValue(16);
    layout->addWidget(m_weeksSpinner, 1, 1);
    
    layout->addWidget(new QLabel("Available Lanes:"), 2, 0, Qt::AlignTop);
    m_lanesList = new QListWidget;
    for (int i = 1; i <= m_totalLanes; ++i) {
        QListWidgetItem *item = new QListWidgetItem(QString("Lane %1").arg(i));
        item->setData(Qt::UserRole, i);
        item->setCheckState(Qt::Checked);
        m_lanesList->addItem(item);
    }
    layout->addWidget(m_lanesList, 2, 1);
    
    m_tabWidget->addTab(m_scheduleTab, "Schedule");
}

void EnhancedLeagueSetupDialog::setupAverageCalculationTab()
{
    m_avgCalcTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_avgCalcTab);
    
    QGroupBox *calcGroup = new QGroupBox("Average Calculation Method");
    QVBoxLayout *calcLayout = new QVBoxLayout(calcGroup);
    
    m_avgCalcGroup = new QButtonGroup(this);
    
    m_avgPerGameRadio = new QRadioButton("Total pins ÷ games played");
    m_avgPerGameRadio->setChecked(true);
    m_avgCalcGroup->addButton(m_avgPerGameRadio, 0);
    calcLayout->addWidget(m_avgPerGameRadio);
    
    m_avgPeriodicRadio = new QRadioButton("Periodic update every 3 games");
    m_avgCalcGroup->addButton(m_avgPeriodicRadio, 1);
    calcLayout->addWidget(m_avgPeriodicRadio);
    
    layout->addWidget(calcGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(m_avgCalcTab, "Average Calculation");
}

void EnhancedLeagueSetupDialog::setupHandicapCalculationTab()
{
    m_hdcpCalcTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_hdcpCalcTab);
    
    QGroupBox *methodGroup = new QGroupBox("Handicap Calculation");
    QVBoxLayout *methodLayout = new QVBoxLayout(methodGroup);
    
    m_hdcpCalcGroup = new QButtonGroup(this);
    
    m_hdcpPercentageRadio = new QRadioButton("Percentage Based (High - Avg) × 80%");
    m_hdcpPercentageRadio->setChecked(true);
    m_hdcpCalcGroup->addButton(m_hdcpPercentageRadio, 0);
    methodLayout->addWidget(m_hdcpPercentageRadio);
    
    QHBoxLayout *percentageLayout = new QHBoxLayout;
    percentageLayout->addWidget(new QLabel("High Value:"));
    m_hdcpHighValueSpinner = new QSpinBox;
    m_hdcpHighValueSpinner->setRange(150, 300);
    m_hdcpHighValueSpinner->setValue(225);
    percentageLayout->addWidget(m_hdcpHighValueSpinner);
    percentageLayout->addStretch();
    methodLayout->addLayout(percentageLayout);
    
    m_hdcpStraightRadio = new QRadioButton("Straight Difference (High - Average)");
    m_hdcpCalcGroup->addButton(m_hdcpStraightRadio, 1);
    methodLayout->addWidget(m_hdcpStraightRadio);
    
    layout->addWidget(methodGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(m_hdcpCalcTab, "Handicap Calculation");
}

void EnhancedLeagueSetupDialog::setupAbsentHandlingTab()
{
    m_absentTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_absentTab);
    
    QGroupBox *absentGroup = new QGroupBox("Handle Absent Players");
    QVBoxLayout *absentLayout = new QVBoxLayout(absentGroup);
    
    m_absentGroup = new QButtonGroup(this);
    
    m_absentPercentageRadio = new QRadioButton("90% of bowler average");
    m_absentPercentageRadio->setChecked(true);
    m_absentGroup->addButton(m_absentPercentageRadio, 0);
    absentLayout->addWidget(m_absentPercentageRadio);
    
    m_absentFixedRadio = new QRadioButton("Fixed value: 100");
    m_absentGroup->addButton(m_absentFixedRadio, 1);
    absentLayout->addWidget(m_absentFixedRadio);
    
    m_absentAverageRadio = new QRadioButton("Use bowler's current average");
    m_absentGroup->addButton(m_absentAverageRadio, 2);
    absentLayout->addWidget(m_absentAverageRadio);
    
    layout->addWidget(absentGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(m_absentTab, "Absent Handling");
}

void EnhancedLeagueSetupDialog::setupPreBowlTab()
{
    m_preBowlTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_preBowlTab);
    
    m_preBowlEnabledCheck = new QCheckBox("Enable Pre-Bowl Games");
    layout->addWidget(m_preBowlEnabledCheck);
    
    layout->addStretch();
    
    m_tabWidget->addTab(m_preBowlTab, "Pre-Bowl Rules");
}

void EnhancedLeagueSetupDialog::setupPointSystemTab()
{
    m_pointSystemTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_pointSystemTab);
    
    QGroupBox *typeGroup = new QGroupBox("Point System Type");
    QVBoxLayout *typeLayout = new QVBoxLayout(typeGroup);
    
    m_pointTypeGroup = new QButtonGroup(this);
    
    m_winLossTieRadio = new QRadioButton("Win/Loss/Tie System");
    m_winLossTieRadio->setChecked(true);
    m_pointTypeGroup->addButton(m_winLossTieRadio, 0);
    typeLayout->addWidget(m_winLossTieRadio);
    
    m_teamVsTeamRadio = new QRadioButton("Team vs Team Overall");
    m_pointTypeGroup->addButton(m_teamVsTeamRadio, 1);
    typeLayout->addWidget(m_teamVsTeamRadio);
    
    layout->addWidget(typeGroup);
    layout->addStretch();
    
    m_tabWidget->addTab(m_pointSystemTab, "Point System");
}

void EnhancedLeagueSetupDialog::setupTeamsTab()
{
    m_teamsTab = new QWidget;
    QHBoxLayout *layout = new QHBoxLayout(m_teamsTab);
    
    // Left side - Available bowlers
    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(new QLabel("Available Bowlers:"));
    
    m_availableBowlersList = new QListWidget;
    leftLayout->addWidget(m_availableBowlersList);
    
    layout->addLayout(leftLayout);
    
    // Right side - Teams
    QVBoxLayout *rightLayout = new QVBoxLayout;
    rightLayout->addWidget(new QLabel("Teams:"));
    
    QHBoxLayout *teamCreateLayout = new QHBoxLayout;
    m_teamNameEdit = new QLineEdit;
    m_teamNameEdit->setPlaceholderText("Team name...");
    teamCreateLayout->addWidget(m_teamNameEdit);
    
    m_addTeamBtn = new QPushButton("Add Team");
    connect(m_addTeamBtn, &QPushButton::clicked, this, &EnhancedLeagueSetupDialog::onAddTeamClicked);
    teamCreateLayout->addWidget(m_addTeamBtn);
    
    rightLayout->addLayout(teamCreateLayout);
    
    m_teamsList = new QListWidget;
    rightLayout->addWidget(m_teamsList);
    
    m_removeTeamBtn = new QPushButton("Remove Team");
    connect(m_removeTeamBtn, &QPushButton::clicked, this, &EnhancedLeagueSetupDialog::onRemoveTeamClicked);
    rightLayout->addWidget(m_removeTeamBtn);
    
    layout->addLayout(rightLayout);
    
    m_tabWidget->addTab(m_teamsTab, "Teams");
}

void EnhancedLeagueSetupDialog::setupSummaryTab()
{
    m_summaryTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_summaryTab);
    
    layout->addWidget(new QLabel("League Configuration Summary:"));
    
    m_summaryText = new QTextEdit;
    m_summaryText->setReadOnly(true);
    layout->addWidget(m_summaryText);
    
    m_tabWidget->addTab(m_summaryTab, "Summary");
}

LeagueConfig EnhancedLeagueSetupDialog::getLeagueConfiguration() const
{
    LeagueConfig config;
    
    // Basic information
    config.name = m_leagueNameEdit->text().trimmed();
    config.startDate = m_startDateEdit->date();
    config.endDate = config.startDate.addDays(m_weeksSpinner->value() * 7);
    config.numberOfWeeks = m_weeksSpinner->value();
    config.status = "scheduled";
    
    // Selected lanes
    config.laneIds.clear();
    for (int i = 0; i < m_lanesList->count(); ++i) {
        QListWidgetItem *item = m_lanesList->item(i);
        if (item->checkState() == Qt::Checked) {
            config.laneIds.append(item->data(Qt::UserRole).toInt());
        }
    }
    
    // Average calculation
    config.avgCalc.type = m_avgPerGameRadio->isChecked() ? 
                         LeagueConfig::AverageCalculation::TotalPinsPerGame :
                         LeagueConfig::AverageCalculation::PeriodicUpdate;
    
    // Handicap calculation
    config.hdcpCalc.type = m_hdcpPercentageRadio->isChecked() ?
                          LeagueConfig::HandicapCalculation::PercentageBased :
                          LeagueConfig::HandicapCalculation::StraightDifference;
    config.hdcpCalc.highValue = m_hdcpHighValueSpinner->value();
    config.hdcpCalc.percentage = 0.8;
    
    // Absent handling
    if (m_absentPercentageRadio->isChecked()) {
        config.absentHandling.type = LeagueConfig::AbsentHandling::PercentageOfAverage;
        config.absentHandling.percentage = 0.9;
    } else if (m_absentFixedRadio->isChecked()) {
        config.absentHandling.type = LeagueConfig::AbsentHandling::FixedValue;
        config.absentHandling.fixedValue = 100;
    } else {
        config.absentHandling.type = LeagueConfig::AbsentHandling::UseAverage;
    }
    
    // Pre-bowl rules
    config.preBowlRules.enabled = m_preBowlEnabledCheck->isChecked();
    
    // Point system
    config.pointSystem.type = m_winLossTieRadio->isChecked() ?
                             LeagueConfig::PointSystem::WinLossTie :
                             LeagueConfig::PointSystem::TeamVsTeam;
    
    return config;
}

bool EnhancedLeagueSetupDialog::validateConfiguration() const
{
    if (m_leagueNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(const_cast<EnhancedLeagueSetupDialog*>(this), 
                           "Missing Information", "Please enter a league name.");
        return false;
    }
    
    // Check if at least one lane is selected
    bool anyLaneSelected = false;
    for (int i = 0; i < m_lanesList->count(); ++i) {
        if (m_lanesList->item(i)->checkState() == Qt::Checked) {
            anyLaneSelected = true;
            break;
        }
    }
    
    if (!anyLaneSelected) {
        QMessageBox::warning(const_cast<EnhancedLeagueSetupDialog*>(this), 
                           "No Lanes Selected", "Please select at least one lane.");
        return false;
    }
    
    return true;
}

void EnhancedLeagueSetupDialog::onCreateLeagueClicked()
{
    if (validateConfiguration()) {
        accept();
    }
}

void EnhancedLeagueSetupDialog::onCancelClicked()
{
    reject();
}

void EnhancedLeagueSetupDialog::onTabChanged(int index)
{
    if (index == m_tabWidget->count() - 1) { // Summary tab
        updateSummary();
    }
}

void EnhancedLeagueSetupDialog::onAddTeamClicked()
{
    QString teamName = m_teamNameEdit->text().trimmed();
    if (!teamName.isEmpty()) {
        m_teamsList->addItem(teamName);
        m_teamNameEdit->clear();
    }
}

void EnhancedLeagueSetupDialog::onRemoveTeamClicked()
{
    int currentRow = m_teamsList->currentRow();
    if (currentRow >= 0) {
        delete m_teamsList->takeItem(currentRow);
    }
}

void EnhancedLeagueSetupDialog::updateSummary()
{
    LeagueConfig config = getLeagueConfiguration();
    
    QString summary = QString("<h3>%1</h3>").arg(config.name.isEmpty() ? "Unnamed League" : config.name);
    summary += QString("<b>Duration:</b> %1 weeks<br>").arg(config.numberOfWeeks);
    summary += QString("<b>Lanes:</b> %1 selected<br>").arg(config.laneIds.size());
    summary += QString("<b>Teams:</b> %1 created<br><br>").arg(m_teamsList->count());
    
    summary += QString("<b>Average:</b> ");
    if (config.avgCalc.type == LeagueConfig::AverageCalculation::TotalPinsPerGame) {
        summary += "Total Pins ÷ Games<br>";
    } else {
        summary += "Periodic Update<br>";
    }
    
    summary += QString("<b>Handicap:</b> ");
    if (config.hdcpCalc.type == LeagueConfig::HandicapCalculation::PercentageBased) {
        summary += QString("(%1 - Avg) × 80%<br>").arg(config.hdcpCalc.highValue);
    } else {
        summary += QString("%1 - Average<br>").arg(config.hdcpCalc.highValue);
    }
    
    summary += QString("<b>Pre-Bowl:</b> %1<br>").arg(config.preBowlRules.enabled ? "Enabled" : "Disabled");
    
    m_summaryText->setHtml(summary);
}

// Update BowlerInfo type alias to match DatabaseManager
using BowlerInfo = BowlerData;

void LeagueManagementDialog::loadBowlersFromDatabase()
{
    DatabaseManager* db = DatabaseManager::instance();
    m_availableBowlers = db->getAllBowlers();
}

void EnhancedLeagueSetupDialog::populateTeamsList()
{
    m_availableBowlersList->clear();
    for (const BowlerData &bowler : m_availableBowlers) {
        QString displayText = QString("%1 %2 (Avg: %3)")
                             .arg(bowler.firstName)
                             .arg(bowler.lastName)
                             .arg(bowler.average);
        
        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, bowler.id);
        m_availableBowlersList->addItem(item);
    }
}

// Implement real database integration for league creation
void LeagueManagementDialog::showCreateLeagueDialog()
{
    EnhancedLeagueSetupDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        LeagueConfig config = dialog.getLeagueConfiguration();
        
        // Save basic league info to database
        DatabaseManager* db = DatabaseManager::instance();
        int leagueId = db->addLeague(config.name);
        
        if (leagueId > 0) {
            // Create enhanced league info with real database ID
            EnhancedLeagueInfo enhancedLeague;
            enhancedLeague.basicInfo.id = leagueId;
            enhancedLeague.basicInfo.name = config.name;
            enhancedLeague.basicInfo.teamCount = 0;
            enhancedLeague.advancedConfig = config;
            enhancedLeague.startDate = config.startDate.toString("yyyy-MM-dd");
            enhancedLeague.endDate = config.endDate.toString("yyyy-MM-dd");
            enhancedLeague.numberOfWeeks = config.numberOfWeeks;
            enhancedLeague.status = "Scheduled";
            enhancedLeague.createdAt = QDateTime::currentDateTime().toString("yyyy-MM-dd");
            enhancedLeague.totalTeams = 0;
            enhancedLeague.activeBowlers = 0;
            enhancedLeague.averageScore = 0.0;
            enhancedLeague.completedWeeks = 0;
            
            // Add to in-memory storage for now
            m_enhancedLeagues.append(enhancedLeague);
            
            // Create calendar events for the league schedule
            if (!config.laneIds.isEmpty()) {
                QTime startTime(19, 0); // Default 7 PM start
                int duration = 180; // 3 hours
                int frequency = 7; // Weekly
                
                QVector<int> eventIds = db->addLeagueSchedule(
                    leagueId, config.name, config.startDate, startTime,
                    duration, frequency, config.numberOfWeeks,
                    config.laneIds, "League Coordinator", 
                    "555-BOWL", "", "Weekly league games"
                );
                
                QString eventMessage = QString("Created %1 calendar events for the league schedule.")
                                      .arg(eventIds.size());
                QMessageBox::information(this, "League Created", 
                                       QString("League '%1' created successfully with ID %2.\n%3")
                                       .arg(config.name).arg(leagueId).arg(eventMessage));
            } else {
                QMessageBox::information(this, "League Created", 
                                       QString("League '%1' created successfully with ID %2.")
                                       .arg(config.name).arg(leagueId));
            }
            
            updateLeaguesList();
        } else {
            QMessageBox::warning(this, "Error", "Failed to create league in database.");
        }
    }
}

void LeagueManagementDialog::loadLeaguesFromDatabase()
{
    DatabaseManager* db = DatabaseManager::instance();
    QVector<LeagueData> dbLeagues = db->getAllLeagues();
    
    m_enhancedLeagues.clear();
    
    // Convert LeagueData to EnhancedLeagueInfo
    for (const LeagueData& dbLeague : dbLeagues) {
        EnhancedLeagueInfo league;
        league.basicInfo = dbLeague;
        
        // Set defaults for enhanced fields (would load from extended config table in real implementation)
        league.startDate = "2024-09-01";
        league.endDate = "2024-12-15";
        league.numberOfWeeks = 16;
        league.status = "Scheduled";
        league.createdAt = dbLeague.createdAt;
        league.totalTeams = dbLeague.teamCount;
        league.activeBowlers = dbLeague.teamCount * 4; // Estimate 4 bowlers per team
        league.averageScore = 165.0;
        league.completedWeeks = 0;
        
        // Initialize default advanced config
        league.advancedConfig.leagueId = dbLeague.id;
        league.advancedConfig.name = dbLeague.name;
        league.advancedConfig.avgCalc.type = LeagueConfig::AverageCalculation::TotalPinsPerGame;
        league.advancedConfig.hdcpCalc.type = LeagueConfig::HandicapCalculation::PercentageBased;
        league.advancedConfig.hdcpCalc.highValue = 225;
        league.advancedConfig.hdcpCalc.percentage = 0.8;
        league.advancedConfig.pointSystem.type = LeagueConfig::PointSystem::WinLossTie;
        league.advancedConfig.preBowlRules.enabled = false;
        league.advancedConfig.divisions.count = 1;
        
        m_enhancedLeagues.append(league);
    }
}

// Real team and bowler management dialogs
void LeagueManagementDialog::showTeamManagementDialog()
{
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    // Create a comprehensive team management dialog
    QDialog teamDialog(this);
    teamDialog.setWindowTitle(QString("Team Management - %1").arg(league.basicInfo.name));
    teamDialog.setModal(true);
    teamDialog.resize(800, 600);
    
    QVBoxLayout *layout = new QVBoxLayout(&teamDialog);
    
    // Teams list
    QLabel *teamsLabel = new QLabel("Current Teams:");
    layout->addWidget(teamsLabel);
    
    QTreeWidget *teamsTree = new QTreeWidget;
    teamsTree->setHeaderLabels({"Team Name", "Bowlers", "Division", "Status"});
    
    // Load teams from database
    DatabaseManager* db = DatabaseManager::instance();
    QVector<TeamData> teams = db->getAllTeams();
    
    for (const TeamData &team : teams) {
        QTreeWidgetItem *item = new QTreeWidgetItem(teamsTree);
        item->setText(0, team.name);
        item->setText(1, QString::number(team.bowlerCount));
        item->setText(2, "Division 1"); // Default
        item->setText(3, "Active");
        item->setData(0, Qt::UserRole, team.id);
    }
    
    layout->addWidget(teamsTree);
    
    // Action buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    
    QPushButton *addTeamBtn = new QPushButton("Add Team");
    QPushButton *editTeamBtn = new QPushButton("Edit Team");
    QPushButton *removeTeamBtn = new QPushButton("Remove Team");
    QPushButton *assignBowlersBtn = new QPushButton("Assign Bowlers");
    QPushButton *closeBtn = new QPushButton("Close");
    
    buttonLayout->addWidget(addTeamBtn);
    buttonLayout->addWidget(editTeamBtn);
    buttonLayout->addWidget(removeTeamBtn);
    buttonLayout->addWidget(assignBowlersBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeBtn);
    
    layout->addLayout(buttonLayout);
    
    // Connect buttons
    connect(addTeamBtn, &QPushButton::clicked, [&]() {
        bool ok;
        QString teamName = QInputDialog::getText(&teamDialog, "Add Team", 
                                               "Team Name:", QLineEdit::Normal, "", &ok);
        if (ok && !teamName.isEmpty()) {
            int teamId = db->addTeam(teamName);
            if (teamId > 0) {
                QTreeWidgetItem *item = new QTreeWidgetItem(teamsTree);
                item->setText(0, teamName);
                item->setText(1, "0");
                item->setText(2, "Division 1");
                item->setText(3, "Active");
                item->setData(0, Qt::UserRole, teamId);
                QMessageBox::information(&teamDialog, "Success", "Team added successfully.");
            }
        }
    });
    
    connect(closeBtn, &QPushButton::clicked, &teamDialog, &QDialog::accept);
    
    teamDialog.exec();
}

void LeagueManagementDialog::showBowlerManagementDialog()
{
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    // Open the full bowler management dialog
    BowlerManagementDialog dialog(m_mainWindow, this);
    dialog.exec();
}

// Schedule and Standings with real data
void LeagueManagementDialog::showScheduleDialog()
{
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    QDialog scheduleDialog(this);
    scheduleDialog.setWindowTitle(QString("Schedule - %1").arg(league.basicInfo.name));
    scheduleDialog.setModal(true);
    scheduleDialog.resize(900, 600);
    
    QVBoxLayout *layout = new QVBoxLayout(&scheduleDialog);
    
    // Load calendar events for this league from database
    DatabaseManager* db = DatabaseManager::instance();
    QVector<CalendarEventData> events = db->getAllCalendarEvents();
    
    QTreeWidget *scheduleTree = new QTreeWidget;
    scheduleTree->setHeaderLabels({"Week", "Date", "Time", "Lane", "Status"});
    
    int weekNumber = 1;
    for (const CalendarEventData &event : events) {
        if (event.leagueId == league.basicInfo.id) {
            QTreeWidgetItem *item = new QTreeWidgetItem(scheduleTree);
            item->setText(0, QString::number(weekNumber++));
            item->setText(1, event.date.toString("MMM dd, yyyy"));
            item->setText(2, event.startTime.toString("h:mm AP"));
            item->setText(3, QString("Lane %1").arg(event.laneId));
            item->setText(4, event.date < QDate::currentDate() ? "Completed" : "Scheduled");
        }
    }
    
    layout->addWidget(scheduleTree);
    
    QPushButton *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, &scheduleDialog, &QDialog::accept);
    layout->addWidget(closeBtn);
    
    scheduleDialog.exec();
}

void LeagueManagementDialog::showStandingsDialog()
{
    EnhancedLeagueInfo league = getSelectedLeagueInfo();
    if (league.basicInfo.id <= 0) return;
    
    QDialog standingsDialog(this);
    standingsDialog.setWindowTitle(QString("Standings - %1").arg(league.basicInfo.name));
    standingsDialog.setModal(true);
    standingsDialog.resize(800, 500);
    
    QVBoxLayout *layout = new QVBoxLayout(&standingsDialog);
    
    QTreeWidget *standingsTree = new QTreeWidget;
    standingsTree->setHeaderLabels({"Rank", "Team", "Wins", "Losses", "Ties", "Points", "Average"});
    
    // Sample standings data (would come from league manager calculations)
    QStringList teamNames = {"Strike Force", "Pin Busters", "Spare Me", "Gutter Balls", 
                            "Rolling Thunder", "Split Happens", "Ten Pin Terrors", "Lane Legends"};
    
    for (int i = 0; i < teamNames.size(); ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem(standingsTree);
        item->setText(0, QString::number(i + 1));
        item->setText(1, teamNames[i]);
        item->setText(2, QString::number(12 - i));     // Wins
        item->setText(3, QString::number(4 + i));      // Losses  
        item->setText(4, QString::number(0));          // Ties
        item->setText(5, QString::number(24 - (i*2))); // Points
        item->setText(6, QString::number(175 - (i*3))); // Average
    }
    
    layout->addWidget(standingsTree);
    
    QPushButton *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, &standingsDialog, &QDialog::accept);
    layout->addWidget(closeBtn);
    
    standingsDialog.exec();
}

// Constructor update to load from database
LeagueManagementDialog::LeagueManagementDialog(MainWindow *mainWindow, QWidget *parent)
    : QDialog(parent)
    , m_mainWindow(mainWindow)
    , m_leagueManager(nullptr)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
{
    setupUI();
    setWindowTitle("League Management");
    setModal(true);
    resize(1200, 800);
    
    // Center the dialog
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // Load real data from database instead of sample data
    loadLeaguesFromDatabase();
    loadBowlersFromDatabase();
    
    // Load initial data
    updateLeaguesList();
}

void EnhancedLeagueSetupDialog::populateFromExisting(const EnhancedLeagueInfo &league)
{
    // Populate basic info
    m_leagueNameEdit->setText(league.basicInfo.name);
    
    // Populate schedule
    m_startDateEdit->setDate(QDate::fromString(league.startDate, "yyyy-MM-dd"));
    m_weeksSpinner->setValue(league.numberOfWeeks);
    
    // Populate average calculation
    if (league.advancedConfig.avgCalc.type == LeagueConfig::AverageCalculation::TotalPinsPerGame) {
        m_avgPerGameRadio->setChecked(true);
    } else {
        m_avgPeriodicRadio->setChecked(true);
    }
    
    // Populate handicap calculation
    if (league.advancedConfig.hdcpCalc.type == LeagueConfig::HandicapCalculation::PercentageBased) {
        m_hdcpPercentageRadio->setChecked(true);
    } else {
        m_hdcpStraightRadio->setChecked(true);
    }
    m_hdcpHighValueSpinner->setValue(league.advancedConfig.hdcpCalc.highValue);
    
    // Populate absent handling
    switch (league.advancedConfig.absentHandling.type) {
    case LeagueConfig::AbsentHandling::PercentageOfAverage:
        m_absentPercentageRadio->setChecked(true);
        break;
    case LeagueConfig::AbsentHandling::FixedValue:
        m_absentFixedRadio->setChecked(true);
        break;
    case LeagueConfig::AbsentHandling::UseAverage:
        m_absentAverageRadio->setChecked(true);
        break;
    }
    
    // Populate pre-bowl
    m_preBowlEnabledCheck->setChecked(league.advancedConfig.preBowlRules.enabled);
    
    // Populate point system
    if (league.advancedConfig.pointSystem.type == LeagueConfig::PointSystem::WinLossTie) {
        m_winLossTieRadio->setChecked(true);
    } else {
        m_teamVsTeamRadio->setChecked(true);
    }
}

// Complete slot implementations
void EnhancedLeagueSetupDialog::onAvgCalculationChanged()
{
    // Enable/disable interval spinner based on periodic selection
    bool isPeriodicUpdate = (m_avgCalcGroup->checkedId() == 1);
    if (isPeriodicUpdate) {
        // Could add interval spinner control here if needed
        updateSummary();
    }
}

void EnhancedLeagueSetupDialog::onHdcpCalculationChanged()
{
    // Enable/disable high value spinner based on calculation type
    bool isPercentageBased = (m_hdcpCalcGroup->checkedId() == 0);
    m_hdcpHighValueSpinner->setEnabled(isPercentageBased || m_hdcpCalcGroup->checkedId() == 1);
    updateSummary();
}

void EnhancedLeagueSetupDialog::onAbsentHandlingChanged()
{
    // Update UI based on absent handling selection
    int selectedType = m_absentGroup->checkedId();
    
    // Could add specific value controls here based on selection
    // For now just update the summary
    updateSummary();
}

void EnhancedLeagueSetupDialog::onPreBowlToggled(bool enabled)
{
    // Update summary when pre-bowl is toggled
    updateSummary();
    
    if (enabled) {
        QMessageBox::information(this, "Pre-Bowl Games", 
                               "Pre-bowl games are now enabled. Players can bowl games "
                               "in advance which will be used when they are absent.");
    }
}

void EnhancedLeagueSetupDialog::onDivisionCountChanged(int count)
{
    // Update team assignment based on division count
    if (count > 1 && m_teamsList->count() > 0) {
        int teamsPerDivision = qCeil(static_cast<double>(m_teamsList->count()) / count);
        
        QString message = QString("With %1 teams and %2 divisions, "
                                "you'll have approximately %3 teams per division.")
                         .arg(m_teamsList->count())
                         .arg(count)
                         .arg(teamsPerDivision);
        
        // Could show division assignment preview here
        QMessageBox::information(this, "Division Assignment", message);
    }
    
    updateSummary();
}

void EnhancedLeagueSetupDialog::onPlayoffTypeChanged()
{
    // Handle playoff type selection changes
    // Enable/disable related controls based on playoff type
    updateSummary();
}

void EnhancedLeagueSetupDialog::onPointSystemChanged()
{
    // Update available options based on point system selection
    int selectedType = m_pointTypeGroup->checkedId();
    
    QString systemName;
    switch (selectedType) {
    case 0: systemName = "Win/Loss/Tie"; break;
    case 1: systemName = "Team vs Team"; break;
    default: systemName = "Custom"; break;
    }
    
    // Could show system-specific options here
    updateSummary();
}

void EnhancedLeagueSetupDialog::onAssignDivisionsClicked()
{
    if (m_teamsList->count() == 0) {
        QMessageBox::information(this, "No Teams", 
                               "Please add some teams first before assigning divisions.");
        return;
    }
    
    // Simple automatic division assignment
    int divisionCount = 2; // Default to 2 divisions
    int teamsPerDivision = qCeil(static_cast<double>(m_teamsList->count()) / divisionCount);
    
    QString assignment = "Division Assignment Preview:\n\n";
    
    for (int div = 0; div < divisionCount; ++div) {
        assignment += QString("Division %1:\n").arg(div + 1);
        
        int startIdx = div * teamsPerDivision;
        int endIdx = qMin(startIdx + teamsPerDivision, m_teamsList->count());
        
        for (int i = startIdx; i < endIdx; ++i) {
            if (i < m_teamsList->count()) {
                assignment += QString("  - %1\n").arg(m_teamsList->item(i)->text());
            }
        }
        assignment += "\n";
    }
    
    QMessageBox::information(this, "Division Assignment", assignment);
}

void EnhancedLeagueSetupDialog::assignTeamsToDivisions()
{
    // Perform automatic team-to-division assignment
    // This would typically balance teams across divisions
    
    if (m_teamsList->count() < 2) {
        return; // Need at least 2 teams
    }
    
    // Simple round-robin assignment to divisions
    int divisionCount = qMax(1, m_teamsList->count() / 4); // Roughly 4 teams per division
    
    for (int i = 0; i < m_teamsList->count(); ++i) {
        int assignedDivision = (i % divisionCount) + 1;
        
        // Store division assignment (could be added to team data structure)
        QString itemText = m_teamsList->item(i)->text();
        if (!itemText.contains("(Div")) {
            m_teamsList->item(i)->setText(itemText + QString(" (Div %1)").arg(assignedDivision));
        }
    }
}