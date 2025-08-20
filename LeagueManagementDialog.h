#ifndef LEAGUEMANAGEMENTDIALOG_H
#define LEAGUEMANAGEMENTDIALOG_H

#include "DatabaseManager.h"
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

class MainWindow; 

// Extended league information that's not in the DatabaseManager's LeagueData
struct LeagueExtendedInfo {
    QString startDate;
    QString endDate;
    int numberOfWeeks;
    QString status;
    QString createdAt;
};

class LeagueManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LeagueManagementDialog(MainWindow *mainWindow, QWidget *parent = nullptr);

private slots:
    void onLeagueSelectionChanged();
    void onAddLeagueClicked();
    void onEditLeagueClicked();
    void onDeleteLeagueClicked();
    void onViewStandingsClicked();
    void onScheduleClicked();
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
    void showAddLeagueDialog();
    void showEditLeagueDialog();
    LeagueData getSelectedLeague();
    
    MainWindow *m_mainWindow;
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
    QTreeWidget *m_teamsTree;
    
    // Right pane - Actions and details
    QFrame *m_rightFrame;
    QVBoxLayout *m_rightLayout;
    QGroupBox *m_actionsGroup;
    QGroupBox *m_detailsGroup;
    QLabel *m_detailsLabel;
    
    // Action buttons
    QPushButton *m_addLeagueBtn;
    QPushButton *m_editLeagueBtn;
    QPushButton *m_deleteLeagueBtn;
    QPushButton *m_viewStandingsBtn;
    QPushButton *m_scheduleBtn;
    QPushButton *m_closeBtn;
    
    // Sample data
    QVector<LeagueData> m_leagues;
    QMap<int, LeagueExtendedInfo> m_leagueExtendedInfo; // Additional info keyed by league ID
};

class AddLeagueDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddLeagueDialog(QWidget *parent = nullptr);
    explicit AddLeagueDialog(const LeagueData &league, QWidget *parent = nullptr);
    
    LeagueData getLeagueData() const;

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUI();
    void populateFields(const LeagueData &league);
    bool validateInput();
    
    bool m_editMode;
    QVBoxLayout *m_mainLayout;
    QGridLayout *m_formLayout;
    QFrame *m_formFrame;
    
    QLineEdit *m_nameEdit;
    QDateEdit *m_startDateEdit;
    QDateEdit *m_endDateEdit;
    QSpinBox *m_weeksSpinBox;
    QComboBox *m_statusCombo;
    
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
};

#endif // LEAGUEMANAGEMENTDIALOG_H