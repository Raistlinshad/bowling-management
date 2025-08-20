#include "TeamManagementDialog.h"
#include "MainWindow.h"
#include "BowlerManagementDialog.h"
#include <QDateTime>

TeamManagementDialog::TeamManagementDialog(MainWindow *mainWindow, QWidget *parent)
    : QDialog(parent)
    , m_mainWindow(mainWindow)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
{
    setupUI();
    setWindowTitle("Team Management");
    setModal(true);
    resize(1000, 700);
    
    // Center the dialog
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // Load initial data
    updateTeamsList();
}

void TeamManagementDialog::setupUI()
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

void TeamManagementDialog::setupLeftPane()
{
    m_leftFrame = new QFrame;
    m_leftFrame->setFrameStyle(QFrame::StyledPanel);
    m_leftFrame->setStyleSheet("QFrame { background-color: #2a2a2a; border: 1px solid #333; }");
    
    m_leftLayout = new QVBoxLayout(m_leftFrame);
    m_leftLayout->setContentsMargins(10, 10, 10, 10);
    
    // Title
    QLabel *teamsLabel = new QLabel("TEAMS");
    teamsLabel->setAlignment(Qt::AlignCenter);
    teamsLabel->setStyleSheet("QLabel { "
                             "color: white; "
                             "font-size: 14px; "
                             "font-weight: bold; "
                             "margin-bottom: 10px; "
                             "}");
    m_leftLayout->addWidget(teamsLabel);
    
    // Search section
    QLabel *searchLabel = new QLabel("Search Teams:");
    searchLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
    m_leftLayout->addWidget(searchLabel);
    
    m_teamSearchEdit = new QLineEdit;
    m_teamSearchEdit->setPlaceholderText("Enter team name...");
    m_teamSearchEdit->setStyleSheet("QLineEdit { "
                                   "background-color: #333; "
                                   "color: white; "
                                   "border: 1px solid #555; "
                                   "padding: 5px; "
                                   "border-radius: 3px; "
                                   "}");
    connect(m_teamSearchEdit, &QLineEdit::textChanged, this, &TeamManagementDialog::onSearchTextChanged);
    m_leftLayout->addWidget(m_teamSearchEdit);
    
    // Teams list
    m_teamsList = new QListWidget;
    m_teamsList->setStyleSheet("QListWidget { "
                              "background-color: #1a1a1a; "
                              "color: white; "
                              "border: 1px solid #333; "
                              "selection-background-color: #4A90E2; "
                              "font-size: 12px; "
                              "}");
    connect(m_teamsList, &QListWidget::currentRowChanged, 
            this, &TeamManagementDialog::onTeamSelectionChanged);
    m_leftLayout->addWidget(m_teamsList);
    
    m_splitter->addWidget(m_leftFrame);
}

void TeamManagementDialog::setupMiddlePane()
{
    m_middleFrame = new QFrame;
    m_middleFrame->setFrameStyle(QFrame::StyledPanel);
    m_middleFrame->setStyleSheet("QFrame { background-color: #2a2a2a; border: 1px solid #333; }");
    
    m_middleLayout = new QVBoxLayout(m_middleFrame);
    m_middleLayout->setContentsMargins(10, 10, 10, 10);
    
    // Title
    QLabel *bowlersLabel = new QLabel("TEAM BOWLERS");
    bowlersLabel->setAlignment(Qt::AlignCenter);
    bowlersLabel->setStyleSheet("QLabel { "
                               "color: white; "
                               "font-size: 14px; "
                               "font-weight: bold; "
                               "margin-bottom: 10px; "
                               "}");
    m_middleLayout->addWidget(bowlersLabel);
    
    // Bowlers tree
    m_bowlersTree = new QTreeWidget;
    m_bowlersTree->setStyleSheet("QTreeWidget { "
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
    headers << "Name" << "Average" << "Phone" << "Status";
    m_bowlersTree->setHeaderLabels(headers);
    m_bowlersTree->header()->setStretchLastSection(false);
    m_bowlersTree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_bowlersTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_bowlersTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    m_bowlersTree->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents);
    
    m_middleLayout->addWidget(m_bowlersTree);
    
    // Bowler management buttons
    QHBoxLayout *bowlerButtonsLayout = new QHBoxLayout;
    
    m_addBowlersBtn = new QPushButton("Add Bowlers");
    m_addBowlersBtn->setStyleSheet("QPushButton { "
                                  "background-color: #4A90E2; "
                                  "color: white; "
                                  "border: none; "
                                  "padding: 6px 12px; "
                                  "border-radius: 3px; "
                                  "} "
                                  "QPushButton:hover { background-color: #357ABD; } "
                                  "QPushButton:disabled { background-color: #666; }");
    m_addBowlersBtn->setEnabled(false);
    connect(m_addBowlersBtn, &QPushButton::clicked, this, &TeamManagementDialog::onAddBowlersClicked);
    bowlerButtonsLayout->addWidget(m_addBowlersBtn);
    
    m_removeBowlersBtn = new QPushButton("Remove Selected");
    m_removeBowlersBtn->setStyleSheet("QPushButton { "
                                     "background-color: #E74C3C; "
                                     "color: white; "
                                     "border: none; "
                                     "padding: 6px 12px; "
                                     "border-radius: 3px; "
                                     "} "
                                     "QPushButton:hover { background-color: #C0392B; } "
                                     "QPushButton:disabled { background-color: #666; }");
    m_removeBowlersBtn->setEnabled(false);
    connect(m_removeBowlersBtn, &QPushButton::clicked, this, &TeamManagementDialog::onRemoveBowlersClicked);
    bowlerButtonsLayout->addWidget(m_removeBowlersBtn);
    
    bowlerButtonsLayout->addStretch();
    m_middleLayout->addLayout(bowlerButtonsLayout);
    
    m_splitter->addWidget(m_middleFrame);
}

void TeamManagementDialog::setupRightPane()
{
    m_rightFrame = new QFrame;
    m_rightFrame->setFrameStyle(QFrame::StyledPanel);
    m_rightFrame->setStyleSheet("QFrame { background-color: #2a2a2a; border: 1px solid #333; }");
    
    m_rightLayout = new QVBoxLayout(m_rightFrame);
    m_rightLayout->setContentsMargins(10, 10, 10, 10);
    
    // Actions group
    m_actionsGroup = new QGroupBox("TEAM ACTIONS");
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
    
    m_addTeamBtn = new QPushButton("Create New Team");
    m_addTeamBtn->setStyleSheet(buttonStyle);
    connect(m_addTeamBtn, &QPushButton::clicked, this, &TeamManagementDialog::onAddTeamClicked);
    actionsLayout->addWidget(m_addTeamBtn);
    
    m_editTeamBtn = new QPushButton("Edit Team Name");
    m_editTeamBtn->setStyleSheet(buttonStyle);
    m_editTeamBtn->setEnabled(false);
    connect(m_editTeamBtn, &QPushButton::clicked, this, &TeamManagementDialog::onEditTeamClicked);
    actionsLayout->addWidget(m_editTeamBtn);
    
    m_deleteTeamBtn = new QPushButton("Delete Team");
    m_deleteTeamBtn->setStyleSheet(buttonStyle);
    m_deleteTeamBtn->setEnabled(false);
    connect(m_deleteTeamBtn, &QPushButton::clicked, this, &TeamManagementDialog::onDeleteTeamClicked);
    actionsLayout->addWidget(m_deleteTeamBtn);
    
    m_teamInfoBtn = new QPushButton("Team Statistics");
    m_teamInfoBtn->setStyleSheet(buttonStyle);
    m_teamInfoBtn->setEnabled(false);
    connect(m_teamInfoBtn, &QPushButton::clicked, this, &TeamManagementDialog::onTeamInfoClicked);
    actionsLayout->addWidget(m_teamInfoBtn);
    
    m_rightLayout->addWidget(m_actionsGroup);
    
    // Details group
    m_detailsGroup = new QGroupBox("Team Details");
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
    
    m_detailsLabel = new QLabel("Select a team to view details");
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
    connect(m_closeBtn, &QPushButton::clicked, this, &TeamManagementDialog::onCloseClicked);
    
    m_rightLayout->addStretch();
    m_rightLayout->addWidget(m_closeBtn);
    
    m_splitter->addWidget(m_rightFrame);
}

void TeamManagementDialog::onTeamSelectionChanged()
{
    int currentRow = m_teamsList->currentRow();
    bool hasSelection = currentRow >= 0;
    
    m_editTeamBtn->setEnabled(hasSelection);
    m_deleteTeamBtn->setEnabled(hasSelection);
    m_addBowlersBtn->setEnabled(hasSelection);
    m_removeBowlersBtn->setEnabled(hasSelection);
    m_teamInfoBtn->setEnabled(hasSelection);
    
    updateTeamBowlers();
    updateTeamDetails();
}

void TeamManagementDialog::onAddTeamClicked()
{
    showAddTeamDialog();
}

void TeamManagementDialog::onEditTeamClicked()
{
    showEditTeamDialog();
}

void TeamManagementDialog::onDeleteTeamClicked()
{
    TeamData team = getSelectedTeam();
    if (team.id <= 0) return;
    
    int ret = QMessageBox::question(this, "Confirm Deletion",
                                   QString("Are you sure you want to delete team '%1'?\n\n"
                                          "This will remove all bowler associations and league assignments.")
                                   .arg(team.name),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // Remove from sample data
        for (int i = 0; i < m_teams.size(); ++i) {
            if (m_teams[i].id == team.id) {
                m_teams.removeAt(i);
                break;
            }
        }
        
        QMessageBox::information(this, "Success", 
                               QString("Team '%1' deleted successfully.").arg(team.name));
        updateTeamsList();
    }
}

void TeamManagementDialog::onAddBowlersClicked()
{
    TeamData team = getSelectedTeam();
    if (team.id <= 0) return;
    
    showAddBowlersDialog();
}

void TeamManagementDialog::onRemoveBowlersClicked()
{
    QMessageBox::information(this, "Remove Bowlers", "Remove bowlers functionality coming soon.");
}

void TeamManagementDialog::onTeamInfoClicked()
{
    TeamData team = getSelectedTeam();
    if (team.id <= 0) return;
    
    QMessageBox::information(this, "Team Statistics", 
                           QString("Detailed statistics for team '%1' coming soon.\n\n"
                                  "This will show:\n"
                                  "- Team average\n"
                                  "- Win/Loss record\n"
                                  "- Individual bowler stats\n"
                                  "- Game history")
                           .arg(team.name));
}

void TeamManagementDialog::onSearchTextChanged(const QString &text)
{
    Q_UNUSED(text)
    updateTeamsList();
}

void TeamManagementDialog::onCloseClicked()
{
    close();
}

void TeamManagementDialog::updateTeamsList()
{
    QString searchFilter = m_teamSearchEdit->text();
    
    m_teamsList->clear();
    m_teamIdMap.clear();
    
    int index = 0;
    for (const TeamData &team : m_teams) {
        // Apply search filter
        if (!searchFilter.isEmpty() && 
            !team.name.contains(searchFilter, Qt::CaseInsensitive)) {
            continue;
        }
        
        QString listItem = QString("%1 (%2 bowlers)").arg(team.name).arg(team.bowlerCount);
        m_teamsList->addItem(listItem);
        m_teamIdMap[index] = team.id;
        index++;
    }
}

void TeamManagementDialog::updateTeamBowlers()
{
    m_bowlersTree->clear();
    
    TeamData team = getSelectedTeam();
    if (team.id <= 0) return;
    
    // Show bowlers for the selected team
    for (const QString &bowlerName : team.bowlers) {
        QTreeWidgetItem *item = new QTreeWidgetItem(m_bowlersTree);
        
        // Find matching bowler data for additional info including ID
        QString firstName = bowlerName.split(" ").first();
        QString lastName = bowlerName.split(" ").last();
        
        bool found = false;
        for (const BowlerData &bowler : m_sampleBowlers) {
            if (bowler.firstName == firstName && bowler.lastName == lastName) {
                item->setText(0, QString("ID: %1 - %2").arg(bowler.id).arg(bowlerName)); // Show ID with name
                item->setText(1, QString::number(bowler.average));
                item->setText(2, bowler.phone);
                item->setText(3, "Active");
                found = true;
                break;
            }
        }
        
        // Default values if not found
        if (!found) {
            item->setText(0, QString("Unknown ID - %1").arg(bowlerName));
            item->setText(1, "N/A");
            item->setText(2, "N/A");
            item->setText(3, "Active");
        }
    }
}

void TeamManagementDialog::updateTeamDetails()
{
    TeamData team = getSelectedTeam();
    if (team.id <= 0) {
        m_detailsLabel->setText("Select a team to view details");
        return;
    }
    
    QString details = QString("Team: %1\n\n").arg(team.name);
    details += QString("Team ID: %1\n").arg(team.id);
    details += QString("Number of Bowlers: %1\n").arg(team.bowlerCount);
    details += QString("Created: %1\n\n").arg(team.createdAt);
    
    if (team.bowlerCount == 0) {
        details += "No bowlers assigned to this team.\n";
        details += "Use 'Add Bowlers' to assign bowlers.";
    } else {
        details += "Team Members:\n";
        for (const QString &bowlerName : team.bowlers) {
            details += QString("• %1\n").arg(bowlerName);
        }
        details += "\nBowler details shown in the middle panel.";
    }
    
    m_detailsLabel->setText(details);
}

void TeamManagementDialog::showAddTeamDialog()
{
    AddTeamDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString teamName = dialog.getTeamName();
        
        // Create new team
        TeamData newTeam;
        newTeam.id = m_teams.size() + 1; // Simple ID assignment
        newTeam.name = teamName;
        newTeam.bowlerCount = 0;
        newTeam.createdAt = QDateTime::currentDateTime().toString("yyyy-MM-dd");
        
        m_teams.append(newTeam);
        
        QMessageBox::information(this, "Success", 
                               QString("Team '%1' created successfully.").arg(teamName));
        updateTeamsList();
    }
}

void TeamManagementDialog::showEditTeamDialog()
{
    TeamData team = getSelectedTeam();
    if (team.id <= 0) return;
    
    AddTeamDialog dialog(team, this);
    dialog.setWindowTitle("Edit Team");
    
    if (dialog.exec() == QDialog::Accepted) {
        QString newTeamName = dialog.getTeamName();
        
        // Update team in sample data
        for (int i = 0; i < m_teams.size(); ++i) {
            if (m_teams[i].id == team.id) {
                m_teams[i].name = newTeamName;
                break;
            }
        }
        
        QMessageBox::information(this, "Success", 
                               QString("Team updated to '%1' successfully.").arg(newTeamName));
        updateTeamsList();
    }
}

void TeamManagementDialog::showAddBowlersDialog()
{
    TeamData team = getSelectedTeam();
    if (team.id <= 0) return;
    
    // Get currently assigned bowlers to exclude them from selection
    QVector<BowlerInfo> excludeBowlers;
    for (const QString &bowlerName : team.bowlers) {
        // Parse the name to find the bowler ID
        QStringList nameParts = bowlerName.split(" ");
        if (nameParts.size() >= 2) {
            QString firstName = nameParts[0];
            QString lastName = nameParts.last();
            
            // Find the bowler in our sample data
            for (const BowlerData &bowler : m_sampleBowlers) {
                if (bowler.firstName == firstName && bowler.lastName == lastName) {
                    BowlerInfo bowlerInfo;
                    bowlerInfo.id = bowler.id;
                    bowlerInfo.firstName = bowler.firstName;
                    bowlerInfo.lastName = bowler.lastName;
                    bowlerInfo.sex = bowler.sex;
                    bowlerInfo.average = bowler.average;
                    bowlerInfo.over18 = bowler.over18;
                    bowlerInfo.phone = bowler.phone;
                    bowlerInfo.address = bowler.address;
                    excludeBowlers.append(bowlerInfo);
                    break;
                }
            }
        }
    }
    
    // Use the new bowler selection dialog
    QVector<BowlerInfo> selectedBowlers = BowlerManagementDialog::selectBowlersDialog(excludeBowlers, this);
    
    if (!selectedBowlers.isEmpty()) {
        // Add selected bowlers to the team
        for (int i = 0; i < m_teams.size(); ++i) {
            if (m_teams[i].id == team.id) {
                for (const BowlerInfo &bowler : selectedBowlers) {
                    QString fullName = QString("%1 %2").arg(bowler.firstName).arg(bowler.lastName);
                    if (!m_teams[i].bowlers.contains(fullName)) {
                        m_teams[i].bowlers.append(fullName);
                        m_teams[i].bowlerCount++;
                    }
                }
                break;
            }
        }
        
        QMessageBox::information(this, "Success", 
                               QString("Added %1 bowlers to team '%2'.")
                               .arg(selectedBowlers.size()).arg(team.name));
        updateTeamBowlers();
        updateTeamDetails();
        updateTeamsList(); // Update the bowler count in the list
    }
}

TeamData TeamManagementDialog::getSelectedTeam()
{
    int currentRow = m_teamsList->currentRow();
    if (currentRow < 0 || !m_teamIdMap.contains(currentRow)) {
        return TeamData(); // Return empty team
    }
    
    int teamId = m_teamIdMap[currentRow];
    for (const TeamData &team : m_teams) {
        if (team.id == teamId) {
            return team;
        }
    }
    
    return TeamData(); // Return empty team if not found
}

// AddTeamDialog Implementation
AddTeamDialog::AddTeamDialog(QWidget *parent)
    : QDialog(parent)
    , m_editMode(false)
{
    setupUI();
    setWindowTitle("Create New Team");
    setModal(true);
    resize(400, 200);
    
    // Center the dialog
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

AddTeamDialog::AddTeamDialog(const TeamData &team, QWidget *parent)
    : QDialog(parent)
    , m_editMode(true)
{
    setupUI();
    m_teamNameEdit->setText(team.name);
    setWindowTitle("Edit Team");
    setModal(true);
    resize(400, 200);
    
    // Center the dialog
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void AddTeamDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    m_mainLayout->setSpacing(15);
    
    // Team name input
    QLabel *nameLabel = new QLabel("Team Name:");
    nameLabel->setStyleSheet("QLabel { color: white; font-weight: bold; font-size: 12px; }");
    m_mainLayout->addWidget(nameLabel);
    
    m_teamNameEdit = new QLineEdit;
    m_teamNameEdit->setPlaceholderText("Enter team name...");
    m_teamNameEdit->setStyleSheet("QLineEdit { "
                                 "background-color: #333; "
                                 "color: white; "
                                 "border: 1px solid #555; "
                                 "padding: 8px; "
                                 "border-radius: 3px; "
                                 "font-size: 12px; "
                                 "}");
    m_mainLayout->addWidget(m_teamNameEdit);
    
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
    connect(m_cancelBtn, &QPushButton::clicked, this, &AddTeamDialog::onCancelClicked);
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
    connect(m_saveBtn, &QPushButton::clicked, this, &AddTeamDialog::onSaveClicked);
    buttonLayout->addWidget(m_saveBtn);
    
    m_mainLayout->addLayout(buttonLayout);
    
    // Focus on team name field
    m_teamNameEdit->setFocus();
}

QString AddTeamDialog::getTeamName() const
{
    return m_teamNameEdit->text().trimmed();
}

void AddTeamDialog::onSaveClicked()
{
    if (validateInput()) {
        accept();
    }
}

void AddTeamDialog::onCancelClicked()
{
    reject();
}

bool AddTeamDialog::validateInput()
{
    QString teamName = m_teamNameEdit->text().trimmed();
    
    if (teamName.isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Team name is required.");
        m_teamNameEdit->setFocus();
        return false;
    }
    
    if (teamName.length() > 50) {
        QMessageBox::warning(this, "Invalid Input", "Team name must be 50 characters or less.");
        m_teamNameEdit->setFocus();
        return false;
    }
    
    return true;
}

// AddBowlersToTeamDialog Implementation
AddBowlersToTeamDialog::AddBowlersToTeamDialog(int teamId, const QVector<BowlerData> &availableBowlers, QWidget *parent)
    : QDialog(parent)
    , m_teamId(teamId)
    , m_bowlers(availableBowlers)
{
    setupUI();
    setWindowTitle("Add Bowlers to Team");
    setModal(true);
    resize(500, 600);
    
    // Center the dialog
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    updateAvailableBowlers();
}

void AddBowlersToTeamDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(10);
    
    // Title
    QLabel *titleLabel = new QLabel("Select Bowlers to Add");
    titleLabel->setStyleSheet("QLabel { "
                             "color: white; "
                             "font-size: 14px; "
                             "font-weight: bold; "
                             "margin-bottom: 10px; "
                             "}");
    titleLabel->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(titleLabel);
    
    // Search section
    m_searchLayout = new QHBoxLayout;
    
    QLabel *searchLabel = new QLabel("Search:");
    searchLabel->setStyleSheet("QLabel { color: white; }");
    m_searchLayout->addWidget(searchLabel);
    
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search bowlers...");
    m_searchEdit->setStyleSheet("QLineEdit { "
                               "background-color: #333; "
                               "color: white; "
                               "border: 1px solid #555; "
                               "padding: 5px; "
                               "border-radius: 3px; "
                               "}");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &AddBowlersToTeamDialog::onSearchTextChanged);
    m_searchLayout->addWidget(m_searchEdit);
    
    m_mainLayout->addLayout(m_searchLayout);
    
    // Available bowlers list
    QLabel *listLabel = new QLabel("Available Bowlers:");
    listLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
    m_mainLayout->addWidget(listLabel);
    
    m_availableBowlersList = new QListWidget;
    m_availableBowlersList->setStyleSheet("QListWidget { "
                                         "background-color: #1a1a1a; "
                                         "color: white; "
                                         "border: 1px solid #333; "
                                         "selection-background-color: #4A90E2; "
                                         "}");
    m_availableBowlersList->setSelectionMode(QAbstractItemView::MultiSelection);
    connect(m_availableBowlersList, &QListWidget::itemSelectionChanged, 
            this, &AddBowlersToTeamDialog::updateSelectedCount);
    m_mainLayout->addWidget(m_availableBowlersList);
    
    // Selection buttons
    QHBoxLayout *selectionLayout = new QHBoxLayout;
    
    m_selectAllBtn = new QPushButton("Select All");
    m_selectAllBtn->setStyleSheet("QPushButton { "
                                 "background-color: #4A90E2; "
                                 "color: white; "
                                 "border: none; "
                                 "padding: 6px 12px; "
                                 "border-radius: 3px; "
                                 "} "
                                 "QPushButton:hover { background-color: #357ABD; }");
    connect(m_selectAllBtn, &QPushButton::clicked, this, &AddBowlersToTeamDialog::onSelectAllClicked);
    selectionLayout->addWidget(m_selectAllBtn);
    
    m_deselectAllBtn = new QPushButton("Deselect All");
    m_deselectAllBtn->setStyleSheet("QPushButton { "
                                   "background-color: #666; "
                                   "color: white; "
                                   "border: none; "
                                   "padding: 6px 12px; "
                                   "border-radius: 3px; "
                                   "} "
                                   "QPushButton:hover { background-color: #777; }");
    connect(m_deselectAllBtn, &QPushButton::clicked, this, &AddBowlersToTeamDialog::onDeselectAllClicked);
    selectionLayout->addWidget(m_deselectAllBtn);
    
    selectionLayout->addStretch();
    
    m_selectedCountLabel = new QLabel("Selected: 0");
    m_selectedCountLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
    selectionLayout->addWidget(m_selectedCountLabel);
    
    m_mainLayout->addLayout(selectionLayout);
    
    // Action buttons
    m_buttonLayout = new QHBoxLayout;
    
    m_cancelBtn = new QPushButton("Cancel");
    m_cancelBtn->setStyleSheet("QPushButton { "
                              "background-color: #666; "
                              "color: white; "
                              "border: none; "
                              "padding: 10px 20px; "
                              "border-radius: 3px; "
                              "} "
                              "QPushButton:hover { background-color: #777; }");
    connect(m_cancelBtn, &QPushButton::clicked, this, &AddBowlersToTeamDialog::onCancelClicked);
    m_buttonLayout->addWidget(m_cancelBtn);
    
    m_buttonLayout->addStretch();
    
    m_saveBtn = new QPushButton("Add to Team");
    m_saveBtn->setStyleSheet("QPushButton { "
                            "background-color: #4A90E2; "
                            "color: white; "
                            "border: none; "
                            "padding: 10px 20px; "
                            "border-radius: 3px; "
                            "} "
                            "QPushButton:hover { background-color: #357ABD; } "
                            "QPushButton:disabled { background-color: #666; }");
    m_saveBtn->setEnabled(false);
    connect(m_saveBtn, &QPushButton::clicked, this, &AddBowlersToTeamDialog::onSaveClicked);
    m_buttonLayout->addWidget(m_saveBtn);
    
    m_mainLayout->addLayout(m_buttonLayout);
}

QVector<int> AddBowlersToTeamDialog::getSelectedBowlerIds() const
{
    QVector<int> selectedIds;
    QList<QListWidgetItem*> selectedItems = m_availableBowlersList->selectedItems();
    
    for (QListWidgetItem* item : selectedItems) {
        int row = m_availableBowlersList->row(item);
        if (m_bowlerIdMap.contains(row)) {
            selectedIds.append(m_bowlerIdMap[row]);
        }
    }
    
    return selectedIds;
}

void AddBowlersToTeamDialog::onSearchTextChanged(const QString &text)
{
    Q_UNUSED(text)
    updateAvailableBowlers();
}

void AddBowlersToTeamDialog::onSelectAllClicked()
{
    m_availableBowlersList->selectAll();
}

void AddBowlersToTeamDialog::onDeselectAllClicked()
{
    m_availableBowlersList->clearSelection();
}

void AddBowlersToTeamDialog::onSaveClicked()
{
    QVector<int> selectedIds = getSelectedBowlerIds();
    if (selectedIds.isEmpty()) {
        QMessageBox::warning(this, "No Selection", "Please select at least one bowler to add.");
        return;
    }
    
    accept();
}

void AddBowlersToTeamDialog::onCancelClicked()
{
    reject();
}

void AddBowlersToTeamDialog::updateAvailableBowlers()
{
    QString searchFilter = m_searchEdit->text();
    
    m_availableBowlersList->clear();
    m_bowlerIdMap.clear();
    
    for (int i = 0; i < m_bowlers.size(); ++i) {
        const BowlerData &bowler = m_bowlers[i];
        
        // Apply search filter
        QString fullName = QString("%1 %2").arg(bowler.firstName).arg(bowler.lastName);
        if (!searchFilter.isEmpty() && 
            !fullName.contains(searchFilter, Qt::CaseInsensitive)) {
            continue;
        }
        
        QString displayText = QString("%1 %2 - Avg: %3")
                             .arg(bowler.firstName)
                             .arg(bowler.lastName)
                             .arg(bowler.average > 0 ? QString::number(bowler.average) : "N/A");
        
        m_availableBowlersList->addItem(displayText);
        m_bowlerIdMap[m_availableBowlersList->count() - 1] = bowler.id;
    }
    
    updateSelectedCount();
}

void AddBowlersToTeamDialog::updateSelectedCount()
{
    int selectedCount = m_availableBowlersList->selectedItems().size();
    m_selectedCountLabel->setText(QString("Selected: %1").arg(selectedCount));
    m_saveBtn->setEnabled(selectedCount > 0);
}