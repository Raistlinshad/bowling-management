// Enhanced LeagueManagementDialog.h
#ifndef LEAGUEMANAGEMENTDIALOG_H
#define LEAGUEMANAGEMENTDIALOG_H

#include "DatabaseManager.h"
#include "LeagueManager.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QFrame>
#include <QGroupBox>
#include <QComboBox>
#include <QTreeWidget>
#include <QHeaderView>
#include <QDateEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include <QMap>
#include <QTabWidget>
#include <QRadioButton> 

class MainWindow; 

// Extended league information that includes all the advanced features
struct EnhancedLeagueInfo {
    LeagueData basicInfo;
    LeagueConfig advancedConfig;
    QString startDate;
    QString endDate;
    int numberOfWeeks;
    QString status;
    QString createdAt;
    int totalTeams;
    int activeBowlers;
    double averageScore;
    int completedWeeks;
};

class LeagueManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LeagueManagementDialog(MainWindow *mainWindow, QWidget *parent = nullptr);

private slots:
    void onLeagueSelectionChanged();
    void onCreateLeagueClicked();
    void onEditLeagueClicked();
    void onDeleteLeagueClicked();
    void onViewStandingsClicked();
    void onScheduleClicked();
    void onManageTeamsClicked();
    void onManageBowlersClicked();
    void onViewStatisticsClicked();
    void onPreBowlManagementClicked();
    void onDivisionManagementClicked();
    void onPlayoffSetupClicked();
    void onSearchTextChanged(const QString &text);
    void onCloseClicked();

private:
    void setupUI();
    void setupLeftPane();
    void setupMiddlePane();
    void setupRightPane();
    void updateLeaguesList();
    void updateLeagueTeams();
    void updateLeagueDetails();
    void showCreateLeagueDialog();
    void showEditLeagueDialog();
    void showStandingsDialog();
    void showScheduleDialog();
    void showTeamManagementDialog();
    void showBowlerManagementDialog();
    void showStatisticsDialog();
    void showPreBowlDialog();
    void showDivisionDialog();
    void showPlayoffDialog();
    EnhancedLeagueInfo getSelectedLeagueInfo();
    
    MainWindow *m_mainWindow;
    LeagueManager *m_leagueManager;
    
    QHBoxLayout *m_mainLayout;
    QSplitter *m_splitter;
    
    // Left pane - Leagues list
    QFrame *m_leftFrame;
    QVBoxLayout *m_leftLayout;
    QLineEdit *m_leagueSearchEdit;
    QListWidget *m_leaguesList;
    QMap<int, int> m_leagueIdMap; // Maps list index to league ID
    
    // Middle pane - League teams/schedule
    QFrame *m_middleFrame;
    QVBoxLayout *m_middleLayout;
    QTabWidget *m_middleTabWidget;
    QTreeWidget *m_teamsTree;
    QTreeWidget *m_scheduleTree;
    QTreeWidget *m_standingsTree;
    
    // Right pane - Actions and details
    QFrame *m_rightFrame;
    QVBoxLayout *m_rightLayout;
    QGroupBox *m_actionsGroup;
    QGroupBox *m_detailsGroup;
    QTextEdit *m_detailsText;
    
    // Enhanced action buttons
    QPushButton *m_createLeagueBtn;
    QPushButton *m_editLeagueBtn;
    QPushButton *m_deleteLeagueBtn;
    QPushButton *m_viewStandingsBtn;
    QPushButton *m_scheduleBtn;
    QPushButton *m_manageTeamsBtn;
    QPushButton *m_manageBowlersBtn;
    QPushButton *m_statisticsBtn;
    QPushButton *m_preBowlBtn;
    QPushButton *m_divisionsBtn;
    QPushButton *m_playoffsBtn;
    QPushButton *m_closeBtn;
    
    // Sample enhanced data
    QVector<EnhancedLeagueInfo> m_enhancedLeagues;
};

class EnhancedLeagueSetupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EnhancedLeagueSetupDialog(QWidget *parent = nullptr);
    explicit EnhancedLeagueSetupDialog(const EnhancedLeagueInfo &league, QWidget *parent = nullptr);
    
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
    void populateFromExisting(const EnhancedLeagueInfo &league);
    
    bool m_editMode;
    int m_editingLeagueId;
    
    // UI Components (same as CompleteLeagueSetupDialog but integrated)
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
    
    // Point System Tab
    QWidget *m_pointSystemTab;
    QButtonGroup *m_pointTypeGroup;
    QRadioButton *m_winLossTieRadio;
    QRadioButton *m_teamVsTeamRadio;
    QRadioButton *m_customPointsRadio;
    QSpinBox *m_winPointsSpinner;
    QSpinBox *m_lossPointsSpinner;
    QSpinBox *m_tiePointsSpinner;
    QCheckBox *m_headsUpEnabledCheck;
    QCheckBox *m_headsUpHandicapCheck;
    
    // Teams Tab
    QWidget *m_teamsTab;
    QListWidget *m_availableBowlersList;
    QListWidget *m_teamsList;
    QLineEdit *m_teamNameEdit;
    QPushButton *m_addTeamBtn;
    QPushButton *m_removeTeamBtn;
    
    // Summary Tab
    QWidget *m_summaryTab;
    QTextEdit *m_summaryText;
    
    // Data
    QVector<TeamData> m_teams;
    int m_totalLanes;
};

#endif // LEAGUEMANAGEMENTDIALOG_H