#ifndef BOWLERMANAGEMENTDIALOG_H
#define BOWLERMANAGEMENTDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QTreeWidget>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QFrame>
#include <QGroupBox>
#include <QComboBox>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include <QCheckBox>
#include <QSpinBox>
#include <QAbstractItemView>
#include "NewBowlerDialog.h"
#include "DatabaseManager.h"

class MainWindow;

class BowlerManagementDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BowlerManagementDialog(MainWindow *mainWindow, QWidget *parent = nullptr);
    
    // Static method for selecting bowlers (used by team management)
    static QVector<BowlerInfo> selectBowlersDialog(const QVector<BowlerInfo> &excludeBowlers, QWidget *parent = nullptr);

private slots:
    void onBowlerSelectionChanged();
    void onAddBowlerClicked();
    void onEditBowlerClicked();
    void onDeleteBowlerClicked();
    void onBowlerInfoClicked();
    void onSearchTextChanged(const QString &text);
    void onFilterChanged();
    void onCloseClicked();
    void onRefreshClicked();

private:
    void setupUI();
    void setupLeftPane();
    void setupRightPane();
    void updateBowlersList();
    void updateBowlerDetails();
    void showAddBowlerDialog();
    void showEditBowlerDialog();
    BowlerInfo getSelectedBowler();
    void loadBowlersFromDatabase();
    
    MainWindow *m_mainWindow;
    QHBoxLayout *m_mainLayout;
    QSplitter *m_splitter;
    
    // Left pane - Bowlers list and filters
    QFrame *m_leftFrame;
    QVBoxLayout *m_leftLayout;
    QLineEdit *m_searchEdit;
    QComboBox *m_sexFilterCombo;
    QComboBox *m_ageFilterCombo;
    QSpinBox *m_minAverageSpinner;
    QSpinBox *m_maxAverageSpinner;
    QTreeWidget *m_bowlersTree;
    QPushButton *m_refreshBtn;
    
    // Right pane - Actions and details
    QFrame *m_rightFrame;
    QVBoxLayout *m_rightLayout;
    QGroupBox *m_actionsGroup;
    QGroupBox *m_detailsGroup;
    QTextEdit *m_detailsText;
    
    // Action buttons
    QPushButton *m_addBowlerBtn;
    QPushButton *m_editBowlerBtn;
    QPushButton *m_deleteBowlerBtn;
    QPushButton *m_bowlerInfoBtn;
    QPushButton *m_closeBtn;
    
    // Data
    QVector<BowlerInfo> m_bowlers;
    QMap<int, int> m_bowlerIdMap; // Maps tree row to bowler ID
};

// Dialog for selecting multiple bowlers (used by team management)
class BowlerSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BowlerSelectionDialog(const QVector<BowlerInfo> &availableBowlers, 
                                  const QVector<BowlerInfo> &excludeBowlers = QVector<BowlerInfo>(),
                                  QWidget *parent = nullptr);
    
    QVector<BowlerInfo> getSelectedBowlers() const;

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
    
    QVector<BowlerInfo> m_allBowlers;
    QVector<BowlerInfo> m_excludeBowlers;
    QVBoxLayout *m_mainLayout;
    QHBoxLayout *m_searchLayout;
    QHBoxLayout *m_buttonLayout;
    
    QLineEdit *m_searchEdit;
    QTreeWidget *m_availableBowlersTree;
    QLabel *m_selectedCountLabel;
    QPushButton *m_selectAllBtn;
    QPushButton *m_deselectAllBtn;
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
    
    QMap<int, int> m_bowlerIdMap; // Maps tree row to bowler ID
};

#endif // BOWLERMANAGEMENTDIALOG_H