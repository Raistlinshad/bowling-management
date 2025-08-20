// LeagueScheduleDialog.h
#ifndef LEAGUESCHEDULEDIALOG_H
#define LEAGUESCHEDULEDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDateEdit>
#include <QTimeEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QGroupBox>
#include <QScrollArea>
#include <QMessageBox>
#include <QDate>
#include <QTime>
#include <QVector>
#include "DatabaseManager.h"

class LeagueScheduleDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LeagueScheduleDialog(QWidget *parent = nullptr);

private slots:
    void onCreateScheduleClicked();
    void onCancelClicked();
    void onFrequencyChanged();

private:
    void setupUI();
    void setupBasicInfo();
    void setupScheduleInfo();
    void setupLaneSelection();
    void setupButtons();
    void updateFrequencyInfo();
    bool validateForm();
    void createSchedule();
    
    DatabaseManager *m_dbManager;
    
    // Main layout
    QVBoxLayout *m_mainLayout;
    QScrollArea *m_scrollArea;
    QWidget *m_scrollContent;
    QVBoxLayout *m_contentLayout;
    
    // Basic info section
    QGroupBox *m_basicInfoGroup;
    QGridLayout *m_basicInfoLayout;
    QLineEdit *m_leagueNameEdit;
    QLineEdit *m_contactNameEdit;
    QLineEdit *m_contactPhoneEdit;
    QLineEdit *m_contactEmailEdit;
    
    // Schedule info section
    QGroupBox *m_scheduleInfoGroup;
    QGridLayout *m_scheduleInfoLayout;
    QDateEdit *m_startDateEdit;
    QTimeEdit *m_startTimeEdit;
    QSpinBox *m_durationSpinner;
    QComboBox *m_frequencyCombo;
    QSpinBox *m_weeksSpinner;
    QLabel *m_frequencyInfoLabel;
    
    // Lane selection section
    QGroupBox *m_laneSelectionGroup;
    QVBoxLayout *m_laneSelectionLayout;
    QVector<QCheckBox*> m_laneCheckboxes;
    QPushButton *m_selectAllLanesBtn;
    QPushButton *m_clearAllLanesBtn;
    
    // Buttons
    QHBoxLayout *m_buttonLayout;
    QPushButton *m_createBtn;
    QPushButton *m_cancelBtn;
    
    int m_totalLanes;
};

#endif // LEAGUESCHEDULEDIALOG_H