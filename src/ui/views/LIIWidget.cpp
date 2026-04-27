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
static const QString BTN_NEUTRAL =
    "QPushButton { background-color: #ecf0f1; color: #003366; font-weight: bold;"
    "  padding: 8px 16px; border-radius: 4px; border: 1px solid #003366; font-size: 10pt; }"
    "QPushButton:hover { background-color: #003366; color: white; }";

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

LIIWidget::LIIWidget(QWidget *parent) : QWidget(parent), itemCounter(1) {
    setupUI(); loadData();
}

void LIIWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15,15,15,15); mainLayout->setSpacing(12);
    QLabel *title = new QLabel("<h2>List of Inventory Items (LII) - Operations</h2>");
    title->setStyleSheet("color: #003366;"); mainLayout->addWidget(title);
    setupReportHeader(mainLayout); setupInputForm(mainLayout); setupTable(mainLayout);
}

void LIIWidget::setupReportHeader(QVBoxLayout *layout) {
    QGroupBox *grp = new QGroupBox("Report Information");
    grp->setStyleSheet(GB_STYLE);
    QGridLayout *grid = new QGridLayout(grp);
    grid->setHorizontalSpacing(12); grid->setVerticalSpacing(10);
    grid->setContentsMargins(10,8,10,10);

    // Row 0: Country (searchable) | Report Date
    grid->addWidget(new QLabel("Country:"), 0, 0);
    comboCountry = new QComboBox();
    comboCountry->setEditable(true);
    comboCountry->setInsertPolicy(QComboBox::NoInsert);
    comboCountry->completer()->setCompletionMode(QCompleter::PopupCompletion);
    comboCountry->completer()->setFilterMode(Qt::MatchContains);
    for (auto &p : COUNTRY_LIST) comboCountry->addItem(p.first + " — " + p.second, p.first);
    comboCountry->setCurrentIndex(comboCountry->findData("AT"));
    grid->addWidget(comboCountry, 0, 1);

    grid->addWidget(new QLabel("Report Date:"), 0, 2);
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
QSettings liiS;
for (const QString &m : liiS.value("LII/savedMBAs").toStringList())
    if (comboMBA->findText(m) < 0) comboMBA->addItem(m);
QPushButton *btnSaveMBA = new QPushButton("Save MBA");
btnSaveMBA->setStyleSheet(BTN_NEUTRAL);
btnSaveMBA->setFixedHeight(30);
btnSaveMBA->setCursor(Qt::PointingHandCursor);
connect(btnSaveMBA, &QPushButton::clicked, [this](){
    QString t = comboMBA->currentText().trimmed(); if (t.isEmpty()) return;
    if (comboMBA->findText(t) < 0) {
        comboMBA->addItem(t); QSettings s;
        QStringList sv = s.value("LII/savedMBAs").toStringList();
        if (!sv.contains(t)) { sv << t; s.setValue("LII/savedMBAs", sv); }
        QMessageBox::information(this, "MBA Saved", QString("'%1' saved.").arg(t));
    } else QMessageBox::information(this, "MBA Exists", QString("'%1' already exists.").arg(t));
});
mbaLay->addWidget(comboMBA, 1);
mbaLay->addWidget(btnSaveMBA);
grid->addLayout(mbaLay, 2, 1, 1, 3);

    layout->addWidget(grp);
}

void LIIWidget::setupInputForm(QVBoxLayout *layout) {
    QGroupBox *grp = new QGroupBox("Add Inventory Item");
    grp->setStyleSheet(GB_STYLE);
    QGridLayout *grid = new QGridLayout(grp);
    grid->setHorizontalSpacing(12); grid->setVerticalSpacing(10);
    grid->setContentsMargins(10,8,10,10);

    // Row 0: KMP | Position | Batch
    grid->addWidget(new QLabel("KMP:"), 0, 0);
    comboKMP = new QComboBox; comboKMP->addItems({"FFS","RRC","SFS","LOF"});
    grid->addWidget(comboKMP, 0, 1);
    grid->addWidget(new QLabel("Position:"), 0, 2);
    txtPosition = new QLineEdit; txtPosition->setPlaceholderText("e.g. A01");
    grid->addWidget(txtPosition, 0, 3);
    grid->addWidget(new QLabel("Batch/Item:"), 0, 4);
    txtBatch = new QLineEdit;
    grid->addWidget(txtBatch, 0, 5);

    // Row 1: Material Code selector
    grid->addWidget(new QLabel("Mat. Code (430):"), 1, 0);
    QHBoxLayout *codeLayout = new QHBoxLayout; codeLayout->setSpacing(6);
    txtMaterialCode = new QLineEdit;
    txtMaterialCode->setPlaceholderText("Click Select →");
    txtMaterialCode->setReadOnly(true);
    txtMaterialCode->setStyleSheet("background-color:#f0f0f0;border:1px solid #ccc;border-radius:3px;padding:4px;");
    QPushButton *btnSelectCode = new QPushButton("Select...");
    btnSelectCode->setStyleSheet(BTN_NEUTRAL); btnSelectCode->setMinimumHeight(32);
    btnSelectCode->setCursor(Qt::PointingHandCursor);
    connect(btnSelectCode, &QPushButton::clicked, this, &LIIWidget::openMaterialCodeSelector);
    codeLayout->addWidget(txtMaterialCode); codeLayout->addWidget(btnSelectCode);
    grid->addLayout(codeLayout, 1, 1, 1, 3);

    // Row 2: Element | Fissile | Plutonium
    grid->addWidget(new QLabel("Element (g):"), 2, 0);
    spinElem = new QDoubleSpinBox; spinElem->setRange(0,1000000); spinElem->setDecimals(2);
    grid->addWidget(spinElem, 2, 1);
    grid->addWidget(new QLabel("Fissile (g):"), 2, 2);
    spinFissile = new QDoubleSpinBox; spinFissile->setRange(0,1000000); spinFissile->setDecimals(3);
    grid->addWidget(spinFissile, 2, 3);
    grid->addWidget(new QLabel("Plutonium (g):"), 2, 4);
    spinPu = new QDoubleSpinBox; spinPu->setRange(0,1000000); spinPu->setDecimals(3);
    grid->addWidget(spinPu, 2, 5);

    // Row 3: Burnup | Buttons
    grid->addWidget(new QLabel("Burnup:"), 3, 0);
    spinBurnup = new QDoubleSpinBox; spinBurnup->setRange(0,100000);
    grid->addWidget(spinBurnup, 3, 1);

    QHBoxLayout *btnLayout = new QHBoxLayout; btnLayout->setSpacing(8);
    QPushButton *btnAdd = new QPushButton("Add Entry"); btnAdd->setStyleSheet(BTN_PRIMARY); btnAdd->setMinimumHeight(32);
    QPushButton *btnExport = new QPushButton("Export PDF"); btnExport->setStyleSheet(BTN_DARK); btnExport->setMinimumHeight(32);
    QPushButton *btnDel = new QPushButton("Delete Selected"); btnDel->setStyleSheet(BTN_DANGER); btnDel->setMinimumHeight(32);
    connect(btnAdd, &QPushButton::clicked, this, &LIIWidget::addItem);
    connect(btnExport, &QPushButton::clicked, this, &LIIWidget::exportPDF);
    connect(btnDel, &QPushButton::clicked, this, &LIIWidget::deleteItem);
    btnLayout->addWidget(btnAdd); btnLayout->addWidget(btnExport); btnLayout->addWidget(btnDel);
    grid->addLayout(btnLayout, 3, 4, 1, 2);
    layout->addWidget(grp);
}

void LIIWidget::openMaterialCodeSelector() {
    MaterialCodeDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) txtMaterialCode->setText(dlg.getSelectedCode());
}

void LIIWidget::setupTable(QVBoxLayout *layout) {
    table = new QTableWidget;
    table->setColumnCount(11);
    table->setHorizontalHeaderLabels({"KMP","Position","Item","Batch","Code (430)",
        "Elem (g)","Fissile (g)","Pu (g)","Th (g)","Burnup","Cooling"});
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

void LIIWidget::addItem() {
    if (txtMaterialCode->text().isEmpty()) { QMessageBox::warning(this,"Missing Data","Please select a Material Description Code."); return; }
    QMap<QString,QVariant> data;
    data["kmp"]            = comboKMP->currentText();
    data["position"]       = txtPosition->text();
    data["batch"]          = txtBatch->text();
    data["desc"]           = txtMaterialCode->text();
    data["weight_elem"]    = spinElem->value();
    data["weight_fissile"] = spinFissile->value();
    data["weight_pu"]      = spinPu->value();
    data["burnup"]         = spinBurnup->value();
    if (DatabaseManager::instance().addLIIEntry(data)) {
        loadData();
        txtPosition->clear(); txtBatch->clear(); txtMaterialCode->clear();
        spinElem->setValue(0); spinFissile->setValue(0); spinPu->setValue(0); spinBurnup->setValue(0);
    } else QMessageBox::critical(this,"Error","Failed to save entry to database.");
}

void LIIWidget::deleteItem() {
    int row = table->currentRow();
    if (row < 0) { QMessageBox::warning(this,"Select Item","Please select a row to delete."); return; }
    if (DatabaseManager::instance().currentDatabaseName().contains("AIR_Training")) {
        PinDialog authDialog(this);
        if (authDialog.exec() != QDialog::Accepted) { qDebug() << "Zero Trust: blocked."; return; }
    }
    int id = table->item(row,0)->data(Qt::UserRole).toInt();
    if (QMessageBox::question(this,"Confirm","Delete this inventory item?",QMessageBox::Yes|QMessageBox::No)==QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteLIIEntry(id)) loadData();
        else QMessageBox::critical(this,"Error","Failed to delete item from database.");
    }
}

void LIIWidget::loadData() {
    table->setRowCount(0);
    QSqlQuery q = DatabaseManager::instance().getLIIEntries();
    while (q.next()) {
        int r = table->rowCount(); table->insertRow(r);
        int dbID = q.value("id").toInt();
        QTableWidgetItem *itemKMP = new QTableWidgetItem(q.value("kmp").toString());
        itemKMP->setData(Qt::UserRole, dbID); table->setItem(r,0,itemKMP);
        table->setItem(r,1,new QTableWidgetItem(q.value("position").toString()));
        table->setItem(r,2,new QTableWidgetItem("1"));
        table->setItem(r,3,new QTableWidgetItem(q.value("batch").toString()));
        table->setItem(r,4,new QTableWidgetItem(q.value("desc").toString()));
        table->setItem(r,5,new QTableWidgetItem(QString::number(q.value("weight_elem").toDouble(),'f',2)));
        table->setItem(r,6,new QTableWidgetItem(QString::number(q.value("weight_fissile").toDouble(),'f',3)));
        table->setItem(r,7,new QTableWidgetItem(QString::number(q.value("weight_pu").toDouble(),'f',3)));
        table->setItem(r,8,new QTableWidgetItem("0.0"));
        table->setItem(r,9,new QTableWidgetItem(QString::number(q.value("burnup").toDouble())));
        table->setItem(r,10,new QTableWidgetItem("-"));
    }
}

void LIIWidget::exportPDF() {
    if (table->rowCount() == 0) { QMessageBox::warning(this,"Export Error","The list is empty. Add items first."); return; }
    if (DatabaseManager::instance().currentDatabaseName().contains("AIR_Training")) {
        PinDialog authDialog(this);
        if (authDialog.exec() != QDialog::Accepted) { qDebug() << "Zero Trust: blocked."; return; }
    }
    QString fileName = QFileDialog::getSaveFileName(this,"Save LII Report","LII_Report.pdf","PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;
    QString countryCode = comboCountry->currentData().toString();
    if (countryCode.isEmpty()) countryCode = comboCountry->currentText().left(2).toUpper();
    QMap<QString,QString> headerData;
    headerData["country"]  = countryCode;
    headerData["facility"] = txtFacility->text();
    headerData["mba"] = comboMBA->currentText();
    headerData["date"]     = dateReport->text();
    headerData["reportNo"] = QString::number(spinReportNo->value());
    {
        QSqlDatabase tempDb = QSqlDatabase::addDatabase("QSQLITE","TempLIIExport");
        tempDb.setDatabaseName(":memory:"); if (!tempDb.open()) return;
        QSqlQuery q(tempDb);
        q.exec("CREATE TABLE temp_batches (kmp TEXT, position TEXT, batch TEXT, desc TEXT,"
               "weight_elem REAL, weight_fissile REAL, weight_pu REAL, burnup REAL)");
        for (int i=0; i<table->rowCount(); i++) {
            q.prepare("INSERT INTO temp_batches VALUES (?,?,?,?,?,?,?,?)");
            q.addBindValue(table->item(i,0)->text()); q.addBindValue(table->item(i,1)->text());
            q.addBindValue(table->item(i,3)->text()); q.addBindValue(table->item(i,4)->text());
            q.addBindValue(table->item(i,5)->text().toDouble()); q.addBindValue(table->item(i,6)->text().toDouble());
            q.addBindValue(table->item(i,7)->text().toDouble()); q.addBindValue(table->item(i,9)->text().toDouble());
            q.exec();
        }
        QSqlQuery rq(tempDb); rq.exec("SELECT * FROM temp_batches ORDER BY kmp ASC, batch ASC");
        if (ReportGenerator::generateLII_PDF(fileName, headerData, rq))
            QMessageBox::information(this,"Success","LII Report generated successfully.");
        else QMessageBox::critical(this,"Error","Failed to generate PDF.");
    }
    QSqlDatabase::removeDatabase("TempLIIExport");
}
