#include "LoginDialog.h"
// Ensure this path matches your project structure. 
// If you merged logic into the main DB manager, change to "DatabaseManager.h"
#include "../../db/UserDatabaseManager.h" 

#include <QVBoxLayout>
#include <QGridLayout>
#include <QRandomGenerator>
#include <QApplication>
#include <QStyle>

LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent), failedAttempts(0) {
    setWindowTitle("AIR - System Login");
    setModal(true);
    
    // Set a reasonable fixed width, let height grow dynamically
    setFixedWidth(400); 
    setupUI();
}

void LoginDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(30, 30, 30, 30);
    
    // 1. Header
    QLabel *header = new QLabel("<h2>AIR System Login</h2>");
    header->setAlignment(Qt::AlignCenter);
    header->setStyleSheet("color: #003366;"); // Brand color
    mainLayout->addWidget(header);

    // 2. Error Label
    lblError = new QLabel("");
    lblError->setStyleSheet("color: #c0392b; font-weight: bold; font-size: 10pt;");
    lblError->setAlignment(Qt::AlignCenter);
    lblError->setWordWrap(true);
    mainLayout->addWidget(lblError);

    // 3. Input Fields
    QGridLayout *form = new QGridLayout;
    form->setSpacing(10);

    txtUser = new QLineEdit; 
    txtUser->setPlaceholderText("Enter Username");
    txtUser->setMinimumHeight(30);

    txtPass = new QLineEdit; 
    txtPass->setPlaceholderText("Enter Password");
    txtPass->setEchoMode(QLineEdit::Password);
    txtPass->setMinimumHeight(30);
    
    form->addWidget(new QLabel("Username:"), 0, 0); 
    form->addWidget(txtUser, 0, 1);
    
    form->addWidget(new QLabel("Password:"), 1, 0); 
    form->addWidget(txtPass, 1, 1);

    mainLayout->addLayout(form);

    // 4. CAPTCHA SECTION (Initially Hidden)
    captchaContainer = new QWidget;
    QVBoxLayout *capLay = new QVBoxLayout(captchaContainer);
    capLay->setContentsMargins(0, 10, 0, 0);
    
    lblCaptchaDisplay = new QLabel("ABCD");
    lblCaptchaDisplay->setStyleSheet(
        "font-family: 'Comic Sans MS', 'Arial', sans-serif;" // Added font fallback
        "font-size: 24px; font-weight: bold; letter-spacing: 6px;"
        "color: #444; background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #f3f3f3, stop:1 #e6e6e6);"
        "border: 1px solid #bbb; border-radius: 4px; padding: 10px;"
    );
    lblCaptchaDisplay->setAlignment(Qt::AlignCenter);
    
    txtCaptcha = new QLineEdit;
    txtCaptcha->setPlaceholderText("Enter security code");
    txtCaptcha->setMinimumHeight(30);
    
    capLay->addWidget(new QLabel("<b>Security Check Required:</b>"));
    capLay->addWidget(lblCaptchaDisplay);
    capLay->addWidget(txtCaptcha);
    
    captchaContainer->setVisible(false); 
    mainLayout->addWidget(captchaContainer);

    // 5. Login Button
    mainLayout->addStretch();
    
    QPushButton *btnLogin = new QPushButton("Login");
    btnLogin->setCursor(Qt::PointingHandCursor);
    btnLogin->setStyleSheet(
        "QPushButton { background-color: #0056b3; color: white; padding: 10px; font-weight: bold; border-radius: 4px; font-size: 11pt; }"
        "QPushButton:hover { background-color: #004494; }"
    );
    connect(btnLogin, &QPushButton::clicked, this, &LoginDialog::attemptLogin);
    
    // Allow pressing "Enter" to login
    connect(txtPass, &QLineEdit::returnPressed, btnLogin, &QPushButton::click);
    connect(txtCaptcha, &QLineEdit::returnPressed, btnLogin, &QPushButton::click);
    
    mainLayout->addWidget(btnLogin);
}

void LoginDialog::generateCaptcha() {
    const QString chars = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789"; 
    currentCaptcha.clear();
    for(int i=0; i<5; i++) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        currentCaptcha.append(chars.at(index));
    }
    lblCaptchaDisplay->setText(currentCaptcha); 
    txtCaptcha->clear();
}

void LoginDialog::attemptLogin() {
    lblError->setText(""); // Clear previous errors

    // 1. Robot / Captcha Check
    if(failedAttempts >= 3) {
        if(txtCaptcha->text().toUpper() != currentCaptcha) {
            lblError->setText("Incorrect Security Code. Please try again.");
            generateCaptcha();
            txtCaptcha->setFocus();
            return;
        }
    }

    // 2. Authentication Check
    // Ensure UserDatabaseManager exists, or change to DatabaseManager::instance()
    if(UserDatabaseManager::instance().authenticateUser(txtUser->text(), txtPass->text(), userRole, assignedMBAs)) {
        accept(); // Closes dialog with QDialog::Accepted
    } else {
        failedAttempts++;
        lblError->setText("Invalid Username or Password.");
        txtPass->clear();
        
        // Trigger Robot Protection if needed
        if(failedAttempts >= 3) {
            if (!captchaContainer->isVisible()) {
                captchaContainer->setVisible(true);
                generateCaptcha();
                lblError->setText("Security check required due to multiple failed attempts.");
                
                // Smoothly adjust size to fit captcha
                adjustSize(); 
            } else {
                generateCaptcha(); // Refresh captcha on subsequent fails
            }
        }
    }
}
