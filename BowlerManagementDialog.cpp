#include "BowlerManagementDialog.h"
#include "MainWindow.h"
#include "DatabaseManager.h"
#include <QDateTime>

BowlerManagementDialog::BowlerManagementDialog(MainWindow *mainWindow, QWidget *parent)
    : QDialog(parent)
    , m_mainWindow(mainWindow)
    , m_mainLayout(nullptr)
    , m_splitter(nullptr)
{
    setupUI();
    setWindowTitle("Bowler Management");
    setModal(true);
    resize(1200, 800);
    
    // Center the dialog
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // Load initial data from database
    loadBowlersFromDatabase();
    updateBowlersList();
}

void BowlerManagementDialog::setupUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    
    m_splitter = new QSplitter(Qt::Horizontal);
    m_mainLayout->addWidget(m_splitter);
    
    setupLeftPane();
    setupRightPane();
    
    // Set splitter proportions (3:2 ratio)
    m_splitter->setStretchFactor(0, 3);
    m_splitter->setStretchFactor(1, 2);
}

void BowlerManagementDialog::setupLeftPane()
{
    m_leftFrame = new QFrame;
    m_leftFrame->setFrameStyle(QFrame::StyledPanel);
    m_leftFrame->setStyleSheet("QFrame { background-color: #2a2a2a; border: 1px solid #333; }");
    
    m_leftLayout = new QVBoxLayout(m_leftFrame);
    m_leftLayout->setContentsMargins(10, 10, 10, 10);
    
    // Title
    QLabel *bowlersLabel = new QLabel("BOWLERS DATABASE");
    bowlersLabel->setAlignment(Qt::AlignCenter);
    bowlersLabel->setStyleSheet("QLabel { "
                               "color: white; "
                               "font-size: 14px; "
                               "font-weight: bold; "
                               "margin-bottom: 10px; "
                               "}");
    m_leftLayout->addWidget(bowlersLabel);
    
    // Search section
    QLabel *searchLabel = new QLabel("Search Bowlers:");
    searchLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
    m_leftLayout->addWidget(searchLabel);
    
    m_searchEdit = new QLineEdit;
    m_searchEdit->setPlaceholderText("Search by name, phone, or address...");
    m_searchEdit->setStyleSheet("QLineEdit { "
                               "background-color: #333; "
                               "color: white; "
                               "border: 1px solid #555; "
                               "padding: 5px; "
                               "border-radius: 3px; "
                               "}");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &BowlerManagementDialog::onSearchTextChanged);
    m_leftLayout->addWidget(m_searchEdit);
    
    // Filters section
    QGroupBox *filtersGroup = new QGroupBox("Filters");
    filtersGroup->setStyleSheet("QGroupBox { "
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
    
    QGridLayout *filtersLayout = new QGridLayout(filtersGroup);
    filtersLayout->setSpacing(5);
    
    QString filterLabelStyle = "QLabel { color: white; font-size: 10px; }";
    QString filterComboStyle = "QComboBox, QSpinBox { "
                              "background-color: #333; "
                              "color: white; "
                              "border: 1px solid #555; "
                              "padding: 3px; "
                              "border-radius: 2px; "
                              "font-size: 10px; "
                              "}";
    
    // Sex filter
    QLabel *sexLabel = new QLabel("Sex:");
    sexLabel->setStyleSheet(filterLabelStyle);
    filtersLayout->addWidget(sexLabel, 0, 0);
    
    m_sexFilterCombo = new QComboBox;
    m_sexFilterCombo->setStyleSheet(filterComboStyle);
    m_sexFilterCombo->addItems({"All", "Male", "Female", "Other"});
    connect(m_sexFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BowlerManagementDialog::onFilterChanged);
    filtersLayout->addWidget(m_sexFilterCombo, 0, 1);
    
    // Age filter
    QLabel *ageLabel = new QLabel("Age:");
    ageLabel->setStyleSheet(filterLabelStyle);
    filtersLayout->addWidget(ageLabel, 1, 0);
    
    m_ageFilterCombo = new QComboBox;
    m_ageFilterCombo->setStyleSheet(filterComboStyle);
    m_ageFilterCombo->addItems({"All", "Adult (18+)", "Youth (<18)"});
    connect(m_ageFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &BowlerManagementDialog::onFilterChanged);
    filtersLayout->addWidget(m_ageFilterCombo, 1, 1);
    
    // Average range filters
    QLabel *avgLabel = new QLabel("Average Range:");
    avgLabel->setStyleSheet(filterLabelStyle);
    filtersLayout->addWidget(avgLabel, 2, 0, 1, 2);
    
    QHBoxLayout *avgLayout = new QHBoxLayout;
    
    m_minAverageSpinner = new QSpinBox;
    m_minAverageSpinner->setStyleSheet(filterComboStyle);
    m_minAverageSpinner->setRange(0, 300);
    m_minAverageSpinner->setValue(0);
    m_minAverageSpinner->setSpecialValueText("Min");
    connect(m_minAverageSpinner, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BowlerManagementDialog::onFilterChanged);
    avgLayout->addWidget(m_minAverageSpinner);
    
    QLabel *toLabel = new QLabel(" to ");
    toLabel->setStyleSheet(filterLabelStyle);
    avgLayout->addWidget(toLabel);
    
    m_maxAverageSpinner = new QSpinBox;
    m_maxAverageSpinner->setStyleSheet(filterComboStyle);
    m_maxAverageSpinner->setRange(0, 300);
    m_maxAverageSpinner->setValue(300);
    m_maxAverageSpinner->setSpecialValueText("Max");
    connect(m_maxAverageSpinner, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &BowlerManagementDialog::onFilterChanged);
    avgLayout->addWidget(m_maxAverageSpinner);
    
    filtersLayout->addLayout(avgLayout, 3, 0, 1, 2);
    
    m_leftLayout->addWidget(filtersGroup);
    
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
    headers << "ID" << "Name" << "Sex" << "Average" << "Age" << "Phone";
    m_bowlersTree->setHeaderLabels(headers);
    m_bowlersTree->header()->setStretchLastSection(false);
    m_bowlersTree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents); // ID
    m_bowlersTree->header()->setSectionResizeMode(1, QHeaderView::Stretch);           // Name
    m_bowlersTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Sex
    m_bowlersTree->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Average
    m_bowlersTree->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents); // Age
    m_bowlersTree->header()->setSectionResizeMode(5, QHeaderView::ResizeToContents); // Phone
    
    connect(m_bowlersTree, &QTreeWidget::currentItemChanged,
            this, &BowlerManagementDialog::onBowlerSelectionChanged);
    m_leftLayout->addWidget(m_bowlersTree);
    
    // Refresh button
    m_refreshBtn = new QPushButton("Refresh List");
    m_refreshBtn->setStyleSheet("QPushButton { "
                               "background-color: #4A90E2; "
                               "color: white; "
                               "border: none; "
                               "padding: 6px 12px; "
                               "border-radius: 3px; "
                               "} "
                               "QPushButton:hover { background-color: #357ABD; }");
    connect(m_refreshBtn, &QPushButton::clicked, this, &BowlerManagementDialog::onRefreshClicked);
    m_leftLayout->addWidget(m_refreshBtn);
    
    m_splitter->addWidget(m_leftFrame);
}

void BowlerManagementDialog::setupRightPane()
{
    m_rightFrame = new QFrame;
    m_rightFrame->setFrameStyle(QFrame::StyledPanel);
    m_rightFrame->setStyleSheet("QFrame { background-color: #2a2a2a; border: 1px solid #333; }");
    
    m_rightLayout = new QVBoxLayout(m_rightFrame);
    m_rightLayout->setContentsMargins(10, 10, 10, 10);
    
    // Actions group
    m_actionsGroup = new QGroupBox("BOWLER ACTIONS");
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
    
    m_addBowlerBtn = new QPushButton("Add New Bowler");
    m_addBowlerBtn->setStyleSheet(buttonStyle);
    connect(m_addBowlerBtn, &QPushButton::clicked, this, &BowlerManagementDialog::onAddBowlerClicked);
    actionsLayout->addWidget(m_addBowlerBtn);
    
    m_editBowlerBtn = new QPushButton("Edit Selected Bowler");
    m_editBowlerBtn->setStyleSheet(buttonStyle);
    m_editBowlerBtn->setEnabled(false);
    connect(m_editBowlerBtn, &QPushButton::clicked, this, &BowlerManagementDialog::onEditBowlerClicked);
    actionsLayout->addWidget(m_editBowlerBtn);
    
    m_deleteBowlerBtn = new QPushButton("Delete Selected Bowler");
    m_deleteBowlerBtn->setStyleSheet(buttonStyle);
    m_deleteBowlerBtn->setEnabled(false);
    connect(m_deleteBowlerBtn, &QPushButton::clicked, this, &BowlerManagementDialog::onDeleteBowlerClicked);
    actionsLayout->addWidget(m_deleteBowlerBtn);
    
    m_bowlerInfoBtn = new QPushButton("Detailed Information");
    m_bowlerInfoBtn->setStyleSheet(buttonStyle);
    m_bowlerInfoBtn->setEnabled(false);
    connect(m_bowlerInfoBtn, &QPushButton::clicked, this, &BowlerManagementDialog::onBowlerInfoClicked);
    actionsLayout->addWidget(m_bowlerInfoBtn);
    
    m_rightLayout->addWidget(m_actionsGroup);
    
    // Details group
    m_detailsGroup = new QGroupBox("Bowler Details");
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
                                "background-color: #1a1a1a; "
                                "border: 1px solid #333; "
                                "border-radius: 3px; "
                                "font-size: 11px; "
                                "padding: 10px; "
                                "}");
    m_detailsText->setReadOnly(true);
    m_detailsText->setText("Select a bowler to view details");
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
    connect(m_closeBtn, &QPushButton::clicked, this, &BowlerManagementDialog::onCloseClicked);
    
    m_rightLayout->addStretch();
    m_rightLayout->addWidget(m_closeBtn);
    
    m_splitter->addWidget(m_rightFrame);
}

void BowlerManagementDialog::onBowlerSelectionChanged()
{
    QTreeWidgetItem *currentItem = m_bowlersTree->currentItem();
    bool hasSelection = currentItem != nullptr;
    
    m_editBowlerBtn->setEnabled(hasSelection);
    m_deleteBowlerBtn->setEnabled(hasSelection);
    m_bowlerInfoBtn->setEnabled(hasSelection);
    
    updateBowlerDetails();
}

void BowlerManagementDialog::onAddBowlerClicked()
{
    showAddBowlerDialog();
}

void BowlerManagementDialog::onEditBowlerClicked()
{
    showEditBowlerDialog();
}

void BowlerManagementDialog::onDeleteBowlerClicked()
{
    BowlerInfo bowler = getSelectedBowler();
    if (bowler.id <= 0) return;
    
    int ret = QMessageBox::question(this, "Confirm Deletion",
                                   QString("Are you sure you want to delete bowler '%1 %2'?\n\n"
                                          "This will remove them from all teams and leagues.")
                                   .arg(bowler.firstName).arg(bowler.lastName),
                                   QMessageBox::Yes | QMessageBox::No);
    
    if (ret == QMessageBox::Yes) {
        // Remove from database
        DatabaseManager* db = DatabaseManager::instance();
        if (db->deleteBowler(bowler.id)) {
            QMessageBox::information(this, "Success", 
                                   QString("Bowler '%1 %2' deleted successfully.")
                                   .arg(bowler.firstName).arg(bowler.lastName));
            loadBowlersFromDatabase(); // Reload from database
            updateBowlersList();
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete bowler from database.");
        }
    }
}

void BowlerManagementDialog::onBowlerInfoClicked()
{
    BowlerInfo bowler = getSelectedBowler();
    if (bowler.id <= 0) return;
    
    QString info = QString("Detailed Information for %1 %2\n\n").arg(bowler.firstName).arg(bowler.lastName);
    info += QString("Bowler ID: %1\n").arg(bowler.id);
    info += QString("Sex: %1\n").arg(bowler.sex);
    info += QString("Average: %1\n").arg(bowler.average > 0 ? QString::number(bowler.average) : "N/A");
    info += QString("Birthday: %1\n").arg(bowler.birthday.isEmpty() ? "Not provided" : bowler.birthday);
    info += QString("Age Category: %1\n").arg(bowler.over18 ? "Adult (18+)" : "Youth (<18)");
    info += QString("Phone: %1\n").arg(bowler.phone.isEmpty() ? "Not provided" : bowler.phone);
    info += QString("Address: %1\n\n").arg(bowler.address.isEmpty() ? "Not provided" : bowler.address);
    
    // Get team memberships from database
    DatabaseManager* db = DatabaseManager::instance();
    QVector<TeamData> teams = db->getAllTeams();
    QStringList teamNames;
    for (const TeamData& team : teams) {
        // Check if bowler is in this team (you'll need to implement this in DatabaseManager)
        // For now, just show placeholder
    }
    
    info += "Team Memberships: ";
    if (teamNames.isEmpty()) {
        info += "Not assigned to any teams\n";
    } else {
        info += teamNames.join(", ") + "\n";
    }
    
    info += "\nAdditional features:\n";
    info += "• Game history and statistics\n";
    info += "• Performance trends\n";
    info += "• League participation";
    
    QMessageBox::information(this, QString("Bowler Information - %1 %2").arg(bowler.firstName).arg(bowler.lastName), info);
}

void BowlerManagementDialog::onSearchTextChanged(const QString &text)
{
    Q_UNUSED(text)
    updateBowlersList();
}

void BowlerManagementDialog::onFilterChanged()
{
    updateBowlersList();
}

void BowlerManagementDialog::onCloseClicked()
{
    close();
}

void BowlerManagementDialog::onRefreshClicked()
{
    loadBowlersFromDatabase();
    updateBowlersList();
    QMessageBox::information(this, "Refreshed", "Bowler list has been refreshed from the database.");
}

void BowlerManagementDialog::updateBowlersList()
{
    QString searchFilter = m_searchEdit->text().toLower();
    QString sexFilter = m_sexFilterCombo->currentText();
    QString ageFilter = m_ageFilterCombo->currentText();
    int minAverage = m_minAverageSpinner->value();
    int maxAverage = m_maxAverageSpinner->value();
    
    m_bowlersTree->clear();
    m_bowlerIdMap.clear();
    
    int rowIndex = 0;
    for (const BowlerInfo &bowler : m_bowlers) {
        // Apply search filter
        if (!searchFilter.isEmpty()) {
            QString fullName = QString("%1 %2").arg(bowler.firstName).arg(bowler.lastName).toLower();
            QString phone = bowler.phone.toLower();
            QString address = bowler.address.toLower();
            
            if (!fullName.contains(searchFilter) && 
                !phone.contains(searchFilter) && 
                !address.contains(searchFilter)) {
                continue;
            }
        }
        
        // Apply sex filter
        if (sexFilter != "All" && bowler.sex != sexFilter) {
            continue;
        }
        
        // Apply age filter
        if (ageFilter != "All") {
            if ((ageFilter == "Adult (18+)" && !bowler.over18) ||
                (ageFilter == "Youth (<18)" && bowler.over18)) {
                continue;
            }
        }
        
        // Apply average filter
        if (bowler.average < minAverage || bowler.average > maxAverage) {
            continue;
        }
        
        QTreeWidgetItem *item = new QTreeWidgetItem(m_bowlersTree);
        item->setText(0, QString::number(bowler.id));
        item->setText(1, QString("%1 %2").arg(bowler.firstName).arg(bowler.lastName));
        item->setText(2, bowler.sex);
        item->setText(3, bowler.average > 0 ? QString::number(bowler.average) : "N/A");
        item->setText(4, bowler.over18 ? "Adult" : "Youth");
        item->setText(5, bowler.phone.isEmpty() ? "N/A" : bowler.phone);
        
        m_bowlerIdMap[rowIndex] = bowler.id;
        rowIndex++;
    }
    
    // Update details if current selection is no longer valid
    if (!m_bowlersTree->currentItem()) {
        updateBowlerDetails();
    }
}

void BowlerManagementDialog::updateBowlerDetails()
{
    BowlerInfo bowler = getSelectedBowler();
    if (bowler.id <= 0) {
        m_detailsText->setText("Select a bowler to view details");
        return;
    }
    
    QString details = QString("<b>%1 %2</b> (ID: %3)<br><br>").arg(bowler.firstName).arg(bowler.lastName).arg(bowler.id);
    details += QString("<b>Sex:</b> %1<br>").arg(bowler.sex);
    details += QString("<b>Average:</b> %1<br>").arg(bowler.average > 0 ? QString::number(bowler.average) : "Not set");
    details += QString("<b>Birthday:</b> %1<br>").arg(bowler.birthday.isEmpty() ? "Not provided" : bowler.birthday);
    details += QString("<b>Age Category:</b> %1<br>").arg(bowler.over18 ? "Adult (18+)" : "Youth (<18)");
    details += QString("<b>Phone:</b> %1<br>").arg(bowler.phone.isEmpty() ? "Not provided" : bowler.phone);
    details += QString("<b>Address:</b> %1<br><br>").arg(bowler.address.isEmpty() ? "Not provided" : bowler.address);
    
    details += "<b>Teams:</b> ";
    if (bowler.teams.isEmpty()) {
        details += "Not assigned to any teams<br>";
    } else {
        details += bowler.teams.join(", ") + "<br>";
    }
    
    details += "<br><i>Use the action buttons to edit or get detailed information.</i>";
    
    m_detailsText->setHtml(details);
}

void BowlerManagementDialog::showAddBowlerDialog()
{
    NewBowlerDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        BowlerInfo newBowler = dialog.getBowlerInfo();
        
        // Create BowlerData object for database
        BowlerData bowlerData;
        bowlerData.firstName = newBowler.firstName;
        bowlerData.lastName = newBowler.lastName;
        bowlerData.sex = newBowler.sex;
        bowlerData.average = newBowler.average;
        bowlerData.birthday = newBowler.birthday;
        bowlerData.over18 = newBowler.over18;
        bowlerData.phone = newBowler.phone;
        bowlerData.address = newBowler.address;
        
        // Save to database
        DatabaseManager* db = DatabaseManager::instance();
        int newId = db->addBowler(bowlerData);
        
        if (newId > 0) {
            QMessageBox::information(this, "Success", 
                                   QString("Bowler '%1 %2' added successfully with ID %3.")
                                   .arg(newBowler.firstName).arg(newBowler.lastName).arg(newId));
            loadBowlersFromDatabase(); // Reload from database
            updateBowlersList();
        } else {
            QMessageBox::warning(this, "Error", "Failed to add bowler to database.");
        }
    }
}

void BowlerManagementDialog::showEditBowlerDialog()
{
    BowlerInfo bowler = getSelectedBowler();
    if (bowler.id <= 0) return;
    
    NewBowlerDialog dialog(bowler, this);
    if (dialog.exec() == QDialog::Accepted) {
        BowlerInfo updatedBowler = dialog.getBowlerInfo();
        updatedBowler.id = bowler.id; // Keep the same ID
        
        // Create BowlerData object for database
        BowlerData bowlerData;
        bowlerData.id = updatedBowler.id;
        bowlerData.firstName = updatedBowler.firstName;
        bowlerData.lastName = updatedBowler.lastName;
        bowlerData.sex = updatedBowler.sex;
        bowlerData.average = updatedBowler.average;
        bowlerData.birthday = updatedBowler.birthday;
        bowlerData.over18 = updatedBowler.over18;
        bowlerData.phone = updatedBowler.phone;
        bowlerData.address = updatedBowler.address;
        
        // Update in database
        DatabaseManager* db = DatabaseManager::instance();
        if (db->updateBowler(bowlerData)) {
            QMessageBox::information(this, "Success", 
                                   QString("Bowler '%1 %2' updated successfully.")
                                   .arg(updatedBowler.firstName).arg(updatedBowler.lastName));
            loadBowlersFromDatabase(); // Reload from database
            updateBowlersList();
        } else {
            QMessageBox::warning(this, "Error", "Failed to update bowler in database.");
        }
    }
}

BowlerInfo BowlerManagementDialog::getSelectedBowler()
{
    QTreeWidgetItem *currentItem = m_bowlersTree->currentItem();
    if (!currentItem) return BowlerInfo();
    
    int currentRow = m_bowlersTree->indexOfTopLevelItem(currentItem);
    if (!m_bowlerIdMap.contains(currentRow)) return BowlerInfo();
    
    int bowlerId = m_bowlerIdMap[currentRow];
    for (const BowlerInfo &bowler : m_bowlers) {
        if (bowler.id == bowlerId) {
            return bowler;
        }
    }
    
    return BowlerInfo();
}

void BowlerManagementDialog::loadBowlersFromDatabase()
{
    DatabaseManager* db = DatabaseManager::instance();
    QVector<BowlerData> dbBowlers = db->getAllBowlers();
    
    m_bowlers.clear();
    
    // Convert BowlerData to BowlerInfo
    for (const BowlerData& dbBowler : dbBowlers) {
        BowlerInfo bowler;
        bowler.id = dbBowler.id;
        bowler.firstName = dbBowler.firstName;
        bowler.lastName = dbBowler.lastName;
        bowler.sex = dbBowler.sex;
        bowler.average = dbBowler.average;
        bowler.birthday = dbBowler.birthday;
        bowler.over18 = dbBowler.over18;
        bowler.phone = dbBowler.phone;
        bowler.address = dbBowler.address;
        
        // TODO: Load team memberships from database
        // bowler.teams = db->getBowlerTeams(bowler.id);
        
        m_bowlers.append(bowler);
    }
}

// Static method for selecting bowlers
QVector<BowlerInfo> BowlerManagementDialog::selectBowlersDialog(const QVector<BowlerInfo> &excludeBowlers, QWidget *parent)
{
    // Load all bowlers from database
    DatabaseManager* db = DatabaseManager::instance();
    QVector<BowlerData> dbBowlers = db->getAllBowlers();

    QVector<BowlerInfo> allBowlers;
    
    // Convert BowlerData to BowlerInfo
    for (const BowlerData& dbBowler : dbBowlers) {
        BowlerInfo bowler;
        bowler.id = dbBowler.id;
        bowler.firstName = dbBowler.firstName;
        bowler.lastName = dbBowler.lastName;
        bowler.sex = dbBowler.sex;
        bowler.average = dbBowler.average;
        bowler.birthday = dbBowler.birthday;
        bowler.over18 = dbBowler.over18;
        bowler.phone = dbBowler.phone;
        bowler.address = dbBowler.address;
        
        allBowlers.append(bowler);
    }
    
    BowlerSelectionDialog dialog(allBowlers, excludeBowlers, parent);
    if (dialog.exec() == QDialog::Accepted) {
        return dialog.getSelectedBowlers();
    }
    
    return QVector<BowlerInfo>();
}

// BowlerSelectionDialog Implementation
BowlerSelectionDialog::BowlerSelectionDialog(const QVector<BowlerInfo> &availableBowlers, 
                                           const QVector<BowlerInfo> &excludeBowlers,
                                           QWidget *parent)
    : QDialog(parent)
    , m_allBowlers(availableBowlers)
    , m_excludeBowlers(excludeBowlers)
{
    setupUI();
    setWindowTitle("Select Bowlers");
    setModal(true);
    resize(600, 700);
    
    // Center the dialog
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    updateAvailableBowlers();
}

void BowlerSelectionDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(10);
    
    // Title
    QLabel *titleLabel = new QLabel("Select Bowlers");
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
    m_searchEdit->setPlaceholderText("Search by name, average, or phone...");
    m_searchEdit->setStyleSheet("QLineEdit { "
                               "background-color: #333; "
                               "color: white; "
                               "border: 1px solid #555; "
                               "padding: 5px; "
                               "border-radius: 3px; "
                               "}");
    connect(m_searchEdit, &QLineEdit::textChanged, this, &BowlerSelectionDialog::onSearchTextChanged);
    m_searchLayout->addWidget(m_searchEdit);
    
    m_mainLayout->addLayout(m_searchLayout);
    
    // Available bowlers tree
    QLabel *listLabel = new QLabel("Available Bowlers:");
    listLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
    m_mainLayout->addWidget(listLabel);
    
    m_availableBowlersTree = new QTreeWidget;
    m_availableBowlersTree->setStyleSheet("QTreeWidget { "
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
    headers << "ID" << "Name" << "Sex" << "Average" << "Phone";
    m_availableBowlersTree->setHeaderLabels(headers);
    m_availableBowlersTree->header()->setStretchLastSection(false);
    m_availableBowlersTree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents); // ID
    m_availableBowlersTree->header()->setSectionResizeMode(1, QHeaderView::Stretch);           // Name
    m_availableBowlersTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents); // Sex
    m_availableBowlersTree->header()->setSectionResizeMode(3, QHeaderView::ResizeToContents); // Average
    m_availableBowlersTree->header()->setSectionResizeMode(4, QHeaderView::ResizeToContents); // Phone
    
    m_availableBowlersTree->setSelectionMode(QAbstractItemView::MultiSelection);
    connect(m_availableBowlersTree, &QTreeWidget::itemSelectionChanged, 
            this, &BowlerSelectionDialog::updateSelectedCount);
    m_mainLayout->addWidget(m_availableBowlersTree);
    
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
    connect(m_selectAllBtn, &QPushButton::clicked, this, &BowlerSelectionDialog::onSelectAllClicked);
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
    connect(m_deselectAllBtn, &QPushButton::clicked, this, &BowlerSelectionDialog::onDeselectAllClicked);
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
    connect(m_cancelBtn, &QPushButton::clicked, this, &BowlerSelectionDialog::onCancelClicked);
    m_buttonLayout->addWidget(m_cancelBtn);
    
    m_buttonLayout->addStretch();
    
    m_saveBtn = new QPushButton("Add Selected");
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
    connect(m_saveBtn, &QPushButton::clicked, this, &BowlerSelectionDialog::onSaveClicked);
    m_buttonLayout->addWidget(m_saveBtn);
    
    m_mainLayout->addLayout(m_buttonLayout);
}

QVector<BowlerInfo> BowlerSelectionDialog::getSelectedBowlers() const
{
    QVector<BowlerInfo> selectedBowlers;
    QList<QTreeWidgetItem*> selectedItems = m_availableBowlersTree->selectedItems();
    
    for (QTreeWidgetItem* item : selectedItems) {
        int row = m_availableBowlersTree->indexOfTopLevelItem(item);
        if (m_bowlerIdMap.contains(row)) {
            int bowlerId = m_bowlerIdMap[row];
            for (const BowlerInfo &bowler : m_allBowlers) {
                if (bowler.id == bowlerId) {
                    selectedBowlers.append(bowler);
                    break;
                }
            }
        }
    }
    
    return selectedBowlers;
}

void BowlerSelectionDialog::onSearchTextChanged(const QString &text)
{
    Q_UNUSED(text)
    updateAvailableBowlers();
}

void BowlerSelectionDialog::onSelectAllClicked()
{
    m_availableBowlersTree->selectAll();
}

void BowlerSelectionDialog::onDeselectAllClicked()
{
    m_availableBowlersTree->clearSelection();
}

void BowlerSelectionDialog::onSaveClicked()
{
    QVector<BowlerInfo> selectedBowlers = getSelectedBowlers();
    if (selectedBowlers.isEmpty()) {
        QMessageBox::warning(this, "No Selection", "Please select at least one bowler.");
        return;
    }
    
    accept();
}

void BowlerSelectionDialog::onCancelClicked()
{
    reject();
}

void BowlerSelectionDialog::updateAvailableBowlers()
{
    QString searchFilter = m_searchEdit->text().toLower();
    
    m_availableBowlersTree->clear();
    m_bowlerIdMap.clear();
    
    // Create set of excluded bowler IDs for fast lookup
    QSet<int> excludedIds;
    for (const BowlerInfo &bowler : m_excludeBowlers) {
        excludedIds.insert(bowler.id);
    }
    
    int rowIndex = 0;
    for (const BowlerInfo &bowler : m_allBowlers) {
        // Skip excluded bowlers
        if (excludedIds.contains(bowler.id)) {
            continue;
        }
        
        // Apply search filter
        if (!searchFilter.isEmpty()) {
            QString fullName = QString("%1 %2").arg(bowler.firstName).arg(bowler.lastName).toLower();
            QString phone = bowler.phone.toLower();
            QString average = QString::number(bowler.average);
            
            if (!fullName.contains(searchFilter) && 
                !phone.contains(searchFilter) && 
                !average.contains(searchFilter)) {
                continue;
            }
        }
        
        QTreeWidgetItem *item = new QTreeWidgetItem(m_availableBowlersTree);
        item->setText(0, QString::number(bowler.id));
        item->setText(1, QString("%1 %2").arg(bowler.firstName).arg(bowler.lastName));
        item->setText(2, bowler.sex);
        item->setText(3, bowler.average > 0 ? QString::number(bowler.average) : "N/A");
        item->setText(4, bowler.phone.isEmpty() ? "N/A" : bowler.phone);
        
        m_bowlerIdMap[rowIndex] = bowler.id;
        rowIndex++;
    }
    
    updateSelectedCount();
}

void BowlerSelectionDialog::updateSelectedCount()
{
    int selectedCount = m_availableBowlersTree->selectedItems().size();
    m_selectedCountLabel->setText(QString("Selected: %1").arg(selectedCount));
    m_saveBtn->setEnabled(selectedCount > 0);
}