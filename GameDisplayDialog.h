#ifndef GAMEDISPLAYDIALOG_H
#define GAMEDISPLAYDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QJsonObject>
#include <QJsonArray>
#include <QGroupBox>
#include <QButtonGroup>
#include <QCheckBox>

struct PinState {
    bool pins[5] = {true, true, true, true, true}; // 5-pin bowling
    int getValue() const;
    void setValue(int value);
    QString getDisplayString() const;
};

class PinConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PinConfigWidget(QWidget *parent = nullptr);
    void setPinState(const PinState &state);
    PinState getPinState() const;

signals:
    void pinStateChanged(const PinState &newState);

private slots:
    void onPinClicked();

private:
    void setupUI();
    void updateDisplay();
    
    PinState m_pinState;
    QVector<QPushButton*> m_pinButtons;
    QLabel *m_valueLabel;
};

class FrameWidget : public QWidget
{
    Q_OBJECT

public:
    explicit FrameWidget(int frameNumber, QWidget *parent = nullptr);
    
    void setBallResult(int ballNumber, int value);
    void setBallResult(int ballNumber, const PinState &pins);
    void setFrameTotal(int total);
    void setRunningTotal(int total);
    void setIsCurrentBall(int ballNumber, bool current);

signals:
    void ballClicked(int frameNumber, int ballNumber);

private slots:
    void onBallButtonClicked();

private:
    void setupUI();
    void updateDisplay();
    QString formatBallDisplay(int value, bool isStrike = false, bool isSpare = false);
    
    int m_frameNumber;
    QVector<int> m_ballValues;
    QVector<PinState> m_pinStates;
    int m_frameTotal = 0;
    int m_runningTotal = 0;
    int m_currentBall = -1;
    
    QVBoxLayout *m_layout;
    QHBoxLayout *m_ballsLayout;
    QVector<QPushButton*> m_ballButtons;
    QLabel *m_frameTotalLabel;
    QLabel *m_runningTotalLabel;
    
    static const int MAX_BALLS_PER_FRAME = 3; // Canadian 5-pin allows up to 3 balls
};

class BowlerScoreWidget : public QWidget
{
    Q_OBJECT

public:
    explicit BowlerScoreWidget(const QString &bowlerName, QWidget *parent = nullptr);
    
    void setBowlerName(const QString &name);
    void updateFromGameData(const QJsonObject &bowlerData);
    void setCurrentFrame(int frame, int ball);

signals:
    void ballClicked(const QString &bowlerName, int frame, int ball);

private:
    void setupUI();
    void createFrameWidgets();
    
    QString m_bowlerName;
    QVector<FrameWidget*> m_frameWidgets;
    QLabel *m_nameLabel;
    QLabel *m_totalScoreLabel;
    QScrollArea *m_framesScrollArea;
    QWidget *m_framesContainer;
    QHBoxLayout *m_framesLayout;
    
    static const int MAX_FRAMES = 10;
};

class GameDisplayDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GameDisplayDialog(int laneNumber, const QJsonObject &gameData, QWidget *parent = nullptr);
    void updateGameInfo();

signals:
    void holdToggled(int laneNumber, bool hold);
    void ballValueChanged(int laneNumber, const QString &bowlerName, int frame, int ball, int newValue);
    void revertLastBall(int laneNumber);
    void gameEdited(int laneNumber, const QJsonObject &updatedData);
    void laneShutdown(int laneNumber);

private slots:
    void onBallClicked(const QString &bowlerName, int frame, int ball);
    void onPinConfigChanged(const PinState &newState);
    void onHoldClicked();
    void onRevertLastBallClicked();
    void onEditGameClicked();
    void onCloseClicked();
    void onShutdownLaneClicked();

private:
    void setupUI();
    void setupGameInfo();
    void setupScoreDisplay();
    void setupEditPanel();
    void setupButtons();
    void showPinConfigDialog(const QString &bowlerName, int frame, int ball);
    bool isGameCompleted() const;
    
    int m_laneNumber;
    QJsonObject m_gameData;
    
    // UI Components
    QVBoxLayout *m_mainLayout;
    
    // Game info section
    QGroupBox *m_gameInfoGroup;
    QLabel *m_laneLabel;
    QLabel *m_gameTypeLabel;
    QLabel *m_gameStatusLabel;
    QLabel *m_currentPlayerLabel;
    
    // Score display
    QGroupBox *m_scoreGroup;
    QScrollArea *m_scoreScrollArea;
    QWidget *m_scoreContainer;
    QVBoxLayout *m_scoreLayout;
    QVector<BowlerScoreWidget*> m_bowlerWidgets;
    
    // Edit panel (only shown when editing)
    QGroupBox *m_editGroup;
    PinConfigWidget *m_pinConfigWidget;
    QPushButton *m_applyPinChangeBtn;
    
    // Control buttons
    QFrame *m_buttonFrame;
    QPushButton *m_holdBtn;
    QPushButton *m_revertBtn;
    QPushButton *m_editBtn;
    QPushButton *m_shutdownBtn;
    QPushButton *m_closeBtn;
    
    // Edit state
    QString m_editingBowler;
    int m_editingFrame = -1;
    int m_editingBall = -1;
    bool m_isEditMode = false;
    bool m_isHeld = false;
};

#endif // GAMEDISPLAYDIALOG_H