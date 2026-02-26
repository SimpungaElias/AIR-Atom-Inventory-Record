#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QMap>
#include <QList>
#include <QDebug>
#include <QStringList>

class DatabaseManager {
public:
    static DatabaseManager& instance();
    bool connect();
    
    // Manual Ledger
    bool addManualLedgerEntry(const QMap<QString, QVariant> &data);
    QSqlQuery getManualLedgerEntries();
    
    // Receipt
    bool registerReceipt(const QMap<QString, QVariant> &data);
    QSqlQuery getReceipts();
    // Backup / Restore
    bool createBackup(const QString &title, const QString &description);
    bool restoreBackup(int backupId);
    QSqlQuery getBackups();
    bool deleteBackup(int backupId);

    // --- Reporting ---
    QSqlQuery getICRData(const QString &mba, const QString &startDate, const QString &endDate);
    QSqlQuery getLIIData(const QString &mba, const QString &date);
    
    // General Ledger
    QSqlQuery getGeneralLedgerData(const QString &mba, const QString &elementFilter);
    
    // LII Manual Entries
    bool addLIIEntry(const QMap<QString, QVariant> &data);
    QSqlQuery getLIIEntries();
    
    // NLI Manual Entries
    bool addNLIEntry(const QMap<QString, QVariant> &data);
    QSqlQuery getNLIEntries();
    
    // MBR (Material Balance Report)
    bool addMBREntry(const QMap<QString, QVariant> &data);
    QSqlQuery getMBREntries(int limit = 0); // 0 means all, >0 limits rows for Home screen
    
    // New functions for Training Mode
    void connectToScenario(const QString &scenarioName); // Connects to a specific scenario DB
    void resetToRealDatabase(); // Reconnects to the main operational DB
    QString currentDatabaseName() const; // Helper to see which DB is active
    void injectScenarioData(const QString &scenarioName); // <--- ADD THIS
    
    bool deleteReceipt(int id);
    bool deleteLIIEntry(int id);
    bool deleteNLIEntry(int id);
    bool deleteManualLedgerEntry(int id);
    bool deleteMBREntry(int id);

private:
    DatabaseManager() {} // Singleton
    void initTables();
    QSqlDatabase db;
};

#endif // DATABASEMANAGER_H
