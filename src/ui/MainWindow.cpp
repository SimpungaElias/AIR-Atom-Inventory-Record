#include "MainWindow.h"
#include "../db/DatabaseManager.h"

#include "views/HomeWidget.h"
#include "views/ReceiptWidget.h"
#include "views/GeneralLedgerWidget.h"
#include "views/LIIWidget.h"
#include "views/NLIWidget.h"
#include "views/AdminWidget.h"
#include "views/BackupRestoreWidget.h"
#include "views/TrainingWidget.h"
#include "views/MBRWidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QMenu>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    DatabaseManager::instance().connect(); 

    QFile file(":/resources/styles/main.qss");
    if(file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&file);
        qApp->setStyleSheet(stream.readAll());
        file.close();
    }

    setWindowTitle("AIR - Atom Inventory Record");
    resize(1280, 850);
    setupUI();
}

void MainWindow::setupUI() {
    QWidget *central = new QWidget;
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupHeader(mainLayout);

    // STACKED PAGES
    stack = new QStackedWidget();

    homeWidget = new HomeWidget();          stack->addWidget(homeWidget); // 0
    receiptWidget = new ReceiptWidget();    stack->addWidget(receiptWidget); // 1
    glWidget = new GeneralLedgerWidget();   stack->addWidget(glWidget); // 2
    adminWidget = new AdminWidget();        stack->addWidget(adminWidget); // 3
    backupWidget = new BackupRestoreWidget(); stack->addWidget(backupWidget); // 4
    liiWidget = new LIIWidget();            stack->addWidget(liiWidget); // 5
    nliWidget = new NLIWidget();            stack->addWidget(nliWidget); // 6
    trainingWidget = new TrainingWidget();  stack->addWidget(trainingWidget); // 7
    mbrWidget = new MBRWidget();            stack->addWidget(mbrWidget); // 8

    QWidget *body = new QWidget();
    QVBoxLayout *bodyLayout = new QVBoxLayout(body);
    bodyLayout->setContentsMargins(10, 10, 10, 10);
    bodyLayout->addWidget(stack);
    
    mainLayout->addWidget(body);

    // CONNECTIONS
    connect(receiptWidget, &ReceiptWidget::dataChanged, homeWidget, &HomeWidget::refreshData); 
    connect(receiptWidget, &ReceiptWidget::dataChanged, [this](){ glWidget->refreshData(); });
    connect(glWidget, &GeneralLedgerWidget::dataChanged, homeWidget, &HomeWidget::refreshData);
    connect(mbrWidget, &MBRWidget::dataChanged, homeWidget, &HomeWidget::refreshData);
    
    // --- SCENARIO START LOGIC ---
    connect(trainingWidget, &TrainingWidget::scenarioStarted, [this](QString name){
        btnQuitScenario->setVisible(true);
        btnQuitScenario->setText("Exit Simulation");

        switchView(0); 
        
        homeWidget->refreshData();
        glWidget->refreshData();
        receiptWidget->refreshTable();
        liiWidget->loadData();
        nliWidget->loadData();
        mbrWidget->loadData();
        
        setWindowTitle("AIR - TRAINING SIMULATOR");
        
        // Sim Mode Styling
        QWidget *nav = this->findChild<QWidget*>("NavContainer");
        if(nav) {
            nav->setStyleSheet(
                "QWidget#NavContainer { background-color: #f8f9fa; border-bottom: 1px solid #ccc; }"
                "QPushButton { color: white; font-weight: bold; }"
                "QPushButton:hover { background-color: #d35400; }"
                "QPushButton[active='true'] { background-color: #a04000; color: white; border: 1px solid white; }"
            );
        }
    });
} 

void MainWindow::setUserRole(const QString &role) {
    currentUserRole = role;
    QWidget *central = centralWidget();
    QWidget *navBox = central->findChild<QWidget*>("NavContainer");
    if(navBox) {
        QList<QPushButton*> btns = navBox->findChildren<QPushButton*>();
        for(QPushButton *btn : btns) {
            if(btn->text() == "Administration") {
                btn->setVisible(currentUserRole == "Administrator");
            }
        }
    }
}

void MainWindow::setupHeader(QVBoxLayout *layout) {
    // --- 1. BRAND TOP BAR ---
    QWidget *brandBox = new QWidget();
    brandBox->setObjectName("BrandingContainer");
    brandBox->setStyleSheet("background-color: white; border-bottom: 1px solid #ddd;");
    QHBoxLayout *brandLay = new QHBoxLayout(brandBox);
    brandLay->setContentsMargins(20, 10, 20, 10);
    
    // LEFT: Logo & Title
    QLabel *logo = new QLabel("AIR"); 
    logo->setObjectName("LogoLabel");
    logo->setStyleSheet("font-size: 25px; font-weight: bold; color: #003366; margin-right: 10px; border: none; text-decoration: none;");
    
    QLabel *sub = new QLabel("Atom Inventory Record"); 
    sub->setObjectName("SubLabel");
    sub->setStyleSheet("font-size: 17px; font-weight: bold; color: #555; border: none; text-decoration: none;");
    
    brandLay->addWidget(logo);
    brandLay->addWidget(sub); 
    brandLay->addStretch();

    // LOGOUT
    btnLogout = new QPushButton("Logout");
    btnLogout->setCursor(Qt::PointingHandCursor);
    btnLogout->setStyleSheet("background-color: transparent; color: #c0392b; font-weight: bold; border: none;");
    connect(btnLogout, &QPushButton::clicked, this, &MainWindow::logout);
    brandLay->addWidget(btnLogout);
    
    // --- 2. NAVIGATION BAR ---
    QWidget *navBox = new QWidget();
    navBox->setObjectName("NavContainer");
    navBox->setStyleSheet("background-color: #f8f9fa; border-bottom: 1px solid #ccc;");
    QHBoxLayout *navLay = new QHBoxLayout(navBox);
    navLay->setContentsMargins(20, 5, 20, 5); 
    navLay->setSpacing(0); 

    // Helper to Create Standard Nav Buttons
    auto createNavBtn = [this](QString text, bool active = false) -> QPushButton* {
        QPushButton *btn = new QPushButton(text);
        btn->setProperty("class", "NavButton");
        btn->setFlat(true);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(
            "QPushButton { padding: 8px 15px; font-weight: bold; color: #333; border: none; border-radius: 4px; }"
            "QPushButton:hover { background-color: #e2e6ea; }"
            "QPushButton[active='true'] { background-color: #003366; color: white; }"
        );
        if(active) btn->setProperty("active", true);
        return btn;
    };

    // Helper to Update Active State
    auto updateActiveBtn = [navBox, this](QPushButton *target){
        QList<QPushButton*> all = navBox->findChildren<QPushButton*>();
        for(QPushButton *b : all) {
            if(b == btnQuitScenario) continue; 
            b->setProperty("active", (b == target));
            b->style()->unpolish(b); b->style()->polish(b);
        }
    };

    // 1. HOME (Far Left)
    QPushButton *btnHome = createNavBtn("Home", true);
    connect(btnHome, &QPushButton::clicked, [this, btnHome, updateActiveBtn](){ 
        switchView(0);
        updateActiveBtn(btnHome);
    });
    navLay->addWidget(btnHome);

    // --- SPACER 1 ---
    navLay->addStretch();

    // 2. OPERATIONS (Middle-Left)
    QPushButton *btnOps = createNavBtn("Operations");
    QMenu *opsMenu = new QMenu(btnOps);
    
    // --- FIX: Added MBR to the very top of the menu, linking to index 8 ---
    opsMenu->addAction("Material Balance Report (MBR)", [this, btnOps, updateActiveBtn](){ switchView(8); updateActiveBtn(btnOps); });
    opsMenu->addAction("Inventory Change Report (ICR)", [this, btnOps, updateActiveBtn](){ switchView(1); receiptWidget->refreshTable(); updateActiveBtn(btnOps); });
    opsMenu->addAction("List of Inventory Items (LII)", [this, btnOps, updateActiveBtn](){ switchView(5); updateActiveBtn(btnOps); });
    opsMenu->addAction("Nuclear Loss Items (NLI)", [this, btnOps, updateActiveBtn](){ switchView(6); updateActiveBtn(btnOps); });
    opsMenu->addAction("General Ledger (GL)", [this, btnOps, updateActiveBtn](){ switchView(2); glWidget->refreshData(); updateActiveBtn(btnOps); });
    
    btnOps->setMenu(opsMenu);
    opsMenu->setStyleSheet("QMenu { background-color: #003366; color: white; } QMenu::item { padding: 8px 20px; } QMenu::item:selected { background-color: #002244; }");
    navLay->addWidget(btnOps);

    // --- SPACER 2 ---
    navLay->addStretch();

    // 3. TRAINING SIMULATOR (Middle-Right)
    QPushButton *btnTrain = createNavBtn("Training Simulator");
    connect(btnTrain, &QPushButton::clicked, [this, btnTrain, updateActiveBtn](){
        switchView(7);
        updateActiveBtn(btnTrain);
    });
    navLay->addWidget(btnTrain);

    // --- SPACER 3 ---
    navLay->addStretch();

    // 4. ADMINISTRATION (Far Right)
    QPushButton *btnAdmin = createNavBtn("Administration");
    QMenu *adminMenu = new QMenu(btnAdmin);
    adminMenu->addAction("User Management", [this, btnAdmin, updateActiveBtn](){ switchView(3); updateActiveBtn(btnAdmin); });
    adminMenu->addAction("Backup / Restore", [this, btnAdmin, updateActiveBtn](){ switchView(4); updateActiveBtn(btnAdmin); });
    btnAdmin->setMenu(adminMenu);
    adminMenu->setStyleSheet("QMenu { background-color: #003366; color: white; } QMenu::item { padding: 8px 20px; } QMenu::item:selected { background-color: #002244; }");
    navLay->addWidget(btnAdmin);

    // 5. QUIT SCENARIO BUTTON (Hidden margin logic)
    btnQuitScenario = new QPushButton("Exit Simulation");
    btnQuitScenario->setVisible(false); 
    btnQuitScenario->setCursor(Qt::PointingHandCursor);
    btnQuitScenario->setStyleSheet("background-color: #c0392b; color: white; font-weight: bold; border-radius: 4px; padding: 5px 15px; margin-left: 10px;");
    connect(btnQuitScenario, &QPushButton::clicked, this, &MainWindow::quitScenario);
    navLay->addWidget(btnQuitScenario); 

    layout->addWidget(brandBox);
    layout->addWidget(navBox);
}

void MainWindow::switchView(int index) {
    stack->setCurrentIndex(index);
}

void MainWindow::quitScenario() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Exit Simulation", 
                                  "Are you sure you want to quit the training scenario?\nUnsaved progress in the scenario will be lost.",
                                  QMessageBox::Yes|QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        // 1. SWITCH DATABASE ENGINE FIRST
        DatabaseManager::instance().resetToRealDatabase();

        // 2. UI RESET
        setWindowTitle("AIR - Atom Inventory Record");
        btnQuitScenario->setVisible(false);
        
        // Reset Header Style
        QWidget *nav = this->findChild<QWidget*>("NavContainer");
        if(nav) {
            nav->setStyleSheet(
                "QWidget#NavContainer { background-color: #f8f9fa; border-bottom: 1px solid #ccc; }"
                "QPushButton { color: #333; font-weight: bold; border: none; border-radius: 4px; padding: 8px 15px; }"
                "QPushButton:hover { background-color: #e2e6ea; }"
                "QPushButton[active='true'] { background-color: #003366; color: white; }"
            );
        }

        // 3. FORCE REFRESH OF ALL WIDGETS
        homeWidget->refreshData();    // Reloads GL Summary on Home
        glWidget->refreshData();      // Reloads detailed GL
        receiptWidget->refreshTable(); // Reloads Receipts table
        liiWidget->loadData();        // Reloads LII
        nliWidget->loadData();
        mbrWidget->loadData();
        
        // 4. Navigate Home
        switchView(0);
        
        QMessageBox::information(this, "Standard Mode", "Operational data restored.");
    }
}

void MainWindow::logout() {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Logout", "Are you sure you want to logout?",
                                  QMessageBox::Yes|QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        if(btnQuitScenario->isVisible()) {
            DatabaseManager::instance().resetToRealDatabase();
        }
        this->hide();
        emit logoutRequested();
    }
}
