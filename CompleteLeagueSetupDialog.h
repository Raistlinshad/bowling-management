// CompleteLeagueSetupDialog.h
#ifndef COMPLETELEAGUESETUPDIALOG_H
#define COMPLETELEAGUESETUPDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QTabWidget>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QDateEdit>
#include <QTimeEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QPushButton>
#include <QListWidget>
#include <QTableWidget>
#include <QScrollArea>
#include <QTextEdit>
#include "LeagueManager.h"

class CompleteLeagueSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CompleteLeagueSetupDialog(QWidget *parent = nullptr);
    
    LeagueConfig getLeagueConfiguration() const;
    bool validateConfiguration() const;

private slots:
    void onCreateLeagueClicked();
    void onCancelClicked();
    void onTabChanged(int index);
    void onAvgCalculationChanged();
    void onHdcpCalculationChanged();
    void onAbsentHandlingChanged();
    void onPreBowlToggled(bool enabled);
    void onDivisionCountChanged(int count);
    void onPlayoffTypeChanged();
    void onPointSystemChanged();
    void onAddTeamClicked();
    void onRemoveTeamClicked();
    void onAssignDivisionsClicked();

private:
    void setupUI();
    void setupBasicInfoTab();
    void setupScheduleTab();
    void setupAverageCalculationTab();
    void setupHandicapCalculationTab();
    void setupAbsentHandlingTab();
    void setupPreBowlTab();
    void setupDivisionsTab();
    void setupPlayoffsTab();
    void setupPointSystemTab();
    void setupTeamsTab();
    void setupSummaryTab();
    
    void updateSummary();
    void populateTeamsList();
    void assignTeamsToDivisions();
    
    // UI Components
    QVBoxLayout *m_mainLayout;
    QTabWidget *m_tabWidget;
    QPushButton *m_createBtn;
    QPushButton *m_cancelBtn;
    
    // Basic Info Tab
    QWidget *m_basicInfoTab;
    QLineEdit *m_leagueNameEdit;
    QLineEdit *m_contactNameEdit;
    QLineEdit *m_contactPhoneEdit;
    QLineEdit *m_contactEmailEdit;
    QTextEdit *m_leagueDescriptionEdit;
    
    // Schedule Tab
    QWidget *m_scheduleTab;
    QDateEdit *m_startDateEdit;
    QTimeEdit *m_startTimeEdit;
    QSpinBox *m_durationSpinner;
    QSpinBox *m_weeksSpinner;
    QComboBox *m_frequencyCombo;
    QListWidget *m_lanesList;
    
    // Average Calculation Tab
    QWidget *m_avgCalcTab;
    QButtonGroup *m_avgCalcGroup;
    QRadioButton *m_avgPerGameRadio;
    QRadioButton *m_avgPerBallRadio;
    QRadioButton *m_avgPeriodicRadio;
    QSpinBox *m_avgUpdateIntervalSpinner;
    QSpinBox *m_avgDelayGamesSpinner;
    
    // Handicap Calculation Tab
    QWidget *m_hdcpCalcTab;
    QButtonGroup *m_hdcpCalcGroup;
    QRadioButton *m_hdcpPercentageRadio;
    QRadioButton *m_hdcpStraightRadio;
    QRadioButton *m_hdcpDeductionRadio;
    QSpinBox *m_hdcpHighValueSpinner;
    QDoubleSpinBox *m_hdcpPercentageSpinner;
    QSpinBox *m_hdcpDeductionSpinner;
    QSpinBox *m_hdcpDelayGamesSpinner;
    
    // Absent Handling Tab
    QWidget *m_absentTab;
    QButtonGroup *m_absentGroup;
    QRadioButton *m_absentPercentageRadio;
    QRadioButton *m_absentFixedRadio;
    QRadioButton *m_absentAverageRadio;
    QDoubleSpinBox *m_absentPercentageSpinner;
    QSpinBox *m_absentFixedSpinner;
    
    // Pre-Bowl Tab
    QWidget *m_preBowlTab;
    QCheckBox *m_preBowlEnabledCheck;
    QCheckBox *m_preBowlCarryOverCheck;
    QCheckBox *m_preBowlRandomUseCheck;
    QButtonGroup *m_preBowlUseGroup;
    QRadioButton *m_preBowlByGameRadio;
    QRadioButton *m_preBowlBySetRadio;
    QSpinBox *m_preBowlMaxUsesSpinner;
    
    // Divisions Tab
    QWidget *m_divisionsTab;
    QSpinBox *m_divisionCountSpinner;
    QCheckBox *m_reorderMidSeasonCheck;
    QButtonGroup *m_divisionOrderGroup;
    QRadioButton *m_manualOrderRadio;
    QRadioButton *m_systemOrderRadio;
    QTableWidget *m_divisionAssignmentTable;
    QPushButton *m_assignDivisionsBtn;
    
    // Playoffs Tab
    QWidget *m_playoffsTab;
    QButtonGroup *m_playoffTypeGroup;
    QRadioButton *m_noPlayoffsRadio;
    QRadioButton *m_roundRobinRadio;
    QRadioButton *m_roundRobinPlacementRadio;
    QRadioButton *m_placementOnlyRadio;
    QRadioButton *m_roundRobinPlusPlacementRadio;
    QCheckBox *m_divisionOnlyPlayoffsCheck;
    QTableWidget *m_placementPairsTable;
    
    // Point System Tab
    QWidget *m_pointSystemTab;
    QButtonGroup *m_pointTypeGroup;
    QRadioButton *m_winLossTieRadio;
    QRadioButton *m_teamVsTeamRadio;
    QRadioButton *m_customPointsRadio;
    
    // Win/Loss/Tie settings
    QGroupBox *m_winLossGroup;
    QSpinBox *m_winPointsSpinner;
    QSpinBox *m_lossPointsSpinner;
    QSpinBox *m_tiePointsSpinner;
    QCheckBox *m_headsUpEnabledCheck;
    QCheckBox *m_headsUpHandicapCheck;
    
    // Team vs Team settings
    QGroupBox *m_teamVsTeamGroup;
    QCheckBox *m_stackedTiesCheck;
    
    // Custom settings
    QGroupBox *m_customPointsGroup;
    QCheckBox *m_trackHeadsUpCheck;
    QCheckBox *m_trackTeamVsCheck;
    QCheckBox *m_trackDivisionVsCheck;
    QCheckBox *m_trackLeagueVsCheck;
    QCheckBox *m_trackScratchCheck;
    QCheckBox *m_trackHandicapCheck;
    
    // Teams Tab
    QWidget *m_teamsTab;
    QListWidget *m_availableBowlersList;
    QListWidget *m_teamsList;
    QLineEdit *m_teamNameEdit;
    QPushButton *m_addTeamBtn;
    QPushButton *m_removeTeamBtn;
    QTableWidget *m_teamMembersTable;
    
    // Summary Tab
    QWidget *m_summaryTab;
    QTextEdit *m_summaryText;
    
    // Data
    QVector<TeamData> m_teams;
    QVector<BowlerInfo> m_availableBowlers;
    int m_totalLanes;
};
