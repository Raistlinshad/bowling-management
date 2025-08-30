// CompleteLeagueSetupDialog.cpp Implementation
CompleteLeagueSetupDialog::CompleteLeagueSetupDialog(QWidget *parent)
    : QDialog(parent)
    , m_totalLanes(8)
{
    setupUI();
    setWindowTitle("Complete League Setup");
    setModal(true);
    resize(800, 600);
    
    // Set default values
    m_startDateEdit->setDate(QDate::currentDate().addDays(7));
    m_startTimeEdit->setTime(QTime(19, 0));
    m_durationSpinner->setValue(180);
    m_weeksSpinner->setValue(16);
    m_hdcpHighValueSpinner->setValue(225);
    m_hdcpPercentageSpinner->setValue(0.8);
    m_avgUpdateIntervalSpinner->setValue(3);
    m_hdcpDelayGamesSpinner->setValue(3);
    m_avgDelayGamesSpinner->setValue(0);

    // Load available bowlers from database
    DatabaseManager *db = DatabaseManager::instance();
    m_availableBowlers = db->getAllBowlers();
    populateTeamsList();
    
    // Connect signals
    connect(m_createBtn, &QPushButton::clicked, this, &CompleteLeagueSetupDialog::onCreateLeagueClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &CompleteLeagueSetupDialog::onCancelClicked);
    connect(m_tabWidget, &QTabWidget::currentChanged, this, &CompleteLeagueSetupDialog::onTabChanged);
    
    updateSummary();
}

void CompleteLeagueSetupDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // Create tab widget
    m_tabWidget = new QTabWidget;
    m_mainLayout->addWidget(m_tabWidget);
    
    setupBasicInfoTab();
    setupScheduleTab();
    setupAverageCalculationTab();
    setupHandicapCalculationTab();
    setupAbsentHandlingTab();
    setupPreBowlTab();
    setupDivisionsTab();
    setupPlayoffsTab();
    setupPointSystemTab();
    setupTeamsTab();
    setupSummaryTab();
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    m_cancelBtn = new QPushButton("Cancel");
    m_createBtn = new QPushButton("Create League");
    m_createBtn->setStyleSheet("QPushButton { "
                              "background-color: #28a745; "
                              "color: white; "
                              "font-weight: bold; "
                              "padding: 10px 20px; "
                              "}");
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_cancelBtn);
    buttonLayout->addWidget(m_createBtn);
    
    m_mainLayout->addLayout(buttonLayout);
}

void CompleteLeagueSetupDialog::setupBasicInfoTab()
{
    m_basicInfoTab = new QWidget;
    QGridLayout *layout = new QGridLayout(m_basicInfoTab);
    
    int row = 0;
    layout->addWidget(new QLabel("League Name:"), row, 0);
    m_leagueNameEdit = new QLineEdit;
    layout->addWidget(m_leagueNameEdit, row, 1);
    row++;
    
    layout->addWidget(new QLabel("Contact Name:"), row, 0);
    m_contactNameEdit = new QLineEdit;
    layout->addWidget(m_contactNameEdit, row, 1);
    row++;
    
    layout->addWidget(new QLabel("Contact Phone:"), row, 0);
    m_contactPhoneEdit = new QLineEdit;
    layout->addWidget(m_contactPhoneEdit, row, 1);
    row++;
    
    layout->addWidget(new QLabel("Contact Email:"), row, 0);
    m_contactEmailEdit = new QLineEdit;
    layout->addWidget(m_contactEmailEdit, row, 1);
    row++;
    
    layout->addWidget(new QLabel("League Description:"), row, 0, Qt::AlignTop);
    m_leagueDescriptionEdit = new QTextEdit;
    m_leagueDescriptionEdit->setMaximumHeight(100);
    layout->addWidget(m_leagueDescriptionEdit, row, 1);
    
    layout->setRowStretch(row + 1, 1);
    
    m_tabWidget->addTab(m_basicInfoTab, "Basic Info");
}

void CompleteLeagueSetupDialog::setupScheduleTab()
{
    m_scheduleTab = new QWidget;
    QGridLayout *layout = new QGridLayout(m_scheduleTab);
    
    int row = 0;
    
    // Start Date
    layout->addWidget(new QLabel("Start Date:"), row, 0);
    m_startDateEdit = new QDateEdit;
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setMinimumDate(QDate::currentDate());
    layout->addWidget(m_startDateEdit, row, 1);
    row++;
    
    // Start Time
    layout->addWidget(new QLabel("Start Time:"), row, 0);
    m_startTimeEdit = new QTimeEdit;
    m_startTimeEdit->setDisplayFormat("h:mm AP");
    layout->addWidget(m_startTimeEdit, row, 1);
    row++;
    
    // Duration
    layout->addWidget(new QLabel("Duration (minutes):"), row, 0);
    m_durationSpinner = new QSpinBox;
    m_durationSpinner->setRange(60, 300);
    m_durationSpinner->setSingleStep(15);
    layout->addWidget(m_durationSpinner, row, 1);
    row++;
    
    // Number of Weeks
    layout->addWidget(new QLabel("Number of Weeks:"), row, 0);
    m_weeksSpinner = new QSpinBox;
    m_weeksSpinner->setRange(1, 52);
    layout->addWidget(m_weeksSpinner, row, 1);
    row++;
    
    // Frequency
    layout->addWidget(new QLabel("Frequency:"), row, 0);
    m_frequencyCombo = new QComboBox;
    m_frequencyCombo->addItem("Weekly", 7);
    m_frequencyCombo->addItem("Bi-weekly", 14);
    m_frequencyCombo->addItem("Monthly", 30);
    layout->addWidget(m_frequencyCombo, row, 1);
    row++;
    
    // Lane Selection
    layout->addWidget(new QLabel("Available Lanes:"), row, 0, Qt::AlignTop);
    m_lanesList = new QListWidget;
    for (int i = 1; i <= m_totalLanes; ++i) {
        QListWidgetItem *item = new QListWidgetItem(QString("Lane %1").arg(i));
        item->setData(Qt::UserRole, i);
        item->setCheckState(Qt::Unchecked);
        m_lanesList->addItem(item);
    }
    layout->addWidget(m_lanesList, row, 1);
    
    m_tabWidget->addTab(m_scheduleTab, "Schedule");
}

void CompleteLeagueSetupDialog::setupAverageCalculationTab()
{
    m_avgCalcTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_avgCalcTab);
    
    // Average Calculation Type
    QGroupBox *calcTypeGroup = new QGroupBox("Average Calculation Method");
    QVBoxLayout *calcTypeLayout = new QVBoxLayout(calcTypeGroup);
    
    m_avgCalcGroup = new QButtonGroup(this);
    
    m_avgPerGameRadio = new QRadioButton("Total Pins ÷ Games Played (Updated after each game)");
    m_avgPerGameRadio->setChecked(true);
    m_avgCalcGroup->addButton(m_avgPerGameRadio, 0);
    calcTypeLayout->addWidget(m_avgPerGameRadio);
    
    m_avgPerBallRadio = new QRadioButton("Total Pins ÷ Balls Thrown (Updated after each game)");
    m_avgCalcGroup->addButton(m_avgPerBallRadio, 1);
    calcTypeLayout->addWidget(m_avgPerBallRadio);
    
    QHBoxLayout *periodicLayout = new QHBoxLayout;
    m_avgPeriodicRadio = new QRadioButton("Total Pins ÷ Games Played every");
    m_avgCalcGroup->addButton(m_avgPeriodicRadio, 2);
    periodicLayout->addWidget(m_avgPeriodicRadio);
    
    m_avgUpdateIntervalSpinner = new QSpinBox;
    m_avgUpdateIntervalSpinner->setRange(1, 10);
    m_avgUpdateIntervalSpinner->setSuffix(" games");
    periodicLayout->addWidget(m_avgUpdateIntervalSpinner);
    periodicLayout->addStretch();
    
    calcTypeLayout->addLayout(periodicLayout);
    layout->addWidget(calcTypeGroup);
    
    // Delay Settings
    QGroupBox *delayGroup = new QGroupBox("Delay Settings");
    QHBoxLayout *delayLayout = new QHBoxLayout(delayGroup);
    
    delayLayout->addWidget(new QLabel("Delay average calculation for"));
    m_avgDelayGamesSpinner = new QSpinBox;
    m_avgDelayGamesSpinner->setRange(0, 10);
    m_avgDelayGamesSpinner->setSuffix(" games");
    delayLayout->addWidget(m_avgDelayGamesSpinner);
    delayLayout->addWidget(new QLabel("(Default: 0)"));
    delayLayout->addStretch();
    
    layout->addWidget(delayGroup);
    layout->addStretch();
    
    connect(m_avgCalcGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &CompleteLeagueSetupDialog::onAvgCalculationChanged);
    
    m_tabWidget->addTab(m_avgCalcTab, "Average Calculation");
}

void CompleteLeagueSetupDialog::setupHandicapCalculationTab()
{
    m_hdcpCalcTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_hdcpCalcTab);
    
    // Handicap Calculation Type
    QGroupBox *calcTypeGroup = new QGroupBox("Handicap Calculation Method");
    QVBoxLayout *calcTypeLayout = new QVBoxLayout(calcTypeGroup);
    
    m_hdcpCalcGroup = new QButtonGroup(this);
    
    // Option 1: Percentage Based
    QVBoxLayout *option1Layout = new QVBoxLayout;
    m_hdcpPercentageRadio = new QRadioButton("Option 1: Percentage Based");
    m_hdcpPercentageRadio->setChecked(true);
    m_hdcpCalcGroup->addButton(m_hdcpPercentageRadio, 0);
    option1Layout->addWidget(m_hdcpPercentageRadio);
    
    QHBoxLayout *percentageSetupLayout = new QHBoxLayout;
    percentageSetupLayout->addWidget(new QLabel("    High:"));
    m_hdcpHighValueSpinner = new QSpinBox;
    m_hdcpHighValueSpinner->setRange(150, 300);
    percentageSetupLayout->addWidget(m_hdcpHighValueSpinner);
    
    percentageSetupLayout->addWidget(new QLabel("Percentage:"));
    m_hdcpPercentageSpinner = new QDoubleSpinBox;
    m_hdcpPercentageSpinner->setRange(0.1, 1.0);
    m_hdcpPercentageSpinner->setSingleStep(0.1);
    m_hdcpPercentageSpinner->setSuffix("%");
    m_hdcpPercentageSpinner->setDecimals(1);
    percentageSetupLayout->addWidget(m_hdcpPercentageSpinner);
    percentageSetupLayout->addStretch();
    
    option1Layout->addLayout(percentageSetupLayout);
    
    QLabel *formula1 = new QLabel("    Formula: HDCP = (High - Average) × Percentage");
    formula1->setStyleSheet("color: #666; font-style: italic;");
    option1Layout->addWidget(formula1);
    
    calcTypeLayout->addLayout(option1Layout);
    
    // Option 2: Straight Difference
    m_hdcpStraightRadio = new QRadioButton("Option 2: Straight Difference - HDCP = High - Average");
    m_hdcpCalcGroup->addButton(m_hdcpStraightRadio, 1);
    calcTypeLayout->addWidget(m_hdcpStraightRadio);
    
    // Option 3: With Deduction
    QVBoxLayout *option3Layout = new QVBoxLayout;
    m_hdcpDeductionRadio = new QRadioButton("Option 3: With Deduction");
    m_hdcpCalcGroup->addButton(m_hdcpDeductionRadio, 2);
    option3Layout->addWidget(m_hdcpDeductionRadio);
    
    QHBoxLayout *deductionLayout = new QHBoxLayout;
    deductionLayout->addWidget(new QLabel("    Deduction:"));
    m_hdcpDeductionSpinner = new QSpinBox;
    m_hdcpDeductionSpinner->setRange(0, 50);
    deductionLayout->addWidget(m_hdcpDeductionSpinner);
    deductionLayout->addWidget(new QLabel("HDCP = (High - Average) - Deduction"));
    deductionLayout->addStretch();
    
    option3Layout->addLayout(deductionLayout);
    calcTypeLayout->addLayout(option3Layout);
    
    layout->addWidget(calcTypeGroup);
    
    // Delay Settings
    QGroupBox *delayGroup = new QGroupBox("Handicap Delay");
    QHBoxLayout *delayLayout = new QHBoxLayout(delayGroup);
    
    delayLayout->addWidget(new QLabel("Delay handicap calculation for"));
    m_hdcpDelayGamesSpinner = new QSpinBox;
    m_hdcpDelayGamesSpinner->setRange(0, 10);
    m_hdcpDelayGamesSpinner->setSuffix(" games bowled");
    delayLayout->addWidget(m_hdcpDelayGamesSpinner);
    delayLayout->addWidget(new QLabel("(Default: 3)"));
    delayLayout->addStretch();
    
    layout->addWidget(delayGroup);
    layout->addStretch();
    
    connect(m_hdcpCalcGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &CompleteLeagueSetupDialog::onHdcpCalculationChanged);
    
    m_tabWidget->addTab(m_hdcpCalcTab, "Handicap Calculation");
}

void CompleteLeagueSetupDialog::setupAbsentHandlingTab()
{
    m_absentTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_absentTab);
    
    QGroupBox *absentGroup = new QGroupBox("Handle Absent Players");
    QVBoxLayout *absentLayout = new QVBoxLayout(absentGroup);
    
    m_absentGroup = new QButtonGroup(this);
    
    // Option 1: Percentage of Average
    QHBoxLayout *percentLayout = new QHBoxLayout;
    m_absentPercentageRadio = new QRadioButton("Set % of bowler average:");
    m_absentPercentageRadio->setChecked(true);
    m_absentGroup->addButton(m_absentPercentageRadio, 0);
    percentLayout->addWidget(m_absentPercentageRadio);
    
    m_absentPercentageSpinner = new QDoubleSpinBox;
    m_absentPercentageSpinner->setRange(0.5, 1.0);
    m_absentPercentageSpinner->setSingleStep(0.05);
    m_absentPercentageSpinner->setValue(0.9);
    m_absentPercentageSpinner->setSuffix("%");
    percentLayout->addWidget(m_absentPercentageSpinner);
    percentLayout->addStretch();
    
    absentLayout->addLayout(percentLayout);
    
    // Option 2: Fixed Value
    QHBoxLayout *fixedLayout = new QHBoxLayout;
    m_absentFixedRadio = new QRadioButton("Set fixed value:");
    m_absentGroup->addButton(m_absentFixedRadio, 1);
    fixedLayout->addWidget(m_absentFixedRadio);
    
    m_absentFixedSpinner = new QSpinBox;
    m_absentFixedSpinner->setRange(50, 200);
    m_absentFixedSpinner->setValue(100);
    fixedLayout->addWidget(m_absentFixedSpinner);
    fixedLayout->addStretch();
    
    absentLayout->addLayout(fixedLayout);
    
    // Option 3: Use Average
    m_absentAverageRadio = new QRadioButton("Use bowler's current average");
    m_absentGroup->addButton(m_absentAverageRadio, 2);
    absentLayout->addWidget(m_absentAverageRadio);
    
    layout->addWidget(absentGroup);
    layout->addStretch();
    
    connect(m_absentGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &CompleteLeagueSetupDialog::onAbsentHandlingChanged);
    
    m_tabWidget->addTab(m_absentTab, "Absent Handling");
}

void CompleteLeagueSetupDialog::setupPreBowlTab()
{
    m_preBowlTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_preBowlTab);
    
    // Enable Pre-Bowl
    m_preBowlEnabledCheck = new QCheckBox("Enable Pre-Bowl Games");
    layout->addWidget(m_preBowlEnabledCheck);
    
    // Pre-Bowl Settings Group
    QGroupBox *preBowlGroup = new QGroupBox("Pre-Bowl Settings");
    QVBoxLayout *preBowlLayout = new QVBoxLayout(preBowlGroup);
    
    // Carry to next season
    m_preBowlCarryOverCheck = new QCheckBox("Can old pre-bowls be used next season?");
    QLabel *carryOverNote = new QLabel("(If unchecked, end of season stats will include all unused pre-bowls for clearing)");
    carryOverNote->setStyleSheet("color: #666; font-style: italic;");
    preBowlLayout->addWidget(m_preBowlCarryOverCheck);
    preBowlLayout->addWidget(carryOverNote);
    
    // Random use when absent
    m_preBowlRandomUseCheck = new QCheckBox("Randomly use existing pre-bowl when absent?");
    preBowlLayout->addWidget(m_preBowlRandomUseCheck);
    
    // Usage method
    QLabel *usageLabel = new QLabel("If random use is enabled, use by:");
    preBowlLayout->addWidget(usageLabel);
    
    m_preBowlUseGroup = new QButtonGroup(this);
    m_preBowlByGameRadio = new QRadioButton("By individual game");
    m_preBowlBySetRadio = new QRadioButton("By 3-game set");
    m_preBowlBySetRadio->setChecked(true);
    
    m_preBowlUseGroup->addButton(m_preBowlByGameRadio, 0);
    m_preBowlUseGroup->addButton(m_preBowlBySetRadio, 1);
    
    preBowlLayout->addWidget(m_preBowlByGameRadio);
    preBowlLayout->addWidget(m_preBowlBySetRadio);
    
    // Max uses per game
    QHBoxLayout *maxUsesLayout = new QHBoxLayout;
    maxUsesLayout->addWidget(new QLabel("Store each game to be used"));
    m_preBowlMaxUsesSpinner = new QSpinBox;
    m_preBowlMaxUsesSpinner->setRange(1, 10);
    m_preBowlMaxUsesSpinner->setValue(1);
    m_preBowlMaxUsesSpinner->setSuffix(" times within a season");
    maxUsesLayout->addWidget(m_preBowlMaxUsesSpinner);
    maxUsesLayout->addStretch();
    
    preBowlLayout->addLayout(maxUsesLayout);
    
    layout->addWidget(preBowlGroup);
    layout->addStretch();
    
    // Enable/disable settings based on checkbox
    connect(m_preBowlEnabledCheck, &QCheckBox::toggled, 
            this, &CompleteLeagueSetupDialog::onPreBowlToggled);
    connect(m_preBowlEnabledCheck, &QCheckBox::toggled, preBowlGroup, &QGroupBox::setEnabled);
    
    // Initially disable if not checked
    preBowlGroup->setEnabled(m_preBowlEnabledCheck->isChecked());
    
    m_tabWidget->addTab(m_preBowlTab, "Pre-Bowl Rules");
}

LeagueConfig CompleteLeagueSetupDialog::getLeagueConfiguration() const
{
    LeagueConfig config;
    
    // Basic information
    config.name = m_leagueNameEdit->text().trimmed();
    config.startDate = m_startDateEdit->date();
    config.endDate = config.startDate.addDays(m_weeksSpinner->value() * m_frequencyCombo->currentData().toInt());
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
    int avgCalcType = m_avgCalcGroup->checkedId();
    config.avgCalc.type = static_cast<LeagueConfig::AverageCalculation::Type>(avgCalcType);
    config.avgCalc.updateInterval = m_avgUpdateIntervalSpinner->value();
    config.avgCalc.delayGames = m_avgDelayGamesSpinner->value();
    
    // Handicap calculation
    int hdcpCalcType = m_hdcpCalcGroup->checkedId();
    config.hdcpCalc.type = static_cast<LeagueConfig::HandicapCalculation::Type>(hdcpCalcType);
    config.hdcpCalc.highValue = m_hdcpHighValueSpinner->value();
    config.hdcpCalc.percentage = m_hdcpPercentageSpinner->value();
    config.hdcpCalc.deduction = m_hdcpDeductionSpinner->value();
    config.hdcpCalc.delayGames = m_hdcpDelayGamesSpinner->value();
    
    // Absent handling
    int absentType = m_absentGroup->checkedId();
    config.absentHandling.type = static_cast<LeagueConfig::AbsentHandling::Type>(absentType);
    config.absentHandling.percentage = m_absentPercentageSpinner->value();
    config.absentHandling.fixedValue = m_absentFixedSpinner->value();
    
    // Pre-bowl rules
    config.preBowlRules.enabled = m_preBowlEnabledCheck->isChecked();
    config.preBowlRules.carryToNextSeason = m_preBowlCarryOverCheck->isChecked();
    config.preBowlRules.randomUseWhenAbsent = m_preBowlRandomUseCheck->isChecked();
    config.preBowlRules.useBy = m_preBowlUseGroup->checkedId() == 0 ? 
                               LeagueConfig::PreBowlRules::ByGame : 
                               LeagueConfig::PreBowlRules::ByThreeGameSet;
    config.preBowlRules.maxUsesPerGame = m_preBowlMaxUsesSpinner->value();
    
    return config;
}

void CompleteLeagueSetupDialog::onCreateLeagueClicked()
{
    if (validateConfiguration()) {
        accept();
    }
}

void CompleteLeagueSetupDialog::onCancelClicked()
{
    reject();
}

void CompleteLeagueSetupDialog::onTabChanged(int index)
{
    if (index == m_tabWidget->count() - 1) { // Summary tab
        updateSummary();
    }
}

void CompleteLeagueSetupDialog::updateSummary()
{
    LeagueConfig config = getLeagueConfiguration();
    
    QString summary = QString("<h3>%1</h3>").arg(config.name.isEmpty() ? "Unnamed League" : config.name);
    
    summary += QString("<b>Schedule:</b><br>");
    summary += QString("Start: %1<br>").arg(config.startDate.toString("MMM d, yyyy"));
    summary += QString("Duration: %1 weeks<br>").arg(config.numberOfWeeks);
    summary += QString("Lanes: %1 selected<br><br>").arg(config.laneIds.size());
    
    summary += QString("<b>Average Calculation:</b><br>");
    switch (config.avgCalc.type) {
    case LeagueConfig::AverageCalculation::TotalPinsPerGame:
        summary += "Total Pins ÷ Games Played (after each game)<br>";
        break;
    case LeagueConfig::AverageCalculation::TotalPinsPerBall:
        summary += "Total Pins ÷ Balls Thrown (after each game)<br>";
        break;
    case LeagueConfig::AverageCalculation::PeriodicUpdate:
        summary += QString("Total Pins ÷ Games Played every %1 games<br>").arg(config.avgCalc.updateInterval);
        break;
    }
    if (config.avgCalc.delayGames > 0) {
        summary += QString("Delayed for %1 games<br>").arg(config.avgCalc.delayGames);
    }
    summary += "<br>";
    
    summary += QString("<b>Handicap Calculation:</b><br>");
    switch (config.hdcpCalc.type) {
    case LeagueConfig::HandicapCalculation::PercentageBased:
        summary += QString("(%1 - Average) × %2%<br>").arg(config.hdcpCalc.highValue).arg(config.hdcpCalc.percentage * 100);
        break;
    case LeagueConfig::HandicapCalculation::StraightDifference:
        summary += QString("%1 - Average<br>").arg(config.hdcpCalc.highValue);
        break;
    case LeagueConfig::HandicapCalculation::WithDeduction:
        summary += QString("(%1 - Average) - %2<br>").arg(config.hdcpCalc.highValue).arg(config.hdcpCalc.deduction);
        break;
    }
    if (config.hdcpCalc.delayGames > 0) {
        summary += QString("Delayed for %1 games<br>").arg(config.hdcpCalc.delayGames);
    }
    summary += "<br>";
    
    summary += QString("<b>Absent Player Handling:</b><br>");
    switch (config.absentHandling.type) {
    case LeagueConfig::AbsentHandling::PercentageOfAverage:
        summary += QString("%1% of bowler's average<br>").arg(config.absentHandling.percentage * 100);
        break;
    case LeagueConfig::AbsentHandling::FixedValue:
        summary += QString("Fixed value: %1<br>").arg(config.absentHandling.fixedValue);
        break;
    case LeagueConfig::AbsentHandling::UseAverage:
        summary += "Use bowler's current average<br>";
        break;
    }
    summary += "<br>";
    
    if (config.preBowlRules.enabled) {
        summary += QString("<b>Pre-Bowl Rules:</b><br>");
        summary += "Pre-bowl games enabled<br>";
        if (config.preBowlRules.carryToNextSeason) {
            summary += "Can carry over to next season<br>";
        }
        if (config.preBowlRules.randomUseWhenAbsent) {
            summary += "Randomly use when absent<br>";
            summary += QString("Use by: %1<br>").arg(
                config.preBowlRules.useBy == LeagueConfig::PreBowlRules::ByGame ? "Individual game" : "3-game set");
        }
        summary += QString("Max uses per game: %1<br><br>").arg(config.preBowlRules.maxUsesPerGame);
    }
    
    m_summaryText->setHtml(summary);
}

bool CompleteLeagueSetupDialog::validateConfiguration() const
{
    if (m_leagueNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(const_cast<CompleteLeagueSetupDialog*>(this), 
                           "Missing Information", "Please enter a league name.");
        m_tabWidget->setCurrentIndex(0);
        m_leagueNameEdit->setFocus();
        return false;
    }
    
    if (m_contactNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(const_cast<CompleteLeagueSetupDialog*>(this), 
                           "Missing Information", "Please enter a contact name.");
        m_tabWidget->setCurrentIndex(0);
        m_contactNameEdit->setFocus();
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
        QMessageBox::warning(const_cast<CompleteLeagueSetupDialog*>(this), 
                           "No Lanes Selected", "Please select at least one lane for the league.");
        m_tabWidget->setCurrentIndex(1);
        return false;
    }
    
    if (m_weeksSpinner->value() <= 0) {
        QMessageBox::warning(const_cast<CompleteLeagueSetupDialog*>(this), 
                           "Invalid Duration", "Number of weeks must be greater than 0.");
        m_tabWidget->setCurrentIndex(1);
        return false;
    }
    
    return true;
}

void CompleteLeagueSetupDialog::setupDivisionsTab()
{
    m_divisionsTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_divisionsTab);
    
    // Number of divisions
    QHBoxLayout *countLayout = new QHBoxLayout;
    countLayout->addWidget(new QLabel("Number of Divisions:"));
    m_divisionCountSpinner = new QSpinBox;
    m_divisionCountSpinner->setRange(1, 6);
    m_divisionCountSpinner->setValue(1);
    countLayout->addWidget(m_divisionCountSpinner);
    countLayout->addStretch();
    layout->addLayout(countLayout);
    
    // Reorder mid-season
    m_reorderMidSeasonCheck = new QCheckBox("Re-order divisions halfway through season?");
    layout->addWidget(m_reorderMidSeasonCheck);
    
    // Ordering method
    QGroupBox *orderGroup = new QGroupBox("Division Assignment Method");
    QVBoxLayout *orderLayout = new QVBoxLayout(orderGroup);
    
    m_divisionOrderGroup = new QButtonGroup(this);
    m_systemOrderRadio = new QRadioButton("Set by system (performance-based)");
    m_systemOrderRadio->setChecked(true);
    m_divisionOrderGroup->addButton(m_systemOrderRadio, 0);
    orderLayout->addWidget(m_systemOrderRadio);
    
    m_manualOrderRadio = new QRadioButton("Set by league coordinator (manual assignment)");
    m_divisionOrderGroup->addButton(m_manualOrderRadio, 1);
    orderLayout->addWidget(m_manualOrderRadio);
    
    QLabel *manualNote = new QLabel("When league starts, a window will appear to drag teams into divisions with division naming options.");
    manualNote->setWordWrap(true);
    manualNote->setStyleSheet("color: #666; font-style: italic; margin-left: 20px;");
    orderLayout->addWidget(manualNote);
    
    layout->addWidget(orderGroup);
    
    // Division assignment table
    m_divisionAssignmentTable = new QTableWidget;
    m_divisionAssignmentTable->setColumnCount(2);
    m_divisionAssignmentTable->setHorizontalHeaderLabels({"Division", "Teams"});
    m_divisionAssignmentTable->horizontalHeader()->setStretchLastSection(true);
    layout->addWidget(m_divisionAssignmentTable);
    
    m_assignDivisionsBtn = new QPushButton("Preview Division Assignment");
    connect(m_assignDivisionsBtn, &QPushButton::clicked, 
            this, &CompleteLeagueSetupDialog::onAssignDivisionsClicked);
    layout->addWidget(m_assignDivisionsBtn);
    
    connect(m_divisionCountSpinner, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &CompleteLeagueSetupDialog::onDivisionCountChanged);
    
    m_tabWidget->addTab(m_divisionsTab, "Divisions");
}

void CompleteLeagueSetupDialog::setupPlayoffsTab()
{
    m_playoffsTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_playoffsTab);
    
    QGroupBox *playoffGroup = new QGroupBox("Playoff Type");
    QVBoxLayout *playoffLayout = new QVBoxLayout(playoffGroup);
    
    m_playoffTypeGroup = new QButtonGroup(this);
    
    m_noPlayoffsRadio = new QRadioButton("No Playoffs");
    m_noPlayoffsRadio->setChecked(true);
    m_playoffTypeGroup->addButton(m_noPlayoffsRadio, 0);
    playoffLayout->addWidget(m_noPlayoffsRadio);
    
    m_roundRobinRadio = new QRadioButton("Round Robin - All teams or division only");
    m_playoffTypeGroup->addButton(m_roundRobinRadio, 1);
    playoffLayout->addWidget(m_roundRobinRadio);
    
    m_roundRobinPlacementRadio = new QRadioButton("Round Robin with placement vs final round");
    m_playoffTypeGroup->addButton(m_roundRobinPlacementRadio, 2);
    playoffLayout->addWidget(m_roundRobinPlacementRadio);
    
    m_placementOnlyRadio = new QRadioButton("Placement only (1 vs 8, 2 vs 7, etc.)");
    m_playoffTypeGroup->addButton(m_placementOnlyRadio, 3);
    playoffLayout->addWidget(m_placementOnlyRadio);
    
    m_roundRobinPlusPlacementRadio = new QRadioButton("Round Robin + Placement (selectable)");
    m_playoffTypeGroup->addButton(m_roundRobinPlusPlacementRadio, 4);
    playoffLayout->addWidget(m_roundRobinPlusPlacementRadio);
    
    layout->addWidget(playoffGroup);
    
    // Division only option
    m_divisionOnlyPlayoffsCheck = new QCheckBox("Division only playoffs (when applicable)");
    layout->addWidget(m_divisionOnlyPlayoffsCheck);
    
    // Placement pairs table
    QLabel *placementLabel = new QLabel("Custom Placement Matchups:");
    layout->addWidget(placementLabel);
    
    m_placementPairsTable = new QTableWidget;
    m_placementPairsTable->setColumnCount(3);
    m_placementPairsTable->setHorizontalHeaderLabels({"Round", "Team 1 Seed", "Team 2 Seed"});
    layout->addWidget(m_placementPairsTable);
    
    connect(m_playoffTypeGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &CompleteLeagueSetupDialog::onPlayoffTypeChanged);
    
    layout->addStretch();
    
    m_tabWidget->addTab(m_playoffsTab, "Playoffs");
}

void CompleteLeagueSetupDialog::setupPointSystemTab()
{
    m_pointSystemTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_pointSystemTab);
    
    // Point system type
    QGroupBox *pointTypeGroup = new QGroupBox("Point System Type");
    QVBoxLayout *pointTypeLayout = new QVBoxLayout(pointTypeGroup);
    
    m_pointTypeGroup = new QButtonGroup(this);
    
    m_winLossTieRadio = new QRadioButton("Win/Loss/Tie System");
    m_winLossTieRadio->setChecked(true);
    m_pointTypeGroup->addButton(m_winLossTieRadio, 0);
    pointTypeLayout->addWidget(m_winLossTieRadio);
    
    m_teamVsTeamRadio = new QRadioButton("Team vs Team Overall Scoring");
    m_pointTypeGroup->addButton(m_teamVsTeamRadio, 1);
    pointTypeLayout->addWidget(m_teamVsTeamRadio);
    
    m_customPointsRadio = new QRadioButton("Custom Point System");
    m_pointTypeGroup->addButton(m_customPointsRadio, 2);
    pointTypeLayout->addWidget(m_customPointsRadio);
    
    layout->addWidget(pointTypeGroup);
    
    // Win/Loss/Tie Settings
    m_winLossGroup = new QGroupBox("Win/Loss/Tie Settings");
    QGridLayout *wlLayout = new QGridLayout(m_winLossGroup);
    
    wlLayout->addWidget(new QLabel("Win Points:"), 0, 0);
    m_winPointsSpinner = new QSpinBox;
    m_winPointsSpinner->setRange(1, 10);
    m_winPointsSpinner->setValue(2);
    wlLayout->addWidget(m_winPointsSpinner, 0, 1);
    
    wlLayout->addWidget(new QLabel("Loss Points:"), 1, 0);
    m_lossPointsSpinner = new QSpinBox;
    m_lossPointsSpinner->setRange(0, 5);
    m_lossPointsSpinner->setValue(0);
    wlLayout->addWidget(m_lossPointsSpinner, 1, 1);
    
    wlLayout->addWidget(new QLabel("Tie Points:"), 2, 0);
    m_tiePointsSpinner = new QSpinBox;
    m_tiePointsSpinner->setRange(0, 5);
    m_tiePointsSpinner->setValue(1);
    wlLayout->addWidget(m_tiePointsSpinner, 2, 1);
    
    m_headsUpEnabledCheck = new QCheckBox("Include heads-up individual scoring");
    m_headsUpEnabledCheck->setChecked(true);
    wlLayout->addWidget(m_headsUpEnabledCheck, 3, 0, 1, 2);
    
    m_headsUpHandicapCheck = new QCheckBox("Heads-up with handicap");
    m_headsUpHandicapCheck->setChecked(true);
    wlLayout->addWidget(m_headsUpHandicapCheck, 4, 0, 1, 2);
    
    layout->addWidget(m_winLossGroup);
    
    // Team vs Team Settings
    m_teamVsTeamGroup = new QGroupBox("Team vs Team Settings");
    QVBoxLayout *tvtLayout = new QVBoxLayout(m_teamVsTeamGroup);
    
    QLabel *tvtLabel = new QLabel("Points awarded highest to lowest team scores.\nHow should tied scores be handled?");
    tvtLayout->addWidget(tvtLabel);
    
    m_stackedTiesCheck = new QCheckBox("Stacked points (e.g., 8,7,6,6,5,4,3,2)");
    m_stackedTiesCheck->setChecked(true);
    tvtLayout->addWidget(m_stackedTiesCheck);
    
    QLabel *linearLabel = new QLabel("Unchecked = Linear points (e.g., 8,7,6,6,4,3,2,1)");
    linearLabel->setStyleSheet("color: #666; font-style: italic;");
    tvtLayout->addWidget(linearLabel);
    
    layout->addWidget(m_teamVsTeamGroup);
    
    // Custom Settings
    m_customPointsGroup = new QGroupBox("Custom Point Tracking");
    QVBoxLayout *customLayout = new QVBoxLayout(m_customPointsGroup);
    
    customLayout->addWidget(new QLabel("Track points for:"));
    
    m_trackHeadsUpCheck = new QCheckBox("Heads-up individual vs individual");
    m_trackHeadsUpCheck->setChecked(true);
    customLayout->addWidget(m_trackHeadsUpCheck);
    
    m_trackTeamVsCheck = new QCheckBox("Team vs Team overall");
    m_trackTeamVsCheck->setChecked(true);
    customLayout->addWidget(m_trackTeamVsCheck);
    
    m_trackDivisionVsCheck = new QCheckBox("Division vs Division");
    customLayout->addWidget(m_trackDivisionVsCheck);
    
    m_trackLeagueVsCheck = new QCheckBox("League vs League");
    customLayout->addWidget(m_trackLeagueVsCheck);
    
    m_trackScratchCheck = new QCheckBox("Scratch scoring");
    customLayout->addWidget(m_trackScratchCheck);
    
    m_trackHandicapCheck = new QCheckBox("Handicap scoring");
    m_trackHandicapCheck->setChecked(true);
    customLayout->addWidget(m_trackHandicapCheck);
    
    layout->addWidget(m_customPointsGroup);
    
    connect(m_pointTypeGroup, QOverload<int>::of(&QButtonGroup::buttonClicked),
            this, &CompleteLeagueSetupDialog::onPointSystemChanged);
    
    // Enable/disable groups based on selection
    onPointSystemChanged();
    
    layout->addStretch();
    
    m_tabWidget->addTab(m_pointSystemTab, "Point System");
}

void CompleteLeagueSetupDialog::setupTeamsTab()
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
    rightLayout->addWidget(new QLabel("League Teams:"));
    
    // Team creation
    QHBoxLayout *teamCreateLayout = new QHBoxLayout;
    m_teamNameEdit = new QLineEdit;
    m_teamNameEdit->setPlaceholderText("Enter team name...");
    teamCreateLayout->addWidget(m_teamNameEdit);
    
    m_addTeamBtn = new QPushButton("Add Team");
    connect(m_addTeamBtn, &QPushButton::clicked, 
            this, &CompleteLeagueSetupDialog::onAddTeamClicked);
    teamCreateLayout->addWidget(m_addTeamBtn);
    
    rightLayout->addLayout(teamCreateLayout);
    
    // Teams list
    m_teamsList = new QListWidget;
    rightLayout->addWidget(m_teamsList);
    
    // Team members table
    rightLayout->addWidget(new QLabel("Team Members:"));
    m_teamMembersTable = new QTableWidget;
    m_teamMembersTable->setColumnCount(3);
    m_teamMembersTable->setHorizontalHeaderLabels({"Name", "Average", "Remove"});
    rightLayout->addWidget(m_teamMembersTable);
    
    m_removeTeamBtn = new QPushButton("Remove Selected Team");
    connect(m_removeTeamBtn, &QPushButton::clicked, 
            this, &CompleteLeagueSetupDialog::onRemoveTeamClicked);
    rightLayout->addWidget(m_removeTeamBtn);
    
    layout->addLayout(rightLayout);
    
    m_tabWidget->addTab(m_teamsTab, "Teams");
}

void CompleteLeagueSetupDialog::setupSummaryTab()
{
    m_summaryTab = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout(m_summaryTab);
    
    layout->addWidget(new QLabel("League Configuration Summary:"));
    
    m_summaryText = new QTextEdit;
    m_summaryText->setReadOnly(true);
    layout->addWidget(m_summaryText);
    
    m_tabWidget->addTab(m_summaryTab, "Summary");
}

// Event handlers
void CompleteLeagueSetupDialog::onAvgCalculationChanged()
{
    int type = m_avgCalcGroup->checkedId();
    m_avgUpdateIntervalSpinner->setEnabled(type == 2); // Periodic update
}

void CompleteLeagueSetupDialog::onHdcpCalculationChanged()
{
    int type = m_hdcpCalcGroup->checkedId();
    m_hdcpPercentageSpinner->setEnabled(type == 0); // Percentage based
    m_hdcpDeductionSpinner->setEnabled(type == 2);   // With deduction
}

void CompleteLeagueSetupDialog::onAbsentHandlingChanged()
{
    int type = m_absentGroup->checkedId();
    m_absentPercentageSpinner->setEnabled(type == 0); // Percentage
    m_absentFixedSpinner->setEnabled(type == 1);       // Fixed value
}

void CompleteLeagueSetupDialog::onPreBowlToggled(bool enabled)
{
    Q_UNUSED(enabled)
    // Group enabling is handled by direct connection in setupPreBowlTab
}

void CompleteLeagueSetupDialog::onDivisionCountChanged(int count)
{
    // Update division assignment table
    m_divisionAssignmentTable->setRowCount(count);
    for (int i = 0; i < count; ++i) {
        m_divisionAssignmentTable->setItem(i, 0, new QTableWidgetItem(QString("Division %1").arg(i + 1)));
        m_divisionAssignmentTable->setItem(i, 1, new QTableWidgetItem("(No teams assigned)"));
    }
}

void CompleteLeagueSetupDialog::onPlayoffTypeChanged()
{
    int type = m_playoffTypeGroup->checkedId();
    
    // Enable/disable controls based on playoff type
    bool needsPlacement = (type == 2 || type == 3 || type == 4); // Types that use placement
    m_placementPairsTable->setEnabled(needsPlacement);
    
    bool canBeDivisionOnly = (type == 1 || type == 2); // Round robin types
    m_divisionOnlyPlayoffsCheck->setEnabled(canBeDivisionOnly);
}

void CompleteLeagueSetupDialog::onPointSystemChanged()
{
    int type = m_pointTypeGroup->checkedId();
    
    m_winLossGroup->setEnabled(type == 0);        // Win/Loss/Tie
    m_teamVsTeamGroup->setEnabled(type == 1);     // Team vs Team
    m_customPointsGroup->setEnabled(type == 2);   // Custom
}

void CompleteLeagueSetupDialog::onAddTeamClicked()
{
    QString teamName = m_teamNameEdit->text().trimmed();
    if (teamName.isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Please enter a team name.");
        return;
    }
    
    // Check for duplicate team names
    for (const TeamData &team : m_teams) {
        if (team.name == teamName) {
            QMessageBox::warning(this, "Duplicate Team", "A team with this name already exists.");
            return;
        }
    }
    
    // Create new team
    TeamData newTeam;
    newTeam.name = teamName;
    newTeam.teamId = m_teams.size() + 1; // Simple ID assignment
    
    m_teams.append(newTeam);
    m_teamsList->addItem(teamName);
    m_teamNameEdit->clear();
    
    // Update division assignment if needed
    onDivisionCountChanged(m_divisionCountSpinner->value());
}

void CompleteLeagueSetupDialog::onRemoveTeamClicked()
{
    int currentRow = m_teamsList->currentRow();
    if (currentRow < 0) {
        QMessageBox::information(this, "No Selection", "Please select a team to remove.");
        return;
    }
    
    m_teams.removeAt(currentRow);
    delete m_teamsList->takeItem(currentRow);
    
    // Update division assignment
    onDivisionCountChanged(m_divisionCountSpinner->value());
}

void CompleteLeagueSetupDialog::onAssignDivisionsClicked()
{
    // Simple automatic division assignment for preview
    int teamsPerDivision = qCeil(static_cast<double>(m_teams.size()) / m_divisionCountSpinner->value());
    
    for (int div = 0; div < m_divisionCountSpinner->value(); ++div) {
        QStringList divisionTeams;
        int startIdx = div * teamsPerDivision;
        int endIdx = qMin(startIdx + teamsPerDivision, m_teams.size());
        
        for (int i = startIdx; i < endIdx; ++i) {
            divisionTeams.append(m_teams[i].name);
        }
        
        QString teamsText = divisionTeams.isEmpty() ? "(No teams)" : divisionTeams.join(", ");
        m_divisionAssignmentTable->setItem(div, 1, new QTableWidgetItem(teamsText));
    }
}

void CompleteLeagueSetupDialog::populateTeamsList()
{
    m_availableBowlersList->clear();
    for (const BowlerInfo &bowler : m_availableBowlers) {
        QString displayText = QString("%1 %2 (Avg: %3)")
                             .arg(bowler.firstName)
                             .arg(bowler.lastName)
                             .arg(bowler.average, 0, 'f', 1);
        
        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, bowler.id);
        m_availableBowlersList->addItem(item);
    }
}