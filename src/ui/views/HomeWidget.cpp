#include "HomeWidget.h"
#include "../../db/DatabaseManager.h"
#include <QHeaderView>
#include <QSqlQuery>
#include <QCryptographicHash> // <--- REQUIRED
#include <QColor>
#include <QVBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QtMath>

HomeWidget::HomeWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
    refreshData();
}

void HomeWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    // --- 1. General Ledger Section ---
    QLabel *glHeader = new QLabel("<h2>General Ledger Preview</h2>");
    glHeader->setStyleSheet("color: #003366;");
    glHeader->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(glHeader);

    setupGLPreview(mainLayout);

    // --- 2. MBR Section ---
    QLabel *mbrHeader = new QLabel("<h2>Material Balance Preview</h2>");
    mbrHeader->setStyleSheet("color: #003366; margin-top: 20px;");
    mbrHeader->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(mbrHeader);

    tableMBR = new QTableWidget();
    tableMBR->setColumnCount(7); 
    tableMBR->setHorizontalHeaderLabels({"Line", "Entry", "Element", "Weight", "Unit", "Fissile", "Isotope"});
    tableMBR->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    tableMBR->verticalHeader()->setVisible(false);
    tableMBR->setEditTriggers(QAbstractItemView::NoEditTriggers); 
    tableMBR->setAlternatingRowColors(true);
    tableMBR->setStyleSheet("QHeaderView::section { background-color: #f0f0f0; font-weight: bold; border: 1px solid #ccc; }"
                            "QTableWidget { border: 1px solid #ccc; }");
    
    mainLayout->addWidget(tableMBR);
}

void HomeWidget::setupGLPreview(QVBoxLayout *layout) {
    glTable = new QTableWidget;
    glTable->setColumnCount(16);
    glTable->verticalHeader()->setVisible(false);
    glTable->horizontalHeader()->setVisible(false);
    glTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    glTable->setAlternatingRowColors(true);

    glTable->insertRow(0); glTable->insertRow(1); glTable->insertRow(2);

    auto setH = [&](int r, int c, QString txt) {
        QTableWidgetItem *it = new QTableWidgetItem(txt);
        it->setTextAlignment(Qt::AlignCenter);
        it->setBackground(QColor("#e0e0e0")); 
        it->setFont(QFont("Arial", 8, QFont::Bold));
        glTable->setItem(r, c, it);
    };

    setH(0, 0, "Line"); glTable->setSpan(0, 0, 3, 1);
    setH(0, 1, "Date"); glTable->setSpan(0, 1, 3, 1);
    setH(0, 2, "Ref");  glTable->setSpan(0, 2, 3, 1);
    setH(0, 3, "Code"); glTable->setSpan(0, 3, 3, 1);
    setH(0, 4, "Items"); glTable->setSpan(0, 4, 3, 1);

    setH(0, 5, "Increases"); glTable->setSpan(0, 5, 1, 4);
    setH(0, 9, "Decreases"); glTable->setSpan(0, 9, 1, 4);
    setH(0, 13, "Inventory"); glTable->setSpan(0, 13, 1, 2);
    setH(0, 15, "Items"); glTable->setSpan(0, 15, 3, 1);

    setH(1, 5, "Rcpt"); glTable->setSpan(1, 5, 1, 2);
    setH(1, 7, "Oth");  glTable->setSpan(1, 7, 1, 2);
    setH(1, 9, "Ship"); glTable->setSpan(1, 9, 1, 2);
    setH(1, 11, "Oth"); glTable->setSpan(1, 11, 1, 2);
    setH(1, 13, "Bal"); glTable->setSpan(1, 13, 1, 2);

    for(int i : {5, 7, 9, 11, 13}) setH(2, i, "U");
    for(int i : {6, 8, 10, 12, 14}) setH(2, i, "235");

    glTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    glTable->setColumnWidth(0, 30); 

    layout->addWidget(glTable);
}

void HomeWidget::refreshData() {
    // ==========================================
    // 1. REFRESH GENERAL LEDGER (WITH TAMPER CHECK)
    // ==========================================
    glTable->setRowCount(3); 
    balU = 0; balU235 = 0; balItems = 0; lineCounter = 1;

    QSqlQuery qGL = DatabaseManager::instance().getManualLedgerEntries();
    while(qGL.next()) {
        int r = glTable->rowCount();
        glTable->insertRow(r);

        QString date = qGL.value("date").toString();
        QString ref = qGL.value("ref").toString();
        QString code = qGL.value("code").toString();
        QString type = qGL.value("type").toString();
        double u = qGL.value("u_weight").toDouble();
        double u235 = qGL.value("u235_weight").toDouble();
        int items = qGL.value("items").toInt();
        QString storedHash = qGL.value("signature").toString();

        // 1A. Security Validation Check
        QString rawGL = QString("%1|%2|%3|%4|%5|%6|%7")
            .arg(date).arg(ref).arg(code).arg(type)
            .arg(u, 0, 'f', 4).arg(u235, 0, 'f', 4).arg(items);
        QString calcHash = QCryptographicHash::hash(rawGL.toUtf8(), QCryptographicHash::Sha256).toHex();
        
        bool isTampered = (!storedHash.isEmpty() && storedHash != calcHash);

        if(type == "Receipt") {
            balU += u; balU235 += u235; balItems += items;
        } else if (type == "Shipment") {
            balU -= u; balU235 -= u235; balItems -= items; 
        } else if (type == "Other Increase") {
            balU += u; balU235 += u235; 
        } else if (type == "Other Decrease") {
            balU -= u; balU235 -= u235; 
        } else if (type == "PIL (Set Balance)") {
            if(lineCounter == 1 && balU == 0) { 
                balU = u; balU235 = u235; balItems = items;
            }
        }

        auto setC = [&](int c, QString t) {
            QTableWidgetItem *item = new QTableWidgetItem(t);
            // If tampered, color the cell RED
            if (isTampered) {
                item->setBackground(QColor("#ffcdd2")); // Light Red
                item->setForeground(QColor("red"));
                item->setToolTip("SECURITY ALERT: Row data altered outside application!");
            }
            glTable->setItem(r, c, item);
        };

        setC(0, QString::number(lineCounter++));
        setC(1, date);
        setC(2, ref);
        setC(3, isTampered ? "TAMPERED" : code); // Display warning
        
        QString displayItems = "";
        if(type == "Receipt" || type == "Shipment") {
             displayItems = (items > 0 ? QString::number(items) : "");
        }
        setC(4, displayItems);

        if(type == "Receipt") {
            setC(5, QString::number(u)); setC(6, QString::number(u235));
        } else if (type == "Other Increase") {
            setC(7, QString::number(u)); setC(8, QString::number(u235));
        } else if (type == "Shipment") {
            setC(9, QString::number(u)); setC(10, QString::number(u235));
        } else if (type == "Other Decrease") {
            setC(11, QString::number(u)); setC(12, QString::number(u235));
        }

        QTableWidgetItem *b1 = new QTableWidgetItem(QString::number(balU));
        b1->setBackground(isTampered ? QColor("#ffcdd2") : QColor("#e8f5e9")); 
        glTable->setItem(r, 13, b1);
        
        QTableWidgetItem *b2 = new QTableWidgetItem(QString::number(balU235));
        b2->setBackground(isTampered ? QColor("#ffcdd2") : QColor("#e8f5e9")); 
        glTable->setItem(r, 14, b2);

        QTableWidgetItem *b3 = new QTableWidgetItem(QString::number(balItems));
        b3->setBackground(isTampered ? QColor("#ffcdd2") : QColor("#e8f5e9")); 
        glTable->setItem(r, 15, b3);
    }

    // ==========================================
    // 2. REFRESH MBR DATA (WITH TAMPER CHECK)
    // ==========================================
    if(tableMBR) {
        tableMBR->setRowCount(0);
        QSqlQuery qMBR = DatabaseManager::instance().getMBREntries(10); 
        
        int line = 1;
        while(qMBR.next()) {
            int r = tableMBR->rowCount();
            tableMBR->insertRow(r);
            
            QString cont = qMBR.value("continuation").toString();
            QString name = qMBR.value("entry_name").toString();
            QString elem = qMBR.value("element").toString();
            double w = qMBR.value("weight").toDouble();
            QString unit = qMBR.value("unit").toString();
            double f = qMBR.value("fissile").toDouble();
            QString iso = qMBR.value("isotope").toString();
            QString rep = qMBR.value("report_no").toString();
            QString storedHash = qMBR.value("signature").toString();

            // 2A. Security Validation Check
            QString rawMBR = QString("%1|%2|%3|%4|%5|%6|%7|%8")
                .arg(cont).arg(name).arg(elem)
                .arg(w, 0, 'f', 4).arg(unit).arg(f, 0, 'f', 4).arg(iso).arg(rep);
                
            QString calcHash = QCryptographicHash::hash(rawMBR.toUtf8(), QCryptographicHash::Sha256).toHex();
            
            bool isTampered = (!storedHash.isEmpty() && storedHash != calcHash);

            auto setM = [&](int c, QString t) {
                QTableWidgetItem *item = new QTableWidgetItem(t);
                if (isTampered) {
                    item->setBackground(QColor("#ffcdd2")); 
                    item->setForeground(QColor("red"));
                    item->setToolTip("SECURITY ALERT: Row data altered outside application!");
                }
                tableMBR->setItem(r, c, item);
            };

            setM(0, QString::number(line++));
            setM(1, isTampered ? "TAMPER ALERT" : name);
            setM(2, elem);
            setM(3, QString::number(w, 'f', w == qRound(w) ? 0 : 2));
            setM(4, unit);
            setM(5, QString::number(f, 'f', f == qRound(f) ? 0 : 2));
            setM(6, iso);
        }
    }
}
