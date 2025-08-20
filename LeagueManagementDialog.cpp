#include "LeagueManagementDialog.h"
#include "MainWindow.h"
#include <QDateTime>

LeagueManagementDialog::LeagueManagementDialog(MainWindow *mainWindow, QWidget *parent)
    : QDialog(parent)
    , m_mainWindow(mainWindow)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
{
    setupUI();
    setWindowTitle("League Management");
    setModal(true);
    resize(1000, 700);
    
    // Center the dialog
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // Initialize sample data - create LeagueData objects properly
    LeagueData league1;
    league1.id = 1;
    league1.name = "Monday Night League";
    league1.teamCount = 8;
    m_leagues.append(league1);
    
    LeagueData league2;
    league2.id = 2;
    league2.name = "Friday Mixed Doubles";
    league2.teamCount = 12;
    m_leagues.append(league2);
    
    LeagueData league3;
    league3.id = 3;
    league3.name = "Saturday Youth League";
    league3.teamCount = 6;
    m_leagues.append(league3);
    
    LeagueData league4;
    league4.id = 4;
    league4.name = "Corporate Challenge";
    league4.teamCount = 10;
    m_leagues.append(league4);
    
    // Store additional league info separately since LeagueData doesn't have these fields
    m_leagueExtendedInfo[1] = {"2024-09-01", "2024-12-15", 16, "Active", "2024-08-15"};
    m_leagueExtendedInfo[2] = {"2024-09-06", "2024-12-20", 16, "Active", "2024-08-20"};
    m_leagueExtendedInfo[3] = {"2024-09-07", "2024-12-21", 16, "Scheduled", "2024-08-22"};
    m_leagueExtendedInfo[4] = {"2024-07-01", "2024-08-30", 8, "Completed", "2024-06-15"};
    
    // Load initial data
    updateLeaguesList();
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
    
    // Set splitter proportions (2:3:2 ratio)
    m_splitter->setStretchFactor(0, 2);
    m_splitter->setStretchFactor(1, 3);
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
    QLabel *teamsLabel = new QLabel("LEAGUE TEAMS");
    teamsLabel->setAlignment(Qt::AlignCenter);
    teamsLabel->setStyleSheet("QLabel { "
                             "color: white; "
                             "font-size: 14px; "
                             "font-weight: bold; "
                             "margin-bottom: 10px; "
                             "}");
    m_middleLayout->addWidget(teamsLabel);
    
    // Teams tree
    m_teamsTree = new QTreeWidget;
    m_teamsTree->setStyleSheet("QTreeWidget { "
                              "background-color: #1a1a1a; "
                              "color: white; "
                              "border: 1px solid #333; "
                              "selection-background-color: #4A90E2; "
                              "font-size: 11px; "
                              "} "
                              "QTreeWidget::item { "
                              "padding: 3px; "
                              "} "
                              "QHeaderView::section { "
                              "background-color: #333; "
                              "color: white; "
                              "border: 1px solid #555; "
                              "padding: 4px; "
                              "}");
    
    QStringList headers;
    headers << "Team Name" << "Wins" << "Losses" << "Points" << "Average";
    m_teamsTree->setHeaderLabels(headers);
    m_teamsTree->header()->setStretchLastSection(false);
    m_teamsTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_teamsTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_teamsTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_teamsTree->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    m_teamsTree->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents);
    
    m_middleLayout->addWidget(m_teamsTree);
    
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
    actionsLayout->setSpacing(8);
    
    QString buttonStyle = "QPushButton { "
                         "background-color: #333; "
                         "color: white; "
                         "border: 1px solid #555; "
                         "padding: 10px; "
                         "text-align: left; "
                         "border-radius: 3px; "
                         "font-size: 12px; "
                         "} "
                         "QPushButton:hover { background-color: #4A90E2; } "
                         "QPushButton:pressed { background-color: #357ABD; } "
                         "QPushButton:disabled { background-color: #666; color: #999; }";
    
    m_addLeagueBtn = new QPushButton("Create New League");
    m_addLeagueBtn->setStyleSheet(buttonStyle);
    connect(m_addLeagueBtn, &QPushButton::clicked, this, &LeagueManagementDialog::onAddLeagueClicked);
    actionsLayout->addWidget(m_addLeagueBtn);
    
    m_editLeagueBtn = new QPushButton("Edit League");
    m_editLeagueBtn->setStyleSheet(buttonStyle);
    m_editLeagueBtn->setEnabled(false);
    connect(m_editLeagueBtn, &QPushButton::clicked, this, &LeagueManagementDialog::onEditLeagueClicked);
    actionsLayout->addWidget(m_editLeagueBtn);
    
    m_deleteLeagueBtn = new QPushButton("Delete League");
    m_deleteLeagueBtn->setStyleSheet(buttonStyle);
    m_deleteLeagueBtn->setEnabled(false);
    connect(m_deleteLeagueBtn, &QPushButton::clicked, this, &LeagueManagementDialog::onDeleteLeagueClicked);
    actionsLayout->addWidget(m_deleteLeagueBtn);
    
    m_viewStandingsBtn = new QPushButton("View Standings");
    m_viewStandingsBtn->setStyleSheet(buttonStyle);
    m_viewStandingsBtn->setEnabled(false);
    connect(m_viewStandingsBtn, &QPushButton::clicked, this, &LeagueManagementDialog::onViewStandingsClicked);
    actionsLayout->addWidget(m_viewStandingsBtn);
    
    m_scheduleBtn = new QPushButton("View Schedule");
    m_scheduleBtn->setStyleSheet(buttonStyle);
    m_scheduleBtn->setEnabled(false);
    connect(m_scheduleBtn, &QPushButton::clicked, this, &LeagueManagementDialog::onScheduleClicked);
    actionsLayout->addWidget(m_scheduleBtn);
    
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
    
    m_detailsLabel = new QLabel("Select a league to view details");
    m_detailsLabel->setStyleSheet("QLabel { "
                                 "color: white; "
                                 "padding: 15px; "
                                 "background-color: #1a1a1a; "
                                 "border: 1px solid #333; "
                                 "border-radius: 3px; "
                                 "font-size: 11px; "
                                 "}");
    m_detailsLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    m_detailsLabel->setWordWrap(true);
    m_detailsLabel->setMinimumHeight(150);
    detailsLayout->addWidget(m_detailsLabel);
    
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
    
    updateLeagueTeams();
    updateLeagueDetails();
}

void LeagueManagementDialog::onAddLeagueClicked()
{
    showAddLeagueDialog();
}

void LeagueManagementDialog::onEditLeagueClicked()
{
    showEditLeagueDialog();
}

void LeagueManagementDialog::onDeleteLeagueClicked()
{
    LeagueData league = getSelectedLeague();
    if (league.id <= 0) return;
    
    int ret = QMessageBox::question(this, "Confirm Deletion",
                                   QString("Are you sure you want to delete league '%1'?\n\n"
                                          "This will remove all associated data including teams and scores.")
                                   .arg(league.name),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // Remove from sample data
        for (int i = 0; i < m_leagues.size(); ++i) {
            if (m_leagues[i].id == league.id) {
                m_leagues.removeAt(i);
                m_leagueExtendedInfo.remove(league.id);
                break;
            }
        }
        
        QMessageBox::information(this, "Success", 
                               QString("League '%1' deleted successfully.").arg(league.name));
        updateLeaguesList();
    }
}

void LeagueManagementDialog::onViewStandingsClicked()
{
    LeagueData league = getSelectedLeague();
    if (league.id <= 0) return;
    
    QMessageBox::information(this, "League Standings", 
                           QString("Detailed standings for '%1' coming soon.\n\n"
                                  "This will show:\n"
                                  "- Team rankings\n"
                                  "- Win/Loss records\n"
                                  "- Points standings\n"
                                  "- Individual statistics")
                           .arg(league.name));
}

void LeagueManagementDialog::onScheduleClicked()
{
    LeagueData league = getSelectedLeague();
    if (league.id <= 0) return;
    
    QMessageBox::information(this, "League Schedule", 
                           QString("Schedule view for '%1' coming soon.\n\n"
                                  "This will show:\n"
                                  "- Weekly matchups\n"
                                  "- Lane assignments\n"
                                  "- Game times\n"
                                  "- Make-up games")
                           .arg(league.name));
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
    for (const LeagueData &league : m_leagues) {
        // Apply search filter
        if (!searchFilter.isEmpty() && 
            !league.name.contains(searchFilter, Qt::CaseInsensitive)) {
            continue;
        }
        
        QString status = "Unknown";
        if (m_leagueExtendedInfo.contains(league.id)) {
            status = m_leagueExtendedInfo[league.id].status;
        }
        
        QString listItem = QString("%1 (%2 teams, %3)")
                          .arg(league.name)
                          .arg(league.teamCount)
                          .arg(status);
        m_leaguesList->addItem(listItem);
        m_leagueIdMap[index] = league.id;
        index++;
    }
}

void LeagueManagementDialog::updateLeagueTeams()
{
    m_teamsTree->clear();
    
    LeagueData league = getSelectedLeague();
    if (league.id <= 0) return;
    
    // Sample team data for demonstration
    QStringList teamNames = {"Strike Force", "Pin Busters", "Spare Me", "Gutter Balls", 
                            "Rolling Thunder", "Split Happens", "Ten Pin Terrors", "Lane Legends"};
    
    for (int i = 0; i < qMin(league.teamCount, teamNames.size()); ++i) {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_teamsTree);
        item->setText(0, teamNames[i]);
        item->setText(1, QString::number(10 + (i * 2))); // Wins
        item->setText(2, QString::number(6 - (i * 2)));  // Losses
        item->setText(3, QString::number(20 + (i * 4))); // Points
        item->setText(4, QString::number(150 + (i * 5))); // Average
    }
}

void LeagueManagementDialog::updateLeagueDetails()
{
    LeagueData league = getSelectedLeague();
    if (league.id <= 0) {
        m_detailsLabel->setText("Select a league to view details");
        return;
    }
    
    QString details = QString("League: %1\n\n").arg(league.name);
    
    if (m_leagueExtendedInfo.contains(league.id)) {
        const LeagueExtendedInfo &info = m_leagueExtendedInfo[league.id];
        details += QString("Status: %1\n").arg(info.status);
        details += QString("Start Date: %1\n").arg(info.startDate);
        details += QString("End Date: %1\n").arg(info.endDate);
        details += QString("Number of Weeks: %1\n").arg(info.numberOfWeeks);
        details += QString("Teams: %1\n").arg(league.teamCount);
        details += QString("Created: %1\n\n").arg(info.createdAt);
        
        if (info.status == "Active") {
            details += "League is currently in progress.\n";
            details += "View standings and schedule for current week information.";
        } else if (info.status == "Scheduled") {
            details += "League has not started yet.\n";
            details += "Teams can still be added or modified.";
        } else if (info.status == "Completed") {
            details += "League has finished.\n";
            details += "Final standings and statistics are available.";
        }
    } else {
        details += QString("Teams: %1\n").arg(league.teamCount);
        details += "No additional information available.";
    }
    
    m_detailsLabel->setText(details);
}

void LeagueManagementDialog::showAddLeagueDialog()
{
    AddLeagueDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        LeagueData newLeague = dialog.getLeagueData();
        newLeague.id = m_leagues.size() + 1; // Simple ID assignment
        
        // Add extended info
        LeagueExtendedInfo extInfo;
        extInfo.startDate = QDateTime::currentDateTime().toString("yyyy-MM-dd");
        extInfo.endDate = QDateTime::currentDateTime().addDays(112).toString("yyyy-MM-dd");
        extInfo.numberOfWeeks = 16;
        extInfo.status = "Scheduled";
        extInfo.createdAt = QDateTime::currentDateTime().toString("yyyy-MM-dd");
        
        m_leagues.append(newLeague);
        m_leagueExtendedInfo[newLeague.id] = extInfo;
        
        QMessageBox::information(this, "Success", 
                               QString("League '%1' created successfully.").arg(newLeague.name));
        updateLeaguesList();
    }
}

void LeagueManagementDialog::showEditLeagueDialog()
{
    LeagueData league = getSelectedLeague();
    if (league.id <= 0) return;
    
    AddLeagueDialog dialog(league, this);
    dialog.setWindowTitle("Edit League");
    
    if (dialog.exec() == QDialog::Accepted) {
        LeagueData updatedLeague = dialog.getLeagueData();
        updatedLeague.id = league.id; // Preserve ID
        
        // Update in sample data
        for (int i = 0; i < m_leagues.size(); ++i) {
            if (m_leagues[i].id == league.id) {
                m_leagues[i] = updatedLeague;
                break;
            }
        }
        
        QMessageBox::information(this, "Success", 
                               QString("League '%1' updated successfully.").arg(updatedLeague.name));
        updateLeaguesList();
    }
}

LeagueData LeagueManagementDialog::getSelectedLeague()
{
    int currentRow = m_leaguesList->currentRow();
    if (currentRow < 0 || !m_leagueIdMap.contains(currentRow)) {
        return LeagueData(); // Return empty league
    }
    
    int leagueId = m_leagueIdMap[currentRow];
    for (const LeagueData &league : m_leagues) {
        if (league.id == leagueId) {
            return league;
        }
    }
    
    return LeagueData(); // Return empty league if not found
}

// AddLeagueDialog Implementation
AddLeagueDialog::AddLeagueDialog(QWidget *parent)
    : QDialog(parent)
    , m_editMode(false)
{
    setupUI();
    setWindowTitle("Create New League");
    setModal(true);
    resize(450, 350);
    
    // Center the dialog
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

AddLeagueDialog::AddLeagueDialog(const LeagueData &league, QWidget *parent)
    : QDialog(parent)
    , m_editMode(true)
{
    setupUI();
    populateFields(league);
    setWindowTitle("Edit League");
    setModal(true);
    resize(450, 350);
    
    // Center the dialog
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void AddLeagueDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(15);
    
    // Form frame
    m_formFrame = new QFrame;
    m_formFrame->setStyleSheet("QFrame { background-color: #2a2a2a; }");
    
    m_formLayout = new QGridLayout(m_formFrame);
    m_formLayout->setContentsMargins(15, 15, 15, 15);
    m_formLayout->setSpacing(10);
    
    QString labelStyle = "QLabel { color: white; font-weight: bold; font-size: 12px; }";
    QString editStyle = "QLineEdit, QDateEdit, QSpinBox, QComboBox { "
                       "background-color: #333; "
                       "color: white; "
                       "border: 1px solid #555; "
                       "padding: 8px; "
                       "border-radius: 3px; "
                       "font-size: 12px; "
                       "}";
    
    int row = 0;
    
    // League Name
    QLabel *nameLabel = new QLabel("League Name:");
    nameLabel->setStyleSheet(labelStyle);
    m_formLayout->addWidget(nameLabel, row, 0);
    
    m_nameEdit = new QLineEdit;
    m_nameEdit->setStyleSheet(editStyle);
    m_nameEdit->setPlaceholderText("Enter league name");
    m_formLayout->addWidget(m_nameEdit, row, 1); // Fixed: Added semicolon
    row++;
    
    // Start Date
    QLabel *startLabel = new QLabel("Start Date:");
    startLabel->setStyleSheet(labelStyle);
    m_formLayout->addWidget(startLabel, row, 0);
    
    m_startDateEdit = new QDateEdit;
    m_startDateEdit->setStyleSheet(editStyle);
    m_startDateEdit->setDate(QDate::currentDate().addDays(7));
    m_startDateEdit->setCalendarPopup(true);
    m_formLayout->addWidget(m_startDateEdit, row, 1);
    row++;
    
    // End Date  
    QLabel *endLabel = new QLabel("End Date:");
    endLabel->setStyleSheet(labelStyle);
    m_formLayout->addWidget(endLabel, row, 0);
    
    m_endDateEdit = new QDateEdit;
    m_endDateEdit->setStyleSheet(editStyle);
    m_endDateEdit->setDate(QDate::currentDate().addDays(119)); // ~17 weeks
    m_endDateEdit->setCalendarPopup(true);
    m_formLayout->addWidget(m_endDateEdit, row, 1);
    row++;
    
    // Number of Weeks
    QLabel *weeksLabel = new QLabel("Number of Weeks:");
    weeksLabel->setStyleSheet(labelStyle);
    m_formLayout->addWidget(weeksLabel, row, 0);
    
    m_weeksSpinBox = new QSpinBox;
    m_weeksSpinBox->setStyleSheet(editStyle);
    m_weeksSpinBox->setRange(1, 52);
    m_weeksSpinBox->setValue(16);
    m_formLayout->addWidget(m_weeksSpinBox, row, 1);
    row++;
    
    // Status
    QLabel *statusLabel = new QLabel("Status:");
    statusLabel->setStyleSheet(labelStyle);
    m_formLayout->addWidget(statusLabel, row, 0);
    
    m_statusCombo = new QComboBox;
    m_statusCombo->setStyleSheet(editStyle);
    m_statusCombo->addItems({"Scheduled", "Active", "Completed"});
    m_formLayout->addWidget(m_statusCombo, row, 1);
    
    m_mainLayout->addWidget(m_formFrame);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    
    m_cancelBtn = new QPushButton("Cancel");
    m_cancelBtn->setStyleSheet("QPushButton { "
                              "background-color: #666; "
                              "color: white; "
                              "border: none; "
                              "padding: 10px 20px; "
                              "border-radius: 3px; "
                              "} "
                              "QPushButton:hover { background-color: #777; }");
    connect(m_cancelBtn, &QPushButton::clicked, this, &AddLeagueDialog::onCancelClicked);
    buttonLayout->addWidget(m_cancelBtn);
    
    buttonLayout->addStretch();
    
    m_saveBtn = new QPushButton(m_editMode ? "Update" : "Create");
    m_saveBtn->setStyleSheet("QPushButton { "
                            "background-color: #4A90E2; "
                            "color: white; "
                            "border: none; "
                            "padding: 10px 20px; "
                            "border-radius: 3px; "
                            "} "
                            "QPushButton:hover { background-color: #357ABD; }");
    connect(m_saveBtn, &QPushButton::clicked, this, &AddLeagueDialog::onSaveClicked);
    buttonLayout->addWidget(m_saveBtn);
    
    m_mainLayout->addLayout(buttonLayout);
    
    m_nameEdit->setFocus();
}

void AddLeagueDialog::populateFields(const LeagueData &league)
{
    m_nameEdit->setText(league.name);
    // Note: LeagueData from DatabaseManager doesn't have these fields, using defaults
    m_startDateEdit->setDate(QDate::currentDate());
    m_endDateEdit->setDate(QDate::currentDate().addDays(112));
    m_weeksSpinBox->setValue(16);
    m_statusCombo->setCurrentText("Scheduled");
}

LeagueData AddLeagueDialog::getLeagueData() const
{
    LeagueData league;
    league.name = m_nameEdit->text().trimmed();
    league.teamCount = 0; // Will be set when teams are added
    // Note: DatabaseManager's LeagueData doesn't have startDate, endDate, etc.
    // Using only the fields that exist
    return league;
}

void AddLeagueDialog::onSaveClicked()
{
    if (validateInput()) {
        accept();
    }
}

void AddLeagueDialog::onCancelClicked()
{
    reject();
}

bool AddLeagueDialog::validateInput()
{
    if (m_nameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "League name is required.");
        m_nameEdit->setFocus();
        return false;
    }
    return true;
}