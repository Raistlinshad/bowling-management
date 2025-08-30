#include "MainWindow.h"
#include "QuickGameDialog.h"
#include <QApplication>
#include <QMenuBar>
#include <QStatusBar>
#include <QSplitter>
#include <QGroupBox>
#include <QMessageBox>
#include <QJsonDocument>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_timeUpdateTimer(new QTimer(this))
    , m_laneServer(nullptr)
    , m_eventBus(new EventBus(this))
    , m_actions(nullptr)
    , m_totalLanes(8) // Default 8 lanes, should be configurable
{
    setupUI();
    
    // Initialize core components
    m_laneServer = new LaneServer(m_eventBus, this);
    m_actions = new Actions(this, m_eventBus, this);
    
    // Connect signals
    connect(m_timeUpdateTimer, &QTimer::timeout, this, &MainWindow::updateTime);
    connect(m_laneServer, &LaneServer::laneStatusChanged, 
            this, &MainWindow::onLaneStatusChanged);
    connect(m_laneServer, &LaneServer::gameDataReceived,
            this, &MainWindow::onGameDataReceived);
    
    // Connect EventBus to lane commands
    connect(m_eventBus, &EventBus::messagePublished,
            [this](const QString &channel, const QString &event, const QJsonObject &data) {
                if (channel == "server" && event == "lane_command") {
                    m_laneServer->onLaneCommand(data);
                }
            });
    
    // Start timer
    m_timeUpdateTimer->start(1000); // Update every second
    
    // Start lane server
    m_laneServer->start();
    
    setWindowTitle("Centre Bowling Management System");
    resize(1200, 800);
}

MainWindow::~MainWindow()
{
    // Clean up game dialogs
    for (GameDisplayDialog *dialog : m_gameDialogs.values()) {
        if (dialog) {
            dialog->close();
            dialog->deleteLater();
        }
    }
    
    if (m_laneServer) {
        m_laneServer->stop();
    }
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget;
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QVBoxLayout(m_centralWidget);
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    
    setupTopBar();
    setupQuickAccessButtons();
    setupMainContent();
    setupLaneWidgets();
}

void MainWindow::setupTopBar()
{
    m_topBar = new QFrame;
    m_topBar->setFrameStyle(QFrame::StyledPanel);
    m_topBar->setFixedHeight(60);
    m_topBar->setStyleSheet("QFrame { background-color: #1a1a1a; border-bottom: 2px solid #333; }");
    
    QHBoxLayout *topLayout = new QHBoxLayout(m_topBar);
    
    // Logo
    m_logoLabel = new QLabel("🎳 CENTRE BOWLING");
    m_logoLabel->setStyleSheet("QLabel { color: #4A90E2; font-size: 18px; font-weight: bold; }");
    
    // Date/Time
    m_dateTimeLabel = new QLabel(getCurrentTime());
    m_dateTimeLabel->setStyleSheet("QLabel { color: #4A90E2; font-size: 14px; }");
    
    topLayout->addWidget(m_logoLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_dateTimeLabel);
    
    m_mainLayout->addWidget(m_topBar);
}

void MainWindow::setupQuickAccessButtons()
{
    // Status bar for quick access shortcuts
    m_statusBar = new QLabel("F1: Quick Start   F2: League Start   F3: New Bowler   F4: Bowler Management   Esc: Main Menu");
    m_statusBar->setStyleSheet("QLabel { background-color: #2a2a2a; color: white; padding: 5px; }");
    m_statusBar->setAlignment(Qt::AlignCenter);
    m_mainLayout->addWidget(m_statusBar);
}

void MainWindow::setupMainContent()
{
    // Create splitter for main content
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal);
    
    // Left side - Quick access buttons
    m_quickAccessFrame = new QFrame;
    m_quickAccessFrame->setFixedWidth(200);
    m_quickAccessFrame->setStyleSheet("QFrame { background-color: #2a2a2a; border-right: 2px solid #333; }");
    
    QVBoxLayout *quickLayout = new QVBoxLayout(m_quickAccessFrame);
    quickLayout->setSpacing(5);
    quickLayout->setContentsMargins(10, 10, 10, 10);
    
    QStringList quickButtons = {"Quick Start", "View Bowler", "Bowler Management", "Calendar", "Teams", 
                               "Leagues", "Party", "Status", "Settings", "Test Lane", "QG Diagnostic", "DB Browser"};
    
    for (const QString &buttonText : quickButtons) {
        QPushButton *button = new QPushButton(buttonText);
        
        // Special styling for important buttons
        if (buttonText == "Bowler Management") {
            button->setStyleSheet("QPushButton { "
                                 "background-color: #4A90E2; "
                                 "color: white; "
                                 "border: 1px solid #357ABD; "
                                 "padding: 8px; "
                                 "text-align: left; "
                                 "font-weight: bold; "
                                 "} "
                                 "QPushButton:hover { background-color: #357ABD; } "
                                 "QPushButton:pressed { background-color: #2E5A87; }");
        } else {
            button->setStyleSheet("QPushButton { "
                                 "background-color: #333; "
                                 "color: white; "
                                 "border: 1px solid #555; "
                                 "padding: 8px; "
                                 "text-align: left; "
                                 "} "
                                 "QPushButton:hover { background-color: #4A90E2; } "
                                 "QPushButton:pressed { background-color: #357ABD; }");
        }
        
        connect(button, &QPushButton::clicked, [this, buttonText]() {
            onQuickAccessButtonClicked(buttonText);
        });
        
        quickLayout->addWidget(button);
    }
    quickLayout->addStretch();
    
    // Right side - Main content area
    m_mainContentArea = new QWidget;
    m_mainContentArea->setStyleSheet("QWidget { background-color: #1a1a1a; }");
    
    mainSplitter->addWidget(m_quickAccessFrame);
    mainSplitter->addWidget(m_mainContentArea);
    mainSplitter->setStretchFactor(1, 1);
    
    m_mainLayout->addWidget(mainSplitter, 1);
}

void MainWindow::setupLaneWidgets()
{
    // Create enhanced lane widgets container at bottom
    QFrame *laneFrame = new QFrame;
    laneFrame->setFixedHeight(160); // Increased height for enhanced widgets
    laneFrame->setStyleSheet("QFrame { background-color: #2a2a2a; border-top: 2px solid #333; }");
    
    m_laneScrollArea = new QScrollArea;
    m_laneScrollArea->setWidgetResizable(true);
    m_laneScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_laneScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_laneScrollArea->setStyleSheet("QScrollArea { border: none; background-color: #2a2a2a; }");
    
    m_laneContainer = new QWidget;
    m_laneLayout = new QHBoxLayout(m_laneContainer); // Changed to horizontal layout
    m_laneLayout->setSpacing(5);
    m_laneLayout->setContentsMargins(10, 10, 10, 10);
    
    // Create enhanced lane widgets
    for (int i = 1; i <= m_totalLanes; ++i) {
        EnhancedLaneWidget *laneWidget = new EnhancedLaneWidget(i);
        
        // Connect all the enhanced signals
        connect(laneWidget, &EnhancedLaneWidget::laneClicked,
                this, &MainWindow::onLaneClicked);
        connect(laneWidget, &EnhancedLaneWidget::holdToggled,
                this, &MainWindow::onLaneHoldToggled);
        connect(laneWidget, &EnhancedLaneWidget::bowlerButtonClicked,
                this, &MainWindow::onBowlerButtonClicked);
        connect(laneWidget, &EnhancedLaneWidget::gameEditRequested,
                this, &MainWindow::onGameEditRequested);
        connect(laneWidget, &EnhancedLaneWidget::gameResultsRequested,
                this, &MainWindow::onGameResultsRequested);
        connect(laneWidget, &EnhancedLaneWidget::laneShutdownRequested,
                this, &MainWindow::onLaneShutdownRequested);
        
        m_laneLayout->addWidget(laneWidget);
        m_laneWidgets.append(laneWidget);
        m_laneStatuses[i] = EnhancedLaneStatus::Disconnected;
    }
    
    m_laneLayout->addStretch(); // Add stretch at end
    
    m_laneScrollArea->setWidget(m_laneContainer);
    
    QVBoxLayout *laneFrameLayout = new QVBoxLayout(laneFrame);
    laneFrameLayout->setContentsMargins(0, 0, 0, 0);
    laneFrameLayout->addWidget(m_laneScrollArea);
    
    m_mainLayout->addWidget(laneFrame);
}

QString MainWindow::getCurrentTime() const
{
    return QDateTime::currentDateTime().toString("ddd, MMM dd yyyy hh:mm:ss");
}

void MainWindow::updateTime()
{
    m_dateTimeLabel->setText(getCurrentTime());
}

EnhancedLaneStatus MainWindow::convertLaneStatus(LaneStatus oldStatus)
{
    switch (oldStatus) {
    case LaneStatus::Idle:
        return EnhancedLaneStatus::Connected;
    case LaneStatus::Active:
        return EnhancedLaneStatus::QuickGame; // Will be updated based on game data
    case LaneStatus::Maintenance:
        return EnhancedLaneStatus::Maintenance;
    case LaneStatus::Error:
        return EnhancedLaneStatus::Error;
    default:
        return EnhancedLaneStatus::Disconnected;
    }
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_F1:
        m_actions->quickStart();
        break;
    case Qt::Key_F2:
        m_actions->leagueStart();
        break;
    case Qt::Key_F3:
        m_actions->newBowler();
        break;
    case Qt::Key_F4:
        onBowlerManagementClicked();
        break;
    case Qt::Key_Escape:
        m_actions->homeScreen();
        break;
    default:
        QMainWindow::keyPressEvent(event);
    }
}

void MainWindow::onQuickAccessButtonClicked(const QString &buttonText)
{
    if (buttonText == "Quick Start") {
        m_actions->quickStart();
    } else if (buttonText == "View Bowler") {
        m_actions->bowlerInfo();
    } else if (buttonText == "Bowler Management") {
        onBowlerManagementClicked();
    } else if (buttonText == "Calendar") {
        m_actions->calendar();
    } else if (buttonText == "Teams") {
        m_actions->teams();
    } else if (buttonText == "Leagues") {
        m_actions->leagues();
    } else if (buttonText == "Party") {
        m_actions->party();
    } else if (buttonText == "Status") {
        m_actions->showStatus();
    } else if (buttonText == "Settings") {
        m_actions->settings();
    } else if (buttonText == "Test Lane") {
        m_actions->testLaneConnection();
    } else if (buttonText == "QG Diagnostic") {
        m_actions->runQuickGameDiagnostic();
    } else if (buttonText == "DB Browser") {
        m_actions->onDatabaseBrowserClicked();
    }
}

void MainWindow::onBowlerManagementClicked()
{
    BowlerManagementDialog dialog(this, this);
    dialog.exec();
}

void MainWindow::onLaneStatusChanged(int laneId, LaneStatus status)
{
    if (laneId <= 0 || laneId > m_laneWidgets.size()) return;
    
    EnhancedLaneStatus enhancedStatus = convertLaneStatus(status);
    m_laneStatuses[laneId] = enhancedStatus;
    m_laneWidgets[laneId - 1]->setStatus(enhancedStatus);
}

void MainWindow::onGameDataReceived(int laneId, const QJsonObject &gameData)
{
    if (laneId <= 0 || laneId > m_laneWidgets.size()) return;
    
    // Store game data
    m_laneGameData[laneId] = gameData;
    
    // Determine enhanced status based on game data
    EnhancedLaneStatus status = EnhancedLaneStatus::Connected;
    QString gameType = gameData["type"].toString();
    bool isHeld = gameData["held"].toBool();
    bool isCompleted = gameData["completed"].toBool();
    
    if (isCompleted) {
        status = EnhancedLaneStatus::Completed;
    } else if (isHeld) {
        status = EnhancedLaneStatus::Hold;
    } else if (gameType == "league_game") {
        status = EnhancedLaneStatus::LeagueGame;
    } else if (gameType == "quick_game") {
        status = EnhancedLaneStatus::QuickGame;
    }
    
    m_laneStatuses[laneId] = status;
    m_laneWidgets[laneId - 1]->setStatus(status);
    m_laneWidgets[laneId - 1]->updateGameData(gameData);
    
    // Update existing game dialog if open
    if (m_gameDialogs.contains(laneId) && m_gameDialogs[laneId]) {
        m_gameDialogs[laneId]->updateGameInfo();
    }
}

void MainWindow::onLaneClicked(int laneNumber)
{
    EnhancedLaneStatus status = m_laneStatuses.value(laneNumber, EnhancedLaneStatus::Disconnected);
    
    switch (status) {
    case EnhancedLaneStatus::Disconnected:
        QMessageBox::information(this, "Lane Disconnected", 
                               QString("Lane %1 is not connected.").arg(laneNumber));
        break;
        
    case EnhancedLaneStatus::Connected:
        // Open quick game dialog with pre-populated lane ID
        showQuickGameDialog(laneNumber);
        break;
        
    case EnhancedLaneStatus::QuickGame:
    case EnhancedLaneStatus::LeagueGame:
    case EnhancedLaneStatus::Hold:
        // Show game display with edit capabilities
        showGameDisplayDialog(laneNumber);
        break;
        
    case EnhancedLaneStatus::Completed:
        // Show game results
        showGameDisplayDialog(laneNumber);
        break;
        
    case EnhancedLaneStatus::Maintenance:
        QMessageBox::information(this, "Lane Maintenance", 
                               QString("Lane %1 is in maintenance mode.").arg(laneNumber));
        break;
        
    case EnhancedLaneStatus::Error:
        QMessageBox::warning(this, "Lane Error", 
                            QString("Lane %1 has an error. Please check lane status.").arg(laneNumber));
        break;
    }
}

void MainWindow::onLaneHoldToggled(int laneNumber, bool held)
{
    // Send hold command to lane
    QJsonObject holdData;
    holdData["hold"] = held;
    sendLaneCommand(laneNumber, "hold_toggle", holdData);
    
    // Update local state immediately for UI responsiveness
    if (m_laneGameData.contains(laneNumber)) {
        QJsonObject gameData = m_laneGameData[laneNumber];
        gameData["held"] = held;
        m_laneGameData[laneNumber] = gameData;
        
        EnhancedLaneStatus newStatus = held ? EnhancedLaneStatus::Hold : 
                                      (gameData["type"].toString() == "league_game" ? 
                                       EnhancedLaneStatus::LeagueGame : EnhancedLaneStatus::QuickGame);
        
        m_laneStatuses[laneNumber] = newStatus;
        m_laneWidgets[laneNumber - 1]->setStatus(newStatus);
        m_laneWidgets[laneNumber - 1]->updateGameData(gameData);
    }
}

void MainWindow::onBowlerButtonClicked(int laneNumber, const QString &bowlerName)
{
    // Show detailed bowler info or open game display
    showGameDisplayDialog(laneNumber);
}

void MainWindow::onGameEditRequested(int laneNumber)
{
    showGameDisplayDialog(laneNumber);
}

void MainWindow::onGameResultsRequested(int laneNumber)
{
    showGameDisplayDialog(laneNumber);
}

void MainWindow::onLaneShutdownRequested(int laneNumber)
{
    // Send shutdown command to lane
    sendLaneCommand(laneNumber, "shutdown");
    
    // Update local state
    m_laneStatuses[laneNumber] = EnhancedLaneStatus::Connected;
    m_laneWidgets[laneNumber - 1]->setStatus(EnhancedLaneStatus::Connected);
    m_laneGameData.remove(laneNumber);
    
    // Close any open game dialog
    if (m_gameDialogs.contains(laneNumber)) {
        m_gameDialogs[laneNumber]->close();
        m_gameDialogs[laneNumber]->deleteLater();
        m_gameDialogs.remove(laneNumber);
    }
}

void MainWindow::showGameDisplayDialog(int laneNumber)
{
    // Close existing dialog for this lane if open
    if (m_gameDialogs.contains(laneNumber) && m_gameDialogs[laneNumber]) {
        m_gameDialogs[laneNumber]->close();
        m_gameDialogs[laneNumber]->deleteLater();
    }
    
    // Get current game data
    QJsonObject gameData = m_laneGameData.value(laneNumber);
    if (gameData.isEmpty()) {
        QMessageBox::information(this, "No Game Data", 
                               QString("No game data available for Lane %1.").arg(laneNumber));
        return;
    }
    
    // Create new game display dialog
    GameDisplayDialog *dialog = new GameDisplayDialog(laneNumber, gameData, this);
    
    // Connect dialog signals
    connect(dialog, &GameDisplayDialog::holdToggled,
            this, &MainWindow::onLaneHoldToggled);
    connect(dialog, &GameDisplayDialog::ballValueChanged,
            this, &MainWindow::onBallValueChanged);
    connect(dialog, &GameDisplayDialog::revertLastBall,
            this, &MainWindow::onRevertLastBall);
    connect(dialog, &GameDisplayDialog::gameEdited,
            this, &MainWindow::onGameEdited);
    connect(dialog, &GameDisplayDialog::laneShutdown,
            this, &MainWindow::onLaneShutdownRequested);
    
    // Clean up when dialog closes
    connect(dialog, &QDialog::finished, [this, laneNumber]() {
        if (m_gameDialogs.contains(laneNumber)) {
            m_gameDialogs[laneNumber]->deleteLater();
            m_gameDialogs.remove(laneNumber);
        }
    });
    
    m_gameDialogs[laneNumber] = dialog;
    dialog->show();
}

void MainWindow::showQuickGameDialog(int laneNumber)
{
    QuickGameDialog dialog(this);
    
    // Pre-populate lane ID
    QLineEdit *laneEdit = dialog.findChild<QLineEdit*>("laneEdit");
    if (laneEdit) {
        laneEdit->setText(QString::number(laneNumber));
    }
    
    if (dialog.exec() == QDialog::Accepted) {
        QJsonObject gameData = dialog.getGameData();
        int laneId = gameData["lane_id"].toInt();
        
        // Send game to lane through event bus
        QJsonObject commandData;
        commandData["lane_id"] = laneId;
        commandData["type"] = "quick_game";
        commandData["data"] = gameData;
        
        m_eventBus->publish("server", "lane_command", commandData);
    }
}

void MainWindow::sendLaneCommand(int laneNumber, const QString &command, const QJsonObject &data)
{
    QJsonObject commandData = data;
    commandData["lane_id"] = laneNumber;
    commandData["command"] = command;
    
    m_eventBus->publish("server", "lane_command", commandData);
}

void MainWindow::onBallValueChanged(int laneNumber, const QString &bowlerName, int frame, int ball, int newValue)
{
    // Send pin update command to lane
    QJsonObject updateData;
    updateData["bowler_name"] = bowlerName;
    updateData["frame"] = frame;
    updateData["ball"] = ball;
    updateData["new_value"] = newValue;
    
    sendLaneCommand(laneNumber, "update_ball", updateData);
    
    qDebug() << "Ball update sent - Lane:" << laneNumber << "Bowler:" << bowlerName 
             << "Frame:" << frame << "Ball:" << ball << "Value:" << newValue;
}

void MainWindow::onRevertLastBall(int laneNumber)
{
    // Send revert command to lane
    sendLaneCommand(laneNumber, "revert_last_ball");
    
    qDebug() << "Revert last ball command sent to lane" << laneNumber;
}

void MainWindow::onGameEdited(int laneNumber, const QJsonObject &updatedData)
{
    // Send updated game data to lane
    sendLaneCommand(laneNumber, "update_game_data", updatedData);
    
    // Update local data
    m_laneGameData[laneNumber] = updatedData;
    
    qDebug() << "Game data updated for lane" << laneNumber;
}