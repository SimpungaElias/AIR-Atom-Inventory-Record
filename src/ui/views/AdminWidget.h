#ifndef ADMINWIDGET_H
#define ADMINWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QProgressBar>
#include <QListWidget>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>
#include <QPushButton> // <--- THIS WAS MISSING

class AdminWidget : public QWidget {
    Q_OBJECT
public:
    explicit AdminWidget(QWidget *parent = nullptr);

private slots:
    void toggleAddForm();
    void validatePassword(const QString &pass);
    void saveUser();
    void deleteUser();

private:
    void setupUI();
    void setupListSection(QVBoxLayout *layout);
    void setupFormSection(QVBoxLayout *layout);
    void refreshList();
    bool checkPasswordRules(const QString &pass);

    QTableWidget *userTable;
    QGroupBox *formGroup;
    QPushButton *btnAdd;
    
    // Form Fields
    QLineEdit *txtUser, *txtFirst, *txtLast, *txtEmail, *txtPass;
    QComboBox *comboRole;
    QListWidget *listMBAs; 
    QProgressBar *passStrength;
    QLabel *lblStrengthMsg;
};

#endif // ADMINWIDGET_H
