#include "GameDisplayDialog.h"
#include <QApplication>
#include <QScreen>
#include <QMessageBox>
#include <QSplitter>

// PinState implementation
int PinState::getValue() const
{
    int value = 0;
    for (int i = 0; i < 5; ++i) {
        if (!pins[i]) value++; // Knocked down pins count toward score
    }
    return value;
}

void PinState::setValue(int value)
{
    // Reset all pins to standing
    for (int i = 0; i < 5; ++i) {
        pins[i] = true;
    }
    
    // Knock down pins from left to right for remaining value
    for (int i = 0; i < 5 && value > 0; ++i) {
        if (i != 2 && pins[i]) { // Skip center pin if already knocked down
            pins[i] = false;
            value--;
        }
    }
}

QString PinState::getDisplayString() const
{
    QString result;
    for (int i = 0; i < 5; ++i) {
        result += pins[i] ? "○" : "●";
    }
    return result;
}

// PinConfigWidget implementation
PinConfigWidget::PinConfigWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
}

void PinConfigWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // Title
    QLabel *titleLabel = new QLabel("Pin Configuration");
    titleLabel->setStyleSheet("QLabel { font-weight: bold; font-size: 14px; color: white; }");
    titleLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(titleLabel);
    
    // Pin display in 5-pin formation
    QWidget *pinWidget = new QWidget;
    QGridLayout *pinLayout = new QGridLayout(pinWidget);
    pinLayout->setSpacing(10);
    
    // 5-pin bowling pin arrangement:
    //   ○ ○
    //  ○ ○ ○
    
    // Back row (pins 0, 1)
    for (int i = 0; i < 2; ++i) {
        QPushButton *pinBtn = new QPushButton;
        pinBtn->setFixedSize(40, 40);
        pinBtn->setCheckable(true);
        pinBtn->setProperty("pinIndex", i);
        connect(pinBtn, &QPushButton::clicked, this, &PinConfigWidget::onPinClicked);
        m_pinButtons.append(pinBtn);
        pinLayout->addWidget(pinBtn, 0, i + 1);
    }
    
    // Front row (pins 2, 3, 4) - pin 2 is the center/headpin
    for (int i = 2; i < 5; ++i) {
        QPushButton *pinBtn = new QPushButton;
        pinBtn->setFixedSize(40, 40);
        pinBtn->setCheckable(true);
        pinBtn->setProperty("pinIndex", i);
        connect(pinBtn, &QPushButton::clicked, this, &PinConfigWidget::onPinClicked);
        m_pinButtons.append(pinBtn);
        pinLayout->addWidget(pinBtn, 1, i - 2);
    }
    
    layout->addWidget(pinWidget);
    
    // Value display
    m_valueLabel = new QLabel("Value: 0");
    m_valueLabel->setStyleSheet("QLabel { font-size: 16px; font-weight: bold; color: white; }");
    m_valueLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_valueLabel);
    
    updateDisplay();
}

void PinConfigWidget::setPinState(const PinState &state)
{
    m_pinState = state;
    updateDisplay();
}

PinState PinConfigWidget::getPinState() const
{
    return m_pinState;
}

void PinConfigWidget::updateDisplay()
{
    for (int i = 0; i < m_pinButtons.size(); ++i) {
        QPushButton *btn = m_pinButtons[i];
        bool standing = m_pinState.pins[i];
        
        btn->setChecked(!standing); // Button checked = pin down
        btn->setText(standing ? "○" : "●");
        
        QString style;
        if (standing) {
            style = "QPushButton { background-color: white; color: black; border: 2px solid #333; border-radius: 20px; font-size: 18px; }";
        } else {
            style = "QPushButton { background-color: black; color: white; border: 2px solid #666; border-radius: 20px; font-size: 18px; }";
        }
        btn->setStyleSheet(style);
    }
    
    int value = m_pinState.getValue();
    m_valueLabel->setText(QString("Value: %1").arg(value));
    
    // Special case display for strikes and spares in 5-pin bowling
    if (value == 15) { // All pins down - strike
        m_valueLabel->setText("Value: 15 (STRIKE)");
    } else if (value == 10) { // Just headpin - common score
        m_valueLabel->setText("Value: 10 (HEADPIN)");
    }
}

void PinConfigWidget::onPinClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    
    int pinIndex = btn->property("pinIndex").toInt();
    m_pinState.pins[pinIndex] = !m_pinState.pins[pinIndex];
    
    updateDisplay();
    emit pinStateChanged(m_pinState);
}

// FrameWidget implementation
FrameWidget::FrameWidget(int frameNumber, QWidget *parent)
    : QWidget(parent)
    , m_frameNumber(frameNumber)
{
    setupUI();
}

void FrameWidget::setupUI()
{
    setFixedSize(80, 100);
    
    m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(2);
    m_layout->setContentsMargins(2, 2, 2, 2);
    
    // Frame number
    QLabel *frameLabel = new QLabel(QString::number(m_frameNumber));
    frameLabel->setAlignment(Qt::AlignCenter);
    frameLabel->setStyleSheet("QLabel { color: white; font-weight: bold; font-size: 12px; }");
    m_layout->addWidget(frameLabel);
    
    // Balls section
    QWidget *ballsWidget = new QWidget;
    m_ballsLayout = new QHBoxLayout(ballsWidget);
    m_ballsLayout->setSpacing(1);
    m_ballsLayout->setContentsMargins(0, 0, 0, 0);
    
    // Create ball buttons (up to 3 for 5-pin bowling)
    for (int i = 0; i < MAX_BALLS_PER_FRAME; ++i) {
        QPushButton *ballBtn = new QPushButton("-");
        ballBtn->setFixedSize(22, 22);
        ballBtn->setProperty("ballNumber", i + 1);
        ballBtn->setStyleSheet("QPushButton { "
                              "background-color: #374151; "
                              "color: white; "
                              "border: 1px solid #4B5563; "
                              "font-size: 9px; "
                              "}");
        connect(ballBtn, &QPushButton::clicked, this, &FrameWidget::onBallButtonClicked);
        m_ballButtons.append(ballBtn);
        m_ballsLayout->addWidget(ballBtn);
    }
    
    m_layout->addWidget(ballsWidget);
    
    // Frame total
    m_frameTotalLabel = new QLabel("0");
    m_frameTotalLabel->setAlignment(Qt::AlignCenter);
    m_frameTotalLabel->setStyleSheet("QLabel { color: white; font-size: 10px; border-top: 1px solid #555; }");
    m_layout->addWidget(m_frameTotalLabel);
    
    // Running total
    m_runningTotalLabel = new QLabel("0");
    m_runningTotalLabel->setAlignment(Qt::AlignCenter);
    m_runningTotalLabel->setStyleSheet("QLabel { color: yellow; font-size: 12px; font-weight: bold; border-top: 1px solid #555; }");
    m_layout->addWidget(m_runningTotalLabel);
    
    // Initialize ball values
    m_ballValues.resize(MAX_BALLS_PER_FRAME);
    m_pinStates.resize(MAX_BALLS_PER_FRAME);
    for (int i = 0; i < MAX_BALLS_PER_FRAME; ++i) {
        m_ballValues[i] = -1; // -1 means not thrown yet
    }
}

void FrameWidget::setBallResult(int ballNumber, int value)
{
    if (ballNumber < 1 || ballNumber > MAX_BALLS_PER_FRAME) return;
    
    int index = ballNumber - 1;
    m_ballValues[index] = value;
    m_pinStates[index].setValue(value);
    updateDisplay();
}

void FrameWidget::setBallResult(int ballNumber, const PinState &pins)
{
    if (ballNumber < 1 || ballNumber > MAX_BALLS_PER_FRAME) return;
    
    int index = ballNumber - 1;
    m_pinStates[index] = pins;
    m_ballValues[index] = pins.getValue();
    updateDisplay();
}

void FrameWidget::setFrameTotal(int total)
{
    m_frameTotal = total;
    updateDisplay();
}

void FrameWidget::setRunningTotal(int total)
{
    m_runningTotal = total;
    updateDisplay();
}

void FrameWidget::setIsCurrentBall(int ballNumber, bool current)
{
    m_currentBall = current ? ballNumber : -1;
    updateDisplay();
}

void FrameWidget::updateDisplay()
{
    // Update ball buttons
    for (int i = 0; i < m_ballButtons.size(); ++i) {
        QPushButton *btn = m_ballButtons[i];
        
        if (m_ballValues[i] == -1) {
            btn->setText("-");
        } else {
            btn->setText(formatBallDisplay(m_ballValues[i]));
        }
        
        // Highlight current ball
        QString style;
        if (m_currentBall == i + 1) {
            style = "QPushButton { "
                   "background-color: #059669; "
                   "color: white; "
                   "border: 2px solid #10B981; "
                   "font-size: 9px; "
                   "font-weight: bold; "
                   "}";
        } else {
            style = "QPushButton { "
                   "background-color: #374151; "
                   "color: white; "
                   "border: 1px solid #4B5563; "
                   "font-size: 9px; "
                   "}";
        }
        btn->setStyleSheet(style);
    }
    
    // Update totals
    m_frameTotalLabel->setText(QString::number(m_frameTotal));
    m_runningTotalLabel->setText(QString::number(m_runningTotal));
}

QString FrameWidget::formatBallDisplay(int value, bool isStrike, bool isSpare)
{
    if (value == 15) return "X"; // Strike in 5-pin bowling
    if (isSpare) return "/";
    return QString::number(value);
}

void FrameWidget::onBallButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    
    int ballNumber = btn->property("ballNumber").toInt();
    emit ballClicked(m_frameNumber, ballNumber);
}

// BowlerScoreWidget implementation
BowlerScoreWidget::BowlerScoreWidget(const QString &bowlerName, QWidget *parent)
    : QWidget(parent)
    , m_bowlerName(bowlerName)
{
    setupUI();
}

void BowlerScoreWidget::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setSpacing(5);
    layout->setContentsMargins(5, 5, 5, 5);
    
    // Bowler name and total
    QHBoxLayout *headerLayout = new QHBoxLayout;
    
    m_nameLabel = new QLabel(m_bowlerName);
    m_nameLabel->setStyleSheet("QLabel { color: white; font-weight: bold; font-size: 14px; }");
    
    m_totalScoreLabel = new QLabel("0");
    m_totalScoreLabel->setStyleSheet("QLabel { color: yellow; font-weight: bold; font-size: 16px; }");
    
    headerLayout->addWidget(m_nameLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_totalScoreLabel);
    
    layout->addLayout(headerLayout);
    
    // Frames scroll area
    m_framesScrollArea = new QScrollArea;
    m_framesScrollArea->setFixedHeight(120);
    m_framesScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_framesScrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_framesScrollArea->setStyleSheet("QScrollArea { border: 1px solid #555; background-color: #2a2a2a; }");
    
    m_framesContainer = new QWidget;
    m_framesLayout = new QHBoxLayout(m_framesContainer);
    m_framesLayout->setSpacing(2);
    m_framesLayout->setContentsMargins(5, 5, 5, 5);
    
    createFrameWidgets();
    
    m_framesScrollArea->setWidget(m_framesContainer);
    layout->addWidget(m_framesScrollArea);
}

void BowlerScoreWidget::createFrameWidgets()
{
    for (int i = 1; i <= MAX_FRAMES; ++i) {
        FrameWidget *frameWidget = new FrameWidget(i);
        connect(frameWidget, &FrameWidget::ballClicked,
                [this, i](int frame, int ball) {
                    emit ballClicked(m_bowlerName, frame, ball);
                });
        
        m_frameWidgets.append(frameWidget);
        m_framesLayout->addWidget(frameWidget);
    }
    m_framesLayout->addStretch();
}

void BowlerScoreWidget::setBowlerName(const QString &name)
{
    m_bowlerName = name;
    m_nameLabel->setText(name);
}

void BowlerScoreWidget::updateFromGameData(const QJsonObject &bowlerData)
{
    // Update total score
    int totalScore = bowlerData["total_score"].toInt();
    m_totalScoreLabel->setText(QString::number(totalScore));
    
    // Update frames
    QJsonArray frames = bowlerData["frames"].toArray();
    for (int f = 0; f < frames.size() && f < m_frameWidgets.size(); ++f) {
        QJsonArray frameData = frames[f].toArray();
        FrameWidget *frameWidget = m_frameWidgets[f];
        
        // Set ball results
        for (int b = 0; b < frameData.size(); ++b) {
            int ballValue = frameData[b].toInt();
            if (ballValue >= 0) {
                frameWidget->setBallResult(b + 1, ballValue);
            }
        }
    }
    
    // Update frame totals
    QJsonArray frameTotals = bowlerData["frame_totals"].toArray();
    for (int i = 0; i < frameTotals.size() && i < m_frameWidgets.size(); ++i) {
        m_frameWidgets[i]->setFrameTotal(frameTotals[i].toInt());
    }
    
    // Update running totals
    QJsonArray runningTotals = bowlerData["running_totals"].toArray();
    for (int i = 0; i < runningTotals.size() && i < m_frameWidgets.size(); ++i) {
        m_frameWidgets[i]->setRunningTotal(runningTotals[i].toInt());
    }
}

void BowlerScoreWidget::setCurrentFrame(int frame, int ball)
{
    // Clear current ball highlighting from all frames
    for (FrameWidget *widget : m_frameWidgets) {
        widget->setIsCurrentBall(0, false);
    }
    
    // Set current ball
    if (frame >= 1 && frame <= m_frameWidgets.size()) {
        m_frameWidgets[frame - 1]->setIsCurrentBall(ball, true);
    }
}

// GameDisplayDialog implementation
GameDisplayDialog::GameDisplayDialog(int laneNumber, const QJsonObject &gameData, QWidget *parent)
    : QDialog(parent)
    , m_laneNumber(laneNumber)
    , m_gameData(gameData)
    , m_isEditMode(false)
    , m_isHeld(false)
{
    setupUI();
    updateGameInfo();
    
    setWindowTitle(QString("Lane %1 - Game Display").arg(laneNumber));
    setModal(false);
    resize(1000, 700);
    
    // Center on screen
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void GameDisplayDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(10);
    m_mainLayout->setContentsMargins(10, 10, 10, 10);
    
    setupGameInfo();
    setupScoreDisplay();
    setupEditPanel();
    setupButtons();
}

void GameDisplayDialog::setupGameInfo()
{
    m_gameInfoGroup = new QGroupBox("Game Information");
    m_gameInfoGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; }");
    
    QHBoxLayout *infoLayout = new QHBoxLayout(m_gameInfoGroup);
    
    m_laneLabel = new QLabel;
    m_laneLabel->setStyleSheet("QLabel { color: white; font-size: 16px; font-weight: bold; }");
    
    m_gameTypeLabel = new QLabel;
    m_gameTypeLabel->setStyleSheet("QLabel { color: white; font-size: 14px; }");
    
    m_gameStatusLabel = new QLabel;
    m_gameStatusLabel->setStyleSheet("QLabel { color: white; font-size: 14px; }");
    
    m_currentPlayerLabel = new QLabel;
    m_currentPlayerLabel->setStyleSheet("QLabel { color: yellow; font-size: 14px; font-weight: bold; }");
    
    infoLayout->addWidget(m_laneLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(m_gameTypeLabel);
    infoLayout->addWidget(m_gameStatusLabel);
    infoLayout->addWidget(m_currentPlayerLabel);
    
    m_mainLayout->addWidget(m_gameInfoGroup);
}

void GameDisplayDialog::setupScoreDisplay()
{
    m_scoreGroup = new QGroupBox("Scores");
    m_scoreGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; }");
    
    QVBoxLayout *scoreLayout = new QVBoxLayout(m_scoreGroup);
    
    m_scoreScrollArea = new QScrollArea;
    m_scoreScrollArea->setWidgetResizable(true);
    m_scoreScrollArea->setStyleSheet("QScrollArea { border: 1px solid #555; background-color: #1a1a1a; }");
    
    m_scoreContainer = new QWidget;
    m_scoreLayout = new QVBoxLayout(m_scoreContainer);
    m_scoreLayout->setSpacing(5);
    
    m_scoreScrollArea->setWidget(m_scoreContainer);
    scoreLayout->addWidget(m_scoreScrollArea);
    
    m_mainLayout->addWidget(m_scoreGroup);
}

void GameDisplayDialog::setupEditPanel()
{
    m_editGroup = new QGroupBox("Edit Ball");
    m_editGroup->setStyleSheet("QGroupBox { color: white; font-weight: bold; }");
    m_editGroup->hide(); // Hidden by default
    
    QHBoxLayout *editLayout = new QHBoxLayout(m_editGroup);
    
    m_pinConfigWidget = new PinConfigWidget;
    connect(m_pinConfigWidget, &PinConfigWidget::pinStateChanged,
            this, &GameDisplayDialog::onPinConfigChanged);
    
    editLayout->addWidget(m_pinConfigWidget);
    
    QVBoxLayout *editButtonLayout = new QVBoxLayout;
    m_applyPinChangeBtn = new QPushButton("Apply Changes");
    m_applyPinChangeBtn->setStyleSheet("QPushButton { "
                                      "background-color: #059669; "
                                      "color: white; "
                                      "border: none; "
                                      "padding: 10px; "
                                      "border-radius: 5px; "
                                      "font-weight: bold; "
                                      "}");
    connect(m_applyPinChangeBtn, &QPushButton::clicked, [this]() {
        if (!m_editingBowler.isEmpty() && m_editingFrame > 0 && m_editingBall > 0) {
            PinState newState = m_pinConfigWidget->getPinState();
            emit ballValueChanged(m_laneNumber, m_editingBowler, m_editingFrame, m_editingBall, newState.getValue());
            m_editGroup->hide();
            m_isEditMode = false;
        }
    });
    
    editButtonLayout->addWidget(m_applyPinChangeBtn);
    editButtonLayout->addStretch();
    
    editLayout->addLayout(editButtonLayout);
    
    m_mainLayout->addWidget(m_editGroup);
}

void GameDisplayDialog::setupButtons()
{
    m_buttonFrame = new QFrame;
    QHBoxLayout *buttonLayout = new QHBoxLayout(m_buttonFrame);
    
    m_holdBtn = new QPushButton("HOLD");
    m_holdBtn->setStyleSheet("QPushButton { "
                            "background-color: #DC2626; "
                            "color: white; "
                            "border: none; "
                            "padding: 10px 20px; "
                            "border-radius: 5px; "
                            "font-weight: bold; "
                            "}");
    connect(m_holdBtn, &QPushButton::clicked, this, &GameDisplayDialog::onHoldClicked);
    
    m_revertBtn = new QPushButton("Revert Last Ball");
    m_revertBtn->setStyleSheet("QPushButton { "
                              "background-color: #D97706; "
                              "color: white; "
                              "border: none; "
                              "padding: 10px 20px; "
                              "border-radius: 5px; "
                              "font-weight: bold; "
                              "}");
    connect(m_revertBtn, &QPushButton::clicked, this, &GameDisplayDialog::onRevertLastBallClicked);
    
    m_editBtn = new QPushButton("Edit Game");
    m_editBtn->setStyleSheet("QPushButton { "
                            "background-color: #2563EB; "
                            "color: white; "
                            "border: none; "
                            "padding: 10px 20px; "
                            "border-radius: 5px; "
                            "font-weight: bold; "
                            "}");
    connect(m_editBtn, &QPushButton::clicked, this, &GameDisplayDialog::onEditGameClicked);
    
    m_closeBtn = new QPushButton("Close");
    connect(m_closeBtn, &QPushButton::clicked, this, &GameDisplayDialog::onCloseClicked);
    
    m_shutdownBtn = new QPushButton("Shutdown Lane");
    m_shutdownBtn->setStyleSheet("QPushButton { "
                                "background-color: #B91C1C; "
                                "color: white; "
                                "border: none; "
                                "padding: 10px 20px; "
                                "border-radius: 5px; "
                                "font-weight: bold; "
                                "}");
    connect(m_shutdownBtn, &QPushButton::clicked, this, &GameDisplayDialog::onShutdownLaneClicked);
    
    buttonLayout->addWidget(m_holdBtn);
    buttonLayout->addWidget(m_revertBtn);
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_editBtn);
    
    // Only show shutdown button if game is completed
    if (isGameCompleted()) {
        buttonLayout->addWidget(m_shutdownBtn);
    }
    
    buttonLayout->addWidget(m_closeBtn);
    
    m_mainLayout->addWidget(m_buttonFrame);
}

void GameDisplayDialog::updateGameInfo()
{
    m_laneLabel->setText(QString("LANE %1").arg(m_laneNumber));
    
    QString gameType = m_gameData["type"].toString();
    if (gameType == "quick_game") {
        m_gameTypeLabel->setText("QUICK GAME");
    } else if (gameType == "league_game") {
        m_gameTypeLabel->setText("LEAGUE GAME");
    }
    
    m_isHeld = m_gameData["held"].toBool();
    if (m_isHeld) {
        m_gameStatusLabel->setText("HOLD");
        m_gameStatusLabel->setStyleSheet("QLabel { color: red; font-size: 14px; font-weight: bold; }");
        m_holdBtn->setText("RESUME");
    } else if (isGameCompleted()) {
        m_gameStatusLabel->setText("COMPLETED");
        m_gameStatusLabel->setStyleSheet("QLabel { color: #1E3A8A; font-size: 14px; font-weight: bold; }");
        m_holdBtn->setEnabled(false);
    } else {
        m_gameStatusLabel->setText("ACTIVE");
        m_gameStatusLabel->setStyleSheet("QLabel { color: green; font-size: 14px; font-weight: bold; }");
        m_holdBtn->setText("HOLD");
    }
    
    // Update current player
    QJsonArray bowlers = m_gameData["bowlers"].toArray();
    int currentBowler = m_gameData["current_bowler"].toInt();
    if (currentBowler < bowlers.size()) {
        QJsonObject bowlerData = bowlers[currentBowler].toObject();
        QString playerName = bowlerData["name"].toString();
        int frame = bowlerData["current_frame"].toInt();
        int ball = bowlerData["current_ball"].toInt();
        m_currentPlayerLabel->setText(QString("Current: %1 (Frame %2, Ball %3)").arg(playerName).arg(frame).arg(ball));
    }
    
    // Create bowler score widgets
    for (BowlerScoreWidget *widget : m_bowlerWidgets) {
        widget->deleteLater();
    }
    m_bowlerWidgets.clear();
    
    for (const QJsonValue &bowlerValue : bowlers) {
        QJsonObject bowlerData = bowlerValue.toObject();
        QString bowlerName = bowlerData["name"].toString();
        
        BowlerScoreWidget *scoreWidget = new BowlerScoreWidget(bowlerName);
        connect(scoreWidget, &BowlerScoreWidget::ballClicked,
                this, &GameDisplayDialog::onBallClicked);
        
        scoreWidget->updateFromGameData(bowlerData);
        
        // Set current frame/ball if this is the active bowler
        if (bowlerData["is_active"].toBool()) {
            int frame = bowlerData["current_frame"].toInt();
            int ball = bowlerData["current_ball"].toInt();
            scoreWidget->setCurrentFrame(frame, ball);
        }
        
        m_bowlerWidgets.append(scoreWidget);
        m_scoreLayout->addWidget(scoreWidget);
    }
}

bool GameDisplayDialog::isGameCompleted() const
{
    return m_gameData["completed"].toBool();
}

void GameDisplayDialog::onBallClicked(const QString &bowlerName, int frame, int ball)
{
    if (isGameCompleted()) {
        QMessageBox::information(this, "Game Completed", "Cannot edit completed games.");
        return;
    }
    
    showPinConfigDialog(bowlerName, frame, ball);
}

void GameDisplayDialog::showPinConfigDialog(const QString &bowlerName, int frame, int ball)
{
    m_editingBowler = bowlerName;
    m_editingFrame = frame;
    m_editingBall = ball;
    m_isEditMode = true;
    
    // Find current pin state for this ball
    QJsonArray bowlers = m_gameData["bowlers"].toArray();
    for (const QJsonValue &bowlerValue : bowlers) {
        QJsonObject bowlerData = bowlerValue.toObject();
        if (bowlerData["name"].toString() == bowlerName) {
            QJsonArray frames = bowlerData["frames"].toArray();
            if (frame - 1 < frames.size()) {
                QJsonArray frameData = frames[frame - 1].toArray();
                if (ball - 1 < frameData.size()) {
                    int currentValue = frameData[ball - 1].toInt();
                    PinState currentState;
                    currentState.setValue(currentValue);
                    m_pinConfigWidget->setPinState(currentState);
                }
            }
            break;
        }
    }
    
    m_editGroup->show();
    m_editGroup->setTitle(QString("Edit Ball - %1 Frame %2 Ball %3").arg(bowlerName).arg(frame).arg(ball));
}

void GameDisplayDialog::onPinConfigChanged(const PinState &newState)
{
    // Pin config changed - button will handle the actual update
}

void GameDisplayDialog::onHoldClicked()
{
    emit holdToggled(m_laneNumber, !m_isHeld);
    close(); // Close dialog when hold is toggled
}

void GameDisplayDialog::onRevertLastBallClicked()
{
    int ret = QMessageBox::question(this, "Revert Last Ball",
                                   "Are you sure you want to revert the last ball thrown?",
                                   QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        emit revertLastBall(m_laneNumber);
    }
}

void GameDisplayDialog::onEditGameClicked()
{
    // Toggle edit mode
    m_isEditMode = !m_isEditMode;
    
    if (m_isEditMode) {
        m_editBtn->setText("Finish Editing");
        QMessageBox::information(this, "Edit Mode", "Click on any ball to edit its pin configuration.");
    } else {
        m_editBtn->setText("Edit Game");
        m_editGroup->hide();
    }
}

void GameDisplayDialog::onCloseClicked()
{
    close();
}

void GameDisplayDialog::onShutdownLaneClicked()
{
    int ret = QMessageBox::question(this, "Shutdown Lane",
                                   QString("Are you sure you want to shutdown Lane %1?\n\n"
                                          "This will end the current game and return the lane to advertising mode.")
                                   .arg(m_laneNumber),
                                   QMessageBox::Yes | QMessageBox::No);
    if (ret == QMessageBox::Yes) {
        emit laneShutdown(m_laneNumber);
        close();
    }
} down the specified number of pins
    // For 5-pin bowling: center pin (headpin) is worth 5, others worth 1 each
    if (value >= 5) {
        pins[2] = false; // Center pin (headpin)
        value -= 5;
    }
    
    // Knock