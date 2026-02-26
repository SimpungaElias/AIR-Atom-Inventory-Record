#include "ui/MainWindow.h"
#include "src/ui/dialogs/LoginDialog.h" 
#include "db/UserDatabaseManager.h" // <--- CRITICAL INCLUDE
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // 1. INITIALIZE USER DATABASE
    // This creates the database file and the default admin user if missing.
    if (!UserDatabaseManager::instance().connect()) {
        qCritical() << "Could not connect to User Database. Exiting.";
        return -1;
    }

    while (true) {
        // 2. Show Login Dialog
        LoginDialog login;
        if (login.exec() != QDialog::Accepted) {
            return 0; // User closed/cancelled login, exit app
        }

        // 3. Show Main Window
        MainWindow w;
        w.setUserRole(login.getRole()); // Pass the role (Administrator/Operator)
        w.show();

        // 4. Loop Logic for Logout
        bool logoutClicked = false;
        
        // Connect the signal from MainWindow to our local boolean
        QObject::connect(&w, &MainWindow::logoutRequested, [&](){
            logoutClicked = true;
            w.close(); // Close window to break the a.exec() loop below
        });

        a.exec(); // Blocks here until the main window is closed

        if (!logoutClicked) {
            break; // User closed with 'X', so break the loop and exit completely
        }
        // If logoutClicked is true, the loop repeats -> New LoginDialog appears
    }

    return 0;
}
