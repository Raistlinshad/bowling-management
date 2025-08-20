#include "CalendarDialog.h"
#include "MainWindow.h"
#include <QSettings>
#include <QDebug>
#include <QJsonObject>

CalendarDialog::CalendarDialog(MainWindow *mainWindow, QWidget *parent)
    : QDialog(parent)
    , m_mainWindow(mainWindow)
    , m_dbManager(DatabaseManager::instance())
    , m_eventBus(new EventBus(this))
    , m_selectedDate(QDate::currentDate())
    , m_totalLanes(8)
    , m_startHour(8)
    , m_endHour(2)  // 2 AM next day
    , m_filteredLane(0)
    , m_editingEventId(-1)
    , m_hasSelectedEvent(false)
{
    setupUI();
    loadCalendarSettings();
    setWindowTitle("Calendar - Booking Management System");
    setModal(true);
    resize(1400, 900);
    
    // Center the dialog
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
    
    // Initialize calendar views
    updateMonthCalendar();
    updateDailySchedule();
}

void CalendarDialog::setupUI()
{
    m_mainLayout = new QHBoxLayout(this);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    m_mainLayout->setSpacing(5);
    
    m_mainSplitter = new QSplitter(Qt::Horizontal);
    m_mainLayout->addWidget(m_mainSplitter);
    
    setupLeftPane();
    setupCenterPane();
    setupRightPane();
    
    // Set splitter proportions: Month (1), Daily (2), Details+Form (2)
    m_mainSplitter->setSizes({300, 500, 600});
    m_mainSplitter->setStretchFactor(0, 0);
    m_mainSplitter->setStretchFactor(1, 1);
    m_mainSplitter->setStretchFactor(2, 1);
}

void CalendarDialog::setupLeftPane()
{
    m_leftFrame = new QFrame;
    m_leftFrame->setFrameStyle(QFrame::StyledPanel);
    m_leftFrame->setStyleSheet("QFrame { background-color: #2a2a2a; border: 1px solid #333; }");
    m_leftFrame->setMinimumWidth(280);
    m_leftFrame->setMaximumWidth(320);
    
    m_leftLayout = new QVBoxLayout(m_leftFrame);
    m_leftLayout->setContentsMargins(10, 10, 10, 10);
    m_leftLayout->setSpacing(8);
    
    // Title
    QLabel *titleLabel = new QLabel("CALENDAR");
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("QLabel { "
                             "color: white; "
                             "font-size: 16px; "
                             "font-weight: bold; "
                             "margin-bottom: 10px; "
                             "}");
    m_leftLayout->addWidget(titleLabel);
    
    // Month navigation
    QFrame *monthNavFrame = new QFrame;
    QHBoxLayout *monthNavLayout = new QHBoxLayout(monthNavFrame);
    monthNavLayout->setContentsMargins(0, 0, 0, 0);
    
    m_prevMonthBtn = new QPushButton("<");
    m_prevMonthBtn->setFixedSize(30, 30);
    m_prevMonthBtn->setStyleSheet("QPushButton { background-color: #333; color: white; border: 1px solid #555; } "
                                 "QPushButton:hover { background-color: #4A90E2; }");
    connect(m_prevMonthBtn, &QPushButton::clicked, this, &CalendarDialog::onPrevMonth);
    
    m_monthLabel = new QLabel(m_selectedDate.toString("MMMM yyyy"));
    m_monthLabel->setAlignment(Qt::AlignCenter);
    m_monthLabel->setStyleSheet("QLabel { color: white; font-weight: bold; font-size: 14px; }");
    
    m_nextMonthBtn = new QPushButton(">");
    m_nextMonthBtn->setFixedSize(30, 30);
    m_nextMonthBtn->setStyleSheet("QPushButton { background-color: #333; color: white; border: 1px solid #555; } "
                                 "QPushButton:hover { background-color: #4A90E2; }");
    connect(m_nextMonthBtn, &QPushButton::clicked, this, &CalendarDialog::onNextMonth);
    
    monthNavLayout->addWidget(m_prevMonthBtn);
    monthNavLayout->addWidget(m_monthLabel, 1);
    monthNavLayout->addWidget(m_nextMonthBtn);
    
    m_leftLayout->addWidget(monthNavFrame);
    
    // Calendar widget
    m_calendar = new QCalendarWidget;
    m_calendar->setStyleSheet(
        "QCalendarWidget QWidget { "
        "background-color: #1a1a1a; "
        "color: white; "
        "} "
        "QCalendarWidget QAbstractItemView { "
        "background-color: #1a1a1a; "
        "color: white; "
        "selection-background-color: #4A90E2; "
        "} "
        "QCalendarWidget QToolButton { "
        "background-color: #333; "
        "color: white; "
        "border: 1px solid #555; "
        "} "
        "QCalendarWidget QToolButton:hover { "
        "background-color: #4A90E2; "
        "} "
        "QCalendarWidget QSpinBox { "
        "background-color: #333; "
        "color: white; "
        "border: 1px solid #555; "
        "}"
    );
    m_calendar->setSelectedDate(m_selectedDate);
    connect(m_calendar, &QCalendarWidget::selectionChanged, this, &CalendarDialog::onDateSelected);
    m_leftLayout->addWidget(m_calendar);
    
    // Action buttons
    m_newBookingBtn = new QPushButton("New Booking");
    m_newBookingBtn->setStyleSheet("QPushButton { "
                                  "background-color: #28a745; "
                                  "color: white; "
                                  "border: none; "
                                  "padding: 10px; "
                                  "border-radius: 5px; "
                                  "font-weight: bold; "
                                  "} "
                                  "QPushButton:hover { background-color: #218838; }");
    connect(m_newBookingBtn, &QPushButton::clicked, this, &CalendarDialog::onNewBookingClicked);
    m_leftLayout->addWidget(m_newBookingBtn);
    
    m_leagueScheduleBtn = new QPushButton("League Schedule");
    m_leagueScheduleBtn->setStyleSheet("QPushButton { "
                                      "background-color: #6f42c1; "
                                      "color: white; "
                                      "border: none; "
                                      "padding: 10px; "
                                      "border-radius: 5px; "
                                      "font-weight: bold; "
                                      "} "
                                      "QPushButton:hover { background-color: #5a2d91; }");
    connect(m_leagueScheduleBtn, &QPushButton::clicked, this, &CalendarDialog::showLeagueScheduleDialog);
    m_leftLayout->addWidget(m_leagueScheduleBtn);
    
    m_leftLayout->addStretch();
    
    m_mainSplitter->addWidget(m_leftFrame);
}

void CalendarDialog::setupCenterPane()
{
    m_centerFrame = new QFrame;
    m_centerFrame->setFrameStyle(QFrame::StyledPanel);
    m_centerFrame->setStyleSheet("QFrame { background-color: #2a2a2a; border: 1px solid #333; }");
    
    m_centerLayout = new QVBoxLayout(m_centerFrame);
    m_centerLayout->setContentsMargins(10, 10, 10, 10);
    m_centerLayout->setSpacing(8);
    
    // Title
    QLabel *dailyTitle = new QLabel("DAILY SCHEDULE");
    dailyTitle->setAlignment(Qt::AlignCenter);
    dailyTitle->setStyleSheet("QLabel { "
                             "color: white; "
                             "font-size: 16px; "
                             "font-weight: bold; "
                             "margin-bottom: 5px; "
                             "}");
    m_centerLayout->addWidget(dailyTitle);
    
    // Day navigation
    QFrame *dayNavFrame = new QFrame;
    QHBoxLayout *dayNavLayout = new QHBoxLayout(dayNavFrame);
    dayNavLayout->setContentsMargins(0, 0, 0, 0);
    
    m_prevDayBtn = new QPushButton("<");
    m_prevDayBtn->setFixedSize(30, 30);
    m_prevDayBtn->setStyleSheet("QPushButton { background-color: #333; color: white; border: 1px solid #555; } "
                               "QPushButton:hover { background-color: #4A90E2; }");
    connect(m_prevDayBtn, &QPushButton::clicked, this, &CalendarDialog::onPrevDay);
    
    m_dayLabel = new QLabel(m_selectedDate.toString("dddd, MMMM d, yyyy"));
    m_dayLabel->setAlignment(Qt::AlignCenter);
    m_dayLabel->setStyleSheet("QLabel { color: white; font-weight: bold; font-size: 14px; }");
    
    m_nextDayBtn = new QPushButton(">");
    m_nextDayBtn->setFixedSize(30, 30);
    m_nextDayBtn->setStyleSheet("QPushButton { background-color: #333; color: white; border: 1px solid #555; } "
                               "QPushButton:hover { background-color: #4A90E2; }");
    connect(m_nextDayBtn, &QPushButton::clicked, this, &CalendarDialog::onNextDay);
    
    dayNavLayout->addWidget(m_prevDayBtn);
    dayNavLayout->addWidget(m_dayLabel, 1);
    dayNavLayout->addWidget(m_nextDayBtn);
    
    m_centerLayout->addWidget(dayNavFrame);
    
    // Lane filter
    m_laneFilterFrame = new QFrame;
    QHBoxLayout *laneFilterLayout = new QHBoxLayout(m_laneFilterFrame);
    laneFilterLayout->setContentsMargins(0, 0, 0, 0);
    
    m_laneFilterLabel = new QLabel("Lane Filter:");
    m_laneFilterLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
    
    m_laneFilterCombo = new QComboBox;
    m_laneFilterCombo->setStyleSheet("QComboBox { "
                                    "background-color: #333; "
                                    "color: white; "
                                    "border: 1px solid #555; "
                                    "padding: 5px; "
                                    "border-radius: 3px; "
                                    "}");
    m_laneFilterCombo->addItem("All Lanes", 0);
    for (int i = 1; i <= m_totalLanes; ++i) {
        m_laneFilterCombo->addItem(QString("Lane %1").arg(i), i);
    }
    connect(m_laneFilterCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &CalendarDialog::onLaneFilterChanged);
    
    laneFilterLayout->addWidget(m_laneFilterLabel);
    laneFilterLayout->addWidget(m_laneFilterCombo);
    laneFilterLayout->addStretch();
    
    m_centerLayout->addWidget(m_laneFilterFrame);
    
    // Schedule scroll area
    m_scheduleScrollArea = new QScrollArea;
    m_scheduleScrollArea->setWidgetResizable(true);
    m_scheduleScrollArea->setStyleSheet("QScrollArea { border: none; background-color: #2a2a2a; }");
    
    m_scheduleContent = new QWidget;
    m_scheduleContent->setStyleSheet("QWidget { background-color: #2a2a2a; }");
    
    m_scheduleLayout = new QVBoxLayout(m_scheduleContent);
    m_scheduleLayout->setSpacing(2);
    m_scheduleLayout->setContentsMargins(5, 5, 5, 5);
    
    m_scheduleScrollArea->setWidget(m_scheduleContent);
    m_centerLayout->addWidget(m_scheduleScrollArea);
    
    m_mainSplitter->addWidget(m_centerFrame);
}

void CalendarDialog::setupRightPane()
{
    m_rightFrame = new QFrame;
    m_rightFrame->setFrameStyle(QFrame::StyledPanel);
    m_rightFrame->setStyleSheet("QFrame { background-color: #2a2a2a; border: 1px solid #333; }");
    
    m_rightLayout = new QVBoxLayout(m_rightFrame);
    m_rightLayout->setContentsMargins(5, 5, 5, 5);
    m_rightLayout->setSpacing(5);
    
    m_rightSplitter = new QSplitter(Qt::Vertical);
    m_rightLayout->addWidget(m_rightSplitter);
    
    setupEventDetails();
    setupBookingForm();
    
    // Set vertical splitter proportions
    m_rightSplitter->setSizes({300, 400});
    m_rightSplitter->setStretchFactor(0, 1);
    m_rightSplitter->setStretchFactor(1, 1);
    
    m_mainSplitter->addWidget(m_rightFrame);
}

void CalendarDialog::setupEventDetails()
{
    m_detailsFrame = new QFrame;
    m_detailsFrame->setFrameStyle(QFrame::StyledPanel);
    m_detailsFrame->setStyleSheet("QFrame { background-color: #1a1a1a; border: 1px solid #333; }");
    
    m_detailsLayout = new QVBoxLayout(m_detailsFrame);
    m_detailsLayout->setContentsMargins(10, 10, 10, 10);
    m_detailsLayout->setSpacing(8);
    
    // Title
    QLabel *detailsTitle = new QLabel("EVENT DETAILS");
    detailsTitle->setAlignment(Qt::AlignCenter);
    detailsTitle->setStyleSheet("QLabel { "
                               "color: white; "
                               "font-size: 14px; "
                               "font-weight: bold; "
                               "margin-bottom: 10px; "
                               "}");
    m_detailsLayout->addWidget(detailsTitle);
    
    // Scroll area for details
    m_detailsScrollArea = new QScrollArea;
    m_detailsScrollArea->setWidgetResizable(true);
    m_detailsScrollArea->setStyleSheet("QScrollArea { border: none; background-color: #1a1a1a; }");
    
    m_detailsContent = new QWidget;
    m_detailsContent->setStyleSheet("QWidget { background-color: #1a1a1a; }");
    
    m_detailsContentLayout = new QVBoxLayout(m_detailsContent);
    m_detailsContentLayout->setSpacing(8);
    m_detailsContentLayout->setContentsMargins(5, 5, 5, 5);
    
    // Initial message
    QLabel *instructionLabel = new QLabel("Click on an event in the daily schedule to view details");
    instructionLabel->setStyleSheet("QLabel { color: #888; font-style: italic; }");
    instructionLabel->setWordWrap(true);
    instructionLabel->setAlignment(Qt::AlignCenter);
    m_detailsContentLayout->addWidget(instructionLabel);
    m_detailsContentLayout->addStretch();
    
    m_detailsScrollArea->setWidget(m_detailsContent);
    m_detailsLayout->addWidget(m_detailsScrollArea);
    
    // Action buttons
    QFrame *detailsButtonFrame = new QFrame;
    QHBoxLayout *detailsButtonLayout = new QHBoxLayout(detailsButtonFrame);
    detailsButtonLayout->setContentsMargins(0, 0, 0, 0);
    
    m_editEventBtn = new QPushButton("Edit Event");
    m_editEventBtn->setStyleSheet("QPushButton { "
                                 "background-color: #4A90E2; "
                                 "color: white; "
                                 "border: none; "
                                 "padding: 8px 16px; "
                                 "border-radius: 3px; "
                                 "font-weight: bold; "
                                 "} "
                                 "QPushButton:hover { background-color: #357ABD; }");
    m_editEventBtn->setEnabled(false);
    connect(m_editEventBtn, &QPushButton::clicked, this, &CalendarDialog::onEditEventClicked);
    
    m_deleteEventBtn = new QPushButton("Delete Event");
    m_deleteEventBtn->setStyleSheet("QPushButton { "
                                   "background-color: #dc3545; "
                                   "color: white; "
                                   "border: none; "
                                   "padding: 8px 16px; "
                                   "border-radius: 3px; "
                                   "font-weight: bold; "
                                   "} "
                                   "QPushButton:hover { background-color: #c82333; }");
    m_deleteEventBtn->setEnabled(false);
    connect(m_deleteEventBtn, &QPushButton::clicked, this, &CalendarDialog::onDeleteEventClicked);
    
    detailsButtonLayout->addWidget(m_editEventBtn);
    detailsButtonLayout->addWidget(m_deleteEventBtn);
    detailsButtonLayout->addStretch();
    
    m_detailsLayout->addWidget(detailsButtonFrame);
    
    m_rightSplitter->addWidget(m_detailsFrame);
}

void CalendarDialog::setupBookingForm()
{
    m_bookingFrame = new QFrame;
    m_bookingFrame->setFrameStyle(QFrame::StyledPanel);
    m_bookingFrame->setStyleSheet("QFrame { background-color: #1a1a1a; border: 1px solid #333; }");
    
    m_bookingLayout = new QVBoxLayout(m_bookingFrame);
    m_bookingLayout->setContentsMargins(10, 10, 10, 10);
    m_bookingLayout->setSpacing(8);
    
    // Title
    QLabel *bookingTitle = new QLabel("BOOKING FORM");
    bookingTitle->setAlignment(Qt::AlignCenter);
    bookingTitle->setStyleSheet("QLabel { "
                               "color: white; "
                               "font-size: 14px; "
                               "font-weight: bold; "
                               "margin-bottom: 10px; "
                               "}");
    m_bookingLayout->addWidget(bookingTitle);
    
    // Scroll area for form
    m_bookingScrollArea = new QScrollArea;
    m_bookingScrollArea->setWidgetResizable(true);
    m_bookingScrollArea->setStyleSheet("QScrollArea { border: none; background-color: #1a1a1a; }");
    
    m_bookingContent = new QWidget;
    m_bookingContent->setStyleSheet("QWidget { background-color: #1a1a1a; }");
    
    m_bookingFormLayout = new QGridLayout(m_bookingContent);
    m_bookingFormLayout->setSpacing(8);
    m_bookingFormLayout->setContentsMargins(5, 5, 5, 5);
    
    QString labelStyle = "QLabel { color: white; font-weight: bold; font-size: 11px; }";
    QString fieldStyle = "QLineEdit, QTimeEdit, QDateEdit, QSpinBox, QComboBox, QTextEdit { "
                        "background-color: #333; "
                        "color: white; "
                        "border: 1px solid #555; "
                        "padding: 6px; "
                        "border-radius: 3px; "
                        "font-size: 11px; "
                        "}";
    
    int row = 0;
    
    // Date
    QLabel *dateLabel = new QLabel("Date:");
    dateLabel->setStyleSheet(labelStyle);
    m_bookingFormLayout->addWidget(dateLabel, row, 0);
    
    m_dateEdit = new QDateEdit(m_selectedDate);
    m_dateEdit->setStyleSheet(fieldStyle);
    m_dateEdit->setCalendarPopup(true);
    m_bookingFormLayout->addWidget(m_dateEdit, row, 1);
    row++;
    
    // Start Time
    QLabel *timeLabel = new QLabel("Start Time:");
    timeLabel->setStyleSheet(labelStyle);
    m_bookingFormLayout->addWidget(timeLabel, row, 0);
    
    m_startTimeEdit = new QTimeEdit(QTime(18, 0));
    m_startTimeEdit->setStyleSheet(fieldStyle);
    m_startTimeEdit->setDisplayFormat("hh:mm AP");
    m_bookingFormLayout->addWidget(m_startTimeEdit, row, 1);
    row++;
    
    // Duration
    QLabel *durationLabel = new QLabel("Duration (min):");
    durationLabel->setStyleSheet(labelStyle);
    m_bookingFormLayout->addWidget(durationLabel, row, 0);
    
    m_durationSpinner = new QSpinBox;
    m_durationSpinner->setStyleSheet(fieldStyle);
    m_durationSpinner->setRange(30, 480);
    m_durationSpinner->setValue(120);
    m_durationSpinner->setSingleStep(30);
    m_bookingFormLayout->addWidget(m_durationSpinner, row, 1);
    row++;
    
    // Lane
    QLabel *laneLabel = new QLabel("Lane:");
    laneLabel->setStyleSheet(labelStyle);
    m_bookingFormLayout->addWidget(laneLabel, row, 0);
    
    m_laneCombo = new QComboBox;
    m_laneCombo->setStyleSheet(fieldStyle);
    for (int i = 1; i <= m_totalLanes; ++i) {
        m_laneCombo->addItem(QString("Lane %1").arg(i), i);
    }
    m_bookingFormLayout->addWidget(m_laneCombo, row, 1);
    row++;
    
    // Event Type
    QLabel *typeLabel = new QLabel("Event Type:");
    typeLabel->setStyleSheet(labelStyle);
    m_bookingFormLayout->addWidget(typeLabel, row, 0);
    
    m_eventTypeCombo = new QComboBox;
    m_eventTypeCombo->setStyleSheet(fieldStyle);
    m_eventTypeCombo->addItems({"Open Bowling", "League", "Tournament", "Party", "Corporate", "Other"});
    m_bookingFormLayout->addWidget(m_eventTypeCombo, row, 1);
    row++;
    
    // Title
    QLabel *titleLabel = new QLabel("Title:");
    titleLabel->setStyleSheet(labelStyle);
    m_bookingFormLayout->addWidget(titleLabel, row, 0);
    
    m_titleEdit = new QLineEdit;
    m_titleEdit->setStyleSheet(fieldStyle);
    m_titleEdit->setPlaceholderText("Event title...");
    m_bookingFormLayout->addWidget(m_titleEdit, row, 1);
    row++;
    
    // Description
    QLabel *descLabel = new QLabel("Description:");
    descLabel->setStyleSheet(labelStyle);
    m_bookingFormLayout->addWidget(descLabel, row, 0);
    
    m_descriptionEdit = new QTextEdit;
    m_descriptionEdit->setStyleSheet(fieldStyle);
    m_descriptionEdit->setMaximumHeight(60);
    m_descriptionEdit->setPlaceholderText("Event description...");
    m_bookingFormLayout->addWidget(m_descriptionEdit, row, 1);
    row++;
    
    // Contact Name
    QLabel *contactLabel = new QLabel("Contact Name:");
    contactLabel->setStyleSheet(labelStyle);
    m_bookingFormLayout->addWidget(contactLabel, row, 0);
    
    m_contactNameEdit = new QLineEdit;
    m_contactNameEdit->setStyleSheet(fieldStyle);
    m_contactNameEdit->setPlaceholderText("Contact person...");
    m_bookingFormLayout->addWidget(m_contactNameEdit, row, 1);
    row++;
    
    // Contact Phone
    QLabel *phoneLabel = new QLabel("Contact Phone:");
    phoneLabel->setStyleSheet(labelStyle);
    m_bookingFormLayout->addWidget(phoneLabel, row, 0);
    
    m_contactPhoneEdit = new QLineEdit;
    m_contactPhoneEdit->setStyleSheet(fieldStyle);
    m_contactPhoneEdit->setPlaceholderText("(555) 123-4567");
    m_bookingFormLayout->addWidget(m_contactPhoneEdit, row, 1);
    row++;
    
    // Contact Email
    QLabel *emailLabel = new QLabel("Contact Email:");
    emailLabel->setStyleSheet(labelStyle);
    m_bookingFormLayout->addWidget(emailLabel, row, 0);
    
    m_contactEmailEdit = new QLineEdit;
    m_contactEmailEdit->setStyleSheet(fieldStyle);
    m_contactEmailEdit->setPlaceholderText("email@example.com");
    m_bookingFormLayout->addWidget(m_contactEmailEdit, row, 1);
    row++;
    
    // Bowler Count
    QLabel *bowlerLabel = new QLabel("Number of Bowlers:");
    bowlerLabel->setStyleSheet(labelStyle);
    m_bookingFormLayout->addWidget(bowlerLabel, row, 0);
    
    m_bowlerCountSpinner = new QSpinBox;
    m_bowlerCountSpinner->setStyleSheet(fieldStyle);
    m_bowlerCountSpinner->setRange(1, 8);
    m_bowlerCountSpinner->setValue(1);
    m_bookingFormLayout->addWidget(m_bowlerCountSpinner, row, 1);
    row++;
    
    // Additional Details
    QLabel *detailsLabel = new QLabel("Additional Details:");
    detailsLabel->setStyleSheet(labelStyle);
    m_bookingFormLayout->addWidget(detailsLabel, row, 0);
    
    m_additionalDetailsEdit = new QTextEdit;
    m_additionalDetailsEdit->setStyleSheet(fieldStyle);
    m_additionalDetailsEdit->setMaximumHeight(80);
    m_additionalDetailsEdit->setPlaceholderText("Additional notes...");
    m_bookingFormLayout->addWidget(m_additionalDetailsEdit, row, 1);
    row++;
    
    // Form buttons
    QFrame *formButtonFrame = new QFrame;
    QHBoxLayout *formButtonLayout = new QHBoxLayout(formButtonFrame);
    formButtonLayout->setContentsMargins(0, 0, 0, 0);
    
    m_checkAvailabilityBtn = new QPushButton("Check Availability");
    m_checkAvailabilityBtn->setStyleSheet("QPushButton { "
                                         "background-color: #17a2b8; "
                                         "color: white; "
                                         "border: none; "
                                         "padding: 8px 12px; "
                                         "border-radius: 3px; "
                                         "font-weight: bold; "
                                         "} "
                                         "QPushButton:hover { background-color: #138496; }");
    connect(m_checkAvailabilityBtn, &QPushButton::clicked, this, &CalendarDialog::onCheckAvailabilityClicked);
    
    m_saveBookingBtn = new QPushButton("Save Booking");
    m_saveBookingBtn->setStyleSheet("QPushButton { "
                                   "background-color: #28a745; "
                                   "color: white; "
                                   "border: none; "
                                   "padding: 8px 16px; "
                                   "border-radius: 3px; "
                                   "font-weight: bold; "
                                   "} "
                                   "QPushButton:hover { background-color: #218838; }");
    connect(m_saveBookingBtn, &QPushButton::clicked, this, &CalendarDialog::onSaveBookingClicked);
    
    m_clearFormBtn = new QPushButton("Clear Form");
    m_clearFormBtn->setStyleSheet("QPushButton { "
                                 "background-color: #6c757d; "
                                 "color: white; "
                                 "border: none; "
                                 "padding: 8px 16px; "
                                 "border-radius: 3px; "
                                 "} "
                                 "QPushButton:hover { background-color: #5a6268; }");
    connect(m_clearFormBtn, &QPushButton::clicked, this, &CalendarDialog::clearBookingForm);
    
    formButtonLayout->addWidget(m_checkAvailabilityBtn);
    formButtonLayout->addWidget(m_saveBookingBtn);
    formButtonLayout->addWidget(m_clearFormBtn);
    
    m_bookingFormLayout->addWidget(formButtonFrame, row, 0, 1, 2);
    row++;
    
    // Close button
    m_closeBtn = new QPushButton("Close Calendar");
    m_closeBtn->setStyleSheet("QPushButton { "
                             "background-color: #dc3545; "
                             "color: white; "
                             "border: none; "
                             "padding: 10px; "
                             "border-radius: 3px; "
                             "font-weight: bold; "
                             "} "
                             "QPushButton:hover { background-color: #c82333; }");
    connect(m_closeBtn, &QPushButton::clicked, this, &CalendarDialog::onCloseClicked);
    m_bookingFormLayout->addWidget(m_closeBtn, row, 0, 1, 2);
    
    m_bookingScrollArea->setWidget(m_bookingContent);
    m_bookingLayout->addWidget(m_bookingScrollArea);
    
    m_rightSplitter->addWidget(m_bookingFrame);
}

void CalendarDialog::loadCalendarSettings()
{
    QSettings settings;
    m_totalLanes = settings.value("lanes/total", 8).toInt();
    m_startHour = settings.value("calendar/start_hour", 8).toInt();
    m_endHour = settings.value("calendar/end_hour", 2).toInt();
    
    // Update lane combo box and filter
    if (m_laneCombo) {
        m_laneCombo->clear();
        for (int i = 1; i <= m_totalLanes; ++i) {
            m_laneCombo->addItem(QString("Lane %1").arg(i), i);
        }
    }
    
    if (m_laneFilterCombo) {
        m_laneFilterCombo->clear();
        m_laneFilterCombo->addItem("All Lanes", 0);
        for (int i = 1; i <= m_totalLanes; ++i) {
            m_laneFilterCombo->addItem(QString("Lane %1").arg(i), i);
        }
    }
}

void CalendarDialog::saveCalendarSettings()
{
    QSettings settings;
    settings.setValue("calendar/start_hour", m_startHour);
    settings.setValue("calendar/end_hour", m_endHour);
}

void CalendarDialog::onDateSelected()
{
    m_selectedDate = m_calendar->selectedDate();
    m_dateEdit->setDate(m_selectedDate);
    m_dayLabel->setText(m_selectedDate.toString("dddd, MMMM d, yyyy"));
    m_monthLabel->setText(m_selectedDate.toString("MMMM yyyy"));
    
    updateDailySchedule();
    clearEventDetails();
}

void CalendarDialog::onNewBookingClicked()
{
    clearBookingForm();
    m_editingEventId = -1;
    m_dateEdit->setDate(m_selectedDate);
}

void CalendarDialog::onSaveBookingClicked()
{
    if (!validateBookingForm()) {
        return;
    }
    
    saveEvent();
}

void CalendarDialog::onCloseClicked()
{
    saveCalendarSettings();
    close();
}

void CalendarDialog::onPrevDay()
{
    m_selectedDate = m_selectedDate.addDays(-1);
    m_calendar->setSelectedDate(m_selectedDate);
    updateDailySchedule();
    updateMonthCalendar();
}

void CalendarDialog::onNextDay()
{
    m_selectedDate = m_selectedDate.addDays(1);
    m_calendar->setSelectedDate(m_selectedDate);
    updateDailySchedule();
    updateMonthCalendar();
}

void CalendarDialog::onPrevMonth()
{
    m_selectedDate = m_selectedDate.addMonths(-1);
    m_calendar->setSelectedDate(m_selectedDate);
    updateMonthCalendar();
    updateDailySchedule();
}

void CalendarDialog::onNextMonth()
{
    m_selectedDate = m_selectedDate.addMonths(1);
    m_calendar->setSelectedDate(m_selectedDate);
    updateMonthCalendar();
    updateDailySchedule();
}

void CalendarDialog::onLaneFilterChanged()
{
    m_filteredLane = m_laneFilterCombo->currentData().toInt();
    updateDailySchedule();
}

void CalendarDialog::onCheckAvailabilityClicked()
{
    QDate date = m_dateEdit->date();
    QTime startTime = m_startTimeEdit->time();
    int duration = m_durationSpinner->value();
    QTime endTime = startTime.addSecs(duration * 60);
    int laneId = m_laneCombo->currentData().toInt();
    
    if (m_dbManager->isLaneAvailable(date, startTime, endTime, laneId, m_editingEventId)) {
        QMessageBox::information(this, "Availability Check", 
                                QString("Lane %1 is available on %2 from %3 to %4")
                                .arg(laneId)
                                .arg(date.toString("MMMM d, yyyy"))
                                .arg(startTime.toString("h:mm AP"))
                                .arg(endTime.toString("h:mm AP")));
    } else {
        QVector<CalendarEventData> conflicts = m_dbManager->getConflictingEvents(
            date, startTime, endTime, laneId, m_editingEventId);
        
        QString conflictMsg = QString("Lane %1 is NOT available during this time.\n\nConflicting events:\n")
                             .arg(laneId);
        
        for (const CalendarEventData &conflict : conflicts) {
            conflictMsg += QString("• %1 (%2 - %3)\n")
                          .arg(conflict.title)
                          .arg(conflict.startTime.toString("h:mm AP"))
                          .arg(conflict.endTime.toString("h:mm AP"));
        }
        
        QMessageBox::warning(this, "Booking Conflict", conflictMsg);
    }
}

void CalendarDialog::onEditEventClicked()
{
    if (m_hasSelectedEvent) {
        editEvent(m_selectedEvent.id);
    }
}

void CalendarDialog::onDeleteEventClicked()
{
    if (m_hasSelectedEvent) {
        deleteEvent(m_selectedEvent.id);
    }
}

void CalendarDialog::updateDailySchedule()
{
    // Clear existing schedule
    while (QLayoutItem* item = m_scheduleLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
    m_timeSlots.clear();
    
    // Get events for the selected date
    QVector<CalendarEventData> events = m_dbManager->getCalendarEventsForDate(m_selectedDate);
    
    // Filter by lane if needed
    if (m_filteredLane > 0) {
        events.erase(std::remove_if(events.begin(), events.end(),
                    [this](const CalendarEventData &event) {
                        return event.laneId != m_filteredLane;
                    }), events.end());
    }
    
    // Create time slots
    populateTimeSlots();
    
    // Add events to appropriate time slots
    for (const CalendarEventData &event : events) {
        addEventToTimeSlot(event.startTime, event);
    }
}

void CalendarDialog::populateTimeSlots()
{
    // Create time slots from start hour to end hour in 30-minute increments
    QTime currentTime(m_startHour, 0);
    QTime endTime(m_endHour, 0);
    
    // Handle overnight schedules (e.g., 8 AM to 2 AM next day)
    if (m_endHour <= m_startHour) {
        endTime = endTime.addSecs(24 * 3600); // Add 24 hours
    }
    
    while (currentTime <= endTime) {
        createTimeSlot(currentTime);
        currentTime = currentTime.addSecs(30 * 60); // Add 30 minutes
        
        // Handle crossing midnight
        if (currentTime.hour() == 0 && currentTime.minute() == 0 && m_endHour <= m_startHour) {
            break;
        }
    }
}

void CalendarDialog::createTimeSlot(const QTime &time)
{
    QFrame *timeSlotFrame = new QFrame;
    timeSlotFrame->setFrameStyle(QFrame::StyledPanel);
    timeSlotFrame->setStyleSheet("QFrame { background-color: #1a1a1a; border: 1px solid #333; margin: 1px; }");
    timeSlotFrame->setMinimumHeight(40);
    
    QHBoxLayout *timeSlotLayout = new QHBoxLayout(timeSlotFrame);
    timeSlotLayout->setContentsMargins(5, 5, 5, 5);
    timeSlotLayout->setSpacing(5);
    
    // Time label
    QLabel *timeLabel = new QLabel(time.toString("h:mm AP"));
    timeLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
    timeLabel->setFixedWidth(80);
    timeSlotLayout->addWidget(timeLabel);
    
    // Events container
    QFrame *eventsFrame = new QFrame;
    eventsFrame->setStyleSheet("QFrame { background-color: #2a2a2a; border: 1px solid #444; }");
    QHBoxLayout *eventsLayout = new QHBoxLayout(eventsFrame);
    eventsLayout->setContentsMargins(5, 5, 5, 5);
    eventsLayout->setSpacing(5);
    
    timeSlotLayout->addWidget(eventsFrame, 1);
    
    m_timeSlots[time] = timeSlotFrame;
    m_scheduleLayout->addWidget(timeSlotFrame);
}

void CalendarDialog::addEventToTimeSlot(const QTime &time, const CalendarEventData &event)
{
    if (!m_timeSlots.contains(time)) {
        return;
    }
    
    QFrame *timeSlotFrame = m_timeSlots[time];
    QHBoxLayout *timeSlotLayout = qobject_cast<QHBoxLayout*>(timeSlotFrame->layout());
    if (!timeSlotLayout || timeSlotLayout->count() < 2) {
        return;
    }
    
    QFrame *eventsFrame = qobject_cast<QFrame*>(timeSlotLayout->itemAt(1)->widget());
    if (!eventsFrame) {
        return;
    }
    
    QHBoxLayout *eventsLayout = qobject_cast<QHBoxLayout*>(eventsFrame->layout());
    if (!eventsLayout) {
        return;
    }
    
    // Create event widget
    QPushButton *eventButton = new QPushButton;
    eventButton->setMinimumHeight(30);
    
    // Set event text and styling based on type
    QString eventText = QString("Lane %1: %2").arg(event.laneId).arg(event.title);
    QTime endTime = event.endTime;
    int duration = event.startTime.secsTo(endTime) / 60;
    eventText += QString(" (%1 min)").arg(duration);
    
    eventButton->setText(eventText);
    
    // Color coding by event type
    QString buttonStyle = "QPushButton { "
                         "text-align: left; "
                         "padding: 5px; "
                         "border-radius: 3px; "
                         "font-weight: bold; "
                         "border: 1px solid #555; "
                         "}";
    
    if (event.eventType == "League") {
        buttonStyle += "QPushButton { background-color: #6f42c1; color: white; } "
                      "QPushButton:hover { background-color: #5a2d91; }";
    } else if (event.eventType == "Tournament") {
        buttonStyle += "QPushButton { background-color: #dc3545; color: white; } "
                      "QPushButton:hover { background-color: #c82333; }";
    } else if (event.eventType == "Party") {
        buttonStyle += "QPushButton { background-color: #fd7e14; color: white; } "
                      "QPushButton:hover { background-color: #e76500; }";
    } else if (event.eventType == "Corporate") {
        buttonStyle += "QPushButton { background-color: #20c997; color: white; } "
                      "QPushButton:hover { background-color: #1ba085; }";
    } else {
        buttonStyle += "QPushButton { background-color: #28a745; color: white; } "
                      "QPushButton:hover { background-color: #218838; }";
    }
    
    eventButton->setStyleSheet(buttonStyle);
    
    // Connect click to show event details
    connect(eventButton, &QPushButton::clicked, [this, event]() {
        showEventDetails(event);
    });
    
    eventsLayout->addWidget(eventButton);
}

void CalendarDialog::updateMonthCalendar()
{
    m_monthLabel->setText(m_selectedDate.toString("MMMM yyyy"));
    m_dayLabel->setText(m_selectedDate.toString("dddd, MMMM d, yyyy"));
    
    // You could enhance this to highlight dates with events
    // For now, just update the labels
}

void CalendarDialog::refreshCalendarViews()
{
    updateMonthCalendar();
    updateDailySchedule();
}

void CalendarDialog::clearBookingForm()
{
    m_dateEdit->setDate(m_selectedDate);
    m_startTimeEdit->setTime(QTime(18, 0));
    m_durationSpinner->setValue(120);
    m_laneCombo->setCurrentIndex(0);
    m_eventTypeCombo->setCurrentIndex(0);
    m_titleEdit->clear();
    m_descriptionEdit->clear();
    m_contactNameEdit->clear();
    m_contactPhoneEdit->clear();
    m_contactEmailEdit->clear();
    m_bowlerCountSpinner->setValue(1);
    m_additionalDetailsEdit->clear();
    m_editingEventId = -1;
}

void CalendarDialog::clearEventDetails()
{
    // Clear the details content
    while (QLayoutItem* item = m_detailsContentLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
    
    // Add instruction label
    QLabel *instructionLabel = new QLabel("Click on an event in the daily schedule to view details");
    instructionLabel->setStyleSheet("QLabel { color: #888; font-style: italic; }");
    instructionLabel->setWordWrap(true);
    instructionLabel->setAlignment(Qt::AlignCenter);
    m_detailsContentLayout->addWidget(instructionLabel);
    m_detailsContentLayout->addStretch();
    
    m_hasSelectedEvent = false;
    m_editEventBtn->setEnabled(false);
    m_deleteEventBtn->setEnabled(false);
}

bool CalendarDialog::validateBookingForm()
{
    if (m_titleEdit->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Please enter a title for the booking.");
        m_titleEdit->setFocus();
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
    
    return true;
}

void CalendarDialog::saveEvent()
{
    CalendarEventData event;
    
    // Basic event data
    event.id = m_editingEventId > 0 ? m_editingEventId : 0;
    event.date = m_dateEdit->date();
    event.startTime = m_startTimeEdit->time();
    event.endTime = m_startTimeEdit->time().addSecs(m_durationSpinner->value() * 60);
    event.laneId = m_laneCombo->currentData().toInt();
    event.eventType = m_eventTypeCombo->currentText();
    event.title = m_titleEdit->text().trimmed();
    event.description = m_descriptionEdit->toPlainText().trimmed();
    event.contactName = m_contactNameEdit->text().trimmed();
    event.contactPhone = m_contactPhoneEdit->text().trimmed();
    event.contactEmail = m_contactEmailEdit->text().trimmed();
    event.bowlerCount = m_bowlerCountSpinner->value();
    event.additionalDetails = m_additionalDetailsEdit->toPlainText().trimmed();
    
    // Check for conflicts
    if (!m_dbManager->isLaneAvailable(event.date, event.startTime, event.endTime, 
                                     event.laneId, m_editingEventId)) {
        QVector<CalendarEventData> conflicts = m_dbManager->getConflictingEvents(
            event.date, event.startTime, event.endTime, event.laneId, m_editingEventId);
        
        QString conflictMsg = QString("This booking conflicts with existing events:\n");
        for (const CalendarEventData &conflict : conflicts) {
            conflictMsg += QString("• %1 (%2 - %3)\n")
                          .arg(conflict.title)
                          .arg(conflict.startTime.toString("h:mm AP"))
                          .arg(conflict.endTime.toString("h:mm AP"));
        }
        
        QMessageBox::warning(this, "Booking Conflict", conflictMsg);
        return;
    }
    
    int result;
    if (m_editingEventId > 0) {
        // Update existing event
        if (m_dbManager->updateCalendarEvent(event)) {
            result = m_editingEventId;
        } else {
            result = -1;
        }
    } else {
        // Add new event
        result = m_dbManager->addCalendarEvent(event);
    }
    
    if (result > 0) {
        QString action = (m_editingEventId > 0) ? "updated" : "saved";
        QMessageBox::information(this, "Booking Saved", 
                               QString("Booking '%1' has been %2 successfully!")
                               .arg(event.title).arg(action));
        
        // Emit event bus notification
        QJsonObject eventData;
        eventData["id"] = result;
        eventData["date"] = event.date.toString(Qt::ISODate);
        eventData["lane_id"] = event.laneId;
        eventData["start_time"] = event.startTime.toString("hh:mm:ss");
        eventData["end_time"] = event.endTime.toString("hh:mm:ss");
        eventData["event_type"] = event.eventType;
        eventData["title"] = event.title;
        
        QString eventType = (m_editingEventId > 0) ? "booking_updated" : "booking_created";
        m_eventBus->publish("calendar", eventType, eventData);
        
        clearBookingForm();
        refreshCalendarViews();
    } else {
        QMessageBox::critical(this, "Error", "Failed to save the booking. Please try again.");
    }
}

void CalendarDialog::editEvent(int eventId)
{
    CalendarEventData event = m_dbManager->getCalendarEventById(eventId);
    if (event.id == 0) {
        QMessageBox::warning(this, "Event Not Found", "The selected event could not be found.");
        return;
    }
    
    // Populate form with event data
    m_editingEventId = eventId;
    m_dateEdit->setDate(event.date);
    m_startTimeEdit->setTime(event.startTime);
    
    int duration = event.startTime.secsTo(event.endTime) / 60;
    m_durationSpinner->setValue(duration);
    
    // Set lane combo
    for (int i = 0; i < m_laneCombo->count(); ++i) {
        if (m_laneCombo->itemData(i).toInt() == event.laneId) {
            m_laneCombo->setCurrentIndex(i);
            break;
        }
    }
    
    // Set event type combo
    int typeIndex = m_eventTypeCombo->findText(event.eventType);
    if (typeIndex >= 0) {
        m_eventTypeCombo->setCurrentIndex(typeIndex);
    }
    
    m_titleEdit->setText(event.title);
    m_descriptionEdit->setPlainText(event.description);
    m_contactNameEdit->setText(event.contactName);
    m_contactPhoneEdit->setText(event.contactPhone);
    m_contactEmailEdit->setText(event.contactEmail);
    m_bowlerCountSpinner->setValue(event.bowlerCount);
    m_additionalDetailsEdit->setPlainText(event.additionalDetails);
}

void CalendarDialog::deleteEvent(int eventId)
{
    CalendarEventData event = m_dbManager->getCalendarEventById(eventId);
    if (event.id == 0) {
        QMessageBox::warning(this, "Event Not Found", "The selected event could not be found.");
        return;
    }
    
    int result = QMessageBox::question(this, "Confirm Deletion",
                                      QString("Are you sure you want to delete the booking '%1'?\n\n"
                                             "Date: %2\n"
                                             "Time: %3 - %4\n"
                                             "Lane: %5")
                                      .arg(event.title)
                                      .arg(event.date.toString("MMMM d, yyyy"))
                                      .arg(event.startTime.toString("h:mm AP"))
                                      .arg(event.endTime.toString("h:mm AP"))
                                      .arg(event.laneId),
                                      QMessageBox::Yes | QMessageBox::No);
    
    if (result == QMessageBox::Yes) {
        if (m_dbManager->deleteCalendarEvent(eventId)) {
            // Emit event bus notification
            QJsonObject eventData;
            eventData["id"] = eventId;
            eventData["date"] = event.date.toString(Qt::ISODate);
            eventData["lane_id"] = event.laneId;
            eventData["title"] = event.title;
            
            m_eventBus->publish("calendar", "booking_cancelled", eventData);
            
            QMessageBox::information(this, "Booking Deleted", 
                                   "The booking has been deleted successfully.");
            
            clearEventDetails();
            refreshCalendarViews();
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete the booking. Please try again.");
        }
    }
}

void CalendarDialog::showEventDetails(const CalendarEventData &event)
{
    m_selectedEvent = event;
    m_hasSelectedEvent = true;
    m_editEventBtn->setEnabled(true);
    m_deleteEventBtn->setEnabled(true);
    
    // Clear existing details
    while (QLayoutItem* item = m_detailsContentLayout->takeAt(0)) {
        if (QWidget* widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
    
    // Event title
    QLabel *titleLabel = new QLabel(event.title);
    titleLabel->setStyleSheet("QLabel { color: white; font-size: 16px; font-weight: bold; margin-bottom: 10px; }");
    titleLabel->setWordWrap(true);
    m_detailsContentLayout->addWidget(titleLabel);
    
    // Basic info grid
    QGridLayout *infoGrid = new QGridLayout;
    infoGrid->setSpacing(8);
    
    int row = 0;
    
    // Date and Time
    QLabel *dateTimeLabel = new QLabel("Date & Time:");
    dateTimeLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
    infoGrid->addWidget(dateTimeLabel, row, 0);
    
    QString dateTimeText = QString("%1\n%2 - %3")
                          .arg(event.date.toString("dddd, MMMM d, yyyy"))
                          .arg(event.startTime.toString("h:mm AP"))
                          .arg(event.endTime.toString("h:mm AP"));
    QLabel *dateTimeValue = new QLabel(dateTimeText);
    dateTimeValue->setStyleSheet("QLabel { color: #ccc; }");
    dateTimeValue->setWordWrap(true);
    infoGrid->addWidget(dateTimeValue, row, 1);
    row++;
    
    // Lane
    QLabel *laneLabel = new QLabel("Lane:");
    laneLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
    infoGrid->addWidget(laneLabel, row, 0);
    
    QLabel *laneValue = new QLabel(QString("Lane %1").arg(event.laneId));
    laneValue->setStyleSheet("QLabel { color: #ccc; }");
    infoGrid->addWidget(laneValue, row, 1);
    row++;
    
    // Event Type
    QLabel *typeLabel = new QLabel("Type:");
    typeLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
    infoGrid->addWidget(typeLabel, row, 0);
    
    QLabel *typeValue = new QLabel(event.eventType);
    typeValue->setStyleSheet("QLabel { color: #ccc; }");
    infoGrid->addWidget(typeValue, row, 1);
    row++;
    
    // Bowlers
    QLabel *bowlersLabel = new QLabel("Bowlers:");
    bowlersLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
    infoGrid->addWidget(bowlersLabel, row, 0);
    
    QLabel *bowlersValue = new QLabel(QString::number(event.bowlerCount));
    bowlersValue->setStyleSheet("QLabel { color: #ccc; }");
    infoGrid->addWidget(bowlersValue, row, 1);
    row++;
    
    QWidget *infoWidget = new QWidget;
    infoWidget->setLayout(infoGrid);
    m_detailsContentLayout->addWidget(infoWidget);
    
    // Contact Information
    QLabel *contactHeader = new QLabel("Contact Information");
    contactHeader->setStyleSheet("QLabel { color: white; font-weight: bold; font-size: 14px; margin-top: 15px; margin-bottom: 10px; }");
    m_detailsContentLayout->addWidget(contactHeader);
    
    QGridLayout *contactGrid = new QGridLayout;
    contactGrid->setSpacing(8);
    
    row = 0;
    
    // Contact Name
    QLabel *nameLabel = new QLabel("Name:");
    nameLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
    contactGrid->addWidget(nameLabel, row, 0);
    
    QLabel *nameValue = new QLabel(event.contactName);
    nameValue->setStyleSheet("QLabel { color: #ccc; }");
    nameValue->setWordWrap(true);
    contactGrid->addWidget(nameValue, row, 1);
    row++;
    
    // Contact Phone
    QLabel *phoneLabel = new QLabel("Phone:");
    phoneLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
    contactGrid->addWidget(phoneLabel, row, 0);
    
    QLabel *phoneValue = new QLabel(event.contactPhone);
    phoneValue->setStyleSheet("QLabel { color: #ccc; }");
    contactGrid->addWidget(phoneValue, row, 1);
    row++;
    
    // Contact Email
    if (!event.contactEmail.isEmpty()) {
        QLabel *emailLabel = new QLabel("Email:");
        emailLabel->setStyleSheet("QLabel { color: white; font-weight: bold; }");
        contactGrid->addWidget(emailLabel, row, 0);
        
        QLabel *emailValue = new QLabel(event.contactEmail);
        emailValue->setStyleSheet("QLabel { color: #ccc; }");
        emailValue->setWordWrap(true);
        contactGrid->addWidget(emailValue, row, 1);
        row++;
    }
    
    QWidget *contactWidget = new QWidget;
    contactWidget->setLayout(contactGrid);
    m_detailsContentLayout->addWidget(contactWidget);
    
    // Description
    if (!event.description.isEmpty()) {
        QLabel *descHeader = new QLabel("Description");
        descHeader->setStyleSheet("QLabel { color: white; font-weight: bold; font-size: 14px; margin-top: 15px; margin-bottom: 10px; }");
        m_detailsContentLayout->addWidget(descHeader);
        
        QFrame *descFrame = new QFrame;
        descFrame->setStyleSheet("QFrame { background-color: #333; border: 1px solid #555; padding: 10px; border-radius: 5px; }");
        QVBoxLayout *descLayout = new QVBoxLayout(descFrame);
        
        QLabel *descValue = new QLabel(event.description);
        descValue->setStyleSheet("QLabel { color: #ccc; }");
        descValue->setWordWrap(true);
        descLayout->addWidget(descValue);
        
        m_detailsContentLayout->addWidget(descFrame);
    }
    
    // Additional Details
    if (!event.additionalDetails.isEmpty()) {
        QLabel *detailsHeader = new QLabel("Additional Details");
        detailsHeader->setStyleSheet("QLabel { color: white; font-weight: bold; font-size: 14px; margin-top: 15px; margin-bottom: 10px; }");
        m_detailsContentLayout->addWidget(detailsHeader);
        
        QFrame *detailsFrame = new QFrame;
        detailsFrame->setStyleSheet("QFrame { background-color: #333; border: 1px solid #555; padding: 10px; border-radius: 5px; }");
        QVBoxLayout *detailsLayout = new QVBoxLayout(detailsFrame);
        
        QLabel *detailsValue = new QLabel(event.additionalDetails);
        detailsValue->setStyleSheet("QLabel { color: #ccc; }");
        detailsValue->setWordWrap(true);
        detailsLayout->addWidget(detailsValue);
        
        m_detailsContentLayout->addWidget(detailsFrame);
    }
    
    m_detailsContentLayout->addStretch();
}

void CalendarDialog::showLeagueScheduleDialog()
{
    LeagueScheduleDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        // Refresh the calendar views to show new league events
        refreshCalendarViews();
    }
}

void CalendarDialog::addLeagueSchedule(const QString &leagueName, const QDate &startDate, 
                                      const QTime &startTime, int duration, int frequency,
                                      int weeks, const QVector<int> &lanes, 
                                      const QString &contact, const QString &phone, const QString &email)
{
    // Implementation for adding league schedules
    // This would use the DatabaseManager::addLeagueSchedule method
    
    QVector<int> createdEvents = m_dbManager->addLeagueSchedule(
        0, // league ID - would come from league management
        leagueName,
        startDate,
        startTime,
        duration,
        frequency,
        weeks,
        lanes,
        contact,
        phone,
        email,
        QString("League schedule for %1").arg(leagueName)
    );
    
    if (!createdEvents.isEmpty()) {
        QMessageBox::information(this, "League Schedule Created", 
                               QString("Successfully created %1 league events for '%2'")
                               .arg(createdEvents.size()).arg(leagueName));
        
        // Emit event bus notification
        QJsonObject eventData;
        eventData["league_name"] = leagueName;
        eventData["start_date"] = startDate.toString(Qt::ISODate);
        eventData["weeks"] = weeks;
        eventData["events_created"] = createdEvents.size();
        
        m_eventBus->publish("calendar", "league_schedule_added", eventData);
        
        refreshCalendarViews();
    } else {
        QMessageBox::warning(this, "League Schedule Error", 
                           "Failed to create league schedule. Please check for conflicts and try again.");
    }

}
