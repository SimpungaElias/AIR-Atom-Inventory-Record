TEMPLATE = app
TARGET = AIR
QT += core gui widgets sql printsupport

CONFIG += c++17

# Source Directories
INCLUDEPATH += src \
               src/db \
               src/ui \
               src/ui/views \
               src/ui/dialogs

# Input Files
HEADERS += \
    src/db/DatabaseManager.h \
    src/ui/MainWindow.h \
    src/ui/views/HomeWidget.h \
    src/ui/views/ReceiptWidget.h \
    src/utils/ReportGenerator.h \
    src/ui/views/NLIWidget.h \
    src/ui/views/TrainingWidget.h \
    src/ui/views/GeneralLedgerWidget.h \
    src/db/UserDatabaseManager.h \
    src/ui/dialogs/LoginDialog.h \
    src/ui/views/MaterialCodeDialog.h \
    src/ui/views/AdminWidget.h \
    src/ui/views/BackupRestoreWidget.h \
    src/ui/views/LIIWidget.h \
    src/ui/views/MBRWidget.h \
    src/ui/views/PinDialog.h \
    
    

SOURCES += \
    src/main.cpp \
    src/db/DatabaseManager.cpp \
    src/ui/MainWindow.cpp \
    src/ui/views/HomeWidget.cpp \
    src/ui/views/ReceiptWidget.cpp \
    src/utils/ReportGenerator.cpp \
    src/ui/views/NLIWidget.cpp \
    src/ui/views/TrainingWidget.cpp \
    src/ui/views/GeneralLedgerWidget.cpp \
    src/db/UserDatabaseManager.cpp \
    src/ui/dialogs/LoginDialog.cpp \
    src/ui/views/AdminWidget.cpp \
    src/ui/views/BackupRestoreWidget.cpp \
    src/ui/views/LIIWidget.cpp \
    src/ui/views/MBRWidget.cpp \
    

RESOURCES += resources.qrc

# Output Setup
DESTDIR = bin
OBJECTS_DIR = build/obj
MOC_DIR = build/moc
