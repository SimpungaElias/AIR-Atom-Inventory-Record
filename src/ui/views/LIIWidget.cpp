#include "LIIWidget.h"
#include "MaterialCodeDialog.h" 
#include "PinDialog.h"
#include "../../db/DatabaseManager.h"
#include "../../utils/ReportGenerator.h"
#include <QHeaderView>
#include <QGridLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QDate>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVariant>
#include <QMap>
#include <QDebug> // Added for qDebug()

LIIWidget::LIIWidget(QWidget *parent) : QWidget(parent), itemCounter(1) {
    setupUI();
    loadData(); 
}

void LIIWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    QLabel *title = new QLabel("<h2>List of Inventory Items (LII) - Operations</h2>");
    title->setStyleSheet("color: #003366;");
    mainLayout->addWidget(title);

    setupReportHeader(mainLayout);
    setupInputForm(mainLayout);
    setupTable(mainLayout);
}

void LIIWidget::setupReportHeader(QVBoxLayout *layout) {
    QGroupBox *grp = new QGroupBox("Report Header Details");
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
    spinReportNo = new QSpinBox;
    spinReportNo->setRange(1, 9999);
    grid->addWidget(spinReportNo, 1, 3);

    // Row 2
    grid->addWidget(new QLabel("MBA:"), 2, 0);
    txtMBA = new QLineEdit("CRRF");
    grid->addWidget(txtMBA, 2, 1);

    layout->addWidget(grp);
}

void LIIWidget::setupInputForm(QVBoxLayout *layout) {
    QGroupBox *grp = new QGroupBox("Add Inventory Item");
    grp->setStyleSheet("QGroupBox { border: 1px solid #003366; font-weight: bold; margin-top: 10px; }");
    QGridLayout *grid = new QGridLayout(grp);

    // Row 0
    grid->addWidget(new QLabel("KMP:"), 0, 0);
    comboKMP = new QComboBox; comboKMP->addItems({"FFS", "RRC", "SFS", "LOF"}); 
    grid->addWidget(comboKMP, 0, 1);

    grid->addWidget(new QLabel("Position:"), 0, 2);
    txtPosition = new QLineEdit; txtPosition->setPlaceholderText("e.g. A01");
    grid->addWidget(txtPosition, 0, 3);

    grid->addWidget(new QLabel("Batch/Item:"), 0, 4);
    txtBatch = new QLineEdit;
    grid->addWidget(txtBatch, 0, 5);

    // Row 1 --- MATERIAL CODE SELECTOR ---
    grid->addWidget(new QLabel("Mat. Code (430):"), 1, 0);
    
    QHBoxLayout *codeLayout = new QHBoxLayout();
    txtMaterialCode = new QLineEdit;
    txtMaterialCode->setPlaceholderText("Click Select ->");
    txtMaterialCode->setReadOnly(true); 
    txtMaterialCode->setStyleSheet("background-color: #f0f0f0;");
    
    QPushButton *btnSelectCode = new QPushButton("Select...");
    btnSelectCode->setCursor(Qt::PointingHandCursor);
    connect(btnSelectCode, &QPushButton::clicked, this, &LIIWidget::openMaterialCodeSelector);

    codeLayout->addWidget(txtMaterialCode);
    codeLayout->addWidget(btnSelectCode);
    
    grid->addLayout(codeLayout, 1, 1, 1, 3);

    // Row 2
    grid->addWidget(new QLabel("Element (g):"), 2, 0);
    spinElem = new QDoubleSpinBox; spinElem->setRange(0, 1000000); spinElem->setDecimals(2);
    grid->addWidget(spinElem, 2, 1);

    grid->addWidget(new QLabel("Fissile (g):"), 2, 2);
    spinFissile = new QDoubleSpinBox; spinFissile->setRange(0, 1000000); spinFissile->setDecimals(3);
    grid->addWidget(spinFissile, 2, 3);

    grid->addWidget(new QLabel("Plutonium (g):"), 2, 4);
    spinPu = new QDoubleSpinBox; spinPu->setRange(0, 1000000); spinPu->setDecimals(3);
    grid->addWidget(spinPu, 2, 5);

    // Row 3
    grid->addWidget(new QLabel("Burnup:"), 3, 0);
    spinBurnup = new QDoubleSpinBox; spinBurnup->setRange(0, 100000);
    grid->addWidget(spinBurnup, 3, 1);

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout;

    QPushButton *btnAdd = new QPushButton("Add Entry");
    btnAdd->setStyleSheet("background-color: #0056b3; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px; border: none;");
    connect(btnAdd, &QPushButton::clicked, this, &LIIWidget::addItem);
    
    QPushButton *btnExport = new QPushButton("Export PDF");
    btnExport->setStyleSheet("background-color: #074282; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px; border: none;");
    connect(btnExport, &QPushButton::clicked, this, &LIIWidget::exportPDF);

    QPushButton *btnDel = new QPushButton("Delete Selected");
    btnDel->setStyleSheet("background-color: #c0392b; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px; border: none;");
    connect(btnDel, &QPushButton::clicked, this, &LIIWidget::deleteItem);

    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnExport);
    btnLayout->addWidget(btnDel); 

    grid->addLayout(btnLayout, 3, 4, 1, 2);

    layout->addWidget(grp);
}

void LIIWidget::openMaterialCodeSelector() {
    MaterialCodeDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        txtMaterialCode->setText(dlg.getSelectedCode());
    }
}

void LIIWidget::setupTable(QVBoxLayout *layout) {
    table = new QTableWidget;
    QStringList headers;
    
    // --- CHANGED: Removed "Fuel Type" ---
    headers << "KMP" << "Position" << "Item" << "Batch" << "Code (430)" 
            << "Elem (g)" << "Fissile (g)" << "Pu (g)" << "Th (g)" << "Burnup" << "Cooling";
    
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows); 
    
    layout->addWidget(table);
}

void LIIWidget::addItem() {
    if(txtMaterialCode->text().isEmpty()) {
        QMessageBox::warning(this, "Missing Data", "Please select a Material Description Code.");
        return;
    }

    QMap<QString, QVariant> data;
    data["kmp"] = comboKMP->currentText();
    data["position"] = txtPosition->text();
    data["batch"] = txtBatch->text();
    data["desc"] = txtMaterialCode->text(); 
    data["weight_elem"] = spinElem->value();
    data["weight_fissile"] = spinFissile->value();
    data["weight_pu"] = spinPu->value();
    data["burnup"] = spinBurnup->value();

    if(DatabaseManager::instance().addLIIEntry(data)) {
        loadData(); 
        
        txtPosition->clear(); txtBatch->clear(); txtMaterialCode->clear();
        spinElem->setValue(0); spinFissile->setValue(0); spinPu->setValue(0); spinBurnup->setValue(0);
    } else {
        QMessageBox::critical(this, "Error", "Failed to save entry to database.");
    }
}

void LIIWidget::deleteItem() {
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

    int id = table->item(row, 0)->data(Qt::UserRole).toInt();

    if(QMessageBox::question(this, "Confirm", "Delete this inventory item?", QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        if(DatabaseManager::instance().deleteLIIEntry(id)) {
            loadData(); 
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete item from database.");
        }
    }
}

void LIIWidget::loadData() {
    table->setRowCount(0);
    
    QSqlQuery q = DatabaseManager::instance().getLIIEntries();
    while(q.next()) {
        int r = table->rowCount();
        table->insertRow(r);
        
        int dbID = q.value("id").toInt(); 

        QTableWidgetItem *itemKMP = new QTableWidgetItem(q.value("kmp").toString());
        itemKMP->setData(Qt::UserRole, dbID); 
        table->setItem(r, 0, itemKMP);

        table->setItem(r, 1, new QTableWidgetItem(q.value("position").toString()));
        table->setItem(r, 2, new QTableWidgetItem("1")); 
        table->setItem(r, 3, new QTableWidgetItem(q.value("batch").toString()));
        
        // --- CHANGED: Removed the "Fuel Type" placeholder ("-") ---
        // Indices shifted down by 1
        
        table->setItem(r, 4, new QTableWidgetItem(q.value("desc").toString())); 
        
        table->setItem(r, 5, new QTableWidgetItem(QString::number(q.value("weight_elem").toDouble(), 'f', 2)));
        table->setItem(r, 6, new QTableWidgetItem(QString::number(q.value("weight_fissile").toDouble(), 'f', 3)));
        table->setItem(r, 7, new QTableWidgetItem(QString::number(q.value("weight_pu").toDouble(), 'f', 3)));
        table->setItem(r, 8, new QTableWidgetItem("0.0")); 
        table->setItem(r, 9, new QTableWidgetItem(QString::number(q.value("burnup").toDouble())));
        table->setItem(r, 10, new QTableWidgetItem("-"));
    }
}

void LIIWidget::exportPDF() {
    if(table->rowCount() == 0) {
        QMessageBox::warning(this, "Export Error", "The list is empty. Add items first.");
        return;
    }

    // --- ZERO TRUST VERIFICATION (TRAINING MODE ONLY) ---
    if (DatabaseManager::instance().currentDatabaseName().contains("AIR_Training")) {
        PinDialog authDialog(this);
        if (authDialog.exec() != QDialog::Accepted) {
            qDebug() << "Zero Trust Policy: Export blocked.";
            return; 
        }
    }
    // ----------------------------------------------------

    QString fileName = QFileDialog::getSaveFileName(this, "Save LII Report", "LII_Report.pdf", "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QMap<QString, QString> headerData;
    headerData["country"] = txtCountry->text();
    headerData["facility"] = txtFacility->text();
    headerData["mba"] = txtMBA->text();
    headerData["date"] = dateReport->text();
    headerData["reportNo"] = QString::number(spinReportNo->value());

    { 
        QSqlDatabase tempDb = QSqlDatabase::addDatabase("QSQLITE", "TempLIIExport");
        tempDb.setDatabaseName(":memory:");
        if (!tempDb.open()) return;

        QSqlQuery q(tempDb);
        q.exec("CREATE TABLE temp_batches ("
               "kmp TEXT, position TEXT, batch TEXT, desc TEXT, "
               "weight_elem REAL, weight_fissile REAL, weight_pu REAL, burnup REAL)");

        for(int i=0; i<table->rowCount(); i++) {
            q.prepare("INSERT INTO temp_batches VALUES (?, ?, ?, ?, ?, ?, ?, ?)");
            // Adjust indices based on new table structure (No Fuel Type column)
            q.addBindValue(table->item(i, 0)->text()); // KMP
            q.addBindValue(table->item(i, 1)->text()); // Pos
            q.addBindValue(table->item(i, 3)->text()); // Batch
            q.addBindValue(table->item(i, 4)->text()); // Desc (Was 5)
            q.addBindValue(table->item(i, 5)->text().toDouble()); // Elem (Was 6)
            q.addBindValue(table->item(i, 6)->text().toDouble()); // Fissile (Was 7)
            q.addBindValue(table->item(i, 7)->text().toDouble()); // Pu (Was 8)
            q.addBindValue(table->item(i, 9)->text().toDouble()); // Burnup (Was 10)
            q.exec();
        }

        QSqlQuery reportQuery(tempDb);
        reportQuery.exec("SELECT * FROM temp_batches ORDER BY kmp ASC, batch ASC");

        if (ReportGenerator::generateLII_PDF(fileName, headerData, reportQuery)) {
            QMessageBox::information(this, "Success", "LII Report generated successfully.");
        } else {
            QMessageBox::critical(this, "Error", "Failed to generate PDF.");
        }
    }
    QSqlDatabase::removeDatabase("TempLIIExport");
}
