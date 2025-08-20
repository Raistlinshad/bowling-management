#ifndef LEAGUEGAMEDIALOG_H
#define LEAGUEGAMEDIALOG_H

#include <QDialog>
#include <QJsonObject>

// Placeholder for league game dialog - similar structure to QuickGameDialog
class LeagueGameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LeagueGameDialog(QWidget *parent = nullptr);
    
    QJsonObject getGameData() const;

private:
    void setupUI();
    
    // Add specific league game controls here
};

#endif // LeagueGameDialog.h