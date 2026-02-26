#include "MBRWidget.h"
#include "PinDialog.h"
#include "../../db/DatabaseManager.h"
#include "../../utils/ReportGenerator.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QHeaderView>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>

MBRWidget::MBRWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
}

void MBRWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(15);
    
    QLabel *title = new QLabel("<h2>Material Balance Report</h2>");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: #003366; font-weight: bold; margin-bottom: 5px;");
    mainLayout->addWidget(title);

    setupHeader();
    setupInputForm(); // Add the input form (which now includes Export PDF)
    setupTable();

    loadData();
}

void MBRWidget::setupHeader() {
    QGroupBox *headerGrp = new QGroupBox("Report Details");
    headerGrp->setStyleSheet("QGroupBox { font-weight: bold; border: 1px solid #ccc; }");
    QGridLayout *headerLayout = new QGridLayout(headerGrp);
    headerLayout->setHorizontalSpacing(15);
    headerLayout->setVerticalSpacing(10);
    
    txtCountry = new QLineEdit("NN");
    txtFacility = new QLineEdit("Compton Research Reactor");
    comboMBA = new QComboBox();
    comboMBA->addItems({"CRRF", "EULE", "EXT", "DKNZ"}); 
    
    dateFrom = new QDateEdit(QDate::currentDate().addMonths(-1));
    dateFrom->setCalendarPopup(true);
    dateFrom->setDisplayFormat("yyyy-MM-dd");

    dateTo = new QDateEdit(QDate::currentDate());
    dateTo->setCalendarPopup(true);
    dateTo->setDisplayFormat("yyyy-MM-dd");

    txtReportNo = new QLineEdit();

    headerLayout->addWidget(new QLabel("Country:"), 0, 0);
    headerLayout->addWidget(txtCountry, 0, 1);
    
    headerLayout->addWidget(new QLabel("Reporting Period From:"), 0, 2);
    headerLayout->addWidget(dateFrom, 0, 3);
    headerLayout->addWidget(new QLabel("To:"), 0, 4);
    headerLayout->addWidget(dateTo, 0, 5);

    headerLayout->addWidget(new QLabel("Facility:"), 1, 0);
    headerLayout->addWidget(txtFacility, 1, 1, 1, 3); 

    headerLayout->addWidget(new QLabel("Report No:"), 1, 4);
    headerLayout->addWidget(txtReportNo, 1, 5);

    headerLayout->addWidget(new QLabel("Material Balance Area:"), 2, 0);
    headerLayout->addWidget(comboMBA, 2, 1);

    static_cast<QVBoxLayout*>(layout())->addWidget(headerGrp);
}

void MBRWidget::setupInputForm() {
    QGroupBox *grp = new QGroupBox("Add Accountancy Entry");
    grp->setStyleSheet("QGroupBox { border: 1px solid #003366; font-weight: bold; margin-top: 5px; }");
    QGridLayout *grid = new QGridLayout(grp);
    grid->setSpacing(10);

    // Row 0
    grid->addWidget(new QLabel("Entry Name:"), 0, 0);
    comboEntryName = new QComboBox();
    comboEntryName->addItems({"PB", "RD", "LN", "SF", "SD", "BA", "PE", "NP", "MF"});
    grid->addWidget(comboEntryName, 0, 1);

    grid->addWidget(new QLabel("Element:"), 0, 2);
    comboElement = new QComboBox();
    comboElement->addItems({"E", "N", "D", "P", "T"}); // Enriched, Natural, Depleted, Plutonium, Thorium
    grid->addWidget(comboElement, 0, 3);

    grid->addWidget(new QLabel("Weight of Element:"), 0, 4);
    spinWeight = new QDoubleSpinBox();
    spinWeight->setRange(-99999999, 99999999); // Allow negatives
    spinWeight->setDecimals(2);
    grid->addWidget(spinWeight, 0, 5);

    // Row 1
    grid->addWidget(new QLabel("Unit (Kg/g):"), 1, 0);
    comboUnit = new QComboBox();
    comboUnit->addItems({"G", "KG"});
    grid->addWidget(comboUnit, 1, 1);

    grid->addWidget(new QLabel("Fissile Weight (g):"), 1, 2);
    spinFissile = new QDoubleSpinBox();
    spinFissile->setRange(-99999999, 99999999);
    spinFissile->setDecimals(2);
    grid->addWidget(spinFissile, 1, 3);

    grid->addWidget(new QLabel("Isotope Code:"), 1, 4);
    txtIsotopeCode = new QLineEdit("G"); // Default G as per image
    grid->addWidget(txtIsotopeCode, 1, 5);

    // Row 2
    grid->addWidget(new QLabel("Continuation:"), 2, 0);
    txtContinuation = new QLineEdit();
    grid->addWidget(txtContinuation, 2, 1);

    grid->addWidget(new QLabel("Report No:"), 2, 2);
    txtEntryReportNo = new QLineEdit();
    grid->addWidget(txtEntryReportNo, 2, 3);

    // --- BUTTONS ALL GROUPED TOGETHER ---
    QHBoxLayout *btnLayout = new QHBoxLayout();
    
    QPushButton *btnAdd = new QPushButton("Add Entry");
    btnAdd->setStyleSheet("background-color: #0056b3; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px;");
    connect(btnAdd, &QPushButton::clicked, this, &MBRWidget::addEntry);
    
        btnExport = new QPushButton("Export PDF");
    btnExport->setStyleSheet("background-color: #074282; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px;");
    connect(btnExport, &QPushButton::clicked, this, &MBRWidget::exportPDF);

    QPushButton *btnDel = new QPushButton("Delete Selected");
    btnDel->setStyleSheet("background-color: #c0392b; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px;");
    connect(btnDel, &QPushButton::clicked, this, &MBRWidget::deleteEntry);

    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnExport);
    btnLayout->addWidget(btnDel);
    
    
    grid->addLayout(btnLayout, 2, 4, 1, 2);

    static_cast<QVBoxLayout*>(layout())->addWidget(grp);
}

void MBRWidget::setupTable() {
    table = new QTableWidget();
    table->setColumnCount(9);
    
    table->setHorizontalHeaderLabels({
        "Entry No.", "Continuation", "Entry\nName", 
        "Element", "Weight of Element", "Unit Kg/g", 
        "Weight Of Fissile\nIsotopes\n(Uranium Only)\n(G)", "Isotope Code", 
        "Report\nNo"
    });
    
    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setStyleSheet("QHeaderView::section { background-color: #f0f0f0; font-weight: bold; border: 1px solid #ccc; }"
                         "QTableWidget { border: 1px solid #ccc; }");

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    table->setColumnWidth(0, 70); 
    table->setColumnWidth(1, 90); 
    table->setColumnWidth(2, 60); 
    table->setColumnWidth(3, 60); 
    table->setColumnWidth(5, 70); 
    table->setColumnWidth(7, 90); 
    table->setColumnWidth(8, 70); 
    table->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Stretch); 
    table->horizontalHeader()->setSectionResizeMode(6, QHeaderView::Stretch); 

    layout()->addWidget(table);
}

void MBRWidget::addEntry() {
    QMap<QString, QVariant> data;
    data["continuation"] = txtContinuation->text();
    data["entry_name"] = comboEntryName->currentText();
    data["element"] = comboElement->currentText();
    data["weight"] = spinWeight->value();
    data["unit"] = comboUnit->currentText();
    data["fissile"] = spinFissile->value();
    data["isotope"] = txtIsotopeCode->text();
    data["report_no"] = txtEntryReportNo->text();

    if (DatabaseManager::instance().addMBREntry(data)) {
        loadData();
        spinWeight->setValue(0);
        spinFissile->setValue(0);
        emit dataChanged(); 
    } else {
        QMessageBox::critical(this, "Error", "Failed to save to database.");
    }
}

void MBRWidget::deleteEntry() {
    int row = table->currentRow();
    if (row < 0) return;

    // --- ZERO TRUST VERIFICATION (TRAINING MODE ONLY) ---
    if (DatabaseManager::instance().currentDatabaseName().contains("AIR_Training")) {
        PinDialog authDialog(this);
        if (authDialog.exec() != QDialog::Accepted) {
            qDebug() << "Zero Trust Policy: Deletion blocked.";
            return; 
        }
    }
    // ----------------------------------------------------

    // Get the hidden ID
    int id = table->item(row, 0)->data(Qt::UserRole).toInt();

    if (QMessageBox::question(this, "Confirm", "Delete this entry?") == QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteMBREntry(id)) {
            loadData();
            emit dataChanged(); 
        }
    }
}

void MBRWidget::loadData() {
    table->setRowCount(0);
    QSqlQuery q = DatabaseManager::instance().getMBREntries();

    int line = 1;
    while(q.next()) {
        int r = table->rowCount();
        table->insertRow(r);
        
        int dbID = q.value("id").toInt();

        QTableWidgetItem *itemLine = new QTableWidgetItem(QString::number(line++));
        itemLine->setData(Qt::UserRole, dbID);
        table->setItem(r, 0, itemLine);
        
        table->setItem(r, 1, new QTableWidgetItem(q.value("continuation").toString()));
        table->setItem(r, 2, new QTableWidgetItem(q.value("entry_name").toString()));
        table->setItem(r, 3, new QTableWidgetItem(q.value("element").toString()));
        
        double w = q.value("weight").toDouble();
        double f = q.value("fissile").toDouble();
        
        table->setItem(r, 4, new QTableWidgetItem(QString::number(w, 'f', w == qRound(w) ? 0 : 2)));
        table->setItem(r, 5, new QTableWidgetItem(q.value("unit").toString()));
        table->setItem(r, 6, new QTableWidgetItem(QString::number(f, 'f', f == qRound(f) ? 0 : 2)));
        table->setItem(r, 7, new QTableWidgetItem(q.value("isotope").toString()));
        table->setItem(r, 8, new QTableWidgetItem(q.value("report_no").toString()));
    }
}

void MBRWidget::exportPDF() {
    if(table->rowCount() == 0) {
        QMessageBox::warning(this, "Export Error", "The list is empty. Add entries first.");
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

    QString fileName = QFileDialog::getSaveFileName(this, "Save MBR", "MBR_Report.pdf", "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QMap<QString, QString> headerData;
    headerData["country"] = txtCountry->text();
    headerData["facility"] = txtFacility->text();
    headerData["mba"] = comboMBA->currentText();
    headerData["periodFrom"] = dateFrom->text();
    headerData["periodTo"] = dateTo->text();
    headerData["reportNo"] = txtReportNo->text();

    if (ReportGenerator::generateMBR_PDF(fileName, headerData, table)) {
        QMessageBox::information(this, "Success", "MBR PDF generated successfully.");
    } else {
        QMessageBox::critical(this, "Error", "Failed to generate PDF.");
    }
}
