// LeagueScheduleDialog.cpp
#include "LeagueScheduleDialog.h"
#include <QSettings>
#include <QDebug>

LeagueScheduleDialog::LeagueScheduleDialog(QWidget *parent)
    : QDialog(parent)
    , m_dbManager(DatabaseManager::instance())
    , m_totalLanes(8)
{
    setupUI();
    setWindowTitle("Create League Schedule");
    setModal(true);
    resize(500, 600);
    
    // Load lane settings
    QSettings settings;
    m_totalLanes = settings.value("lanes/total", 8).toInt();
    
    // Set default values
    m_startDateEdit->setDate(QDate::currentDate().addDays(7)); // Next week
    m_startTimeEdit->setTime(QTime(19, 0)); // 7 PM
    m_durationSpinner->setValue(180); // 3 hours
    m_weeksSpinner->setValue(12); // 12 weeks
    
    updateFrequencyInfo();
}

void LeagueScheduleDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    m_mainLayout->setSpacing(10);
    
    // Create scroll area
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet("QScrollArea { border: none; }");
    
    m_scrollContent = new QWidget;
    m_contentLayout = new QVBoxLayout(m_scrollContent);
    m_contentLayout->setSpacing(15);
    
    setupBasicInfo();
    setupScheduleInfo();
    setupLaneSelection();
    
    m_scrollArea->setWidget(m_scrollContent);
    m_mainLayout->addWidget(m_scrollArea);
    
    setupButtons();
}

void LeagueScheduleDialog::setupBasicInfo()
{
    m_basicInfoGroup = new QGroupBox("League Information");
    m_basicInfoGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    
    m_basicInfoLayout = new QGridLayout(m_basicInfoGroup);
    m_basicInfoLayout->setSpacing(8);
    
    int row = 0;
    
    // League Name
    QLabel *nameLabel = new QLabel("League Name:");
    m_basicInfoLayout->addWidget(nameLabel, row, 0);
    
    m_leagueNameEdit = new QLineEdit;
    m_leagueNameEdit->setPlaceholderText("Enter league name...");
    m_basicInfoLayout->addWidget(m_leagueNameEdit, row, 1);
    row++;
    
    // Contact Name
    QLabel *contactLabel = new QLabel("Contact Name:");
    m_basicInfoLayout->addWidget(contactLabel, row, 0);
    
    m_contactNameEdit = new QLineEdit;
    m_contactNameEdit->setPlaceholderText("League coordinator name...");
    m_basicInfoLayout->addWidget(m_contactNameEdit, row, 1);
    row++;
    
    // Contact Phone
    QLabel *phoneLabel = new QLabel("Contact Phone:");
    m_basicInfoLayout->addWidget(phoneLabel, row, 0);
    
    m_contactPhoneEdit = new QLineEdit;
    m_contactPhoneEdit->setPlaceholderText("(555) 123-4567");
    m_basicInfoLayout->addWidget(m_contactPhoneEdit, row, 1);
    row++;
    
    // Contact Email
    QLabel *emailLabel = new QLabel("Contact Email:");
    m_basicInfoLayout->addWidget(emailLabel, row, 0);
    
    m_contactEmailEdit = new QLineEdit;
    m_contactEmailEdit->setPlaceholderText("coordinator@example.com");
    m_basicInfoLayout->addWidget(m_contactEmailEdit, row, 1);
    row++;
    
    m_contentLayout->addWidget(m_basicInfoGroup);
}

void LeagueScheduleDialog::setupScheduleInfo()
{
    m_scheduleInfoGroup = new QGroupBox("Schedule Information");
    m_scheduleInfoGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    
    m_scheduleInfoLayout = new QGridLayout(m_scheduleInfoGroup);
    m_scheduleInfoLayout->setSpacing(8);
    
    int row = 0;
    
    // Start Date
    QLabel *startLabel = new QLabel("Start Date:");
    m_scheduleInfoLayout->addWidget(startLabel, row, 0);
    
    m_startDateEdit = new QDateEdit;
    m_startDateEdit->setCalendarPopup(true);
    m_startDateEdit->setMinimumDate(QDate::currentDate());
    m_scheduleInfoLayout->addWidget(m_startDateEdit, row, 1);
    row++;
    
    // Start Time
    QLabel *timeLabel = new QLabel("Start Time:");
    m_scheduleInfoLayout->addWidget(timeLabel, row, 0);
    
    m_startTimeEdit = new QTimeEdit;
    m_startTimeEdit->setDisplayFormat("h:mm AP");
    m_scheduleInfoLayout->addWidget(m_startTimeEdit, row, 1);
    row++;
    
    // Duration
    QLabel *durationLabel = new QLabel("Duration (minutes):");
    m_scheduleInfoLayout->addWidget(durationLabel, row, 0);
    
    m_durationSpinner = new QSpinBox;
    m_durationSpinner->setRange(60, 300);
    m_durationSpinner->setSingleStep(30);
    m_durationSpinner->setSuffix(" min");
    m_scheduleInfoLayout->addWidget(m_durationSpinner, row, 1);
    row++;
    
    // Frequency
    QLabel *frequencyLabel = new QLabel("Frequency:");
    m_scheduleInfoLayout->addWidget(frequencyLabel, row, 0);
    
    m_frequencyCombo = new QComboBox;
    m_frequencyCombo->addItem("Weekly", 7);
    m_frequencyCombo->addItem("Bi-weekly", 14);
    m_frequencyCombo->addItem("Monthly", 30);
    connect(m_frequencyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &LeagueScheduleDialog::onFrequencyChanged);
    m_scheduleInfoLayout->addWidget(m_frequencyCombo, row, 1);
    row++;
    
    // Number of Weeks
    QLabel *weeksLabel = new QLabel("Number of Weeks:");
    m_scheduleInfoLayout->addWidget(weeksLabel, row, 0);
    
    m_weeksSpinner = new QSpinBox;
    m_weeksSpinner->setRange(1, 52);
    connect(m_weeksSpinner, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &LeagueScheduleDialog::updateFrequencyInfo);
    m_scheduleInfoLayout->addWidget(m_weeksSpinner, row, 1);
    row++;
    
    // Frequency info
    m_frequencyInfoLabel = new QLabel;
    m_frequencyInfoLabel->setStyleSheet("QLabel { color: #666; font-style: italic; }");
    m_frequencyInfoLabel->setWordWrap(true);
    m_scheduleInfoLayout->addWidget(m_frequencyInfoLabel, row, 0, 1, 2);
    
    m_contentLayout->addWidget(m_scheduleInfoGroup);
}

void LeagueScheduleDialog::setupLaneSelection()
{
    m_laneSelectionGroup = new QGroupBox("Lane Selection");
    m_laneSelectionGroup->setStyleSheet("QGroupBox { font-weight: bold; }");
    
    m_laneSelectionLayout = new QVBoxLayout(m_laneSelectionGroup);
    
    // Selection buttons
    QHBoxLayout *selectionBtnLayout = new QHBoxLayout;
    
    m_selectAllLanesBtn = new QPushButton("Select All");
    m_selectAllLanesBtn->setMaximumWidth(100);
    connect(m_selectAllLanesBtn, &QPushButton::clicked, [this]() {
        for (QCheckBox *checkbox : m_laneCheckboxes) {
            checkbox->setChecked(true);
        }
    });
    
    m_clearAllLanesBtn = new QPushButton("Clear All");
    m_clearAllLanesBtn->setMaximumWidth(100);
    connect(m_clearAllLanesBtn, &QPushButton::clicked, [this]() {
        for (QCheckBox *checkbox : m_laneCheckboxes) {
            checkbox->setChecked(false);
        }
    });
    
    selectionBtnLayout->addWidget(m_selectAllLanesBtn);
    selectionBtnLayout->addWidget(m_clearAllLanesBtn);
    selectionBtnLayout->addStretch();
    
    m_laneSelectionLayout->addLayout(selectionBtnLayout);
    
    // Lane checkboxes in a grid
    QGridLayout *laneGrid = new QGridLayout;
    int columns = 4;
    
    for (int i = 1; i <= m_totalLanes; ++i) {
        QCheckBox *laneCheckbox = new QCheckBox(QString("Lane %1").arg(i));
        laneCheckbox->setProperty("laneId", i);
        
        int row = (i - 1) / columns;
        int col = (i - 1) % columns;
        laneGrid->addWidget(laneCheckbox, row, col);
        
        m_laneCheckboxes.append(laneCheckbox);
    }
    
    QWidget *laneGridWidget = new QWidget;
    laneGridWidget->setLayout(laneGrid);
    m_laneSelectionLayout->addWidget(laneGridWidget);
    
    m_contentLayout->addWidget(m_laneSelectionGroup);
}

void LeagueScheduleDialog::setupButtons()
{
    m_buttonLayout = new QHBoxLayout;
    
    m_createBtn = new QPushButton("Create Schedule");
    m_createBtn->setStyleSheet("QPushButton { "
                              "background-color: #28a745; "
                              "color: white; "
                              "border: none; "
                              "padding: 10px 20px; "
                              "border-radius: 5px; "
                              "font-weight: bold; "
                              "} "
                              "QPushButton:hover { background-color: #218838; }");
    connect(m_createBtn, &QPushButton::clicked, this, &LeagueScheduleDialog::onCreateScheduleClicked);
    
    m_cancelBtn = new QPushButton("Cancel");
    m_cancelBtn->setStyleSheet("QPushButton { "
                              "background-color: #6c757d; "
                              "color: white; "
                              "border: none; "
                              "padding: 10px 20px; "
                              "border-radius: 5px; "
                              "} "
                              "QPushButton:hover { background-color: #5a6268; }");
    connect(m_cancelBtn, &QPushButton::clicked, this, &LeagueScheduleDialog::onCancelClicked);
    
    m_buttonLayout->addStretch();
    m_buttonLayout->addWidget(m_createBtn);
    m_buttonLayout->addWidget(m_cancelBtn);
    
    m_mainLayout->addLayout(m_buttonLayout);
}

void LeagueScheduleDialog::onFrequencyChanged()
{
    updateFrequencyInfo();
}

void LeagueScheduleDialog::updateFrequencyInfo()
{
    int frequency = m_frequencyCombo->currentData().toInt();
    int weeks = m_weeksSpinner->value();
    
    QDate startDate = m_startDateEdit->date();
    QDate endDate = startDate.addDays((weeks - 1) * frequency);
    
    QString info = QString("Schedule will run from %1 to %2 (%3 events total)")
                   .arg(startDate.toString("MMM d, yyyy"))
                   .arg(endDate.toString("MMM d, yyyy"))
                   .arg(weeks);
    
    m_frequencyInfoLabel->setText(info);
}

bool LeagueScheduleDialog::validateForm()
{
    if (m_leagueNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Please enter a league name.");
        m_leagueNameEdit->setFocus();
        return false;
    }
    
    if (m_contactNameEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Please enter a contact name.");
        m_contactNameEdit->setFocus();
        return false;
    }
    
    if (m_contactPhoneEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Please enter a contact phone number.");
        m_contactPhoneEdit->setFocus();
        return false;
    }
    
    // Check if at least one lane is selected
    bool anyLaneSelected = false;
    for (QCheckBox *checkbox : m_laneCheckboxes) {
        if (checkbox->isChecked()) {
            anyLaneSelected = true;
            break;
        }
    }
    
    if (!anyLaneSelected) {
        QMessageBox::warning(this, "No Lanes Selected", "Please select at least one lane for the league.");
        return false;
    }
    
    return true;
}

void LeagueScheduleDialog::onCreateScheduleClicked()
{
    if (!validateForm()) {
        return;
    }
    
    createSchedule();
}

void LeagueScheduleDialog::onCancelClicked()
{
    reject();
}

void LeagueScheduleDialog::createSchedule()
{
    // Collect selected lanes
    QVector<int> selectedLanes;
    for (QCheckBox *checkbox : m_laneCheckboxes) {
        if (checkbox->isChecked()) {
            selectedLanes.append(checkbox->property("laneId").toInt());
        }
    }
    
    // Create league schedule using DatabaseManager
    QVector<int> createdEvents = m_dbManager->addLeagueSchedule(
        0, // league ID - would come from league management system
        m_leagueNameEdit->text().trimmed(),
        m_startDateEdit->date(),
        m_startTimeEdit->time(),
        m_durationSpinner->value(),
        m_frequencyCombo->currentData().toInt(),
        m_weeksSpinner->value(),
        selectedLanes,
        m_contactNameEdit->text().trimmed(),
        m_contactPhoneEdit->text().trimmed(),
        m_contactEmailEdit->text().trimmed(),
        QString("League schedule for %1").arg(m_leagueNameEdit->text().trimmed())
    );
    
    if (!createdEvents.isEmpty()) {
        QMessageBox::information(this, "League Schedule Created", 
                               QString("Successfully created %1 league events for '%2'\n\n"
                                      "Events created: %3\n"
                                      "Lanes used: %4\n"
                                      "Duration: %5 weeks")
                               .arg(createdEvents.size())
                               .arg(m_leagueNameEdit->text())
                               .arg(createdEvents.size())
                               .arg(selectedLanes.size())
                               .arg(m_weeksSpinner->value()));
        
        accept(); // Close dialog
    } else {
        QMessageBox::warning(this, "League Schedule Error", 
                           "Failed to create league schedule. This may be due to:\n\n"
                           "• Conflicts with existing bookings\n"
                           "• Database connection issues\n"
                           "• Invalid date/time combinations\n\n"
                           "Please check for conflicts and try again.");
    }
}