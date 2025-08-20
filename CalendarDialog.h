#ifndef CALENDARDIALOG_H
#define CALENDARDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QCalendarWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QFrame>
#include <QComboBox>
#include <QSpinBox>
#include <QTimeEdit>
#include <QDateEdit>
#include <QScrollArea>
#include <QTreeWidget>
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include <QDate>
#include <QTime>
#include <QDateTime>
#include <QTimer>
#include <QListWidget>
#include <QGroupBox>
#include <QCheckBox>
#include <QSettings>
#include "DatabaseManager.h"
#include "EventBus.h"
#include "LeagueScheduleDialog.h"

class MainWindow;

class CalendarDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalendarDialog(MainWindow *mainWindow, QWidget *parent = nullptr);

private slots:
    void onDateSelected();
    void onNewBookingClicked();
    void onSaveBookingClicked();
    void onCloseClicked();
    void onPrevDay();
    void onNextDay();
    void onPrevMonth();
    void onNextMonth();
    void onEventClicked(QListWidgetItem *item);
    void onEditEventClicked();
    void onDeleteEventClicked();
    void onCheckAvailabilityClicked();
    void onLaneFilterChanged();
    void updateDailySchedule();
    void updateMonthCalendar();
    void refreshCalendarViews();

private:
    void setupUI();
    void setupLeftPane();      // Month calendar + navigation
    void setupCenterPane();    // Daily schedule view
    void setupRightPane();     // Event details + booking form
    void setupBookingForm();
    void setupEventDetails();
    void clearBookingForm();
    void clearEventDetails();
    bool validateBookingForm();
    void saveEvent();
    void editEvent(int eventId);
    void deleteEvent(int eventId);
    void showEventDetails(const CalendarEventData &event);
    void populateTimeSlots();
    void createTimeSlot(const QTime &time);
    void addEventToTimeSlot(const QTime &time, const CalendarEventData &event);
    QStringList getEventTypesForDate(const QDate &date);
    void applyLaneFilter();
    void loadCalendarSettings();
    void saveCalendarSettings();
    
    // Event conflict checking
    bool checkEventConflicts(const QDate &date, const QTime &startTime, 
                           const QTime &endTime, int laneId, int excludeEventId = -1);
    QVector<CalendarEventData> getConflictingEvents(const QDate &date, const QTime &startTime,
                                                   const QTime &endTime, int laneId, int excludeEventId = -1);
    
    // League schedule support
    void showLeagueScheduleDialog();
    void addLeagueSchedule(const QString &leagueName, const QDate &startDate, 
                          const QTime &startTime, int duration, int frequency,
                          int weeks, const QVector<int> &lanes, 
                          const QString &contact, const QString &phone, const QString &email);
    
    MainWindow *m_mainWindow;
    DatabaseManager *m_dbManager;
    EventBus *m_eventBus;
    
    // Main layout
    QHBoxLayout *m_mainLayout;
    QSplitter *m_mainSplitter;
    
    // Left pane - Month calendar
    QFrame *m_leftFrame;
    QVBoxLayout *m_leftLayout;
    QLabel *m_monthLabel;
    QPushButton *m_prevMonthBtn;
    QPushButton *m_nextMonthBtn;
    QCalendarWidget *m_calendar;
    QPushButton *m_newBookingBtn;
    QPushButton *m_leagueScheduleBtn;
    
    // Center pane - Daily schedule
    QFrame *m_centerFrame;
    QVBoxLayout *m_centerLayout;
    QLabel *m_dayLabel;
    QPushButton *m_prevDayBtn;
    QPushButton *m_nextDayBtn;
    QFrame *m_laneFilterFrame;
    QLabel *m_laneFilterLabel;
    QComboBox *m_laneFilterCombo;
    QScrollArea *m_scheduleScrollArea;
    QWidget *m_scheduleContent;
    QVBoxLayout *m_scheduleLayout;
    
    // Right pane - Details and booking
    QFrame *m_rightFrame;
    QVBoxLayout *m_rightLayout;
    QSplitter *m_rightSplitter;
    
    // Event details section
    QFrame *m_detailsFrame;
    QVBoxLayout *m_detailsLayout;
    QScrollArea *m_detailsScrollArea;
    QWidget *m_detailsContent;
    QVBoxLayout *m_detailsContentLayout;
    QPushButton *m_editEventBtn;
    QPushButton *m_deleteEventBtn;
    
    // Booking form section
    QFrame *m_bookingFrame;
    QVBoxLayout *m_bookingLayout;
    QScrollArea *m_bookingScrollArea;
    QWidget *m_bookingContent;
    QGridLayout *m_bookingFormLayout;
    
    // Booking form controls
    QDateEdit *m_dateEdit;
    QTimeEdit *m_startTimeEdit;
    QSpinBox *m_durationSpinner;
    QComboBox *m_laneCombo;
    QComboBox *m_eventTypeCombo;
    QLineEdit *m_titleEdit;
    QTextEdit *m_descriptionEdit;
    QLineEdit *m_contactNameEdit;
    QLineEdit *m_contactPhoneEdit;
    QLineEdit *m_contactEmailEdit;
    QSpinBox *m_bowlerCountSpinner;
    QTextEdit *m_additionalDetailsEdit;
    QPushButton *m_saveBookingBtn;
    QPushButton *m_clearFormBtn;
    QPushButton *m_checkAvailabilityBtn;
    QPushButton *m_closeBtn;
    
    // Current state
    QDate m_selectedDate;
    int m_totalLanes;
    int m_startHour;    // Start of schedule (e.g., 8 AM)
    int m_endHour;      // End of schedule (e.g., 2 AM next day)
    int m_filteredLane; // 0 = show all lanes, >0 = show specific lane
    int m_editingEventId; // ID of event being edited, -1 if adding new
    
    // Time slot widgets for daily view
    QMap<QTime, QFrame*> m_timeSlots;
    QMap<int, QListWidget*> m_eventLists; // Lane ID -> event list widget
    
    // Current selected event
    CalendarEventData m_selectedEvent;
    bool m_hasSelectedEvent;
};

#endif // CALENDARDIALOG_H