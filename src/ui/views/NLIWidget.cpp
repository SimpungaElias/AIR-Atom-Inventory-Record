#include "NLIWidget.h"
#include "PinDialog.h"
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
#include <QSettings>
#include <QCompleter>
#include <QPushButton>
#include <QHBoxLayout>

static const QString GB_STYLE =
    "QGroupBox { border: 2px solid #003366; border-radius: 4px; margin-top: 24px;"
    "  padding-top: 16px; padding-left: 8px; padding-right: 8px; padding-bottom: 8px;"
    "  font-weight: bold; color: #003366; background-color: white; }"
    "QGroupBox::title { subcontrol-origin: margin; subcontrol-position: top left;"
    "  left: 12px; top: 4px; padding: 0px 4px; background-color: transparent; color: #003366; }";

static const QString BTN_PRIMARY =
    "QPushButton { background-color: #0056b3; color: white; font-weight: bold;"
    "  padding: 8px 16px; border-radius: 4px; border: none; font-size: 10pt; }"
    "QPushButton:hover { background-color: #004494; }";
static const QString BTN_DARK =
    "QPushButton { background-color: #074282; color: white; font-weight: bold;"
    "  padding: 8px 16px; border-radius: 4px; border: none; font-size: 10pt; }"
    "QPushButton:hover { background-color: #003366; }";
static const QString BTN_DANGER =
    "QPushButton { background-color: #c0392b; color: white; font-weight: bold;"
    "  padding: 8px 16px; border-radius: 4px; border: none; font-size: 10pt; }"
    "QPushButton:hover { background-color: #a93226; }";

static const QList<QPair<QString,QString>> COUNTRY_LIST = {
    {"AF","Afghanistan"},{"AL","Albania"},{"DZ","Algeria"},{"AR","Argentina"},
    {"AM","Armenia"},{"AU","Australia"},{"AT","Austria"},{"AZ","Azerbaijan"},
    {"BD","Bangladesh"},{"BY","Belarus"},{"BE","Belgium"},{"BR","Brazil"},
    {"BG","Bulgaria"},{"CM","Cameroon"},{"CA","Canada"},{"CL","Chile"},
    {"CN","China"},{"CO","Colombia"},{"HR","Croatia"},{"CU","Cuba"},
    {"CZ","Czech Republic"},{"DK","Denmark"},{"EC","Ecuador"},{"EG","Egypt"},
    {"ET","Ethiopia"},{"FI","Finland"},{"FR","France"},{"GE","Georgia"},
    {"DE","Germany"},{"GH","Ghana"},{"GR","Greece"},{"HU","Hungary"},
    {"IN","India"},{"ID","Indonesia"},{"IR","Iran"},{"IQ","Iraq"},
    {"IE","Ireland"},{"IL","Israel"},{"IT","Italy"},{"JP","Japan"},
    {"JO","Jordan"},{"KZ","Kazakhstan"},{"KE","Kenya"},{"KR","Republic of Korea"},
    {"KW","Kuwait"},{"KG","Kyrgyzstan"},{"LA","Laos"},{"LB","Lebanon"},
    {"LY","Libya"},{"LT","Lithuania"},{"MX","Mexico"},{"MA","Morocco"},
    {"MM","Myanmar"},{"NL","Netherlands"},{"NZ","New Zealand"},{"NG","Nigeria"},
    {"NO","Norway"},{"PK","Pakistan"},{"PE","Peru"},{"PH","Philippines"},
    {"PL","Poland"},{"PT","Portugal"},{"QA","Qatar"},{"RO","Romania"},
    {"RU","Russia"}, {"RW","Rwanda"},{"SA","Saudi Arabia"},{"SN","Senegal"},{"RS","Serbia"},
    {"SK","Slovakia"},{"SI","Slovenia"},{"ZA","South Africa"},{"ES","Spain"},
    {"LK","Sri Lanka"},{"SD","Sudan"},{"SE","Sweden"},{"CH","Switzerland"},
    {"SY","Syria"},{"TW","Taiwan"},{"TJ","Tajikistan"},{"TH","Thailand"},
    {"TN","Tunisia"},{"TR","Turkey"},{"TM","Turkmenistan"},{"UA","Ukraine"},
    {"AE","United Arab Emirates"},{"GB","United Kingdom"},{"US","United States"},
    {"UZ","Uzbekistan"},{"VE","Venezuela"},{"VN","Vietnam"},{"YE","Yemen"},{"ZW","Zimbabwe"},
};

NLIWidget::NLIWidget(QWidget *parent) : QWidget(parent), lineCounter(1) {
    setupUI(); loadData();
}

void NLIWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15,15,15,15); mainLayout->setSpacing(12);
    QLabel *title = new QLabel("<h2>Nuclear Loss Items (NLI) - Operations</h2>");
    title->setStyleSheet("color: #003366;"); mainLayout->addWidget(title);
    setupReportHeader(mainLayout); setupInputForm(mainLayout); setupTable(mainLayout);
}

void NLIWidget::setupReportHeader(QVBoxLayout *layout) {
    QGroupBox *grp = new QGroupBox("Report Information");
    grp->setStyleSheet(GB_STYLE);
    QGridLayout *grid = new QGridLayout(grp);
    grid->setHorizontalSpacing(12); grid->setVerticalSpacing(10);
    grid->setContentsMargins(10,8,10,10);

    // Row 0: Country (searchable) | Date
    grid->addWidget(new QLabel("Country:"), 0, 0);
    comboCountry = new QComboBox();
    comboCountry->setEditable(true);
    comboCountry->setInsertPolicy(QComboBox::NoInsert);
    comboCountry->completer()->setCompletionMode(QCompleter::PopupCompletion);
    comboCountry->completer()->setFilterMode(Qt::MatchContains);
    for (auto &p : COUNTRY_LIST) comboCountry->addItem(p.first + " — " + p.second, p.first);
    comboCountry->setCurrentIndex(comboCountry->findData("AT"));
    grid->addWidget(comboCountry, 0, 1);

    grid->addWidget(new QLabel("Date:"), 0, 2);
    dateReport = new QDateEdit(QDate::currentDate());
    dateReport->setDisplayFormat("yyyy-MM-dd"); dateReport->setCalendarPopup(true);
    grid->addWidget(dateReport, 0, 3);

    // Row 1: Facility (editable+placeholder) | Report No (spinbox)
    grid->addWidget(new QLabel("Facility:"), 1, 0);
    txtFacility = new QLineEdit();
    txtFacility->setPlaceholderText("e.g. Compton Research Reactor");
    txtFacility->setText("Compton Research Reactor");
    grid->addWidget(txtFacility, 1, 1);

    grid->addWidget(new QLabel("Report No:"), 1, 2);
    spinReportNo = new QSpinBox(); spinReportNo->setRange(1,99999); spinReportNo->setValue(1);
    grid->addWidget(spinReportNo, 1, 3);

    // Row 2: MBA (editable + saveable — same as MBR)
grid->addWidget(new QLabel("MBA:"), 2, 0);
QHBoxLayout *mbaLay = new QHBoxLayout;
mbaLay->setSpacing(6);
comboMBA = new QComboBox();
comboMBA->setEditable(true);
comboMBA->setInsertPolicy(QComboBox::NoInsert);
comboMBA->addItems({"CRRF", "EULE", "EXT", "DKNZ"});
QSettings nliS;
for (const QString &m : nliS.value("NLI/savedMBAs").toStringList())
    if (comboMBA->findText(m) < 0) comboMBA->addItem(m);
QPushButton *btnSaveMBA = new QPushButton("Save MBA");
btnSaveMBA->setStyleSheet(
    "QPushButton { background-color:#ecf0f1; color:#003366; font-weight:bold;"
    "  padding:6px 10px; border-radius:4px; border:1px solid #003366; font-size:9pt; }"
    "QPushButton:hover { background-color:#003366; color:white; }");
btnSaveMBA->setFixedHeight(30);
btnSaveMBA->setCursor(Qt::PointingHandCursor);
connect(btnSaveMBA, &QPushButton::clicked, [this](){
    QString t = comboMBA->currentText().trimmed(); if (t.isEmpty()) return;
    if (comboMBA->findText(t) < 0) {
        comboMBA->addItem(t); QSettings s;
        QStringList sv = s.value("NLI/savedMBAs").toStringList();
        if (!sv.contains(t)) { sv << t; s.setValue("NLI/savedMBAs", sv); }
        QMessageBox::information(this, "MBA Saved", QString("'%1' saved.").arg(t));
    } else QMessageBox::information(this, "MBA Exists", QString("'%1' already exists.").arg(t));
});
mbaLay->addWidget(comboMBA, 1);
mbaLay->addWidget(btnSaveMBA);
grid->addLayout(mbaLay, 2, 1, 1, 3);

    layout->addWidget(grp);
}

void NLIWidget::setupInputForm(QVBoxLayout *layout) {
    QGroupBox *grp = new QGroupBox("Add Loss Item");
    grp->setStyleSheet(GB_STYLE);
    QGridLayout *grid = new QGridLayout(grp);
    grid->setHorizontalSpacing(12); grid->setVerticalSpacing(10);
    grid->setContentsMargins(10,8,10,10);

    // Row 0: Batch | No. Items | Change Code
    grid->addWidget(new QLabel("Batch Identity:"), 0, 0);
    txtBatch = new QLineEdit; txtBatch->setPlaceholderText("e.g. NLI-001");
    grid->addWidget(txtBatch, 0, 1);
    grid->addWidget(new QLabel("No. Items:"), 0, 2);
    spinItems = new QSpinBox; spinItems->setValue(1); spinItems->setRange(1,9999);
    grid->addWidget(spinItems, 0, 3);
    grid->addWidget(new QLabel("Change Code:"), 0, 4);
    comboCode = new QComboBox; comboCode->addItems({"LN","LD","LA"});
    grid->addWidget(comboCode, 0, 5);

    // Row 1: Uranium fields
    QLabel *uLabel = new QLabel("U Elem Code:");
    uLabel->setStyleSheet("color: #003366; font-weight: bold;");
    grid->addWidget(uLabel, 1, 0);
    txtUElemCode = new QLineEdit("E"); grid->addWidget(txtUElemCode, 1, 1);
    grid->addWidget(new QLabel("U Iso Code:"), 1, 2);
    txtUIsoCode = new QLineEdit("G"); grid->addWidget(txtUIsoCode, 1, 3);
    grid->addWidget(new QLabel("U Weight (g):"), 1, 4);
    spinUWeight = new QDoubleSpinBox; spinUWeight->setRange(0,100000); spinUWeight->setDecimals(3);
    grid->addWidget(spinUWeight, 1, 5);

    // Row 2: U Iso Weight | Plutonium fields
    grid->addWidget(new QLabel("U Iso Weight (g):"), 2, 0);
    spinUIsoWeight = new QDoubleSpinBox; spinUIsoWeight->setRange(0,100000); spinUIsoWeight->setDecimals(3);
    grid->addWidget(spinUIsoWeight, 2, 1);
    QLabel *pLabel = new QLabel("P Elem Code:");
    pLabel->setStyleSheet("color: #003366; font-weight: bold;");
    grid->addWidget(pLabel, 2, 2);
    txtPElemCode = new QLineEdit("P"); grid->addWidget(txtPElemCode, 2, 3);
    grid->addWidget(new QLabel("P Weight (g):"), 2, 4);
    spinPWeight = new QDoubleSpinBox; spinPWeight->setRange(0,100000); spinPWeight->setDecimals(3);
    grid->addWidget(spinPWeight, 2, 5);

    // Row 3: Buttons
    QHBoxLayout *btnLay = new QHBoxLayout; btnLay->setSpacing(8); btnLay->addStretch();
    QPushButton *btnAdd = new QPushButton("Add Entry"); btnAdd->setStyleSheet(BTN_PRIMARY); btnAdd->setMinimumHeight(32);
    QPushButton *btnExport = new QPushButton("Export PDF"); btnExport->setStyleSheet(BTN_DARK); btnExport->setMinimumHeight(32);
    QPushButton *btnDelete = new QPushButton("Delete Selected"); btnDelete->setStyleSheet(BTN_DANGER); btnDelete->setMinimumHeight(32);
    connect(btnAdd, &QPushButton::clicked, this, &NLIWidget::addEntry);
    connect(btnExport, &QPushButton::clicked, this, &NLIWidget::exportPDF);
    connect(btnDelete, &QPushButton::clicked, this, &NLIWidget::deleteEntry);
    btnLay->addWidget(btnAdd); btnLay->addWidget(btnExport); btnLay->addWidget(btnDelete);
    grid->addLayout(btnLay, 3, 0, 1, 6);
    layout->addWidget(grp);
}

void NLIWidget::setupTable(QVBoxLayout *layout) {
    table = new QTableWidget; table->setColumnCount(10);
    table->setHorizontalHeaderLabels({"Line","Batch","Items","Code",
        "U Elem","U Iso","U Wt (g)","U Iso Wt (g)","P Elem","P Wt (g)"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setStyleSheet("QHeaderView::section { background-color:#f0f0f0; font-weight:bold;"
        "border:1px solid #ccc; padding:4px; color:#003366; }"
        "QTableWidget { border:1px solid #003366; gridline-color:#e0e0e0; }");
    layout->addWidget(table);
}

void NLIWidget::loadData() {
    table->setRowCount(0); lineCounter = 1;
    QSqlQuery q = DatabaseManager::instance().getNLIEntries();
    while (q.next()) {
        int r = table->rowCount(); table->insertRow(r);
        int dbID = q.value("id").toInt();
        QTableWidgetItem *itemLine = new QTableWidgetItem(QString::number(lineCounter++));
        itemLine->setData(Qt::UserRole, dbID); table->setItem(r,0,itemLine);
        table->setItem(r,1,new QTableWidgetItem(q.value("batch").toString()));
        table->setItem(r,2,new QTableWidgetItem(q.value("items").toString()));
        table->setItem(r,3,new QTableWidgetItem(q.value("code").toString()));
        table->setItem(r,4,new QTableWidgetItem(q.value("u_elem_code").toString()));
        table->setItem(r,5,new QTableWidgetItem(q.value("u_iso_code").toString()));
        table->setItem(r,6,new QTableWidgetItem(QString::number(q.value("u_weight").toDouble(),'f',3)));
        table->setItem(r,7,new QTableWidgetItem(QString::number(q.value("u_iso_weight").toDouble(),'f',3)));
        table->setItem(r,8,new QTableWidgetItem(q.value("p_elem_code").toString()));
        table->setItem(r,9,new QTableWidgetItem(QString::number(q.value("p_weight").toDouble(),'f',3)));
    }
}

void NLIWidget::addEntry() {
    QMap<QString,QVariant> data;
    data["batch"]        = txtBatch->text();
    data["items"]        = spinItems->value();
    data["code"]         = comboCode->currentText();
    data["u_elem_code"]  = txtUElemCode->text();
    data["u_iso_code"]   = txtUIsoCode->text();
    data["u_weight"]     = spinUWeight->value();
    data["u_iso_weight"] = spinUIsoWeight->value();
    data["p_elem_code"]  = txtPElemCode->text();
    data["p_weight"]     = spinPWeight->value();
    if (DatabaseManager::instance().addNLIEntry(data)) {
        loadData(); txtBatch->clear();
        spinUWeight->setValue(0); spinUIsoWeight->setValue(0); spinPWeight->setValue(0);
    } else QMessageBox::critical(this,"Error","Failed to save entry to database.");
}

void NLIWidget::deleteEntry() {
    int row = table->currentRow();
    if (row < 0) { QMessageBox::warning(this,"Select Item","Please select a row to delete."); return; }
    if (DatabaseManager::instance().currentDatabaseName().contains("AIR_Training")) {
        PinDialog authDialog(this);
        if (authDialog.exec() != QDialog::Accepted) { qDebug() << "Zero Trust: blocked."; return; }
    }
    int id = table->item(row,0)->data(Qt::UserRole).toInt();
    if (QMessageBox::question(this,"Confirm","Delete this nuclear loss entry?",QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteNLIEntry(id)) loadData();
        else QMessageBox::critical(this,"Error","Failed to delete entry from database.");
    }
}

void NLIWidget::exportPDF() {
    if (table->rowCount() == 0) { QMessageBox::warning(this,"Export Error","The list is empty."); return; }
    if (DatabaseManager::instance().currentDatabaseName().contains("AIR_Training")) {
        PinDialog authDialog(this);
        if (authDialog.exec() != QDialog::Accepted) { qDebug() << "Zero Trust: blocked."; return; }
    }
    QString fileName = QFileDialog::getSaveFileName(this,"Save NLI Report","NLI_Report.pdf","PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;
    QString countryCode = comboCountry->currentData().toString();
    if (countryCode.isEmpty()) countryCode = comboCountry->currentText().left(2).toUpper();
    QMap<QString,QString> header;
    header["country"]  = countryCode;
    header["facility"] = txtFacility->text();
    header["mba"] = comboMBA->currentText();
    header["date"]     = dateReport->text();
    header["reportNo"] = QString::number(spinReportNo->value());
    {
        QSqlDatabase tempDb = QSqlDatabase::addDatabase("QSQLITE","TempNLI");
        tempDb.setDatabaseName(":memory:"); if (!tempDb.open()) return;
        QSqlQuery q(tempDb);
        q.exec("CREATE TABLE temp_nli (batch TEXT, items INTEGER, code TEXT,"
               "u_elem_code TEXT, u_iso_code TEXT, u_weight REAL, u_iso_weight REAL,"
               "p_elem_code TEXT, p_weight REAL)");
        for (int i=0; i<table->rowCount(); i++) {
            q.prepare("INSERT INTO temp_nli VALUES (?,?,?,?,?,?,?,?,?)");
            q.addBindValue(table->item(i,1)->text()); q.addBindValue(table->item(i,2)->text().toInt());
            q.addBindValue(table->item(i,3)->text()); q.addBindValue(table->item(i,4)->text());
            q.addBindValue(table->item(i,5)->text()); q.addBindValue(table->item(i,6)->text().toDouble());
            q.addBindValue(table->item(i,7)->text().toDouble()); q.addBindValue(table->item(i,8)->text());
            q.addBindValue(table->item(i,9)->text().toDouble()); q.exec();
        }
        QSqlQuery pq(tempDb); pq.exec("SELECT * FROM temp_nli");
        if (ReportGenerator::generateNLI_PDF(fileName, header, pq))
            QMessageBox::information(this,"Success","NLI Report saved successfully.");
        else QMessageBox::critical(this,"Error","Failed to generate PDF.");
    }
    QSqlDatabase::removeDatabase("TempNLI");
}
