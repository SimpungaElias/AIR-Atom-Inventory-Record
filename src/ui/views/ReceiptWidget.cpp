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
#include <QSettings>
#include <QCompleter>
#include <QPushButton>

// ── Shared GroupBox style — same as MBRWidget ──────────────────────────────
static const QString GB_STYLE =
    "QGroupBox {"
    "  border: 2px solid #003366;"
    "  border-radius: 4px;"
    "  margin-top: 24px;"
    "  padding-top: 16px;"
    "  padding-left: 8px;"
    "  padding-right: 8px;"
    "  padding-bottom: 8px;"
    "  font-weight: bold;"
    "  color: #003366;"
    "  background-color: white;"
    "}"
    "QGroupBox::title {"
    "  subcontrol-origin: margin;"
    "  subcontrol-position: top left;"
    "  left: 12px;"
    "  top: 4px;"
    "  padding: 0px 4px;"
    "  background-color: transparent;"
    "  color: #003366;"
    "}";

// ── Shared button styles — same across all widgets ─────────────────────────
static const QString BTN_PRIMARY =
    "QPushButton {"
    "  background-color: #0056b3;"
    "  color: white;"
    "  font-weight: bold;"
    "  padding: 8px 16px;"
    "  border-radius: 4px;"
    "  border: none;"
    "  font-size: 10pt;"
    "}"
    "QPushButton:hover { background-color: #004494; }";

static const QString BTN_DARK =
    "QPushButton {"
    "  background-color: #074282;"
    "  color: white;"
    "  font-weight: bold;"
    "  padding: 8px 16px;"
    "  border-radius: 4px;"
    "  border: none;"
    "  font-size: 10pt;"
    "}"
    "QPushButton:hover { background-color: #003366; }";

static const QString BTN_DANGER =
    "QPushButton {"
    "  background-color: #c0392b;"
    "  color: white;"
    "  font-weight: bold;"
    "  padding: 8px 16px;"
    "  border-radius: 4px;"
    "  border: none;"
    "  font-size: 10pt;"
    "}"
    "QPushButton:hover { background-color: #a93226; }";

// ──────────────────────────────────────────────────────────────────────────


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
ReceiptWidget::ReceiptWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
    refreshTable();
}

void ReceiptWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(12);

    QLabel *title = new QLabel("<h2>Inventory Change Report (ICR) - Operations</h2>");
    title->setStyleSheet("color: #003366;");
    mainLayout->addWidget(title);

    setupReportHeader(mainLayout);
    setupInputForm(mainLayout);
    setupTable(mainLayout);
}

void ReceiptWidget::setupReportHeader(QVBoxLayout *layout) {
    QGroupBox *grp = new QGroupBox("Report Information");
    grp->setStyleSheet(GB_STYLE);

    QGridLayout *grid = new QGridLayout(grp);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(10);
    grid->setContentsMargins(10, 8, 10, 10);

    // Row 0: Country (searchable IAEA list) | Report Date
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
    spinReportNo = new QSpinBox(); spinReportNo->setRange(1, 99999); spinReportNo->setValue(1);
    grid->addWidget(spinReportNo, 1, 3);

    // Row 2: MBA (editable + Save button)
    grid->addWidget(new QLabel("MBA:"), 2, 0);
    QHBoxLayout *mbaLay = new QHBoxLayout; mbaLay->setSpacing(6);
    comboMBA = new QComboBox(); comboMBA->setEditable(true);
    comboMBA->setInsertPolicy(QComboBox::NoInsert);
    comboMBA->addItems({"CRRF","EULE","EXT","DKNZ"});
    QSettings icrS; for (const QString &m : icrS.value("ICR/savedMBAs").toStringList())
        if (comboMBA->findText(m) < 0) comboMBA->addItem(m);
    QPushButton *btnSaveMBA = new QPushButton("Save MBA");
    btnSaveMBA->setStyleSheet(
        "QPushButton { background-color:#ecf0f1; color:#003366; font-weight:bold;"
        "  padding:6px 10px; border-radius:4px; border:1px solid #003366; font-size:9pt; }"
        "QPushButton:hover { background-color:#003366; color:white; }");
    btnSaveMBA->setFixedHeight(30); btnSaveMBA->setCursor(Qt::PointingHandCursor);
    connect(btnSaveMBA, &QPushButton::clicked, [this](){
        QString t = comboMBA->currentText().trimmed(); if (t.isEmpty()) return;
        if (comboMBA->findText(t) < 0) {
            comboMBA->addItem(t); QSettings s;
            QStringList sv = s.value("ICR/savedMBAs").toStringList();
            if (!sv.contains(t)) { sv<<t; s.setValue("ICR/savedMBAs",sv); }
            QMessageBox::information(this,"MBA Saved",QString("'%1' saved.").arg(t));
        } else QMessageBox::information(this,"MBA Exists",QString("'%1' already exists.").arg(t));
    });
    mbaLay->addWidget(comboMBA,1); mbaLay->addWidget(btnSaveMBA);
    grid->addLayout(mbaLay, 2, 1, 1, 1);

    // Row 3: Shipper | Receiver
    grid->addWidget(new QLabel("Shipper Name:"), 3, 0);
    txtShipper = new QLineEdit; txtShipper->setPlaceholderText("e.g. Supplier Facility");
    grid->addWidget(txtShipper, 3, 1);
    grid->addWidget(new QLabel("Receiver Name:"), 3, 2);
    txtReceiver = new QLineEdit; txtReceiver->setPlaceholderText("e.g. Receiving Facility");
    grid->addWidget(txtReceiver, 3, 3);

    layout->addWidget(grp);
}

void ReceiptWidget::setupInputForm(QVBoxLayout *layout) {
    QGroupBox *grp = new QGroupBox("Add Receipt Transaction");
    grp->setStyleSheet(GB_STYLE);

    QGridLayout *grid = new QGridLayout(grp);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(10);
    grid->setContentsMargins(10, 8, 10, 10);

    // Row 0: Batch | Recv MBA | From MBA
    grid->addWidget(new QLabel("Batch Identity:"), 0, 0);
    txtBatch = new QLineEdit;
    txtBatch->setPlaceholderText("e.g. CRR01");
    grid->addWidget(txtBatch, 0, 1);

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

    // Row 1: IC Code | Items | Date
    grid->addWidget(new QLabel("Inv. Change Code:"), 1, 0);
    comboCode = new QComboBox;
    comboCode->addItems({
        "RD (Receipt Domestic)", "RF (Receipt Foreign)",
        "RS (Receipt at starting point)", "NP (Nuclear Production)",
        "DU (De-exemption, use)", "DQ (De-exemption, quantity)",
        "SF (Shipment Foreign)", "SD (Shipment Domestic)",
        "LN (Nuclear Loss)", "EU (Exemption, use)",
        "EQ (Exemption, quantity)", "TU (Termination)",
        "LA (Accidental Loss)", "GA (Accidental Gain)"
    });
    grid->addWidget(comboCode, 1, 1);

    grid->addWidget(new QLabel("No. of Items:"), 1, 2);
    spinItems = new QSpinBox;
    spinItems->setRange(1, 1000);
    grid->addWidget(spinItems, 1, 3);

    grid->addWidget(new QLabel("Date (YYYY-MM-DD):"), 1, 4);
    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("yyyy-MM-dd");
    grid->addWidget(dateEdit, 1, 5);

    // Row 2: Element | Elem Wt | Isotope Wt
    grid->addWidget(new QLabel("Element Code:"), 2, 0);
    comboElement = new QComboBox;
    comboElement->addItems({
        "N (Natural Uranium)", "E (Enriched Uranium)",
        "D (Depleted Uranium)", "P (Plutonium)", "T (Thorium)"
    });
    grid->addWidget(comboElement, 2, 1);

    grid->addWidget(new QLabel("Total Element Wt (g):"), 2, 2);
    spinWeightU = new QDoubleSpinBox;
    spinWeightU->setRange(0, 10000000);
    spinWeightU->setDecimals(2);
    grid->addWidget(spinWeightU, 2, 3);

    grid->addWidget(new QLabel("Isotope/Fissile Wt (g):"), 2, 4);
    spinWeightU235 = new QDoubleSpinBox;
    spinWeightU235->setRange(0, 10000000);
    spinWeightU235->setDecimals(3);
    grid->addWidget(spinWeightU235, 2, 5);

    // Row 3: Material Form | Buttons
    grid->addWidget(new QLabel("Material Form:"), 3, 0);
    txtForm = new QLineEdit;
    txtForm->setPlaceholderText("e.g. UO2 Powder");
    grid->addWidget(txtForm, 3, 1, 1, 3);

    // Buttons — consistent sizing, same pattern as MBRWidget
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(8);

    QPushButton *btnSave = new QPushButton("Add Entry");
    btnSave->setStyleSheet(BTN_PRIMARY);
    btnSave->setMinimumHeight(32);
    connect(btnSave, &QPushButton::clicked, this, &ReceiptWidget::saveReceipt);

    QPushButton *btnExport = new QPushButton("Export PDF");
    btnExport->setStyleSheet(BTN_DARK);
    btnExport->setMinimumHeight(32);
    connect(btnExport, &QPushButton::clicked, this, &ReceiptWidget::exportPDF);

    QPushButton *btnDelete = new QPushButton("Delete Selected");
    btnDelete->setStyleSheet(BTN_DANGER);
    btnDelete->setMinimumHeight(32);
    connect(btnDelete, &QPushButton::clicked, this, &ReceiptWidget::deleteSelected);

    btnLayout->addWidget(btnSave);
    btnLayout->addWidget(btnExport);
    btnLayout->addWidget(btnDelete);

    grid->addLayout(btnLayout, 3, 4, 1, 2);

    layout->addWidget(grp);
}

void ReceiptWidget::setupTable(QVBoxLayout *layout) {
    QLabel *tableTitle = new QLabel("<b>Recent Receipts (ICD Format)</b>");
    tableTitle->setStyleSheet("color: #003366; font-size: 10pt; margin-top: 4px;");
    layout->addWidget(tableTitle);

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
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->verticalHeader()->setVisible(false);
    table->setStyleSheet(
        "QHeaderView::section {"
        "  background-color: #f0f0f0;"
        "  font-weight: bold;"
        "  border: 1px solid #ccc;"
        "  padding: 4px;"
        "  color: #003366;"
        "}"
        "QTableWidget {"
        "  border: 1px solid #003366;"
        "  gridline-color: #e0e0e0;"
        "}"
    );

    layout->addWidget(table);
}

// ──────────────────────────────────────────────────────────────────────────
// DATA OPERATIONS
// ──────────────────────────────────────────────────────────────────────────

void ReceiptWidget::saveReceipt() {
    if (txtBatch->text().isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Batch Identity is required.");
        return;
    }
    if (spinWeightU->value() <= 0) {
        QMessageBox::warning(this, "Input Error", "Element weight must be greater than 0.");
        return;
    }

    QMap<QString, QVariant> data;
    data["batch_number"]  = txtBatch->text();
    data["to_mba"]        = comboMbaTo->currentText();
    data["from_mba"]      = comboMbaFrom->currentText();
    data["receipt_code"]  = comboCode->currentText().left(2);
    data["count"]         = spinItems->value();
    data["date"]          = dateEdit->date().toString("yyyy-MM-dd");
    data["element"]       = comboElement->currentText();
    data["weight_u"]      = spinWeightU->value();
    data["weight_u235"]   = spinWeightU235->value();
    data["physical_form"] = txtForm->text();
    data["chemical_form"] = "Oxide";
    data["kmp"]           = "A";
    data["unit"]          = "g";
    data["manufacturer"]  = data["from_mba"];

    if (DatabaseManager::instance().registerReceipt(data)) {
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
    if (row < 0) {
        QMessageBox::warning(this, "Select Item", "Please select a row to delete.");
        return;
    }

    if (DatabaseManager::instance().currentDatabaseName().contains("AIR_Training")) {
        PinDialog authDialog(this);
        if (authDialog.exec() != QDialog::Accepted) {
            qDebug() << "Zero Trust Policy: Deletion blocked.";
            return;
        }
    }

    int id = table->item(row, 0)->data(Qt::UserRole).toInt();

    if (QMessageBox::question(this, "Confirm",
            "Delete this receipt entry?\nThis will remove it from the database.",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteReceipt(id)) {
            refreshTable();
            emit dataChanged();
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete receipt from database.");
        }
    }
}

void ReceiptWidget::refreshTable() {
    table->setRowCount(0);

    QSqlQuery query = DatabaseManager::instance().getReceipts();

    int line = 1;
    while (query.next()) {
        int r = table->rowCount();
        table->insertRow(r);

        int    id     = query.value("id").toInt();
        QString batch = query.value("batch_number").toString();
        QString code  = query.value("change_type").toString();
        int    items  = query.value("items_count").toInt();
        QString rawEl = query.value("element").toString();
        QString elCode = rawEl.left(1);
        double wElem  = query.value("increase_u").toDouble();
        double wIso   = query.value("weight_u235").toDouble();

        QTableWidgetItem *itemLine = new QTableWidgetItem(QString::number(line++));
        itemLine->setData(Qt::UserRole, id);
        table->setItem(r, 0, itemLine);

        table->setItem(r, 1, new QTableWidgetItem(batch));
        table->setItem(r, 2, new QTableWidgetItem(QString::number(items)));
        table->setItem(r, 3, new QTableWidgetItem(code));

        if (elCode != "P") {
            table->setItem(r, 4, new QTableWidgetItem(elCode));
            table->setItem(r, 5, new QTableWidgetItem("G"));
            table->setItem(r, 6, new QTableWidgetItem(QString::number(wElem, 'f', 2)));
            table->setItem(r, 7, new QTableWidgetItem(QString::number(wIso,  'f', 3)));
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
    if (DatabaseManager::instance().currentDatabaseName().contains("AIR_Training")) {
        PinDialog authDialog(this);
        if (authDialog.exec() != QDialog::Accepted) {
            qDebug() << "Zero Trust Policy: Export blocked.";
            return;
        }
    }

    QString fileName = QFileDialog::getSaveFileName(
        this, "Save Receipt Report", "ICR_Full_Report.pdf", "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QMap<QString, QString> headerData;
    headerData["country"]  = comboCountry->currentData().toString().isEmpty()
                             ? comboCountry->currentText().left(2).toUpper()
                             : comboCountry->currentData().toString();
    headerData["facility"] = txtFacility->text();
    headerData["mba"]      = comboMBA->currentText();
    headerData["reportNo"] = QString::number(spinReportNo->value());
    headerData["date"]     = dateReport->text();
    headerData["shipper"]  = txtShipper->text();
    headerData["receiver"] = txtReceiver->text();

    QString connectionName = DatabaseManager::instance().currentDatabaseName().isEmpty()
                             ? "qt_sql_default_connection"
                             : "air_connection";

    QSqlDatabase db = QSqlDatabase::database(connectionName);
    QSqlQuery query(db);

    query.prepare(
        "SELECT h.record_date, h.change_type, h.items_count, b.batch_number, "
        "b.physical_form, b.element, h.increase_u, b.weight_u235, h.decrease_u, h.description "
        "FROM history h JOIN batches b ON h.batch_id = b.id "
        "WHERE h.change_type IN ('RD', 'RF', 'RN') "
        "ORDER BY h.id ASC"
    );
    query.exec();

    if (ReportGenerator::generateICR_PDF(fileName, headerData, query)) {
        QMessageBox::information(this, "Success", "Full Receipt Report generated successfully.");
    } else {
        QMessageBox::critical(this, "Error", "Failed to generate PDF.");
    }
}
