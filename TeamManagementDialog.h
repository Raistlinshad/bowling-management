#ifndef TEAMMANAGEMENTDIALOG_H
#define TEAMMANAGEMENTDIALOG_H

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
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include <QAbstractItemView>
#include "DatabaseManager.h"

class MainWindow; 

class TeamManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TeamManagementDialog(MainWindow *mainWindow, QWidget *parent = nullptr);

private slots:
    void onTeamSelectionChanged();
    void onAddTeamClicked();
    void onEditTeamClicked();
    void onDeleteTeamClicked();
    void onAddBowlersClicked();
    void onRemoveBowlersClicked();
    void onTeamInfoClicked();
    void onSearchTextChanged(const QString &text);
    void onCloseClicked();

private:
    void setupUI();
    void setupLeftPane();
    void setupMiddlePane();
    void setupRightPane();
    void updateTeamsList();
    void updateTeamBowlers();
    void updateTeamDetails();
    void showAddTeamDialog();
    void showEditTeamDialog();
    void showAddBowlersDialog();
    TeamData getSelectedTeam();
    
    MainWindow *m_mainWindow;
    QHBoxLayout *m_mainLayout;
    QSplitter *m_splitter;
    
    // Left pane - Teams list
    QFrame *m_leftFrame;
    QVBoxLayout *m_leftLayout;
    QLineEdit *m_teamSearchEdit;
    QListWidget *m_teamsList;
    QMap<int, int> m_teamIdMap; // Maps list index to team ID
    
    // Middle pane - Team bowlers
    QFrame *m_middleFrame;
    QVBoxLayout *m_middleLayout;
    QTreeWidget *m_bowlersTree;
    
    // Right pane - Actions and details
    QFrame *m_rightFrame;
    QVBoxLayout *m_rightLayout;
    QGroupBox *m_actionsGroup;
    QGroupBox *m_detailsGroup;
    QLabel *m_detailsLabel;
    
    // Action buttons
    QPushButton *m_addTeamBtn;
    QPushButton *m_editTeamBtn;
    QPushButton *m_deleteTeamBtn;
    QPushButton *m_addBowlersBtn;
    QPushButton *m_removeBowlersBtn;
    QPushButton *m_teamInfoBtn;
    QPushButton *m_closeBtn;
    
    // Sample data
    QVector<TeamData> m_teams;
    QVector<BowlerData> m_sampleBowlers;
};

class AddTeamDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddTeamDialog(QWidget *parent = nullptr);
    explicit AddTeamDialog(const TeamData &team, QWidget *parent = nullptr);
    
    QString getTeamName() const;

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUI();
    bool validateInput();
    
    bool m_editMode;
    QVBoxLayout *m_mainLayout;
    QLineEdit *m_teamNameEdit;
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
};

class AddBowlersToTeamDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddBowlersToTeamDialog(int teamId, const QVector<BowlerData> &availableBowlers, QWidget *parent = nullptr);
    
    QVector<int> getSelectedBowlerIds() const;

private slots:
    void onSearchTextChanged(const QString &text);
    void onSelectAllClicked();
    void onDeselectAllClicked();
    void onSaveClicked();
    void onCancelClicked();
    void updateSelectedCount();

private:
    void setupUI();
    void updateAvailableBowlers();
    
    int m_teamId;
    QVector<BowlerData> m_bowlers;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_searchLayout;
    QHBoxLayout *m_buttonLayout;
    
    QLineEdit *m_searchEdit;
    QListWidget *m_availableBowlersList;
    QLabel *m_selectedCountLabel;
    QPushButton *m_selectAllBtn;
    QPushButton *m_deselectAllBtn;
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
    
    QMap<int, int> m_bowlerIdMap; // Maps list index to bowler ID
};

#endif // TEAMMANAGEMENTDIALOG_H