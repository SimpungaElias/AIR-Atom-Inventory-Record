#include "ui/MainWindow.h"
#include "ui/dialogs/LoginDialog.h"
#include "ui/dialogs/AIR_SplashScreen.h"
#include "db/UserDatabaseManager.h"
#include <QApplication>
#include <QSettings>
#include <QDebug>

// --- MAC COLOR FIX ---
#include <QStyleFactory>
#include <QPalette>
// ---------------------

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    // App metadata — used by QSettings to persist splash screen preference
    a.setApplicationName("AIR");
    a.setApplicationVersion("1.0.0");
    a.setOrganizationName("AIR_System");

// =========================================================================
// MAC-ONLY FIX: Prevent Mac Dark Mode from turning the UI black.
// Windows and Linux will completely ignore this block.
// =========================================================================
#ifdef Q_OS_MAC
    a.setStyle(QStyleFactory::create("Fusion"));

    QPalette lightPalette;
    lightPalette.setColor(QPalette::Window, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::WindowText, Qt::black);
    lightPalette.setColor(QPalette::Base, Qt::white);
    lightPalette.setColor(QPalette::AlternateBase, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::ToolTipBase, Qt::white);
    lightPalette.setColor(QPalette::ToolTipText, Qt::black);
    lightPalette.setColor(QPalette::Text, Qt::black);
    lightPalette.setColor(QPalette::Button, QColor(240, 240, 240));
    lightPalette.setColor(QPalette::ButtonText, Qt::black);
    lightPalette.setColor(QPalette::BrightText, Qt::red);
    lightPalette.setColor(QPalette::Link, QColor(42, 130, 218));
    lightPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
    lightPalette.setColor(QPalette::HighlightedText, Qt::white);

    a.setPalette(lightPalette);
#endif
// =========================================================================

    // 1. INITIALIZE USER DATABASE
    // This creates the database file and the default admin user if missing.
    if (!UserDatabaseManager::instance().connect()) {
        qCritical() << "Could not connect to User Database. Exiting.";
        return -1;
    }

    // 2. SHOW SPLASH SCREEN (respects user's "don't show again" preference)
    QSettings settings;
    bool showSplash = settings.value("showSplash", true).toBool();

    if (showSplash) {
        AIRSplashScreen splash;
        int result = splash.exec();

        // Persist the checkbox state for next launch
        settings.setValue("showSplash", splash.shouldShowOnStartup());

        // If user clicked Exit on the splash screen, quit immediately
        if (result != QDialog::Accepted) {
            return 0;
        }
    }

    // 3. LOGIN → MAIN WINDOW LOOP
    while (true) {
        // Show Login Dialog
        LoginDialog login;
        if (login.exec() != QDialog::Accepted) {
            return 0; // User closed/cancelled login, exit app
        }

        // Show Main Window
        MainWindow w;
        w.setUserRole(login.getRole()); // Pass the role (Administrator/Operator)
        w.show();

        // Loop Logic for Logout
        bool logoutClicked = false;

        QObject::connect(&w, &MainWindow::logoutRequested, [&](){
            logoutClicked = true;
            w.close(); // Close window to break the a.exec() loop below
        });

        a.exec(); // Blocks here until the main window is closed

        if (!logoutClicked) {
            break; // User closed with 'X', exit completely
        }
        // If logoutClicked is true, loop repeats → new LoginDialog appears
    }

    return 0;
}
