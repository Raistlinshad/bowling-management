#include "NewBowlerDialog.h"
#include <QMessageBox>
#include <QApplication>
#include <QScreen>
#include <QDebug>

NewBowlerDialog::NewBowlerDialog(QWidget *parent)
    : QDialog(parent)
    , m_editMode(false)
{
    setupUI();
    setWindowTitle("Add New Bowler");
    setModal(true);
    resize(500, 600);
    
    // Center the dialog
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

NewBowlerDialog::NewBowlerDialog(const BowlerInfo &bowler, QWidget *parent)
    : QDialog(parent)
    , m_editMode(true)
{
    setupUI();
    populateFields(bowler);
    setWindowTitle("Edit Bowler");
    setModal(true);
    resize(500, 600);
    
    // Center the dialog
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

void NewBowlerDialog::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(15, 15, 15, 15);
    m_mainLayout->setSpacing(10);
    
    // Create scroll area for the form
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet("QScrollArea { border: none; background-color: #2a2a2a; }");
    
    m_formFrame = new QFrame;
    m_formFrame->setStyleSheet("QFrame { background-color: #2a2a2a; }");
    
    m_formLayout = new QGridLayout(m_formFrame);
    m_formLayout->setContentsMargins(20, 20, 20, 20);
    m_formLayout->setSpacing(15);
    
    QString labelStyle = "QLabel { color: white; font-weight: bold; font-size: 12px; }";
    QString editStyle = "QLineEdit, QSpinBox { "
                       "background-color: #333; "
                       "color: white; "
                       "border: 1px solid #555; "
                       "padding: 8px; "
                       "border-radius: 3px; "
                       "font-size: 12px; "
                       "}";
    QString radioStyle = "QRadioButton { "
                        "color: white; "
                        "font-size: 12px; "
                        "spacing: 5px; "
                        "} "
                        "QRadioButton::indicator { "
                        "width: 15px; "
                        "height: 15px; "
                        "}";
    QString checkboxStyle = "QCheckBox { "
                           "color: white; "
                           "font-size: 12px; "
                           "spacing: 5px; "
                           "}";
    
    int row = 0;
    
    // First Name
    QLabel *firstNameLabel = new QLabel("First Name:");
    firstNameLabel->setStyleSheet(labelStyle);
    m_formLayout->addWidget(firstNameLabel, row, 0);
    
    m_firstNameEdit = new QLineEdit;
    m_firstNameEdit->setStyleSheet(editStyle);
    m_firstNameEdit->setPlaceholderText("Enter first name");
    m_formLayout->addWidget(m_firstNameEdit, row, 1);
    row++;
    
    // Last Name
    QLabel *lastNameLabel = new QLabel("Last Name:");
    lastNameLabel->setStyleSheet(labelStyle);
    m_formLayout->addWidget(lastNameLabel, row, 0);
    
    m_lastNameEdit = new QLineEdit;
    m_lastNameEdit->setStyleSheet(editStyle);
    m_lastNameEdit->setPlaceholderText("Enter last name");
    m_formLayout->addWidget(m_lastNameEdit, row, 1);
    row++;
    
    // Sex
    QLabel *sexLabel = new QLabel("Sex:");
    sexLabel->setStyleSheet(labelStyle);
    m_formLayout->addWidget(sexLabel, row, 0);
    
    QFrame *sexFrame = new QFrame;
    QHBoxLayout *sexLayout = new QHBoxLayout(sexFrame);
    sexLayout->setContentsMargins(0, 0, 0, 0);
    
    m_sexGroup = new QButtonGroup(this);
    
    m_maleRadio = new QRadioButton("Male");
    m_maleRadio->setStyleSheet(radioStyle);
    m_maleRadio->setChecked(true);
    m_sexGroup->addButton(m_maleRadio, 0);
    sexLayout->addWidget(m_maleRadio);
    
    m_femaleRadio = new QRadioButton("Female");
    m_femaleRadio->setStyleSheet(radioStyle);
    m_sexGroup->addButton(m_femaleRadio, 1);
    sexLayout->addWidget(m_femaleRadio);
    
    m_otherRadio = new QRadioButton("Other");
    m_otherRadio->setStyleSheet(radioStyle);
    m_sexGroup->addButton(m_otherRadio, 2);
    sexLayout->addWidget(m_otherRadio);
    
    sexLayout->addStretch();
    m_formLayout->addWidget(sexFrame, row, 1);
    row++;
    
    // Average
    QLabel *averageLabel = new QLabel("Average:");
    averageLabel->setStyleSheet(labelStyle);
    m_formLayout->addWidget(averageLabel, row, 0);
    
    m_averageSpin = new QSpinBox;
    m_averageSpin->setStyleSheet(editStyle);
    m_averageSpin->setRange(0, 300);
    m_averageSpin->setValue(0);
    m_averageSpin->setSpecialValueText("N/A");
    m_formLayout->addWidget(m_averageSpin, row, 1);
    row++;
    
    // Birthday
    QLabel *birthdayLabel = new QLabel("Birthday:");
    birthdayLabel->setStyleSheet(labelStyle);
    m_formLayout->addWidget(birthdayLabel, row, 0);
    
    m_birthdayEdit = new QLineEdit;
    m_birthdayEdit->setStyleSheet(editStyle);
    m_birthdayEdit->setPlaceholderText("MM/DD/YYYY");
    m_formLayout->addWidget(m_birthdayEdit, row, 1);
    row++;
    
    // Over 18
    QLabel *over18Label = new QLabel("Age Category:");
    over18Label->setStyleSheet(labelStyle);
    m_formLayout->addWidget(over18Label, row, 0);
    
    m_over18Check = new QCheckBox("Adult (18+)");
    m_over18Check->setStyleSheet(checkboxStyle);
    m_over18Check->setChecked(true);
    m_formLayout->addWidget(m_over18Check, row, 1);
    row++;
    
    // Phone
    QLabel *phoneLabel = new QLabel("Phone:");
    phoneLabel->setStyleSheet(labelStyle);
    m_formLayout->addWidget(phoneLabel, row, 0);
    
    m_phoneEdit = new QLineEdit;
    m_phoneEdit->setStyleSheet(editStyle);
    m_phoneEdit->setPlaceholderText("(555) 123-4567");
    m_formLayout->addWidget(m_phoneEdit, row, 1);
    row++;
    
    // Address
    QLabel *addressLabel = new QLabel("Address:");
    addressLabel->setStyleSheet(labelStyle);
    m_formLayout->addWidget(addressLabel, row, 0);
    
    m_addressEdit = new QLineEdit;
    m_addressEdit->setStyleSheet(editStyle);
    m_addressEdit->setPlaceholderText("Street, City, State ZIP");
    m_formLayout->addWidget(m_addressEdit, row, 1);
    row++;
    
    m_scrollArea->setWidget(m_formFrame);
    m_mainLayout->addWidget(m_scrollArea);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    
    m_cancelBtn = new QPushButton("Cancel");
    m_cancelBtn->setStyleSheet("QPushButton { "
                              "background-color: #666; "
                              "color: white; "
                              "border: none; "
                              "padding: 10px 20px; "
                              "border-radius: 3px; "
                              "font-size: 12px; "
                              "} "
                              "QPushButton:hover { background-color: #777; }");
    connect(m_cancelBtn, &QPushButton::clicked, this, &NewBowlerDialog::onCancelClicked);
    buttonLayout->addWidget(m_cancelBtn);
    
    buttonLayout->addStretch();
    
    m_saveBtn = new QPushButton(m_editMode ? "Update Bowler" : "Add Bowler");
    m_saveBtn->setStyleSheet("QPushButton { "
                            "background-color: #4A90E2; "
                            "color: white; "
                            "border: none; "
                            "padding: 10px 20px; "
                            "border-radius: 3px; "
                            "font-size: 12px; "
                            "font-weight: bold; "
                            "} "
                            "QPushButton:hover { background-color: #357ABD; }");
    connect(m_saveBtn, &QPushButton::clicked, this, &NewBowlerDialog::onSaveClicked);
    buttonLayout->addWidget(m_saveBtn);
    
    m_mainLayout->addLayout(buttonLayout);
    
    // Set focus to first name field
    m_firstNameEdit->setFocus();
}

void NewBowlerDialog::populateFields(const BowlerInfo &bowler)
{
    m_firstNameEdit->setText(bowler.firstName);
    m_lastNameEdit->setText(bowler.lastName);
    
    if (bowler.sex.toLower() == "male" || bowler.sex.toLower() == "m") {
        m_maleRadio->setChecked(true);
    } else if (bowler.sex.toLower() == "female" || bowler.sex.toLower() == "f") {
        m_femaleRadio->setChecked(true);
    } else {
        m_otherRadio->setChecked(true);
    }
    
    m_averageSpin->setValue(bowler.average);
    m_birthdayEdit->setText(bowler.birthday);
    m_over18Check->setChecked(bowler.over18);
    m_phoneEdit->setText(bowler.phone);
    m_addressEdit->setText(bowler.address);
}

BowlerInfo NewBowlerDialog::getBowlerInfo() const
{
    BowlerInfo bowler;
    
    bowler.firstName = m_firstNameEdit->text().trimmed();
    bowler.lastName = m_lastNameEdit->text().trimmed();
    
    if (m_maleRadio->isChecked()) {
        bowler.sex = "Male";
    } else if (m_femaleRadio->isChecked()) {
        bowler.sex = "Female";
    } else {
        bowler.sex = "Other";
    }
    
    bowler.average = m_averageSpin->value();
    bowler.birthday = m_birthdayEdit->text().trimmed();
    bowler.over18 = m_over18Check->isChecked();
    bowler.phone = m_phoneEdit->text().trimmed();
    bowler.address = m_addressEdit->text().trimmed();
    
    return bowler;
}

void NewBowlerDialog::onSaveClicked()
{
    if (validateInput()) {
        accept();
    }
}

void NewBowlerDialog::onCancelClicked()
{
    reject();
}

bool NewBowlerDialog::validateInput()
{
    QString firstName = m_firstNameEdit->text().trimmed();
    QString lastName = m_lastNameEdit->text().trimmed();
    
    if (firstName.isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "First name is required.");
        m_firstNameEdit->setFocus();
        return false;
    }
    
    if (lastName.isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Last name is required.");
        m_lastNameEdit->setFocus();
        return false;
    }
    
    if (firstName.length() > 50) {
        QMessageBox::warning(this, "Invalid Input", "First name must be 50 characters or less.");
        m_firstNameEdit->setFocus();
        return false;
    }
    
    if (lastName.length() > 50) {
        QMessageBox::warning(this, "Invalid Input", "Last name must be 50 characters or less.");
        m_lastNameEdit->setFocus();
        return false;
    }
    
    // Validate birthday format if provided
    QString birthday = m_birthdayEdit->text().trimmed();
    if (!birthday.isEmpty()) {
        QRegularExpression dateRegex("^\\d{1,2}/\\d{1,2}/\\d{4}$");
        if (!dateRegex.match(birthday).hasMatch()) {
            QMessageBox::warning(this, "Invalid Input", "Birthday must be in MM/DD/YYYY format.");
            m_birthdayEdit->setFocus();
            return false;
        }
    }
    
    return true;
}