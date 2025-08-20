#ifndef NEWBOWLERDIALOG_H
#define NEWBOWLERDIALOG_H

#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QFrame>
#include <QScrollArea>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QRegularExpression>

// Simple BowlerInfo structure for this dialog
struct BowlerInfo {
    int id = 0;
    QString firstName;
    QString lastName;
    QString sex;
    int average = 0;
    QString birthday;
    bool over18 = true;
    QString phone;
    QString address;
    QStringList teams;
};

class NewBowlerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewBowlerDialog(QWidget *parent = nullptr);
    explicit NewBowlerDialog(const BowlerInfo &bowler, QWidget *parent = nullptr);
    
    BowlerInfo getBowlerInfo() const;

private slots:
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUI();
    void populateFields(const BowlerInfo &bowler);
    bool validateInput();
    
    bool m_editMode;
    
    QVBoxLayout *m_mainLayout;
    QScrollArea *m_scrollArea;
    QFrame *m_formFrame;
    QGridLayout *m_formLayout;
    
    // Form fields
    QLineEdit *m_firstNameEdit;
    QLineEdit *m_lastNameEdit;
    QButtonGroup *m_sexGroup;
    QRadioButton *m_maleRadio;
    QRadioButton *m_femaleRadio;
    QRadioButton *m_otherRadio;
    QSpinBox *m_averageSpin;
    QLineEdit *m_birthdayEdit;
    QCheckBox *m_over18Check;
    QLineEdit *m_phoneEdit;
    QLineEdit *m_addressEdit;
    
    QPushButton *m_saveBtn;
    QPushButton *m_cancelBtn;
};

#endif // NEWBOWLERDIALOG_H