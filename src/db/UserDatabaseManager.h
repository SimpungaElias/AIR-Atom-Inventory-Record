#ifndef USERDATABASEMANAGER_H
#define USERDATABASEMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QStringList>
#include <QMap>
#include <QVariant>
#include <QDebug>

class UserDatabaseManager {
public:
    static UserDatabaseManager& instance();
    bool connect();
    
    // Auth
    bool authenticateUser(const QString &username, const QString &password, QString &role, QStringList &assignedMBAs);
    
    // CRUD
    bool createUser(const QMap<QString, QVariant> &data, const QStringList &mbas);
    QSqlQuery getAllUsers();
    bool deleteUser(int userId);
    QStringList getUserMBAs(int userId);

private:
    UserDatabaseManager() {}
    void initTables();
    QSqlDatabase db;
};

#endif // USERDATABASEMANAGER_H
