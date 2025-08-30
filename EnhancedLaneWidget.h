#ifndef ENHANCEDLANEWIDGET_H
#define ENHANCEDLANEWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QJsonArray>
#include <QTimer>
#include <QFrame>
#include <QGroupBox>

// Enhanced lane status enum
enum class EnhancedLaneStatus {
    Disconnected,    // Gray - Lane not connected
    Connected,       // Dark gray - Connected but no game
    QuickGame,       // Green - Quick game running
    LeagueGame,      // Blue - League game running
    Hold,            // Red - Game on hold
    Completed,       // Dark blue - Game completed
    Maintenance,     // Orange - Maintenance mode
    Error           // Red - Error state
};

struct BowlerGameState {
    QString name;
    int currentFrame = 1;
    int currentBall = 1;
    QVector<QVector<int>> frameScores; // [frame][ball] - ball results
    QVector<int> frameTotals;
    int totalScore = 0;
    bool isActive = false;
};

struct GameData {
    QString gameType; // "quick_game", "league_game"
    QVector<BowlerGameState> bowlers;
    int currentBowlerIndex = 0;
    bool isHeld = false;
    bool isCompleted = false;
    QString teamName;
    int gamesPlayed = 0;
    int totalGames = 0;
};

class EnhancedLaneWidget : public QWidget
{
    Q_OBJECT

public:
    explicit EnhancedLaneWidget(int laneNumber, QWidget *parent = nullptr);
    
    void setStatus(EnhancedLaneStatus status);
    void updateGameData(const QJsonObject &gameData);
    void setHoldState(bool held);
    
    int getLaneNumber() const { return m_laneNumber; }
    EnhancedLaneStatus getStatus() const { return m_status; }
    bool isHeld() const { return m_gameData.isHeld; }

signals:
    void laneClicked(int laneNumber);
    void holdToggled(int laneNumber, bool held);
    void bowlerButtonClicked(int laneNumber, const QString &bowlerName);
    void gameEditRequested(int laneNumber);
    void gameResultsRequested(int laneNumber);
    void laneShutdownRequested(int laneNumber);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private slots:
    void onHoldButtonClicked();
    void onBowlerButtonClicked();
    void onInfoButtonClicked();

private:
    void setupUI();
    void updateDisplay();
    void updateButtonsForStatus();
    void createBowlerButtons();
    void clearBowlerButtons();
    QString getStatusDisplayText() const;
    QColor getStatusColor() const;
    
    int m_laneNumber;
    EnhancedLaneStatus m_status;
    GameData m_gameData;
    
    // UI Components
    QVBoxLayout *m_layout;
    QFrame *m_headerFrame;
    QLabel *m_laneLabel;
    QLabel *m_statusLabel;
    
    // Game info section
    QFrame *m_gameInfoFrame;
    QHBoxLayout *m_gameInfoLayout;
    QFrame *m_leftSection;    // Bowler/team buttons
    QFrame *m_rightSection;   // Game info
    
    // Buttons
    QVector<QPushButton*> m_bowlerButtons;
    QPushButton *m_holdButton;
    QPushButton *m_teamButton;
    QPushButton *m_infoButton;
    
    // Info labels
    QLabel *m_gameTypeLabel;
    QLabel *m_gameProgressLabel;
    QLabel *m_scoreLabel;
    
    // Styling
    static const int LANE_WIDTH = 180;
    static const int LANE_HEIGHT_BASIC = 80;
    static const int LANE_HEIGHT_ACTIVE = 140;
};

#endif // ENHANCEDLANEWIDGET_H