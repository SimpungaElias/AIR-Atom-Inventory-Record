#include "UserDatabaseManager.h"
#include <QStandardPaths>
#include <QDir>

UserDatabaseManager& UserDatabaseManager::instance() {
    static UserDatabaseManager _instance;
    return _instance;
}

bool UserDatabaseManager::connect() {
    // We use a unique connection name "UserDBConnection" to avoid conflict with the main DB
    if (QSqlDatabase::contains("UserDBConnection")) {
        db = QSqlDatabase::database("UserDBConnection");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", "UserDBConnection");
        
        // --- THE MAC FIX: Save users DB to the Documents folder ---
        QString dbPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/air_users.db";
        db.setDatabaseName(dbPath); 
        // ----------------------------------------------------------
    }

    if (!db.open()) {
        qCritical() << "User DB Connection Error:" << db.lastError().text();
        return false;
    }
    initTables();
    return true;
}

void UserDatabaseManager::initTables() {
    QSqlQuery query(db);
    
    // 1. Create Users Table
    query.exec("CREATE TABLE IF NOT EXISTS users ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "username TEXT UNIQUE, password TEXT, "
               "firstname TEXT, lastname TEXT, email TEXT, "
               "role TEXT, phone TEXT, description TEXT, "
               "security_q TEXT, security_a TEXT)");

    // 2. Create User-MBA Link Table
    query.exec("CREATE TABLE IF NOT EXISTS user_mbas ("
               "user_id INTEGER, mba_code TEXT, "
               "FOREIGN KEY(user_id) REFERENCES users(id))");

    // 3. Create Default Admin (THE FIX)
    query.exec("SELECT id FROM users WHERE username='admin'");
    if (!query.next()) { // If query.next() is false, the user does NOT exist
        query.prepare("INSERT INTO users (username, password, role, firstname, lastname) "
                      "VALUES (:u, :p, :r, :fn, :ln)");
        query.bindValue(":u", "admin");
        query.bindValue(":p", "Admin@123");
        query.bindValue(":r", "Administrator");
        query.bindValue(":fn", "System");
        query.bindValue(":ln", "Admin");
        
        if(query.exec()) {
            qDebug() << "Default Admin created successfully.";
        } else {
            qCritical() << "Failed to create Admin:" << query.lastError().text();
        }
    }
}

bool UserDatabaseManager::authenticateUser(const QString &username, const QString &password, QString &role, QStringList &assignedMBAs) {
    QSqlQuery query(db);
    query.prepare("SELECT id, role FROM users WHERE username = ? AND password = ?");
    query.addBindValue(username);
    query.addBindValue(password);
    
    if(query.exec() && query.next()) {
        int id = query.value(0).toInt();
        role = query.value(1).toString();
        
        // Fetch assigned MBAs
        QSqlQuery mbaQ(db);
        mbaQ.prepare("SELECT mba_code FROM user_mbas WHERE user_id = ?");
        mbaQ.addBindValue(id);
        mbaQ.exec();
        while(mbaQ.next()) assignedMBAs << mbaQ.value(0).toString();
        
        return true;
    }
    return false;
}

bool UserDatabaseManager::createUser(const QMap<QString, QVariant> &data, const QStringList &mbas) {
    db.transaction();
    QSqlQuery query(db);
    query.prepare("INSERT INTO users (username, password, firstname, lastname, email, role, phone, description, security_q, security_a) "
                  "VALUES (:u, :p, :fn, :ln, :em, :r, :ph, :desc, :sq, :sa)");
    
    query.bindValue(":u", data["username"]);
    query.bindValue(":p", data["password"]);
    query.bindValue(":fn", data["firstname"]);
    query.bindValue(":ln", data["lastname"]);
    query.bindValue(":em", data["email"]);
    query.bindValue(":r", data["role"]);
    query.bindValue(":ph", data["phone"]);
    query.bindValue(":desc", data["description"]);
    query.bindValue(":sq", data["security_q"]);
    query.bindValue(":sa", data["security_a"]);

    if(!query.exec()) { db.rollback(); return false; }
    
    int uid = query.lastInsertId().toInt();
    QSqlQuery mbaQ(db);
    mbaQ.prepare("INSERT INTO user_mbas (user_id, mba_code) VALUES (?, ?)");
    for(const QString &m : mbas) {
        mbaQ.bindValue(0, uid); mbaQ.bindValue(1, m);
        if(!mbaQ.exec()) { db.rollback(); return false; }
    }
    
    db.commit();
    return true;
}

QSqlQuery UserDatabaseManager::getAllUsers() {
    return QSqlQuery("SELECT id, username, firstname, lastname, email, role FROM users", db);
}

bool UserDatabaseManager::deleteUser(int userId) {
    QSqlQuery q(db); 
    q.prepare("DELETE FROM users WHERE id = ?"); 
    q.addBindValue(userId); 
    return q.exec(); 
}

QStringList UserDatabaseManager::getUserMBAs(int userId) {
    QStringList list;
    QSqlQuery q(db);
    q.prepare("SELECT mba_code FROM user_mbas WHERE user_id = ?");
    q.addBindValue(userId);
    q.exec();
    while(q.next()) list << q.value(0).toString();
    return list;
}
