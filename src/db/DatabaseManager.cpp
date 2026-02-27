#include "DatabaseManager.h"
#include <QDateTime>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>
#include <QFile>
#include <QCryptographicHash> // <--- REQUIRED FOR TAMPER-EVIDENT HASHING
#include <QDebug>
#include <QSqlError>


DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager _instance;
    return _instance;
}

bool DatabaseManager::connect() {
    db = QSqlDatabase::addDatabase("QSQLITE");
    
    // --- THE MAC FIX: Save inventory DB to the Documents folder ---
    QString dbPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/air_inventory.db";
    db.setDatabaseName(dbPath);
    // --------------------------------------------------------------
    
    if (!db.open()) {
        qCritical() << "DB Connection Error:" << db.lastError().text();
        return false;
    }
    initTables();
    return true;
}

void DatabaseManager::initTables() {
    QSqlQuery query;
    // 1. Batches Table
    query.exec("CREATE TABLE IF NOT EXISTS batches ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "batch_number TEXT UNIQUE, mba TEXT, kmp TEXT, building TEXT, room TEXT, "
               "physical_form TEXT, chemical_form TEXT, element TEXT, isotope TEXT, "
               "weight_u REAL, weight_u235 REAL, weight_pu REAL, weight_th REAL, "
               "unit TEXT, manufacturer TEXT, insertion_date TEXT, status TEXT)");

    // 2. History Table
    query.exec("CREATE TABLE IF NOT EXISTS history ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "batch_id INTEGER, change_type TEXT, element_code TEXT, "
               "items_count INTEGER, increase_u REAL, decrease_u REAL, "
               "record_date TEXT, description TEXT, "
               "FOREIGN KEY(batch_id) REFERENCES batches(id))");

    // 5. Backups Table
    query.exec("CREATE TABLE IF NOT EXISTS backups ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "title TEXT, description TEXT, "
               "filename TEXT, created_date TEXT, created_by TEXT)");

    // 6. Manual Ledger Table (UPDATED WITH SIGNATURE)
    query.exec("CREATE TABLE IF NOT EXISTS manual_ledger ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "date TEXT, ref TEXT, code TEXT, type TEXT, "
               "u_weight REAL, u235_weight REAL, items INTEGER, signature TEXT)"); // <--- NEW COLUMN
               
    // 7. LII Manual Table
    query.exec("CREATE TABLE IF NOT EXISTS lii_manual ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "kmp TEXT, position TEXT, batch TEXT, desc TEXT, "
               "weight_elem REAL, weight_fissile REAL, weight_pu REAL, "
               "burnup REAL, cooling REAL)");
               
    // 8. NLI Manual Table
    query.exec("CREATE TABLE IF NOT EXISTS nli_manual ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "batch TEXT, items INTEGER, code TEXT, "
               "u_elem_code TEXT, u_iso_code TEXT, u_weight REAL, u_iso_weight REAL, "
               "p_elem_code TEXT, p_weight REAL)");
      
    // 9. MBR Table (UPDATED WITH SIGNATURE)
    query.exec("CREATE TABLE IF NOT EXISTS mbr_entries ("
               "id INTEGER PRIMARY KEY AUTOINCREMENT, "
               "continuation TEXT, entry_name TEXT, element TEXT, "
               "weight REAL, unit TEXT, fissile REAL, "
               "isotope TEXT, report_no TEXT, signature TEXT)"); // <--- NEW COLUMN
               
}

// =========================================================
// OPERATIONS
// =========================================================

bool DatabaseManager::registerReceipt(const QMap<QString, QVariant> &data) {
    QSqlDatabase::database().transaction();

    QSqlQuery query;
    query.prepare("INSERT INTO batches (batch_number, mba, kmp, building, room, "
                  "physical_form, chemical_form, element, weight_u, weight_u235, "
                  "unit, manufacturer, insertion_date, status) "
                  "VALUES (:bn, :mba, :kmp, :bldg, :room, :pf, :cf, :el, "
                  ":wu, :wu235, :unit, :mfg, :date, 'Active')");
    
    query.bindValue(":bn", data["batch_number"]);
    query.bindValue(":mba", data["to_mba"]);
    query.bindValue(":kmp", data["kmp"]);
    query.bindValue(":bldg", data["building"]);
    query.bindValue(":room", data["room"]);
    query.bindValue(":pf", data["physical_form"]);
    query.bindValue(":cf", data["chemical_form"]);
    query.bindValue(":el", data["element"]);
    query.bindValue(":wu", data["weight_u"]);
    query.bindValue(":wu235", data["weight_u235"]);
    query.bindValue(":unit", data["unit"]);
    query.bindValue(":mfg", data["manufacturer"]);
    query.bindValue(":date", data["date"]);

    if(!query.exec()) {
        QSqlDatabase::database().rollback();
        return false;
    }

    int batchId = query.lastInsertId().toInt();
    QSqlQuery hQuery;
    hQuery.prepare("INSERT INTO history (batch_id, change_type, element_code, items_count, increase_u, decrease_u, record_date, description) "
                   "VALUES (?, ?, ?, ?, ?, 0, ?, ?)");
    hQuery.addBindValue(batchId);
    hQuery.addBindValue(data["receipt_code"]); 
    hQuery.addBindValue("D"); 
    hQuery.addBindValue(data["count"]);
    hQuery.addBindValue(data["weight_u"]);
    hQuery.addBindValue(data["date"]);
    hQuery.addBindValue("Receipt from " + data["from_mba"].toString());

    if(!hQuery.exec()) {
        QSqlDatabase::database().rollback();
        return false;
    }
    QSqlDatabase::database().commit();
    return true;
}


// =========================================================
// NEW MANUAL LEDGER FUNCTIONS (WITH CYBER SECURITY)
// =========================================================

bool DatabaseManager::addManualLedgerEntry(const QMap<QString, QVariant> &data) {
    // 1. TAMPER EVIDENT LOGIC: Hash the exact data before saving
    QString rawGL = QString("%1|%2|%3|%4|%5|%6|%7")
        .arg(data["date"].toString())
        .arg(data["ref"].toString())
        .arg(data["code"].toString())
        .arg(data["type"].toString())
        .arg(data["u_weight"].toDouble(), 0, 'f', 4)
        .arg(data["u235_weight"].toDouble(), 0, 'f', 4)
        .arg(data["items"].toInt());
        
    QString hashSig = QCryptographicHash::hash(rawGL.toUtf8(), QCryptographicHash::Sha256).toHex();

    // 2. Save
    QSqlQuery query;
    query.prepare("INSERT INTO manual_ledger (date, ref, code, type, u_weight, u235_weight, items, signature) "
                  "VALUES (:d, :r, :c, :t, :u, :u235, :i, :sig)");
    query.bindValue(":d", data["date"]);
    query.bindValue(":r", data["ref"]);
    query.bindValue(":c", data["code"]);
    query.bindValue(":t", data["type"]);
    query.bindValue(":u", data["u_weight"]);
    query.bindValue(":u235", data["u235_weight"]);
    query.bindValue(":i", data["items"]);
    query.bindValue(":sig", hashSig); // Save Hash
    return query.exec();
}

QSqlQuery DatabaseManager::getManualLedgerEntries() {
    return QSqlQuery("SELECT * FROM manual_ledger ORDER BY id ASC");
}


// =========================================================
// REPORTING
// =========================================================

QSqlQuery DatabaseManager::getICRData(const QString &mba, const QString &startDate, const QString &endDate) {
    QSqlQuery query;
    query.prepare("SELECT h.record_date, h.change_type, h.items_count, b.batch_number, "
                  "b.physical_form, b.element, h.increase_u, h.decrease_u, h.description "
                  "FROM history h "
                  "JOIN batches b ON h.batch_id = b.id "
                  "WHERE b.mba = :mba "
                  "AND h.record_date >= :start AND h.record_date <= :end " 
                  "ORDER BY h.id ASC");
                  
    query.bindValue(":mba", mba);
    query.bindValue(":start", startDate);
    query.bindValue(":end", endDate);
    query.exec();
    return query;
}

QSqlQuery DatabaseManager::getReceipts() {
    QSqlQuery query(db); 
    query.prepare("SELECT h.id, h.record_date, h.change_type, b.batch_number, h.items_count, "
                  "b.element, h.increase_u, b.weight_u235 "
                  "FROM history h JOIN batches b ON h.batch_id = b.id "
                  "WHERE h.change_type IN ('RD', 'RF', 'RN') "
                  "ORDER BY h.id ASC LIMIT 50");
    query.exec();
    return query;
}


QSqlQuery DatabaseManager::getLIIData(const QString &mba, const QString &date) {
    Q_UNUSED(date); 
    
    QSqlQuery query;
    query.prepare("SELECT kmp, building, room, batch_number, physical_form, chemical_form, "
                  "weight_u, weight_u235, weight_pu, weight_th "
                  "FROM batches "
                  "WHERE mba = ? AND status = 'Active' "
                  "ORDER BY kmp, batch_number");
    query.addBindValue(mba);
    query.exec();
    return query;
}

bool DatabaseManager::addLIIEntry(const QMap<QString, QVariant> &data) {
    QSqlQuery query;
    query.prepare("INSERT INTO lii_manual (kmp, position, batch, desc, weight_elem, weight_fissile, weight_pu, burnup, cooling) "
                  "VALUES (:k, :p, :b, :d, :we, :wf, :wp, :bu, :co)");
    query.bindValue(":k", data["kmp"]);
    query.bindValue(":p", data["position"]);
    query.bindValue(":b", data["batch"]);
    query.bindValue(":d", data["desc"]);
    query.bindValue(":we", data["weight_elem"]);
    query.bindValue(":wf", data["weight_fissile"]);
    query.bindValue(":wp", data["weight_pu"]);
    query.bindValue(":bu", data["burnup"]);
    query.bindValue(":co", 0.0);
    return query.exec();
}

QSqlQuery DatabaseManager::getLIIEntries() {
    return QSqlQuery("SELECT * FROM lii_manual ORDER BY id ASC");
}

bool DatabaseManager::addNLIEntry(const QMap<QString, QVariant> &data) {
    QSqlQuery query;
    query.prepare("INSERT INTO nli_manual (batch, items, code, "
                  "u_elem_code, u_iso_code, u_weight, u_iso_weight, "
                  "p_elem_code, p_weight) "
                  "VALUES (:b, :i, :c, :ue, :ui, :uw, :uiw, :pe, :pw)");
    query.bindValue(":b", data["batch"]);
    query.bindValue(":i", data["items"]);
    query.bindValue(":c", data["code"]);
    query.bindValue(":ue", data["u_elem_code"]);
    query.bindValue(":ui", data["u_iso_code"]);
    query.bindValue(":uw", data["u_weight"]);
    query.bindValue(":uiw", data["u_iso_weight"]);
    query.bindValue(":pe", data["p_elem_code"]);
    query.bindValue(":pw", data["p_weight"]);
    return query.exec();
}

QSqlQuery DatabaseManager::getNLIEntries() {
    return QSqlQuery("SELECT * FROM nli_manual ORDER BY id ASC");
}

QSqlQuery DatabaseManager::getGeneralLedgerData(const QString &mba, const QString &elementFilter) {
    QSqlQuery query;
    QString sql = "SELECT h.record_date, b.batch_number, h.change_type, h.element_code, "
                  "h.items_count, h.increase_u, h.decrease_u, "
                  "b.weight_u, b.weight_u235 "
                  "FROM history h "
                  "JOIN batches b ON h.batch_id = b.id "
                  "WHERE b.mba = :mba ";
    
    if (elementFilter == "Depleted Uranium") sql += "AND b.element LIKE 'Depleted%' ";
    else if (elementFilter == "Natural Uranium") sql += "AND b.element LIKE 'Natural%' ";
    else if (elementFilter == "Enriched Uranium") sql += "AND b.element LIKE 'Enriched%' ";
    else if (elementFilter == "Plutonium") sql += "AND b.element LIKE 'Plutonium%' ";
    else if (elementFilter == "Thorium") sql += "AND b.element LIKE 'Thorium%' ";
    
    sql += "ORDER BY h.record_date ASC, h.id ASC";

    query.prepare(sql);
    query.bindValue(":mba", mba);
    query.exec();
    return query;
}


// =========================================================
// MBR FUNCTIONS (WITH CYBER SECURITY)
// =========================================================

bool DatabaseManager::addMBREntry(const QMap<QString, QVariant> &data) {
    // 1. TAMPER EVIDENT LOGIC: Hash data
    QString rawMBR = QString("%1|%2|%3|%4|%5|%6|%7|%8")
        .arg(data["continuation"].toString())
        .arg(data["entry_name"].toString())
        .arg(data["element"].toString())
        .arg(data["weight"].toDouble(), 0, 'f', 4)
        .arg(data["unit"].toString())
        .arg(data["fissile"].toDouble(), 0, 'f', 4)
        .arg(data["isotope"].toString())
        .arg(data["report_no"].toString());
        
    QString hashSig = QCryptographicHash::hash(rawMBR.toUtf8(), QCryptographicHash::Sha256).toHex();

    // 2. Save
    QSqlQuery query(db);
    query.prepare("INSERT INTO mbr_entries (continuation, entry_name, element, weight, unit, fissile, isotope, report_no, signature) "
                  "VALUES (:cont, :name, :elem, :wt, :unit, :fis, :iso, :rep, :sig)");
    query.bindValue(":cont", data["continuation"]);
    query.bindValue(":name", data["entry_name"]);
    query.bindValue(":elem", data["element"]);
    query.bindValue(":wt", data["weight"]);
    query.bindValue(":unit", data["unit"]);
    query.bindValue(":fis", data["fissile"]);
    query.bindValue(":iso", data["isotope"]);
    query.bindValue(":rep", data["report_no"]);
    query.bindValue(":sig", hashSig); // Save Hash
    return query.exec();
}

QSqlQuery DatabaseManager::getMBREntries(int limit) {
    QString sql = "SELECT * FROM mbr_entries ORDER BY id ASC";
    if (limit > 0) {
        sql = QString("SELECT * FROM (SELECT * FROM mbr_entries ORDER BY id DESC LIMIT %1) ORDER BY id ASC").arg(limit);
    }
    return QSqlQuery(sql, db);
}

bool DatabaseManager::deleteMBREntry(int id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM mbr_entries WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

// =========================================================
// BACKUP & RESTORE
// =========================================================

bool DatabaseManager::createBackup(const QString &title, const QString &description) {
    // --- THE MAC FIX: Save backups safely to the Documents folder ---
    QString backupDir = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/AIR_Backups";
    QDir().mkpath(backupDir);

    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString filename = QString("backup_%1.db").arg(timestamp);
    QString destPath = backupDir + "/" + filename;

    QSqlQuery q;
    q.prepare("VACUUM INTO ?");
    q.addBindValue(destPath);
    if(!q.exec()) {
        qCritical() << "Backup failed (VACUUM):" << q.lastError().text();
        return false;
    }

    QSqlQuery metaQ;
    metaQ.prepare("INSERT INTO backups (title, description, filename, created_date, created_by) "
                  "VALUES (:t, :d, :f, :date, :by)");
    metaQ.bindValue(":t", title);
    metaQ.bindValue(":d", description);
    metaQ.bindValue(":f", filename);
    metaQ.bindValue(":date", QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss"));
    metaQ.bindValue(":by", "admin"); 
    
    return metaQ.exec();
}

bool DatabaseManager::restoreBackup(int backupId) {
    QSqlQuery q;
    q.prepare("SELECT filename FROM backups WHERE id = ?");
    q.addBindValue(backupId);
    if(!q.exec() || !q.next()) return false;
    
    QString filename = q.value(0).toString();
    // --- THE MAC FIX: Read backups safely from the Documents folder ---
    QString backupPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/AIR_Backups/" + filename;
    QString currentDb = db.databaseName();

    if(!QFile::exists(backupPath)) return false;

    db.close();

    QFile::remove(currentDb + ".old");
    QFile::rename(currentDb, currentDb + ".old"); 

    if(QFile::copy(backupPath, currentDb)) {
        db.open();
        return true;
    } else {
        QFile::rename(currentDb + ".old", currentDb);
        db.open();
        return false;
    }
}

// (The getBackups function stays exactly the same)

bool DatabaseManager::deleteBackup(int backupId) {
    QSqlQuery q;
    q.prepare("SELECT filename FROM backups WHERE id = ?");
    q.addBindValue(backupId);
    if(q.exec() && q.next()) {
        // --- THE MAC FIX: Delete backups safely from the Documents folder ---
        QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/AIR_Backups/" + q.value(0).toString();
        QFile::remove(path);
    }
    
    QSqlQuery del;
    del.prepare("DELETE FROM backups WHERE id = ?");
    del.addBindValue(backupId);
    return del.exec();
}

void DatabaseManager::connectToScenario(const QString &scenarioName) {
    if (db.isOpen()) {
        db.close();
    }

    // 1. Create a fresh, temporary database for the session
    QString sessionPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) 
                          + "/AIR_Training_" + scenarioName + ".sqlite";

    // Delete any old session data
    if(QFile::exists(sessionPath)) {
        QFile::remove(sessionPath);
    }
    
    // 2. Connect to the new empty database
    db.setDatabaseName(sessionPath);
    if (!db.open()) {
        qCritical() << "Error: connection with scenario database failed:" << db.lastError().text();
        return;
    } 

    qDebug() << "Connected to Training Session:" << sessionPath;
    
    // 3. Build the empty tables
    initTables(); 

    // 4. Inject the specific scenario data!
    injectScenarioData(scenarioName);
}

void DatabaseManager::injectScenarioData(const QString &scenarioName) {
    QSqlQuery query(db);

    // ==========================================
    // LEVEL 1: Fundamentals
    // ==========================================
    if (scenarioName == "scen_baseline" || scenarioName == "scen_reactor") {
        // NO DATA INJECTED. 
        // The student must learn to establish the baseline themselves.
        qDebug() << "Level 1 Scenario loaded: Starting with an empty database.";
    }

    // ==========================================
    // LEVEL 2: Reconciliation (SRD/PIT/MUF)
    // ==========================================
    else if (scenarioName == "scen_srd") {
        // ----------------------------------------------------------------
        // Module 2.1: Shipper/Receiver Differences (SRD)
        // ----------------------------------------------------------------
        // 1. Establish a baseline so the ledger isn't empty
        QMap<QString, QVariant> ledger1;
        ledger1["date"] = QDate::currentDate().addDays(-10).toString("yyMMdd");
        ledger1["ref"] = "PIL-START"; ledger1["code"] = "PB"; ledger1["type"] = "PIL (Set Balance)";
        ledger1["u_weight"] = 10000.0; ledger1["u235_weight"] = 300.0; ledger1["items"] = 5;
        addManualLedgerEntry(ledger1);

        // 2. The disputed Receipt (BOOK INVENTORY says 500g)
        QMap<QString, QVariant> ledger2;
        ledger2["date"] = QDate::currentDate().addDays(-2).toString("yyMMdd");
        ledger2["ref"] = "ICD-SRD-01"; ledger2["code"] = "RF"; ledger2["type"] = "Receipt";
        ledger2["u_weight"] = 500.0; ledger2["u235_weight"] = 25.0; ledger2["items"] = 1;
        addManualLedgerEntry(ledger2);

        // 3. PHYSICAL INVENTORY (LII) 
        QMap<QString, QVariant> lii1; // Baseline items
        lii1["kmp"] = "FFS"; lii1["position"] = "VAULT-A"; lii1["batch"] = "BASE-01"; lii1["desc"] = "LEU";
        lii1["weight_elem"] = 10000.0; lii1["weight_fissile"] = 300.0; lii1["weight_pu"] = 0.0; lii1["burnup"] = 0.0;
        addLIIEntry(lii1);

        // Here is the puzzle: The lab scale only measured 485g for the new receipt!
        QMap<QString, QVariant> lii2;
        lii2["kmp"] = "FFS"; lii2["position"] = "LAB-B"; lii2["batch"] = "SRD-01"; lii2["desc"] = "UO2 Powder";
        lii2["weight_elem"] = 485.0; lii2["weight_fissile"] = 24.25; lii2["weight_pu"] = 0.0; lii2["burnup"] = 0.0;
        addLIIEntry(lii2);
    }
    
    else if (scenarioName == "scen_pit") {
        // ----------------------------------------------------------------
        // Module 2.2: Physical Inventory Taking (PIT)
        // ----------------------------------------------------------------
        // 1. BOOK INVENTORY HISTORY: (15,000 + 5,000 - 4,000 = 16,000g Expected Balance)
        QMap<QString, QVariant> l1;
        l1["date"] = "260101"; l1["ref"] = "PIL-01"; l1["code"] = "PB"; 
        l1["type"] = "PIL (Set Balance)"; l1["u_weight"] = 15000.0; l1["u235_weight"] = 500.0; l1["items"] = 3;
        addManualLedgerEntry(l1);

        QMap<QString, QVariant> l2;
        l2["date"] = "260215"; l2["ref"] = "ICD-102"; l2["code"] = "RD"; 
        l2["type"] = "Receipt"; l2["u_weight"] = 5000.0; l2["u235_weight"] = 150.0; l2["items"] = 1;
        addManualLedgerEntry(l2);

        QMap<QString, QVariant> l3;
        l3["date"] = "260310"; l3["ref"] = "SHIP-05"; l3["code"] = "SD"; 
        l3["type"] = "Shipment"; l3["u_weight"] = 4000.0; l3["u235_weight"] = 120.0; l3["items"] = 1;
        addManualLedgerEntry(l3);

        // 2. PHYSICAL INVENTORY: 4 batches of 4,000g spread across the facility. Matches perfectly.
        // Student Task: Walk through the LII, verify the numbers match the 16,000g ledger balance, and generate the PIL PDF.
        QStringList kmps = {"FFS", "FFS", "RRC", "SFS"};
        QStringList pos = {"R1-A", "R1-B", "CORE-1", "POOL-1"};
        
        for(int i = 0; i < 4; i++) {
            QMap<QString, QVariant> item;
            item["kmp"] = kmps[i]; item["position"] = pos[i]; item["batch"] = "BATCH-0" + QString::number(i+1); 
            item["desc"] = "LEU Assembly"; item["weight_elem"] = 4000.0; item["weight_fissile"] = 132.5; 
            item["weight_pu"] = (kmps[i] == "SFS" ? 15.0 : 0.0); // Add Pu to spent fuel for realism
            item["burnup"] = (kmps[i] == "SFS" ? 2000.0 : 0.0);
            addLIIEntry(item);
        }
    }

    else if (scenarioName == "scen_muf") {
        // ----------------------------------------------------------------
        // Module 2.3: MUF Evaluation
        // ----------------------------------------------------------------
        // BOOK INVENTORY: Expected Balance = 75,000g U.
        QMap<QString, QVariant> l1;
        l1["date"] = "260101"; l1["ref"] = "PIL-01"; l1["code"] = "PB"; 
        l1["type"] = "PIL (Set Balance)"; l1["u_weight"] = 100000.0; l1["u235_weight"] = 2000.0; l1["items"] = 4;
        addManualLedgerEntry(l1);

        QMap<QString, QVariant> l2;
        l2["date"] = "260215"; l2["ref"] = "SHIP-01"; l2["code"] = "SD"; 
        l2["type"] = "Shipment"; l2["u_weight"] = 25000.0; l2["u235_weight"] = 500.0; l2["items"] = 1;
        addManualLedgerEntry(l2);

        // PHYSICAL INVENTORY: Total is only 72,900g U! (2.1kg MUF embedded in FUEL-C)
        QMap<QString, QVariant> i1;
        i1["kmp"] = "RRC"; i1["position"] = "CORE-1"; i1["batch"] = "FUEL-A"; i1["desc"] = "LEU";
        i1["weight_elem"] = 25000.0; i1["weight_fissile"] = 500.0; i1["weight_pu"] = 10.0; i1["burnup"] = 1200.0;
        addLIIEntry(i1);

        QMap<QString, QVariant> i2;
        i2["kmp"] = "RRC"; i2["position"] = "CORE-2"; i2["batch"] = "FUEL-B"; i2["desc"] = "LEU";
        i2["weight_elem"] = 25000.0; i2["weight_fissile"] = 500.0; i2["weight_pu"] = 10.0; i2["burnup"] = 1200.0;
        addLIIEntry(i2);

        QMap<QString, QVariant> i3;
        i3["kmp"] = "SFS"; i3["position"] = "POOL-1"; i3["batch"] = "FUEL-C"; i3["desc"] = "LEU";
        i3["weight_elem"] = 22900.0; i3["weight_fissile"] = 450.0; i3["weight_pu"] = 8.0; i3["burnup"] = 1500.0;
        addLIIEntry(i3);
    }

   // ==========================================
    // LEVEL 3: Advanced Safeguards
    // ==========================================
    else if (scenarioName == "scen_protracted") {
        // Module 3.1: Protracted Diversion
        // Concept: A systemic bias. 5 shipments arrive over 5 months. 
        // The paperwork claims 1000g each time. But the physical reality is always exactly 5g short.
        
        // Initial Balance
        QMap<QString, QVariant> bal;
        bal["date"] = "250101"; bal["ref"] = "PIL-START"; bal["code"] = "PB"; 
        bal["type"] = "PIL (Set Balance)"; bal["u_weight"] = 50000.0; bal["u235_weight"] = 1000.0; bal["items"] = 50;
        addManualLedgerEntry(bal);

        // Inject 5 biased receipts
        for(int i=1; i<=5; i++) {
            QString dateStr = QString("250%1%2").arg(i+1).arg("15"); // e.g., 250215, 250315...
            
            // Ledger claims 1000g received
            QMap<QString, QVariant> rec;
            rec["date"] = dateStr; rec["ref"] = QString("ICD-00%1").arg(i); rec["code"] = "RD"; 
            rec["type"] = "Receipt"; rec["u_weight"] = 1000.0; rec["u235_weight"] = 20.0; rec["items"] = 1;
            addManualLedgerEntry(rec);

            // Physical Inventory (LII) shows 995g (5g siphoned off each time!)
            QMap<QString, QVariant> lii;
            lii["kmp"] = "FFS"; lii["position"] = QString("SHELF-%1").arg(i); lii["batch"] = QString("BATCH-%1").arg(i); 
            lii["desc"] = "LEU"; lii["weight_elem"] = 995.0; lii["weight_fissile"] = 19.9; lii["weight_pu"] = 0.0; lii["burnup"] = 0.0;
            addLIIEntry(lii);
        }
    }
    
    else if (scenarioName == "scen_dummy") {
        // Module 3.2: The Substituted Dummy (and Cyber Tamper!)
        // Concept: An insider stole a fuel assembly, replaced it with a dummy, 
        // and tried to hack the database to hide the discrepancy, breaking the digital signature.
        
        // 1. Valid PIL Baseline
        QMap<QString, QVariant> bal;
        bal["date"] = "260101"; bal["ref"] = "PIL-START"; bal["code"] = "PB"; 
        bal["type"] = "PIL (Set Balance)"; bal["u_weight"] = 10000.0; bal["u235_weight"] = 200.0; bal["items"] = 2;
        addManualLedgerEntry(bal);
        
        // 2. The Hacker's direct database injection (Triggers Red Tamper Alarm)
        // They tried to add a fake shipment to explain the missing material, but couldn't fake the SHA-256 hash.
        QSqlQuery query(db);
        query.prepare("INSERT INTO manual_ledger (date, ref, code, type, u_weight, u235_weight, items, signature) "
                      "VALUES ('260220', 'FAKE-SHIP-01', 'SD', 'Shipment', 5000.0, 100.0, 1, 'INVALID_HACKER_SIGNATURE')");
        query.exec();

        query.prepare("INSERT INTO mbr_entries (continuation, entry_name, element, weight, unit, fissile, isotope, report_no, signature) "
                      "VALUES ('', 'PB', 'E', 4500.0, 'G', 90.0, 'G', '1', 'BROKEN_HASH')");
        query.exec();

        // 3. Physical Inventory: The dummy item.
        QMap<QString, QVariant> lii;
        lii["kmp"] = "SFS"; lii["position"] = "POOL-A"; lii["batch"] = "DUMMY-01"; lii["desc"] = "Irradiated Dummy";
        lii["weight_elem"] = 0.0; lii["weight_fissile"] = 0.0; lii["weight_pu"] = 0.0; lii["burnup"] = 0.0;
        addLIIEntry(lii);
    }
}

void DatabaseManager::resetToRealDatabase() {
    if (db.isOpen()) {
        db.close();
    }
    
    // --- THE MAC FIX: Ensure it reconnects to the Documents folder ---
    QString realDbPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/air_inventory.db"; 
    db.setDatabaseName(realDbPath);
    // -----------------------------------------------------------------
    
    if (!db.open()) {
        qCritical() << "Error: Could not reconnect to real database:" << db.lastError().text();
    } else {
        qDebug() << "Successfully reconnected to Real Database:" << realDbPath;
    }
}

QString DatabaseManager::currentDatabaseName() const {
    return db.databaseName();
}

// ... [DELETE FUNCTIONS REMAIN THE SAME] ...
bool DatabaseManager::deleteReceipt(int id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM history WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

bool DatabaseManager::deleteManualLedgerEntry(int id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM manual_ledger WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

bool DatabaseManager::deleteLIIEntry(int id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM lii_manual WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}

bool DatabaseManager::deleteNLIEntry(int id) {
    QSqlQuery query(db);
    query.prepare("DELETE FROM nli_manual WHERE id = ?");
    query.addBindValue(id);
    return query.exec();
}
