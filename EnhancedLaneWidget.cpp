#include "EnhancedLaneWidget.h"
#include <QApplication>
#include <QMouseEvent>
#include <QJsonArray>

EnhancedLaneWidget::EnhancedLaneWidget(int laneNumber, QWidget *parent)
    : QWidget(parent)
    , m_laneNumber(laneNumber)
    , m_status(EnhancedLaneStatus::Disconnected)
    , m_layout(nullptr)
    , m_headerFrame(nullptr)
    , m_laneLabel(nullptr)
    , m_statusLabel(nullptr)
    , m_gameInfoFrame(nullptr)
    , m_gameInfoLayout(nullptr)
    , m_leftSection(nullptr)
    , m_rightSection(nullptr)
    , m_holdButton(nullptr)
    , m_teamButton(nullptr)
    , m_infoButton(nullptr)
    , m_gameTypeLabel(nullptr)
    , m_gameProgressLabel(nullptr)
    , m_scoreLabel(nullptr)
{
    setupUI();
    setStatus(EnhancedLaneStatus::Disconnected);
}

void EnhancedLaneWidget::setupUI()
{
    setFixedWidth(LANE_WIDTH);
    setMinimumHeight(LANE_HEIGHT_BASIC);
    
    m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(2);
    m_layout->setContentsMargins(2, 2, 2, 2);
    
    // Header section - always visible
    m_headerFrame = new QFrame;
    m_headerFrame->setFixedHeight(40);
    m_headerFrame->setFrameStyle(QFrame::Box);
    
    QVBoxLayout *headerLayout = new QVBoxLayout(m_headerFrame);
    headerLayout->setSpacing(1);
    headerLayout->setContentsMargins(4, 4, 4, 4);
    
    m_laneLabel = new QLabel(QString("LANE %1").arg(m_laneNumber));
    m_laneLabel->setAlignment(Qt::AlignCenter);
    m_laneLabel->setStyleSheet("QLabel { color: white; font-weight: bold; font-size: 11px; }");
    
    m_statusLabel = new QLabel("Disconnected");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("QLabel { color: white; font-size: 9px; }");
    
    headerLayout->addWidget(m_laneLabel);
    headerLayout->addWidget(m_statusLabel);
    
    m_layout->addWidget(m_headerFrame);
    
    // Game info section - only visible when active
    m_gameInfoFrame = new QFrame;
    m_gameInfoFrame->setFrameStyle(QFrame::Box);
    m_gameInfoFrame->hide(); // Hidden by default
    
    m_gameInfoLayout = new QHBoxLayout(m_gameInfoFrame);
    m_gameInfoLayout->setSpacing(2);
    m_gameInfoLayout->setContentsMargins(4, 4, 4, 4);
    
    // Left section - bowler/team buttons
    m_leftSection = new QFrame;
    m_leftSection->setFixedWidth(80);
    QVBoxLayout *leftLayout = new QVBoxLayout(m_leftSection);
    leftLayout->setSpacing(2);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    
    // Right section - game info
    m_rightSection = new QFrame;
    QVBoxLayout *rightLayout = new QVBoxLayout(m_rightSection);
    rightLayout->setSpacing(2);
    rightLayout->setContentsMargins(4, 2, 4, 2);
    
    m_gameTypeLabel = new QLabel;
    m_gameTypeLabel->setStyleSheet("QLabel { color: white; font-size: 9px; font-weight: bold; }");
    
    m_gameProgressLabel = new QLabel;
    m_gameProgressLabel->setStyleSheet("QLabel { color: white; font-size: 8px; }");
    
    m_scoreLabel = new QLabel;
    m_scoreLabel->setStyleSheet("QLabel { color: white; font-size: 9px; }");
    
    rightLayout->addWidget(m_gameTypeLabel);
    rightLayout->addWidget(m_gameProgressLabel);
    rightLayout->addWidget(m_scoreLabel);
    rightLayout->addStretch();
    
    m_gameInfoLayout->addWidget(m_leftSection);
    m_gameInfoLayout->addWidget(m_rightSection);
    
    m_layout->addWidget(m_gameInfoFrame);
    m_layout->addStretch();
}

void EnhancedLaneWidget::setStatus(EnhancedLaneStatus status)
{
    m_status = status;
    updateDisplay();
}

void EnhancedLaneWidget::updateGameData(const QJsonObject &gameData)
{
    // Parse JSON data into GameData structure
    m_gameData.gameType = gameData["type"].toString();
    m_gameData.teamName = gameData["team_name"].toString();
    m_gameData.isHeld = gameData["held"].toBool();
    m_gameData.isCompleted = gameData["completed"].toBool();
    m_gameData.gamesPlayed = gameData["games_played"].toInt();
    m_gameData.totalGames = gameData["total_games"].toInt();
    
    // Parse bowlers
    m_gameData.bowlers.clear();
    QJsonArray bowlers = gameData["bowlers"].toArray();
    for (const QJsonValue &bowlerValue : bowlers) {
        QJsonObject bowlerObj = bowlerValue.toObject();
        BowlerData bowler;
        bowler.name = bowlerObj["name"].toString();
        bowler.currentFrame = bowlerObj["current_frame"].toInt(1);
        bowler.currentBall = bowlerObj["current_ball"].toInt(1);
        bowler.totalScore = bowlerObj["total_score"].toInt();
        bowler.isActive = bowlerObj["is_active"].toBool();
        
        // Parse frame scores
        QJsonArray frames = bowlerObj["frames"].toArray();
        for (int f = 0; f < frames.size(); ++f) {
            QJsonArray frameData = frames[f].toArray();
            QVector<int> frameBalls;
            for (int b = 0; b < frameData.size(); ++b) {
                frameBalls.append(frameData[b].toInt());
            }
            bowler.frameScores.append(frameBalls);
        }
        
        // Parse frame totals
        QJsonArray totals = bowlerObj["frame_totals"].toArray();
        for (const QJsonValue &total : totals) {
            bowler.frameTotals.append(total.toInt());
        }
        
        m_gameData.bowlers.append(bowler);
    }
    
    m_gameData.currentBowlerIndex = gameData["current_bowler"].toInt();
    
    updateDisplay();
}

void EnhancedLaneWidget::setHoldState(bool held)
{
    m_gameData.isHeld = held;
    if (held) {
        setStatus(EnhancedLaneStatus::Hold);
    } else {
        // Return to previous game status
        if (!m_gameData.bowlers.isEmpty()) {
            if (m_gameData.gameType == "league_game") {
                setStatus(EnhancedLaneStatus::LeagueGame);
            } else {
                setStatus(EnhancedLaneStatus::QuickGame);
            }
        }
    }
}

void EnhancedLaneWidget::updateDisplay()
{
    // Update status text
    m_statusLabel->setText(getStatusDisplayText());
    
    // Update colors based on status
    QColor statusColor = getStatusColor();
    QString colorStyle = QString("background-color: %1;").arg(statusColor.name());
    
    m_headerFrame->setStyleSheet(QString("QFrame { %1 border: 1px solid #333; }").arg(colorStyle));
    
    // Show/hide game info based on status
    bool showGameInfo = (m_status == EnhancedLaneStatus::QuickGame || 
                        m_status == EnhancedLaneStatus::LeagueGame ||
                        m_status == EnhancedLaneStatus::Hold ||
                        m_status == EnhancedLaneStatus::Completed);
    
    if (showGameInfo) {
        m_gameInfoFrame->show();
        setFixedHeight(LANE_HEIGHT_ACTIVE);
        updateButtonsForStatus();
        
        // Update game info labels
        if (m_gameData.gameType == "quick_game") {
            m_gameTypeLabel->setText("QUICK GAME");
        } else if (m_gameData.gameType == "league_game") {
            m_gameTypeLabel->setText("LEAGUE");
        }
        
        if (!m_gameData.bowlers.isEmpty()) {
            QString progress;
            if (m_gameData.totalGames > 1) {
                progress = QString("Game %1 of %2").arg(m_gameData.gamesPlayed + 1).arg(m_gameData.totalGames);
            } else {
                BowlerData &current = m_gameData.bowlers[m_gameData.currentBowlerIndex];
                progress = QString("Frame %1, Ball %2").arg(current.currentFrame).arg(current.currentBall);
            }
            m_gameProgressLabel->setText(progress);
            
            // Show current score
            if (m_gameData.currentBowlerIndex < m_gameData.bowlers.size()) {
                int score = m_gameData.bowlers[m_gameData.currentBowlerIndex].totalScore;
                m_scoreLabel->setText(QString("Score: %1").arg(score));
            }
        }
    } else {
        m_gameInfoFrame->hide();
        setFixedHeight(LANE_HEIGHT_BASIC);
        clearBowlerButtons();
    }
    
    update();
}

void EnhancedLaneWidget::updateButtonsForStatus()
{
    // Clear existing buttons
    clearBowlerButtons();
    
    // Create buttons based on game data
    if (m_status == EnhancedLaneStatus::Completed) {
        // Show results button
        m_infoButton = new QPushButton("VIEW RESULTS");
        m_infoButton->setStyleSheet("QPushButton { "
                                   "background-color: #1E3A8A; "
                                   "color: white; "
                                   "border: 1px solid #1E40AF; "
                                   "padding: 4px; "
                                   "font-size: 8px; "
                                   "font-weight: bold; "
                                   "}");
        connect(m_infoButton, &QPushButton::clicked, this, &EnhancedLaneWidget::onInfoButtonClicked);
        m_leftSection->layout()->addWidget(m_infoButton);
        return;
    }
    
    if (!m_gameData.bowlers.isEmpty()) {
        // League games show team button, quick games show bowler buttons
        if (m_gameData.gameType == "league_game" && !m_gameData.teamName.isEmpty()) {
            // Show team button
            m_teamButton = new QPushButton(m_gameData.teamName);
            m_teamButton->setStyleSheet("QPushButton { "
                                       "background-color: #1E40AF; "
                                       "color: white; "
                                       "border: 1px solid #2563EB; "
                                       "padding: 4px; "
                                       "font-size: 8px; "
                                       "font-weight: bold; "
                                       "}");
            connect(m_teamButton, &QPushButton::clicked, this, &EnhancedLaneWidget::onBowlerButtonClicked);
            m_leftSection->layout()->addWidget(m_teamButton);
        } else {
            // Show individual bowler buttons for quick games
            createBowlerButtons();
        }
        
        // Add hold button
        m_holdButton = new QPushButton(m_gameData.isHeld ? "RESUME" : "HOLD");
        QString holdColor = m_gameData.isHeld ? "#DC2626" : "#059669";
        m_holdButton->setStyleSheet(QString("QPushButton { "
                                           "background-color: %1; "
                                           "color: white; "
                                           "border: 1px solid %1; "
                                           "padding: 4px; "
                                           "font-size: 8px; "
                                           "font-weight: bold; "
                                           "}").arg(holdColor));
        connect(m_holdButton, &QPushButton::clicked, this, &EnhancedLaneWidget::onHoldButtonClicked);
        m_leftSection->layout()->addWidget(m_holdButton);
    }
}

void EnhancedLaneWidget::createBowlerButtons()
{
    for (int i = 0; i < m_gameData.bowlers.size(); ++i) {
        const BowlerData &bowler = m_gameData.bowlers[i];
        
        QPushButton *bowlerBtn = new QPushButton(bowler.name);
        
        // Highlight active bowler
        QString buttonStyle;
        if (bowler.isActive) {
            buttonStyle = "QPushButton { "
                         "background-color: #059669; "
                         "color: white; "
                         "border: 1px solid #10B981; "
                         "padding: 4px; "
                         "font-size: 8px; "
                         "font-weight: bold; "
                         "}";
        } else {
            buttonStyle = "QPushButton { "
                         "background-color: #374151; "
                         "color: white; "
                         "border: 1px solid #4B5563; "
                         "padding: 4px; "
                         "font-size: 8px; "
                         "}";
        }
        
        bowlerBtn->setStyleSheet(buttonStyle);
        connect(bowlerBtn, &QPushButton::clicked, this, &EnhancedLaneWidget::onBowlerButtonClicked);
        
        m_bowlerButtons.append(bowlerBtn);
        m_leftSection->layout()->addWidget(bowlerBtn);
    }
}

void EnhancedLaneWidget::clearBowlerButtons()
{
    // Remove all dynamic buttons
    for (QPushButton *btn : m_bowlerButtons) {
        btn->deleteLater();
    }
    m_bowlerButtons.clear();
    
    if (m_holdButton) {
        m_holdButton->deleteLater();
        m_holdButton = nullptr;
    }
    
    if (m_teamButton) {
        m_teamButton->deleteLater();
        m_teamButton = nullptr;
    }
    
    if (m_infoButton) {
        m_infoButton->deleteLater();
        m_infoButton = nullptr;
    }
}

QString EnhancedLaneWidget::getStatusDisplayText() const
{
    switch (m_status) {
    case EnhancedLaneStatus::Disconnected:
        return "Disconnected";
    case EnhancedLaneStatus::Connected:
        return "Connected";
    case EnhancedLaneStatus::QuickGame:
        return "Quick Game";
    case EnhancedLaneStatus::LeagueGame:
        return "League Game";
    case EnhancedLaneStatus::Hold:
        return "HOLD";
    case EnhancedLaneStatus::Completed:
        return "Completed";
    case EnhancedLaneStatus::Maintenance:
        return "Maintenance";
    case EnhancedLaneStatus::Error:
        return "Error";
    default:
        return "Unknown";
    }
}

QColor EnhancedLaneWidget::getStatusColor() const
{
    switch (m_status) {
    case EnhancedLaneStatus::Disconnected:
        return QColor("#4B5563"); // Gray
    case EnhancedLaneStatus::Connected:
        return QColor("#374151"); // Dark gray
    case EnhancedLaneStatus::QuickGame:
        return QColor("#059669"); // Green
    case EnhancedLaneStatus::LeagueGame:
        return QColor("#2563EB"); // Blue
    case EnhancedLaneStatus::Hold:
        return QColor("#DC2626"); // Red
    case EnhancedLaneStatus::Completed:
        return QColor("#1E3A8A"); // Dark blue
    case EnhancedLaneStatus::Maintenance:
        return QColor("#D97706"); // Orange
    case EnhancedLaneStatus::Error:
        return QColor("#B91C1C"); // Dark red
    default:
        return QColor("#4B5563"); // Default gray
    }
}

void EnhancedLaneWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        emit laneClicked(m_laneNumber);
    }
    QWidget::mousePressEvent(event);
}

void EnhancedLaneWidget::onHoldButtonClicked()
{
    bool newHoldState = !m_gameData.isHeld;
    emit holdToggled(m_laneNumber, newHoldState);
}

void EnhancedLaneWidget::onBowlerButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (button) {
        emit bowlerButtonClicked(m_laneNumber, button->text());
    }
}

void EnhancedLaneWidget::onInfoButtonClicked()
{
    if (m_status == EnhancedLaneStatus::Completed) {
        emit gameResultsRequested(m_laneNumber);
    } else {
        emit gameEditRequested(m_laneNumber);
    }
}