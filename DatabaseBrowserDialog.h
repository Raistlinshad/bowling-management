// DatabaseBrowserDialog.h
#ifndef DATABASEBROWSERDIALOG_H
#define DATABASEBROWSERDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QTreeWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QClipboard>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QDesktopServices>
#include <QUrl>
#include "DatabaseManager.h"

class DatabaseBrowserDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DatabaseBrowserDialog(QWidget *parent = nullptr);

private slots:
    void onTableSelected();
    void onRefreshClicked();
    void onExecuteQueryClicked();
    void onCopyPathClicked();
    void onOpenLocationClicked();

private:
    void setupUI();
    void loadDatabaseInfo();
    void loadTables();
    void loadTableData(const QString &tableName);
    void showQueryResult(const QString &query);
    QString getDatabasePath();
    
    DatabaseManager *m_dbManager;
    
    QVBoxLayout *m_mainLayout;
    QSplitter *m_splitter;
    
    QWidget *m_leftPanel;
    QVBoxLayout *m_leftLayout;
    QLabel *m_dbPathLabel;
    QLabel *m_dbSizeLabel;
    QPushButton *m_copyPathBtn;
    QPushButton *m_openLocationBtn;
    QTreeWidget *m_tablesTree;
    QPushButton *m_refreshBtn;
    
    QWidget *m_rightPanel;
    QVBoxLayout *m_rightLayout;
    QTextEdit *m_queryEdit;
    QPushButton *m_executeBtn;
    QTableWidget *m_dataTable;
    QLabel *m_recordCountLabel;
    
    QPushButton *m_closeBtn;
    QString m_currentTable;
};

#endif // DATABASEBROWSERDIALOG_H