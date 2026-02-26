#include "GeneralLedgerWidget.h"
#include "PinDialog.h"
#include "../../db/DatabaseManager.h"
#include "../../utils/ReportGenerator.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QDate>
#include <QDebug>
#include <QMap>

GeneralLedgerWidget::GeneralLedgerWidget(QWidget *parent) 
    : QWidget(parent), lineCounter(1), balU(0), balU235(0), balItems(0) {
    setupUI();
    refreshData(); 
}

void GeneralLedgerWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);

    QLabel *title = new QLabel("<h2>General Ledger</h2>");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: black; font-weight: bold;");
    mainLayout->addWidget(title);
    
    // --- RESTRUCTURED ORDER ---
    setupReportHeader(mainLayout); // 1. Report Info Box
    setupInputForm(mainLayout);    // 2. Input Form (Now contains Add, Export, Delete)
    setupComplexTable(mainLayout); // 3. The Data Table
}

void GeneralLedgerWidget::setupReportHeader(QVBoxLayout *layout) {
    QGroupBox *grp = new QGroupBox("Report Information");
    grp->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #aaa; padding-top: 10px; }");
    QGridLayout *grid = new QGridLayout(grp);
    grid->setSpacing(10);

    grid->addWidget(new QLabel("Facility:"), 0, 0);
    txtFacility = new QLineEdit("Compton Research Reactor");
    grid->addWidget(txtFacility, 0, 1);

    grid->addWidget(new QLabel("MBA:"), 0, 2);
    comboMBA = new QComboBox;
    comboMBA->setEditable(true); 
    grid->addWidget(comboMBA, 0, 3);

    grid->addWidget(new QLabel("Material Description:"), 1, 0);
    txtMatDesc = new QLineEdit("Low Enriched Uranium (U3O8-Al)");
    grid->addWidget(txtMatDesc, 1, 1, 1, 3); 

    QHBoxLayout *codeLay = new QHBoxLayout;
    codeLay->addWidget(new QLabel("Element Code:"));
    txtElemCode = new QLineEdit("E"); txtElemCode->setFixedWidth(40);
    codeLay->addWidget(txtElemCode);

    codeLay->addWidget(new QLabel("Isotope Code:"));
    txtIsoCode = new QLineEdit("G"); txtIsoCode->setFixedWidth(40);
    codeLay->addWidget(txtIsoCode);

    codeLay->addWidget(new QLabel("Unit:"));
    txtUnit = new QLineEdit("g"); txtUnit->setFixedWidth(40);
    codeLay->addWidget(txtUnit);
    
    codeLay->addStretch();
    grid->addLayout(codeLay, 2, 0, 1, 4);

    layout->addWidget(grp);
}

void GeneralLedgerWidget::setupInputForm(QVBoxLayout *layout) {
    QGroupBox *grp = new QGroupBox("Add Transaction Row");
    grp->setStyleSheet("QGroupBox { border: 1px solid #003366; font-weight: bold; }");
    QGridLayout *grid = new QGridLayout(grp);

    // Row 0
    grid->addWidget(new QLabel("Date:"), 0, 0);
    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("yyyy-MM-dd"); 
    grid->addWidget(dateEdit, 0, 1);

    grid->addWidget(new QLabel("Ref (ICD/PIL):"), 0, 2);
    txtRef = new QLineEdit; txtRef->setPlaceholderText("e.g. PIL1");
    grid->addWidget(txtRef, 0, 3);

    grid->addWidget(new QLabel("IC Code:"), 0, 4);
    comboCode = new QComboBox; 
    comboCode->addItems({"", "RF", "RD", "SD", "LN", "LD"});
    comboCode->setEditable(true);
    grid->addWidget(comboCode, 0, 5);

    // Row 1
    grid->addWidget(new QLabel("Transaction Type:"), 1, 0);
    comboType = new QComboBox;
    comboType->addItems({"", "Receipt", "Other Increase", "Shipment", "Other Decrease", "Nuclear Loss", "PIL (Set Balance)"});
    grid->addWidget(comboType, 1, 1);

    grid->addWidget(new QLabel("Element (U):"), 1, 2);
    spinElem = new QDoubleSpinBox; spinElem->setRange(0, 1000000); spinElem->setDecimals(0);
    grid->addWidget(spinElem, 1, 3);

    grid->addWidget(new QLabel("Isotope (U-235):"), 1, 4);
    spinIso = new QDoubleSpinBox; spinIso->setRange(0, 1000000); spinIso->setDecimals(0);
    grid->addWidget(spinIso, 1, 5);
    
    // Row 2
    grid->addWidget(new QLabel("Items (if applicable):"), 2, 0);
    QDoubleSpinBox *spinItems = new QDoubleSpinBox; 
    spinItems->setRange(0, 1000); spinItems->setDecimals(0);
    spinItems->setObjectName("spinItems"); 
    grid->addWidget(spinItems, 2, 1);

    // --- BUTTONS ALL GROUPED TOGETHER ---
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addStretch(); // Pushes buttons to the right to look neat
    
    QPushButton *btnAdd = new QPushButton("Add Entry");
    btnAdd->setStyleSheet("background-color: #0056b3; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px; border: none;");
    connect(btnAdd, &QPushButton::clicked, this, &GeneralLedgerWidget::addEntry);
    
    QPushButton *btnExport = new QPushButton("Export PDF");
    btnExport->setStyleSheet("background-color: #074282; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px; border: none;");
    connect(btnExport, &QPushButton::clicked, this, &GeneralLedgerWidget::exportPDF);

    QPushButton *btnDel = new QPushButton("Delete Selected");
    btnDel->setStyleSheet("background-color: #c0392b; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px; border: none;");
    connect(btnDel, &QPushButton::clicked, this, &GeneralLedgerWidget::deleteEntry);

    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnExport);
    btnLayout->addWidget(btnDel);
    
    grid->addLayout(btnLayout, 2, 2, 1, 4); // Spans across the remaining columns

    layout->addWidget(grp);
}

void GeneralLedgerWidget::setupComplexTable(QVBoxLayout *layout) {
    table = new QTableWidget;
    table->setColumnCount(16);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows); // Select full rows for deletion

    table->insertRow(0);
    table->insertRow(1);
    table->insertRow(2);

    auto setH = [&](int r, int c, QString txt) {
        QTableWidgetItem *it = new QTableWidgetItem(txt);
        it->setTextAlignment(Qt::AlignCenter);
        it->setBackground(QColor("#f0f0f0"));
        it->setFont(QFont("Arial", 9, QFont::Bold));
        table->setItem(r, c, it);
    };

    setH(0, 0, "Line"); table->setSpan(0, 0, 3, 1);
    setH(0, 1, "Date"); table->setSpan(0, 1, 3, 1);
    setH(0, 2, "ICD/PIL"); table->setSpan(0, 2, 3, 1);
    setH(0, 3, "IC Code"); table->setSpan(0, 3, 3, 1);
    setH(0, 4, "No. of\nItems"); table->setSpan(0, 4, 3, 1);

    setH(0, 5, "Increases"); table->setSpan(0, 5, 1, 4);
    setH(0, 9, "Decreases"); table->setSpan(0, 9, 1, 4);
    setH(0, 13, "Inventory"); table->setSpan(0, 13, 1, 2);
    setH(0, 15, "No. of\nitems"); table->setSpan(0, 15, 3, 1);

    setH(1, 5, "Receipts"); table->setSpan(1, 5, 1, 2);
    setH(1, 7, "Other");    table->setSpan(1, 7, 1, 2);
    setH(1, 9, "Shipments"); table->setSpan(1, 9, 1, 2);
    setH(1, 11, "Other");   table->setSpan(1, 11, 1, 2);
    setH(1, 13, "Bal");     table->setSpan(1, 13, 1, 2); 

    for(int i : {5, 7, 9, 11, 13}) setH(2, i, "U");
    for(int i : {6, 8, 10, 12, 14}) setH(2, i, "U-235");

    table->setColumnWidth(0, 40); 
    table->setColumnWidth(1, 80); 
    table->setColumnWidth(3, 60); 
    table->setColumnWidth(4, 50); 

    layout->addWidget(table);
}

void GeneralLedgerWidget::addEntry() {
    QMap<QString, QVariant> data;
    data["date"] = dateEdit->text();
    data["ref"] = txtRef->text();
    data["code"] = comboCode->currentText();
    data["type"] = comboType->currentText();
    data["u_weight"] = spinElem->value();
    data["u235_weight"] = spinIso->value();
    
    QDoubleSpinBox *spinItems = this->findChild<QDoubleSpinBox*>("spinItems");
    data["items"] = spinItems ? (int)spinItems->value() : 0;

    if(DatabaseManager::instance().addManualLedgerEntry(data)) {
        refreshData(); 
        emit dataChanged(); 
        txtRef->clear(); spinElem->setValue(0); spinIso->setValue(0);
        if(spinItems) spinItems->setValue(0);
    }
}

void GeneralLedgerWidget::deleteEntry() {
    int row = table->currentRow();
    
    // Rows 0, 1, 2 are headers, so we cannot delete them.
    if(row < 3) {
        QMessageBox::warning(this, "Selection Error", "Please select a data row to delete (not the headers).");
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

    // Retrieve the Hidden ID we stored in Column 1 (Date Column)
    QTableWidgetItem *item = table->item(row, 1);
    if(!item) return;
    
    int id = item->data(Qt::UserRole).toInt();
    if(id == 0) return; // Should not happen if data loaded correctly

    // Confirm Deletion
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Confirm Deletion", 
        "Are you sure you want to delete this ledger entry?\nThis action cannot be undone.",
        QMessageBox::Yes|QMessageBox::No);

    if(reply == QMessageBox::Yes) {
        if(DatabaseManager::instance().deleteManualLedgerEntry(id)) {
            refreshData();  // Refresh the table
            emit dataChanged(); // Notify MainWindow to refresh Home Dashboard
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete entry from database.");
        }
    }
}

void GeneralLedgerWidget::refreshData() {
    table->setRowCount(3); 
    balU = 0; balU235 = 0; balItems = 0; lineCounter = 1;

    QSqlQuery q = DatabaseManager::instance().getManualLedgerEntries();
    while(q.next()) {
        int r = table->rowCount();
        table->insertRow(r);

        int dbID = q.value("id").toInt(); // GET ID FROM DB
        QString type = q.value("type").toString();
        double u = q.value("u_weight").toDouble();
        double u235 = q.value("u235_weight").toDouble();
        int items = q.value("items").toInt();

        // LOGIC START
        if(type == "Receipt") {
            balU += u; balU235 += u235; balItems += items; 
        } 
        else if (type == "Shipment") {
            balU -= u; balU235 -= u235; balItems -= items; 
        }
        else if (type == "Other Increase") {
            balU += u; balU235 += u235; 
        }
        else if (type == "Other Decrease" || type == "Nuclear Loss") {
            balU -= u; balU235 -= u235; 
        }
        else if (type == "PIL (Set Balance)") {
            if(lineCounter == 1 && balU == 0) { 
                balU = u; balU235 = u235; balItems = items;
            }
        }
        // LOGIC END

        table->setItem(r, 0, new QTableWidgetItem(QString::number(lineCounter++)));
        
        // STORE HIDDEN ID IN COLUMN 1 (Date)
        QTableWidgetItem *dateItem = new QTableWidgetItem(q.value("date").toString());
        dateItem->setData(Qt::UserRole, dbID); // <--- CRITICAL: Store ID here
        table->setItem(r, 1, dateItem);

        table->setItem(r, 2, new QTableWidgetItem(q.value("ref").toString()));
        table->setItem(r, 3, new QTableWidgetItem(q.value("code").toString()));
        
        QString displayItems = "";
        if(type == "Receipt" || type == "Shipment") {
             displayItems = (items > 0 ? QString::number(items) : "");
        }
        table->setItem(r, 4, new QTableWidgetItem(displayItems));

        for(int i=5; i<=12; i++) table->setItem(r, i, new QTableWidgetItem(""));

        if(type == "Receipt") {
            table->setItem(r, 5, new QTableWidgetItem(QString::number(u)));
            table->setItem(r, 6, new QTableWidgetItem(QString::number(u235)));
        } else if (type == "Other Increase") {
            table->setItem(r, 7, new QTableWidgetItem(QString::number(u)));
            table->setItem(r, 8, new QTableWidgetItem(QString::number(u235)));
        } else if (type == "Shipment") {
            table->setItem(r, 9, new QTableWidgetItem(QString::number(u)));
            table->setItem(r, 10, new QTableWidgetItem(QString::number(u235)));
        } else if (type == "Other Decrease" || type == "Nuclear Loss") {
            table->setItem(r, 11, new QTableWidgetItem(QString::number(u)));
            table->setItem(r, 12, new QTableWidgetItem(QString::number(u235)));
        }

        QTableWidgetItem *b1 = new QTableWidgetItem(QString::number(balU));
        b1->setBackground(QColor("#e8f5e9")); table->setItem(r, 13, b1);
        
        QTableWidgetItem *b2 = new QTableWidgetItem(QString::number(balU235));
        b2->setBackground(QColor("#e8f5e9")); table->setItem(r, 14, b2);

        QTableWidgetItem *b3 = new QTableWidgetItem(QString::number(balItems));
        b3->setBackground(QColor("#e8f5e9")); table->setItem(r, 15, b3);
    }
}

void GeneralLedgerWidget::exportPDF() {
    // --- ZERO TRUST VERIFICATION (TRAINING MODE ONLY) ---
    if (DatabaseManager::instance().currentDatabaseName().contains("AIR_Training")) {
        PinDialog authDialog(this);
        if (authDialog.exec() != QDialog::Accepted) {
            qDebug() << "Zero Trust Policy: Export blocked.";
            return; 
        }
    }
    // ----------------------------------------------------

    QString fileName = QFileDialog::getSaveFileName(this, "Save General Ledger", "GL_Report.pdf", "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QMap<QString, QString> header;
    header["facility"] = txtFacility->text();
    header["mba"] = comboMBA->currentText();
    header["desc"] = txtMatDesc->text();
    header["elemCode"] = txtElemCode->text();
    header["isoCode"] = txtIsoCode->text();
    header["unit"] = txtUnit->text();

    QSqlQuery data = DatabaseManager::instance().getManualLedgerEntries();

    if (ReportGenerator::generateGL_PDF(fileName, header, data)) {
        QMessageBox::information(this, "Success", "Report saved successfully.");
    } else {
        QMessageBox::critical(this, "Error", "Failed to save report.");
    }
}
