#include "AdminWidget.h"
#include "../../db/UserDatabaseManager.h"
#include "../../db/DatabaseManager.h" // Needed to fetch available MBAs from Inventory DB
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QRegularExpression>
#include <QMessageBox>
#include <QLabel>
#include <QGridLayout>

AdminWidget::AdminWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
    refreshList();
}

void AdminWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10,10,10,10);

    // Header
    QLabel *title = new QLabel("<h2>User Administration</h2>");
    mainLayout->addWidget(title);

    // Add Button
    btnAdd = new QPushButton("+ Add new record");
    btnAdd->setStyleSheet("background-color: #ecf0f1; border: 1px solid #ccc; padding: 6px; font-weight: bold; color: #333;");
    btnAdd->setCursor(Qt::PointingHandCursor);
    connect(btnAdd, &QPushButton::clicked, this, &AdminWidget::toggleAddForm);
    mainLayout->addWidget(btnAdd);

    // 1. List
    setupListSection(mainLayout);

    // 2. Form (Hidden by default)
    setupFormSection(mainLayout);
}

void AdminWidget::setupListSection(QVBoxLayout *layout) {
    userTable = new QTableWidget();
    userTable->setColumnCount(6);
    userTable->setHorizontalHeaderLabels({"ID", "Username", "Firstname", "Lastname", "Email", "Role"});
    userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    userTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    userTable->setAlternatingRowColors(true);
    
    QPushButton *btnDel = new QPushButton("Delete Selected");
    btnDel->setStyleSheet("background-color: #c0392b; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px; border: none;");
    connect(btnDel, &QPushButton::clicked, this, &AdminWidget::deleteUser);
    
    layout->addWidget(userTable);
    layout->addWidget(btnDel);
}

void AdminWidget::setupFormSection(QVBoxLayout *layout) {
    formGroup = new QGroupBox("User Details");
    formGroup->setVisible(false);
    formGroup->setStyleSheet("background-color: #f9f9f9; border: 1px solid #ccc; margin-top: 10px; padding: 10px;");
    
    QGridLayout *grid = new QGridLayout(formGroup);
    grid->setSpacing(10);

    // Basic Info
    txtUser = new QLineEdit; grid->addWidget(new QLabel("Username:*"), 0, 0); grid->addWidget(txtUser, 0, 1);
    
    comboRole = new QComboBox; 
    comboRole->addItems({"Administrator", "Read-Write", "Read-Only"});
    grid->addWidget(new QLabel("Rights:"), 0, 2); grid->addWidget(comboRole, 0, 3);

    txtFirst = new QLineEdit; grid->addWidget(new QLabel("Firstname:"), 1, 0); grid->addWidget(txtFirst, 1, 1);
    txtLast = new QLineEdit; grid->addWidget(new QLabel("Lastname:"), 1, 2); grid->addWidget(txtLast, 1, 3);
    
    txtEmail = new QLineEdit; grid->addWidget(new QLabel("Email:"), 2, 0); grid->addWidget(txtEmail, 2, 1);
    
    // Password with Strength Meter
    txtPass = new QLineEdit; txtPass->setEchoMode(QLineEdit::Password);
    connect(txtPass, &QLineEdit::textChanged, this, &AdminWidget::validatePassword);
    
    grid->addWidget(new QLabel("Password:*"), 3, 0); 
    grid->addWidget(txtPass, 3, 1);

    passStrength = new QProgressBar; 
    passStrength->setRange(0, 100); 
    passStrength->setValue(0);
    passStrength->setTextVisible(false);
    passStrength->setFixedHeight(5);
    
    lblStrengthMsg = new QLabel("Requirements: 6+ chars, 1 Upper, 1 Number, 1 Symbol");
    lblStrengthMsg->setStyleSheet("font-size: 9px; color: #666;");
    
    QVBoxLayout *passLay = new QVBoxLayout;
    passLay->setSpacing(2);
    passLay->addWidget(passStrength);
    passLay->addWidget(lblStrengthMsg);
    
    grid->addLayout(passLay, 3, 2, 1, 2);

    // MBA Assignment (Multi-select)
    listMBAs = new QListWidget;
    listMBAs->setSelectionMode(QAbstractItemView::MultiSelection);
    // Fetch from Inventory DB
    //listMBAs->addItems(DatabaseManager::instance().getMBANames());
    
    grid->addWidget(new QLabel("Assign MBA(s):"), 4, 0);
    grid->addWidget(listMBAs, 4, 1, 1, 3);

    // Save Button
    QPushButton *btnSave = new QPushButton("Save Record");
    btnSave->setStyleSheet("background-color: #27ae60; color: white; font-weight: bold; padding: 8px;");
    connect(btnSave, &QPushButton::clicked, this, &AdminWidget::saveUser);
    
    grid->addWidget(btnSave, 5, 0, 1, 4, Qt::AlignRight);
    
    layout->addWidget(formGroup);
}

void AdminWidget::toggleAddForm() {
    formGroup->setVisible(!formGroup->isVisible());
}

void AdminWidget::validatePassword(const QString &pass) {
    int score = 0;
    if(pass.length() >= 6) score += 25;
    if(pass.contains(QRegularExpression("[A-Z]"))) score += 25;
    if(pass.contains(QRegularExpression("[0-9]"))) score += 25;
    if(pass.contains(QRegularExpression("[^a-zA-Z0-9]"))) score += 25;

    passStrength->setValue(score);
    
    // Color coding
    QString color = "red";
    if(score >= 50) color = "orange";
    if(score == 100) color = "#27ae60"; // Green

    passStrength->setStyleSheet(QString("QProgressBar::chunk { background-color: %1; border-radius: 2px; }").arg(color));
}

bool AdminWidget::checkPasswordRules(const QString &pass) {
    // 6 chars, 1 Upper, 1 Number, 1 Symbol
    if(pass.length() < 6) return false;
    if(!pass.contains(QRegularExpression("[A-Z]"))) return false;
    if(!pass.contains(QRegularExpression("[0-9]"))) return false;
    if(!pass.contains(QRegularExpression("[^a-zA-Z0-9]"))) return false;
    return true;
}

void AdminWidget::saveUser() {
    if(txtUser->text().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Username is required.");
        return;
    }

    if(!checkPasswordRules(txtPass->text())) {
        QMessageBox::warning(this, "Weak Password", 
            "Password does not meet requirements:\n"
            "- At least 6 characters\n"
            "- At least 1 Number\n"
            "- At least 1 Symbol\n"
            "- At least 1 Uppercase Letter");
        return;
    }

    QMap<QString, QVariant> data;
    data["username"] = txtUser->text();
    data["password"] = txtPass->text(); 
    data["firstname"] = txtFirst->text();
    data["lastname"] = txtLast->text();
    data["email"] = txtEmail->text();
    data["role"] = comboRole->currentText();

    QStringList mbas;
    for(auto item : listMBAs->selectedItems()) {
        mbas << item->text();
    }

    if(UserDatabaseManager::instance().createUser(data, mbas)) {
        QMessageBox::information(this, "Success", "User Created Successfully");
        txtUser->clear(); txtPass->clear(); txtFirst->clear(); txtLast->clear(); txtEmail->clear();
        formGroup->setVisible(false);
        refreshList();
    } else {
        QMessageBox::critical(this, "Error", "Failed to create user. Username might already exist.");
    }
}

void AdminWidget::refreshList() {
    userTable->setRowCount(0);
    QSqlQuery q = UserDatabaseManager::instance().getAllUsers();
    while(q.next()) {
        int row = userTable->rowCount();
        userTable->insertRow(row);
        userTable->setItem(row, 0, new QTableWidgetItem(q.value("id").toString()));
        userTable->setItem(row, 1, new QTableWidgetItem(q.value("username").toString()));
        userTable->setItem(row, 2, new QTableWidgetItem(q.value("firstname").toString()));
        userTable->setItem(row, 3, new QTableWidgetItem(q.value("lastname").toString()));
        userTable->setItem(row, 4, new QTableWidgetItem(q.value("email").toString()));
        userTable->setItem(row, 5, new QTableWidgetItem(q.value("role").toString()));
    }
}

void AdminWidget::deleteUser() {
    int row = userTable->currentRow();
    if(row < 0) return;
    
    QString username = userTable->item(row, 1)->text();
    if(username == "admin") {
        QMessageBox::warning(this, "Action Denied", "Cannot delete the default Administrator.");
        return;
    }
    
    int id = userTable->item(row, 0)->text().toInt();
    if(QMessageBox::question(this, "Confirm", "Delete user " + username + "?") == QMessageBox::Yes) {
        if(UserDatabaseManager::instance().deleteUser(id)) {
            refreshList();
        }
    }
}
