#include "ReceiptWidget.h"
#include "PinDialog.h"
#include "../../db/DatabaseManager.h"
#include "../../utils/ReportGenerator.h" 
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QHeaderView>
#include <QMessageBox>
#include <QDebug>
#include <QMap>
#include <QPushButton> 
#include <QDate>        
#include <QFileDialog> 
#include <QSqlDatabase> 

ReceiptWidget::ReceiptWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
    refreshTable();
}

void ReceiptWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    QLabel *title = new QLabel("<h2>Inventory Change Report (ICR) - Operations</h2>");
    title->setStyleSheet("color: #003366;");
    mainLayout->addWidget(title);

    setupReportHeader(mainLayout);
    setupInputForm(mainLayout);
    setupTable(mainLayout);
}

void ReceiptWidget::setupReportHeader(QVBoxLayout *layout) {
    QGroupBox *grp = new QGroupBox("Report Settings (For PDF Export)");
    grp->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #aaa; }");
    QGridLayout *grid = new QGridLayout(grp);

    // Row 0
    grid->addWidget(new QLabel("Country:"), 0, 0);
    txtCountry = new QLineEdit("AAUSTRIA");
    grid->addWidget(txtCountry, 0, 1);

    grid->addWidget(new QLabel("Report Date:"), 0, 2);
    dateReport = new QDateEdit(QDate::currentDate());
    dateReport->setDisplayFormat("yyyy-MM-dd");
    grid->addWidget(dateReport, 0, 3);

    // Row 1
    grid->addWidget(new QLabel("Facility:"), 1, 0);
    txtFacility = new QLineEdit("Compton Research Reactor");
    grid->addWidget(txtFacility, 1, 1);

    grid->addWidget(new QLabel("Report No:"), 1, 2);
    txtReportNo = new QLineEdit("1");
    grid->addWidget(txtReportNo, 1, 3);
    
    // Row 2
    grid->addWidget(new QLabel("MBA:"), 2, 0);
    txtMBA = new QLineEdit("CRRF");
    grid->addWidget(txtMBA, 2, 1);

    // Row 3
    grid->addWidget(new QLabel("Shipper Name:"), 3, 0);
    txtShipper = new QLineEdit;
    grid->addWidget(txtShipper, 3, 1);

    grid->addWidget(new QLabel("Receiver Name:"), 3, 2);
    txtReceiver = new QLineEdit;
    grid->addWidget(txtReceiver, 3, 3);

    layout->addWidget(grp);
}

void ReceiptWidget::setupInputForm(QVBoxLayout *layout) {
    QGroupBox *grp = new QGroupBox("New Receipt Transaction");
    grp->setStyleSheet("QGroupBox { border: 1px solid #003366; font-weight: bold; margin-top: 10px; }");
    
    QGridLayout *grid = new QGridLayout(grp);
    grid->setSpacing(10);

    // Row 0
    grid->addWidget(new QLabel("Batch Identity:"), 0, 0);
    txtBatch = new QLineEdit; 
    txtBatch->setPlaceholderText("e.g. CRR01");
    grid->addWidget(txtBatch, 0, 1);

    // Hardcoded List
    QStringList mbaList = {"CRRF", "EULE", "EXT", "DKNZ"};

    grid->addWidget(new QLabel("Recv. MBA:"), 0, 2);
    comboMbaTo = new QComboBox; 
    comboMbaTo->addItems(mbaList);
    grid->addWidget(comboMbaTo, 0, 3);

    grid->addWidget(new QLabel("From MBA:"), 0, 4);
    comboMbaFrom = new QComboBox; 
    comboMbaFrom->addItems(mbaList); 
    comboMbaFrom->addItem("EXT (External)");
    grid->addWidget(comboMbaFrom, 0, 5);

    // Row 1
    grid->addWidget(new QLabel("Inv. Change Code:"), 1, 0);
    comboCode = new QComboBox;
    comboCode->addItems({"RD (Receipt Domestic)", "RF (Receipt Foreign)", "RS (Receipt at starting point)", "NP (Nuclear Production)", "DU (De-exemption, use)", "DQ (De-exemption, quantity)", "SF (Shipment Foreign)", "SD (Shipment Domestic)", "LN (Nuclear Loss)", "EU (Exemption, use)", "EQ (Exemption, quantity)", "TU (Termination)", "LA (Accidental Loss)", "GA (Accidental Gain)"});
    grid->addWidget(comboCode, 1, 1);

    grid->addWidget(new QLabel("No. of Items:"), 1, 2);
    spinItems = new QSpinBox; spinItems->setRange(1, 1000);
    grid->addWidget(spinItems, 1, 3);

    grid->addWidget(new QLabel("Date (YYYY-MM-DD):"), 1, 4);
    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("yyyy-MM-dd"); 
    grid->addWidget(dateEdit, 1, 5);

    // Row 2
    grid->addWidget(new QLabel("Element Code:"), 2, 0);
    comboElement = new QComboBox;
    comboElement->addItems({"N (Natural Uranium)", "E (Enriched Uranium)", "D (Depleted Uranium)", "P (Plutonium)", "T (Thorium)"});
    grid->addWidget(comboElement, 2, 1);

    grid->addWidget(new QLabel("Total Element Wt (g):"), 2, 2);
    spinWeightU = new QDoubleSpinBox; 
    spinWeightU->setRange(0, 10000000); 
    spinWeightU->setDecimals(2); // Increased precision
    grid->addWidget(spinWeightU, 2, 3);

    grid->addWidget(new QLabel("Isotope/Fissile Wt (g):"), 2, 4);
    spinWeightU235 = new QDoubleSpinBox; 
    spinWeightU235->setRange(0, 10000000); 
    spinWeightU235->setDecimals(3); // Standard precision for Isotope
    grid->addWidget(spinWeightU235, 2, 5);

    // Row 3
    grid->addWidget(new QLabel("Material Form:"), 3, 0);
    txtForm = new QLineEdit;
    txtForm->setPlaceholderText("e.g. UO2 Powder");
    grid->addWidget(txtForm, 3, 1, 1, 3);

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout;
    
    QPushButton *btnSave = new QPushButton("Add Entry");
    btnSave->setStyleSheet("background-color: #0056b3; color: white; font-weight: bold; padding: 8px; border-radius: 4px;");
    connect(btnSave, &QPushButton::clicked, this, &ReceiptWidget::saveReceipt);
    
    QPushButton *btnExport = new QPushButton("Export PDF");
    btnExport->setStyleSheet("background-color: #074282; color: white; font-weight: bold; padding: 8px; border-radius: 4px;");
    connect(btnExport, &QPushButton::clicked, this, &ReceiptWidget::exportPDF);

    QPushButton *btnDelete = new QPushButton("Delete Selected");
    btnDelete->setStyleSheet("background-color: #c0392b; color: white; font-weight: bold; padding: 8px; border-radius: 4px;");
    connect(btnDelete, &QPushButton::clicked, this, &ReceiptWidget::deleteSelected);

    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnExport);
    btnLayout->addWidget(btnDelete); 

    grid->addLayout(btnLayout, 3, 4, 1, 2);

    layout->addWidget(grp);
}

void ReceiptWidget::setupTable(QVBoxLayout *layout) {
    layout->addWidget(new QLabel("<b>Recent Receipts (ICD Format)</b>"));

    table = new QTableWidget;
    table->setColumnCount(10);
    QStringList headers;
    headers << "Line" << "Batch Identity" << "Items" << "IC Code" 
            << "Elem\nCode (U)" << "Iso\nCode (U)" << "Elem\nWt (g)" << "Iso\nWt (g)" 
            << "Elem\nCode (P)" << "Elem\nWt (g)";
    
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet("QHeaderView::section { background-color: #f0f0f0; font-weight: bold; border: 1px solid #ccc; }"
                         "QTableWidget { border: 1px solid #ccc; }");

    layout->addWidget(table);
}

void ReceiptWidget::saveReceipt() {
    if(txtBatch->text().isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Batch Identity is required.");
        return;
    }
    if(spinWeightU->value() <= 0) {
        QMessageBox::warning(this, "Input Error", "Element weight must be greater than 0.");
        return;
    }

    QMap<QString, QVariant> data;
    data["batch_number"] = txtBatch->text();
    data["to_mba"] = comboMbaTo->currentText();
    data["from_mba"] = comboMbaFrom->currentText();
    data["receipt_code"] = comboCode->currentText().left(2); 
    data["count"] = spinItems->value();
    data["date"] = dateEdit->date().toString("yyyy-MM-dd"); 
    data["element"] = comboElement->currentText(); 
    data["weight_u"] = spinWeightU->value();
    data["weight_u235"] = spinWeightU235->value();
    data["physical_form"] = txtForm->text();
    data["chemical_form"] = "Oxide"; 
    data["kmp"] = "A"; 
    data["unit"] = "g";
    data["manufacturer"] = data["from_mba"];
    
    // SAFE CALL: Uses DatabaseManager's internal connection logic
    if(DatabaseManager::instance().registerReceipt(data)) {
        QMessageBox::information(this, "Success", "Receipt registered successfully.");
        txtBatch->clear();
        spinWeightU->setValue(0);
        spinWeightU235->setValue(0);
        
        refreshTable();
        emit dataChanged(); 
    } else {
        QMessageBox::critical(this, "Error", "Failed to register receipt. Batch ID might exist.");
    }
}

void ReceiptWidget::deleteSelected() {
    int row = table->currentRow();
    if(row < 0) {
        QMessageBox::warning(this, "Select Item", "Please select a row to delete.");
        return;
    }

    // --- ZERO TRUST VERIFICATION (TRAINING MODE ONLY) ---
    if (DatabaseManager::instance().currentDatabaseName().contains("AIR_Training")) {
        PinDialog authDialog(this);
        if (authDialog.exec() != QDialog::Accepted) {
            qDebug() << "Zero Trust Policy: Deletion blocked.";
            return; 
        }
    }
    // ----------------------------------------------------

    QTableWidgetItem *item = table->item(row, 0);
    int id = item->data(Qt::UserRole).toInt();

    if(QMessageBox::question(this, "Confirm", "Delete this receipt entry?\nThis will remove it from the database.", 
                             QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        
        if(DatabaseManager::instance().deleteReceipt(id)) {
            refreshTable(); 
            emit dataChanged(); 
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete receipt from database.");
        }
    }
}

void ReceiptWidget::refreshTable() {
    table->setRowCount(0);
    
    // SAFE CALL: Uses the helper from DatabaseManager to avoid crashes
    QSqlQuery query = DatabaseManager::instance().getReceipts();

    int line = 1;
    while(query.next()) {
        int r = table->rowCount();
        table->insertRow(r);
        
        int id = query.value("id").toInt();
        QString batch = query.value("batch_number").toString();
        QString code = query.value("change_type").toString();
        int items = query.value("items_count").toInt();
        QString rawEl = query.value("element").toString();
        QString elCode = rawEl.left(1); 
        double wElem = query.value("increase_u").toDouble();
        
        // --- FIX HERE: READ DATABASE VALUE INSTEAD OF CALCULATING ---
        double wIso = query.value("weight_u235").toDouble(); 
        // ------------------------------------------------------------

        QTableWidgetItem *itemLine = new QTableWidgetItem(QString::number(line++));
        itemLine->setData(Qt::UserRole, id); 
        table->setItem(r, 0, itemLine);
        
        table->setItem(r, 1, new QTableWidgetItem(batch));
        table->setItem(r, 2, new QTableWidgetItem(QString::number(items)));
        table->setItem(r, 3, new QTableWidgetItem(code));

        if(elCode != "P") {
            table->setItem(r, 4, new QTableWidgetItem(elCode));
            table->setItem(r, 5, new QTableWidgetItem("G")); 
            table->setItem(r, 6, new QTableWidgetItem(QString::number(wElem, 'f', 2)));
            // Display Isotope Weight with 3 decimal places
            table->setItem(r, 7, new QTableWidgetItem(QString::number(wIso, 'f', 3)));
            table->setItem(r, 8, new QTableWidgetItem(""));
            table->setItem(r, 9, new QTableWidgetItem(""));
        } else {
            table->setItem(r, 4, new QTableWidgetItem(""));
            table->setItem(r, 5, new QTableWidgetItem(""));
            table->setItem(r, 6, new QTableWidgetItem(""));
            table->setItem(r, 7, new QTableWidgetItem(""));
            table->setItem(r, 8, new QTableWidgetItem("P"));
            table->setItem(r, 9, new QTableWidgetItem(QString::number(wElem, 'f', 2)));
        }
    }
}

void ReceiptWidget::exportPDF() {
    // --- ZERO TRUST VERIFICATION (TRAINING MODE ONLY) ---
    if (DatabaseManager::instance().currentDatabaseName().contains("AIR_Training")) {
        PinDialog authDialog(this);
        if (authDialog.exec() != QDialog::Accepted) {
            qDebug() << "Zero Trust Policy: Export blocked.";
            return; 
        }
    }
    // ----------------------------------------------------

    QString fileName = QFileDialog::getSaveFileName(this, "Save Receipt Report", "ICR_Full_Report.pdf", "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QMap<QString, QString> headerData;
    headerData["country"] = txtCountry->text();
    headerData["facility"] = txtFacility->text();
    headerData["mba"] = txtMBA->text(); 
    headerData["reportNo"] = txtReportNo->text();
    headerData["date"] = dateReport->text();
    headerData["shipper"] = txtShipper->text();
    headerData["receiver"] = txtReceiver->text();

    QString connectionName = DatabaseManager::instance().currentDatabaseName().isEmpty() 
                           ? "qt_sql_default_connection" 
                           : "air_connection";           
                           
    QSqlDatabase db = QSqlDatabase::database(connectionName);
    QSqlQuery query(db); 
    
    // --- FIX: Added b.weight_u235 to the SELECT statement ---
    query.prepare("SELECT h.record_date, h.change_type, h.items_count, b.batch_number, "
                  "b.physical_form, b.element, h.increase_u, b.weight_u235, h.decrease_u, h.description "
                  "FROM history h JOIN batches b ON h.batch_id = b.id "
                  "WHERE h.change_type IN ('RD', 'RF', 'RN') "
                  "ORDER BY h.id ASC"); 
    query.exec();

    // Note: You may need to ensure ReportGenerator.cpp reads this new column index correctly.
    // Usually, it reads columns in order. I inserted weight_u235 right after increase_u.
    
    if (ReportGenerator::generateICR_PDF(fileName, headerData, query)) {
        QMessageBox::information(this, "Success", "Full Receipt Report generated successfully.");
    } else {
        QMessageBox::critical(this, "Error", "Failed to generate PDF.");
    }
}
