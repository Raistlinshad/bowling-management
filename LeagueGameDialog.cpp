#include "LeagueGameDialog.h"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>

LeagueGameDialog::LeagueGameDialog(QWidget *parent)
    : QDialog(parent)
{
    setupUI();
    setWindowTitle("League Game Setup");
    setMinimumSize(500, 400);
}

void LeagueGameDialog::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    layout->addWidget(new QLabel("League Game Dialog - Implementation Coming Soon"));
    layout->addStretch();
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    
    QPushButton *cancelButton = new QPushButton("Cancel");
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    buttonLayout->addWidget(cancelButton);
    
    QPushButton *okButton = new QPushButton("OK");
    connect(okButton, &QPushButton::clicked, this, &QDialog::accept);
    buttonLayout->addWidget(okButton);
    
    layout->addLayout(buttonLayout);
}

QJsonObject LeagueGameDialog::getGameData() const
{
    QJsonObject data;
    data["type"] = "league_game";
    data["lane_id"] = 1; // Placeholder
    return data;
}
