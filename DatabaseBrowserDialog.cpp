// DatabaseBrowserDialog.cpp
#include "DatabaseBrowserDialog.h"

DatabaseBrowserDialog::DatabaseBrowserDialog(QWidget *parent)
    : QDialog(parent)
    , m_dbManager(DatabaseManager::instance())
{
    setupUI();
    setWindowTitle("Database Browser & Inspector");
    setModal(true);
    resize(1000, 600);
    
    loadDatabaseInfo();
    loadTables();
}

void DatabaseBrowserDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    
    m_splitter = new QSplitter(Qt::Horizontal);
    m_mainLayout->addWidget(m_splitter);
    
    // Left panel
    m_leftPanel = new QWidget;
    m_leftPanel->setMinimumWidth(300);
    m_leftPanel->setMaximumWidth(400);
    m_leftLayout = new QVBoxLayout(m_leftPanel);
    
    QLabel *infoHeader = new QLabel("DATABASE INFORMATION");
    infoHeader->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; margin: 10px 0; }");
    m_leftLayout->addWidget(infoHeader);
    
    m_dbPathLabel = new QLabel("Path: Loading...");
    m_dbPathLabel->setWordWrap(true);
    m_dbPathLabel->setStyleSheet("QLabel { font-size: 10px; margin-bottom: 5px; }");
    m_leftLayout->addWidget(m_dbPathLabel);
    
    m_dbSizeLabel = new QLabel("Size: Calculating...");
    m_dbSizeLabel->setStyleSheet("QLabel { font-size: 10px; margin-bottom: 10px; }");
    m_leftLayout->addWidget(m_dbSizeLabel);
    
    QHBoxLayout *pathButtonsLayout = new QHBoxLayout;
    m_copyPathBtn = new QPushButton("Copy Path");
    m_copyPathBtn->setStyleSheet("QPushButton { padding: 5px 10px; }");
    connect(m_copyPathBtn, &QPushButton::clicked, this, &DatabaseBrowserDialog::onCopyPathClicked);
    pathButtonsLayout->addWidget(m_copyPathBtn);
    
    m_openLocationBtn = new QPushButton("Open Location");
    m_openLocationBtn->setStyleSheet("QPushButton { padding: 5px 10px; }");
    connect(m_openLocationBtn, &QPushButton::clicked, this, &DatabaseBrowserDialog::onOpenLocationClicked);
    pathButtonsLayout->addWidget(m_openLocationBtn);
    
    m_leftLayout->addLayout(pathButtonsLayout);
    
    QLabel *tablesHeader = new QLabel("DATABASE TABLES");
    tablesHeader->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; margin: 20px 0 10px 0; }");
    m_leftLayout->addWidget(tablesHeader);
    
    m_tablesTree = new QTreeWidget;
    m_tablesTree->setHeaderLabels({"Table", "Records"});
    connect(m_tablesTree, &QTreeWidget::itemClicked, this, &DatabaseBrowserDialog::onTableSelected);
    m_leftLayout->addWidget(m_tablesTree);
    
    m_refreshBtn = new QPushButton("Refresh Tables");
    m_refreshBtn->setStyleSheet("QPushButton { padding: 8px; background-color: #28a745; color: white; }");
    connect(m_refreshBtn, &QPushButton::clicked, this, &DatabaseBrowserDialog::onRefreshClicked);
    m_leftLayout->addWidget(m_refreshBtn);
    
    m_splitter->addWidget(m_leftPanel);
    
    // Right panel
    m_rightPanel = new QWidget;
    m_rightLayout = new QVBoxLayout(m_rightPanel);
    
    QLabel *queryLabel = new QLabel("SQL QUERY EXECUTOR");
    queryLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; margin-bottom: 10px; }");
    m_rightLayout->addWidget(queryLabel);
    
    m_queryEdit = new QTextEdit;
    m_queryEdit->setMaximumHeight(80);
    m_queryEdit->setPlaceholderText("Enter SQL query (e.g., SELECT * FROM bowlers LIMIT 10)");
    m_rightLayout->addWidget(m_queryEdit);
    
    m_executeBtn = new QPushButton("Execute Query");
    m_executeBtn->setStyleSheet("QPushButton { padding: 8px 16px; background-color: #007bff; color: white; }");
    connect(m_executeBtn, &QPushButton::clicked, this, &DatabaseBrowserDialog::onExecuteQueryClicked);
    m_rightLayout->addWidget(m_executeBtn);
    
    QLabel *resultsHeader = new QLabel("RESULTS");
    resultsHeader->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; margin: 20px 0 10px 0; }");
    m_rightLayout->addWidget(resultsHeader);
    
    m_dataTable = new QTableWidget;
    m_dataTable->setAlternatingRowColors(true);
    m_dataTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_rightLayout->addWidget(m_dataTable);
    
    m_recordCountLabel = new QLabel("No data loaded");
    m_rightLayout->addWidget(m_recordCountLabel);
    
    m_splitter->addWidget(m_rightPanel);
    
    m_closeBtn = new QPushButton("Close");
    m_closeBtn->setStyleSheet("QPushButton { padding: 10px 20px; background-color: #6c757d; color: white; }");
    connect(m_closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    m_mainLayout->addWidget(m_closeBtn);
}

void DatabaseBrowserDialog::loadDatabaseInfo()
{
    QString dbPath = getDatabasePath();
    m_dbPathLabel->setText(QString("Path: %1").arg(dbPath));
    
    QFileInfo fileInfo(dbPath);
    if (fileInfo.exists()) {
        qint64 sizeBytes = fileInfo.size();
        QString sizeText;
        if (sizeBytes < 1024) {
            sizeText = QString("%1 bytes").arg(sizeBytes);
        } else if (sizeBytes < 1024 * 1024) {
            sizeText = QString("%1 KB").arg(sizeBytes / 1024.0, 0, 'f', 1);
        } else {
            sizeText = QString("%1 MB").arg(sizeBytes / (1024.0 * 1024.0), 0, 'f', 1);
        }
        m_dbSizeLabel->setText(QString("Size: %1").arg(sizeText));
    } else {
        m_dbSizeLabel->setText("Size: File not found!");
        m_dbSizeLabel->setStyleSheet("QLabel { color: red; font-weight: bold; }");
    }
}

void DatabaseBrowserDialog::loadTables()
{
    m_tablesTree->clear();
    
    QSqlQuery query("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name");
    
    if (!query.exec()) {
        QMessageBox::critical(this, "Database Error", 
                            QString("Failed to load tables: %1").arg(query.lastError().text()));
        return;
    }
    
    while (query.next()) {
        QString tableName = query.value(0).toString();
        
        QSqlQuery countQuery(QString("SELECT COUNT(*) FROM %1").arg(tableName));
        int recordCount = 0;
        if (countQuery.exec() && countQuery.next()) {
            recordCount = countQuery.value(0).toInt();
        }
        
        QTreeWidgetItem *item = new QTreeWidgetItem(m_tablesTree);
        item->setText(0, tableName);
        item->setText(1, QString::number(recordCount));
        item->setData(0, Qt::UserRole, tableName);
    }
    
    m_tablesTree->resizeColumnToContents(0);
}

void DatabaseBrowserDialog::onTableSelected()
{
    QTreeWidgetItem *item = m_tablesTree->currentItem();
    if (!item) return;
    
    QString tableName = item->data(0, Qt::UserRole).toString();
    m_currentTable = tableName;
    
    loadTableData(tableName);
    m_queryEdit->setText(QString("SELECT * FROM %1 LIMIT 100").arg(tableName));
}

void DatabaseBrowserDialog::loadTableData(const QString &tableName)
{
    showQueryResult(QString("SELECT * FROM %1 LIMIT 100").arg(tableName));
}

void DatabaseBrowserDialog::showQueryResult(const QString &queryText)
{
    QSqlQuery query(queryText);
    
    if (!query.exec()) {
        QMessageBox::critical(this, "Query Error", 
                            QString("Query failed: %1").arg(query.lastError().text()));
        return;
    }
    
    m_dataTable->clear();
    m_dataTable->setRowCount(0);
    m_dataTable->setColumnCount(0);
    
    QSqlRecord record = query.record();
    int columnCount = record.count();
    
    if (columnCount == 0) {
        m_recordCountLabel->setText("Query executed successfully (no results)");
        return;
    }
    
    m_dataTable->setColumnCount(columnCount);
    QStringList headers;
    for (int i = 0; i < columnCount; ++i) {
        headers << record.fieldName(i);
    }
    m_dataTable->setHorizontalHeaderLabels(headers);
    
    int row = 0;
    while (query.next()) {
        m_dataTable->insertRow(row);
        
        for (int col = 0; col < columnCount; ++col) {
            QVariant value = query.value(col);
            QString displayValue = value.isNull() ? "[NULL]" : value.toString();
            
            QTableWidgetItem *item = new QTableWidgetItem(displayValue);
            if (value.isNull()) {
                item->setForeground(QColor("#999"));
            }
            m_dataTable->setItem(row, col, item);
        }
        row++;
    }
    
    m_dataTable->resizeColumnsToContents();
    m_recordCountLabel->setText(QString("Showing %1 records").arg(row));
}

void DatabaseBrowserDialog::onRefreshClicked()
{
    loadDatabaseInfo();
    loadTables();
    if (!m_currentTable.isEmpty()) {
        loadTableData(m_currentTable);
    }
}

void DatabaseBrowserDialog::onExecuteQueryClicked()
{
    QString queryText = m_queryEdit->toPlainText().trimmed();
    if (queryText.isEmpty()) {
        QMessageBox::warning(this, "No Query", "Please enter a SQL query to execute.");
        return;
    }
    
    showQueryResult(queryText);
}

void DatabaseBrowserDialog::onCopyPathClicked()
{
    QString dbPath = getDatabasePath();
    QApplication::clipboard()->setText(dbPath);
    QMessageBox::information(this, "Copied", "Database path copied to clipboard!");
}

void DatabaseBrowserDialog::onOpenLocationClicked()
{
    QString dbPath = getDatabasePath();
    QFileInfo fileInfo(dbPath);
    QString dirPath = fileInfo.absoluteDir().absolutePath();
    
    if (QDir(dirPath).exists()) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
    } else {
        QMessageBox::warning(this, "Directory Not Found", 
                           QString("Could not find directory: %1").arg(dirPath));
    }
}

QString DatabaseBrowserDialog::getDatabasePath()
{
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    return dataDir + "/bowling.db";
}