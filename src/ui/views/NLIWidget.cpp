#include "NLIWidget.h"
#include "PinDialog.h" // <--- ZTA DIALOG INCLUDED
#include "../../db/DatabaseManager.h"
#include "../../utils/ReportGenerator.h"
#include <QHeaderView>
#include <QGridLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QDate>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QDebug>

NLIWidget::NLIWidget(QWidget *parent) : QWidget(parent), lineCounter(1) {
    setupUI();
    loadData();
}

void NLIWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(10);

    QLabel *title = new QLabel("<h2>Nuclear Loss Items (NLI) - Operations</h2>");
    title->setStyleSheet("color: #003366;");
    mainLayout->addWidget(title);

    setupReportHeader(mainLayout);
    setupInputForm(mainLayout);
    setupTable(mainLayout);
}

void NLIWidget::setupReportHeader(QVBoxLayout *layout) {
    QGroupBox *grp = new QGroupBox("Report Header Details");
    grp->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #aaa; }");
    QGridLayout *grid = new QGridLayout(grp);

    grid->addWidget(new QLabel("Country:"), 0, 0);
    txtCountry = new QLineEdit("AAUSTRIA");
    grid->addWidget(txtCountry, 0, 1);

    grid->addWidget(new QLabel("Date:"), 0, 2);
    dateReport = new QDateEdit(QDate::currentDate());
    dateReport->setDisplayFormat("yyyy-MM-dd");
    grid->addWidget(dateReport, 0, 3);

    grid->addWidget(new QLabel("Facility:"), 1, 0);
    txtFacility = new QLineEdit("Compton Research Reactor");
    grid->addWidget(txtFacility, 1, 1);

    grid->addWidget(new QLabel("Report No:"), 1, 2);
    txtReportNo = new QLineEdit("1");
    grid->addWidget(txtReportNo, 1, 3);

    grid->addWidget(new QLabel("MBA:"), 2, 0);
    txtMBA = new QLineEdit("CRRF");
    grid->addWidget(txtMBA, 2, 1);

    layout->addWidget(grp);
}

void NLIWidget::setupInputForm(QVBoxLayout *layout) {
    QGroupBox *grp = new QGroupBox("Add Transaction");
    grp->setStyleSheet("QGroupBox { border: 1px solid #003366; font-weight: bold; margin-top: 10px; }");
    QGridLayout *grid = new QGridLayout(grp);

    // Row 0: Basic Info
    grid->addWidget(new QLabel("Batch Identity:"), 0, 0);
    txtBatch = new QLineEdit;
    grid->addWidget(txtBatch, 0, 1);

    grid->addWidget(new QLabel("No. Items:"), 0, 2);
    spinItems = new QSpinBox; spinItems->setValue(1);
    grid->addWidget(spinItems, 0, 3);

    grid->addWidget(new QLabel("Change Code:"), 0, 4);
    comboCode = new QComboBox; comboCode->addItems({"LN", "LD", "LA"});
    grid->addWidget(comboCode, 0, 5);

    // Row 1: Uranium
    grid->addWidget(new QLabel("<b>Uranium</b>"), 1, 0);
    
    QHBoxLayout *uLay = new QHBoxLayout;
    txtUElemCode = new QLineEdit("E"); txtUElemCode->setPlaceholderText("Elem Code"); uLay->addWidget(txtUElemCode);
    txtUIsoCode = new QLineEdit("G"); txtUIsoCode->setPlaceholderText("Iso Code"); uLay->addWidget(txtUIsoCode);
    uLay->addWidget(new QLabel("Wt:"));
    spinUWeight = new QDoubleSpinBox; spinUWeight->setRange(0, 100000); uLay->addWidget(spinUWeight);
    uLay->addWidget(new QLabel("Iso Wt:"));
    spinUIsoWeight = new QDoubleSpinBox; spinUIsoWeight->setRange(0, 100000); uLay->addWidget(spinUIsoWeight);
    grid->addLayout(uLay, 1, 1, 1, 5);

    // Row 2: Plutonium
    grid->addWidget(new QLabel("<b>Plutonium</b>"), 2, 0);
    
    QHBoxLayout *pLay = new QHBoxLayout;
    txtPElemCode = new QLineEdit("P"); txtPElemCode->setPlaceholderText("Elem Code"); pLay->addWidget(txtPElemCode);
    pLay->addWidget(new QLabel("Wt:"));
    spinPWeight = new QDoubleSpinBox; spinPWeight->setRange(0, 100000); pLay->addWidget(spinPWeight);
    grid->addLayout(pLay, 2, 1, 1, 5);

    // Buttons
    QHBoxLayout *btnLay = new QHBoxLayout;
    QPushButton *btnAdd = new QPushButton("Add Entry");
    btnAdd->setStyleSheet("background-color: #0056b3; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px; border: none;");
    connect(btnAdd, &QPushButton::clicked, this, &NLIWidget::addEntry);

    QPushButton *btnExport = new QPushButton("Export PDF");
    btnExport->setStyleSheet("background-color: #074282; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px; border: none;");
    connect(btnExport, &QPushButton::clicked, this, &NLIWidget::exportPDF);

    QPushButton *btnDelete = new QPushButton("Delete Selected");
    btnDelete->setStyleSheet("background-color: #c0392b; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px; border: none;");
    connect(btnDelete, &QPushButton::clicked, this, &NLIWidget::deleteEntry);

    btnLay->addWidget(btnAdd);
    btnLay->addWidget(btnExport);
    btnLay->addWidget(btnDelete); 
    grid->addLayout(btnLay, 3, 0, 1, 6);

    layout->addWidget(grp);
}

void NLIWidget::setupTable(QVBoxLayout *layout) {
    table = new QTableWidget;
    table->setColumnCount(10);
    QStringList headers;
    headers << "Line" << "Batch" << "Items" << "Code" 
            << "U Elem" << "U Iso" << "U Wt" << "U Iso Wt"
            << "P Elem" << "P Wt";
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    
    layout->addWidget(table);
}

void NLIWidget::loadData() {
    table->setRowCount(0);
    lineCounter = 1;
    
    QSqlQuery q = DatabaseManager::instance().getNLIEntries();
    while(q.next()) {
        int r = table->rowCount();
        table->insertRow(r);
        
        int dbID = q.value("id").toInt(); 

        QTableWidgetItem *itemLine = new QTableWidgetItem(QString::number(lineCounter++));
        itemLine->setData(Qt::UserRole, dbID); 
        table->setItem(r, 0, itemLine);

        table->setItem(r, 1, new QTableWidgetItem(q.value("batch").toString()));
        table->setItem(r, 2, new QTableWidgetItem(q.value("items").toString()));
        table->setItem(r, 3, new QTableWidgetItem(q.value("code").toString()));
        table->setItem(r, 4, new QTableWidgetItem(q.value("u_elem_code").toString()));
        table->setItem(r, 5, new QTableWidgetItem(q.value("u_iso_code").toString()));
        table->setItem(r, 6, new QTableWidgetItem(q.value("u_weight").toString()));
        table->setItem(r, 7, new QTableWidgetItem(q.value("u_iso_weight").toString()));
        table->setItem(r, 8, new QTableWidgetItem(q.value("p_elem_code").toString()));
        table->setItem(r, 9, new QTableWidgetItem(q.value("p_weight").toString()));
    }
}

void NLIWidget::addEntry() {
    QMap<QString, QVariant> data;
    data["batch"] = txtBatch->text();
    data["items"] = spinItems->value();
    data["code"] = comboCode->currentText();
    data["u_elem_code"] = txtUElemCode->text();
    data["u_iso_code"] = txtUIsoCode->text();
    data["u_weight"] = spinUWeight->value();
    data["u_iso_weight"] = spinUIsoWeight->value();
    data["p_elem_code"] = txtPElemCode->text();
    data["p_weight"] = spinPWeight->value();

    if(DatabaseManager::instance().addNLIEntry(data)) {
        loadData(); 
        txtBatch->clear();
    }
}

void NLIWidget::deleteEntry() {
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

    if(QMessageBox::question(this, "Confirm", "Delete this nuclear loss entry?", QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        if(DatabaseManager::instance().deleteNLIEntry(id)) {
            loadData(); 
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete entry from database.");
        }
    }
}

void NLIWidget::exportPDF() {
    if(table->rowCount() == 0) return;

    // --- ZERO TRUST VERIFICATION (TRAINING MODE ONLY) ---
    if (DatabaseManager::instance().currentDatabaseName().contains("AIR_Training")) {
        PinDialog authDialog(this);
        if (authDialog.exec() != QDialog::Accepted) {
            qDebug() << "Zero Trust Policy: Export blocked.";
            return; 
        }
    }
    // ----------------------------------------------------

    QString fileName = QFileDialog::getSaveFileName(this, "Save NLI Report", "NLI_Report.pdf", "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QMap<QString, QString> header;
    header["country"] = txtCountry->text();
    header["facility"] = txtFacility->text();
    header["mba"] = txtMBA->text();
    header["date"] = dateReport->text();
    header["reportNo"] = txtReportNo->text();

    {
        QSqlDatabase tempDb = QSqlDatabase::addDatabase("QSQLITE", "TempNLI");
        tempDb.setDatabaseName(":memory:");
        if (!tempDb.open()) return;

        QSqlQuery q(tempDb);
        q.exec("CREATE TABLE temp_nli ("
               "batch TEXT, items INTEGER, code TEXT, "
               "u_elem_code TEXT, u_iso_code TEXT, u_weight REAL, u_iso_weight REAL, "
               "p_elem_code TEXT, p_weight REAL)");

        for(int i=0; i<table->rowCount(); i++) {
            q.prepare("INSERT INTO temp_nli VALUES (?,?,?,?,?,?,?,?,?)");
            q.addBindValue(table->item(i, 1)->text()); 
            q.addBindValue(table->item(i, 2)->text().toInt()); 
            q.addBindValue(table->item(i, 3)->text()); 
            q.addBindValue(table->item(i, 4)->text()); 
            q.addBindValue(table->item(i, 5)->text()); 
            q.addBindValue(table->item(i, 6)->text().toDouble()); 
            q.addBindValue(table->item(i, 7)->text().toDouble()); 
            q.addBindValue(table->item(i, 8)->text()); 
            q.addBindValue(table->item(i, 9)->text().toDouble()); 
            q.exec();
        }

        QSqlQuery pq(tempDb);
        pq.exec("SELECT * FROM temp_nli");
        
        if (ReportGenerator::generateNLI_PDF(fileName, header, pq)) {
            QMessageBox::information(this, "Success", "Report saved.");
        }
    }
    QSqlDatabase::removeDatabase("TempNLI");
}
