#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QPushButton> // <--- ADDED: Required for the new buttons
#include "views/HomeWidget.h"
#include "views/AdminWidget.h"
#include "views/BackupRestoreWidget.h"
#include "views/TrainingWidget.h"

// Forward declaration
class ReceiptWidget;
class GeneralLedgerWidget;
class LIIWidget;
class NLIWidget;
class MBRWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void setUserRole(const QString &role); 

signals:
    void logoutRequested(); // <--- ADDED: Signal to tell main.cpp to logout

private slots:
    void switchView(int index);
    void quitScenario(); // <--- ADDED: Slot to exit training mode
    void logout();       // <--- ADDED: Slot to handle logout logic

private:
    void setupUI();                         
    void setupHeader(QVBoxLayout *layout);  
    
    QString currentUserRole;
    
    QStackedWidget *stack;

    // Header Buttons (Dynamic Access)
    QPushButton *btnQuitScenario; // <--- ADDED
    QPushButton *btnLogout;       // <--- ADDED
    
    // Widgets
    HomeWidget *homeWidget;
    AdminWidget *adminWidget; 
    ReceiptWidget *receiptWidget;
    GeneralLedgerWidget *glWidget;
    BackupRestoreWidget *backupWidget;
    LIIWidget *liiWidget; 
    NLIWidget *nliWidget;
    TrainingWidget *trainingWidget;
    MBRWidget *mbrWidget; 
};

#endif // MAINWINDOW_H
