#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>

class LoginDialog : public QDialog {
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    
    // Existing getter
    QString getUserRole() const { return userRole; }
    
    // --- ADDED: The function main.cpp is looking for ---
    QString getRole() const { return userRole; } 

    QStringList getAssignedMBAs() const { return assignedMBAs; }

private slots:
    void attemptLogin();

private:
    void setupUI();
    void generateCaptcha();
    
    QLineEdit *txtUser;
    QLineEdit *txtPass;
    QLineEdit *txtCaptcha;
    QLabel *lblCaptchaDisplay;
    QLabel *lblError;
    QWidget *captchaContainer;

    int failedAttempts;
    QString currentCaptcha;
    
    // Result Data
    QString userRole;
    QStringList assignedMBAs;
};

#endif // LOGINDIALOG_H
