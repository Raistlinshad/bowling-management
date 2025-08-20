#include "LaneWidget.h"
#include "LaneServer.h"
#include <QJsonArray>
#include <QApplication>
#include <QMimeData>
#include <QDrag>

// DraggableTeamButton Implementation
DraggableTeamButton::DraggableTeamButton(const QString &teamName, QWidget *parent)
    : QPushButton(teamName, parent)
    , m_teamData(teamName)
{
    setFixedSize(80, 30);
    setStyleSheet("QPushButton { "
                 "background-color: #4A90E2; "
                 "color: white; "
                 "border: 1px solid #357ABD; "
                 "border-radius: 5px; "
                 "font-size: 12px; "
                 "} "
                 "QPushButton:hover { background-color: #357ABD; }");
}

void DraggableTeamButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragStartPosition = event->pos();
    }
    QPushButton::mousePressEvent(event);
}

void DraggableTeamButton::mouseMoveEvent(QMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton))
        return;

    if ((event->pos() - m_dragStartPosition).manhattanLength() < QApplication::startDragDistance())
        return;

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;
    
    mimeData->setText(m_teamData);
    mimeData->setData("application/x-team-data", m_teamData.toUtf8());
    drag->setMimeData(mimeData);
    
    // Create drag pixmap
    QPixmap pixmap(size());
    render(&pixmap);
    drag->setPixmap(pixmap);
    drag->setHotSpot(m_dragStartPosition);
    
    emit dragStarted(m_teamData);
    
    Qt::DropAction dropAction = drag->exec(Qt::MoveAction);
    Q_UNUSED(dropAction)
}

// LaneWidget Implementation
LaneWidget::LaneWidget(int laneNumber, QWidget *parent)
    : QWidget(parent)
    , m_laneNumber(laneNumber)
    , m_status(LaneStatus::Idle)
    , m_layout(nullptr)
    , m_laneLabel(nullptr)
    , m_statusLabel(nullptr)
    , m_teamButton(nullptr)
{
    setupUI();
    setAcceptDrops(true);
}

void LaneWidget::setupUI()
{
    setFixedSize(100, 90);
    
    m_layout = new QVBoxLayout(this);
    m_layout->setSpacing(2);
    m_layout->setContentsMargins(5, 5, 5, 5);
    
    // Lane number label
    m_laneLabel = new QLabel(QString("Lane %1").arg(m_laneNumber));
    m_laneLabel->setAlignment(Qt::AlignCenter);
    m_laneLabel->setStyleSheet("QLabel { color: white; font-weight: bold; font-size: 12px; }");
    
    // Status label
    m_statusLabel = new QLabel("Disconnected");
    m_statusLabel->setAlignment(Qt::AlignCenter);
    m_statusLabel->setStyleSheet("QLabel { color: white; font-size: 10px; }");
    
    // Team button (initially hidden)
    m_teamButton = new DraggableTeamButton("Team A");
    m_teamButton->hide();
    
    connect(m_teamButton, &DraggableTeamButton::dragStarted,
            this, &LaneWidget::onTeamButtonDragStarted);
    
    m_layout->addWidget(m_laneLabel);
    m_layout->addWidget(m_statusLabel);
    m_layout->addWidget(m_teamButton);
    m_layout->addStretch();
    
    updateBackgroundColor();
}

void LaneWidget::setStatus(LaneStatus status)
{
    m_status = status;
    
    switch (status) {
    case LaneStatus::Idle:
        m_statusLabel->setText("Idle");
        m_teamButton->hide();
        break;
    case LaneStatus::Active:
        m_statusLabel->setText("Active");
        m_teamButton->show(); // Show team button when active
        break;
    case LaneStatus::Maintenance:
        m_statusLabel->setText("Maintenance");
        m_teamButton->hide();
        break;
    case LaneStatus::Error:
        m_statusLabel->setText("Error");
        m_teamButton->hide();
        break;
    }
    
    updateBackgroundColor();
}

void LaneWidget::updateGameData(const QJsonObject &gameData)
{
    m_currentGameData = gameData;
    
    // Update team button text based on game data
    if (gameData.contains("team_name")) {
        m_teamButton->setText(gameData["team_name"].toString());
    } else if (gameData.contains("bowlers")) {
        // If no team name, use first bowler name
        QJsonArray bowlers = gameData["bowlers"].toArray();
        if (!bowlers.isEmpty()) {
            QString teamName = bowlers[0].toString();
            if (teamName.length() > 8) {
                teamName = teamName.left(8) + "...";
            }
            m_teamButton->setText(teamName);
        }
    }
}

void LaneWidget::updateBackgroundColor()
{
    QString backgroundColor;
    
    switch (m_status) {
    case LaneStatus::Idle:
        backgroundColor = "#333333"; // Dark gray
        break;
    case LaneStatus::Active:
        backgroundColor = "#006400"; // Dark green
        break;
    case LaneStatus::Maintenance:
        backgroundColor = "#FF8C00"; // Dark orange
        break;
    case LaneStatus::Error:
        backgroundColor = "#8B0000"; // Dark red
        break;
    }
    
    setStyleSheet(QString("LaneWidget { "
                         "background-color: %1; "
                         "border: 2px solid #333; "
                         "border-radius: 5px; "
                         "}").arg(backgroundColor));
}

void LaneWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-team-data")) {
        event->acceptProposedAction();
    }
}

void LaneWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat("application/x-team-data")) {
        QString teamData = QString::fromUtf8(event->mimeData()->data("application/x-team-data"));
        
        // Find source lane (this is simplified - in real implementation you'd track this better)
        QWidget *source = qobject_cast<QWidget*>(event->source());
        if (source) {
            LaneWidget *sourceLane = qobject_cast<LaneWidget*>(source->parent());
            if (sourceLane && sourceLane != this) {
                emit teamMoveRequested(sourceLane->getLaneNumber(), m_laneNumber, teamData);
            }
        }
        
        event->acceptProposedAction();
    }
}

void LaneWidget::onTeamButtonDragStarted(const QString &teamData)
{
    // This could be used for visual feedback during drag
    Q_UNUSED(teamData)
}