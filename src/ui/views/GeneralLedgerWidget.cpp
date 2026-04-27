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
#include <QSettings>

// ── Shared GroupBox style — identical to MBRWidget, ReceiptWidget, LIIWidget, NLIWidget
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

// ── Shared button styles — identical across all widgets ───────────────────
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

// ─────────────────────────────────────────────────────────────────────────

GeneralLedgerWidget::GeneralLedgerWidget(QWidget *parent)
    : QWidget(parent), lineCounter(1), balU(0), balU235(0), balItems(0) {
    setupUI();
    refreshData();
}

void GeneralLedgerWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(12);

    QLabel *title = new QLabel("<h2>General Ledger (GL)</h2>");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: #003366; font-weight: bold;");
    mainLayout->addWidget(title);

    setupReportHeader(mainLayout);
    setupInputForm(mainLayout);
    setupComplexTable(mainLayout);
}

void GeneralLedgerWidget::setupReportHeader(QVBoxLayout *layout) {
    QGroupBox *grp = new QGroupBox("Report Information");
    grp->setStyleSheet(GB_STYLE);

    QGridLayout *grid = new QGridLayout(grp);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(10);
    grid->setContentsMargins(10, 8, 10, 10);

    // Row 0: Facility | MBA (editable+saveable)
    grid->addWidget(new QLabel("Facility:"), 0, 0);
    txtFacility = new QLineEdit();
    txtFacility->setPlaceholderText("e.g. Compton Research Reactor");
    txtFacility->setText("Compton Research Reactor"); // ← FIXED: restored default
    grid->addWidget(txtFacility, 0, 1);

    grid->addWidget(new QLabel("MBA:"), 0, 2);
    QHBoxLayout *mbaLay = new QHBoxLayout;
    mbaLay->setSpacing(6);
    comboMBA = new QComboBox;
    comboMBA->setEditable(true);
    comboMBA->setInsertPolicy(QComboBox::NoInsert);
    comboMBA->addItems({"CRRF", "EULE", "EXT", "DKNZ"});
    QSettings glSettings;
    for (const QString &m : glSettings.value("GL/savedMBAs").toStringList())
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
            QStringList sv = s.value("GL/savedMBAs").toStringList();
            if (!sv.contains(t)) { sv << t; s.setValue("GL/savedMBAs", sv); }
            QMessageBox::information(this, "MBA Saved", QString("'%1' saved.").arg(t));
        } else QMessageBox::information(this, "MBA Exists", QString("'%1' already exists.").arg(t));
    });
    mbaLay->addWidget(comboMBA, 1);
    mbaLay->addWidget(btnSaveMBA);
    grid->addLayout(mbaLay, 0, 3);

    // Row 1: Material Description (spans full width) ← FIXED
    grid->addWidget(new QLabel("Material Description:"), 1, 0);
    txtMatDesc = new QLineEdit(); // ← FIXED: instantiated here
    txtMatDesc->setPlaceholderText("Low Enriched Uranium (U3O8-Al)");
    txtMatDesc->setText("Low Enriched Uranium (U3O8-Al)");
    grid->addWidget(txtMatDesc, 1, 1, 1, 3); // ← FIXED: correct row and span

    // Row 2: Element Code | Isotope Code | Unit
    QHBoxLayout *codeLay = new QHBoxLayout;
    codeLay->setSpacing(10);
    codeLay->addWidget(new QLabel("Element Code:"));
    txtElemCode = new QLineEdit("E"); txtElemCode->setFixedWidth(50);
    codeLay->addWidget(txtElemCode);
    codeLay->addWidget(new QLabel("Isotope Code:"));
    txtIsoCode = new QLineEdit("G"); txtIsoCode->setFixedWidth(50);
    codeLay->addWidget(txtIsoCode);
    codeLay->addWidget(new QLabel("Unit:"));
    txtUnit = new QLineEdit("g"); txtUnit->setFixedWidth(50);
    codeLay->addWidget(txtUnit);
    codeLay->addStretch();
    grid->addLayout(codeLay, 2, 0, 1, 4);

    layout->addWidget(grp);
}

void GeneralLedgerWidget::setupInputForm(QVBoxLayout *layout) {
    QGroupBox *grp = new QGroupBox("Add Transaction");
    grp->setStyleSheet(GB_STYLE);

    QGridLayout *grid = new QGridLayout(grp);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(10);
    grid->setContentsMargins(10, 8, 10, 10);

    // Row 0: Date | Ref (ICD/PIL) | IC Code
    grid->addWidget(new QLabel("Date:"), 0, 0);
    dateEdit = new QDateEdit(QDate::currentDate());
    dateEdit->setCalendarPopup(true);
    dateEdit->setDisplayFormat("yyyy-MM-dd");
    grid->addWidget(dateEdit, 0, 1);

    grid->addWidget(new QLabel("Ref (ICD/PIL):"), 0, 2);
    txtRef = new QLineEdit;
    txtRef->setPlaceholderText("e.g. PIL1");
    grid->addWidget(txtRef, 0, 3);

    grid->addWidget(new QLabel("IC Code:"), 0, 4);
    comboCode = new QComboBox;
    comboCode->addItems({"", "RF", "RD", "SD", "LN", "LD"});
    comboCode->setEditable(true);
    grid->addWidget(comboCode, 0, 5);

    // Row 1: Transaction Type | Element Type | Element Wt
    grid->addWidget(new QLabel("Transaction Type:"), 1, 0);
    comboType = new QComboBox;
    comboType->addItems({"", "Receipt", "Other Increase", "Shipment",
                         "Other Decrease", "Nuclear Loss", "PIL (Set Balance)"});
    grid->addWidget(comboType, 1, 1);

    grid->addWidget(new QLabel("Element:"), 1, 2);
    comboElem = new QComboBox;
    comboElem->addItems({
        "E — Enriched Uranium",
        "N — Natural Uranium",
        "D — Depleted Uranium",
        "P — Plutonium",
        "T — Thorium"
    });
    grid->addWidget(comboElem, 1, 3);

    grid->addWidget(new QLabel("Element Wt (g):"), 1, 4);
    spinElem = new QDoubleSpinBox;
    spinElem->setRange(0, 1000000);
    spinElem->setDecimals(2);
    grid->addWidget(spinElem, 1, 5);

    // Row 2: Items | Isotope label+spin | Buttons
    grid->addWidget(new QLabel("Items (if applicable):"), 2, 0);
    QDoubleSpinBox *spinItems = new QDoubleSpinBox;
    spinItems->setRange(0, 1000);
    spinItems->setDecimals(0);
    spinItems->setObjectName("spinItems");
    grid->addWidget(spinItems, 2, 1);

    lblIsoField = new QLabel("Isotope (U-235) Wt:");
    grid->addWidget(lblIsoField, 2, 2);
    spinIso = new QDoubleSpinBox;
    spinIso->setRange(0, 1000000);
    spinIso->setDecimals(3);
    grid->addWidget(spinIso, 2, 3);

    // Update isotope label when element type changes
    connect(comboElem, &QComboBox::currentTextChanged, [this](const QString &text){
        QString code = text.left(1);
        if (code == "P") {
            lblIsoField->setText("Pu-239 Wt (g):");
            spinIso->setEnabled(false);
            spinIso->setValue(0);
            spinIso->setToolTip("Not applicable for Plutonium");
        } else if (code == "T") {
            lblIsoField->setText("Th-232 Wt (g):");
            spinIso->setEnabled(true);
            spinIso->setToolTip("");
        } else {
            lblIsoField->setText("Isotope (U-235) Wt:");
            spinIso->setEnabled(true);
            spinIso->setToolTip("");
        }
    });

    // Buttons — right-aligned
    QHBoxLayout *btnLayout = new QHBoxLayout;
    btnLayout->setSpacing(8);
    btnLayout->addStretch();

    QPushButton *btnAdd = new QPushButton("Add Entry");
    btnAdd->setStyleSheet(BTN_PRIMARY); btnAdd->setMinimumHeight(32);
    connect(btnAdd, &QPushButton::clicked, this, &GeneralLedgerWidget::addEntry);

    QPushButton *btnExport = new QPushButton("Export PDF");
    btnExport->setStyleSheet(BTN_DARK); btnExport->setMinimumHeight(32);
    connect(btnExport, &QPushButton::clicked, this, &GeneralLedgerWidget::exportPDF);

    QPushButton *btnDel = new QPushButton("Delete Selected");
    btnDel->setStyleSheet(BTN_DANGER); btnDel->setMinimumHeight(32);
    connect(btnDel, &QPushButton::clicked, this, &GeneralLedgerWidget::deleteEntry);

    btnLayout->addWidget(btnAdd);
    btnLayout->addWidget(btnExport);
    btnLayout->addWidget(btnDel);

    grid->addLayout(btnLayout, 2, 4, 1, 2);

    layout->addWidget(grp);
}

void GeneralLedgerWidget::setupComplexTable(QVBoxLayout *layout) {
    table = new QTableWidget;
    table->setColumnCount(16);
    table->verticalHeader()->setVisible(false);
    table->horizontalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setStyleSheet(
        "QTableWidget {"
        "  border: 1px solid #003366;"
        "  gridline-color: #e0e0e0;"
        "}"
    );

    table->insertRow(0);
    table->insertRow(1);
    table->insertRow(2);

    auto setH = [&](int r, int c, QString txt) {
        QTableWidgetItem *it = new QTableWidgetItem(txt);
        it->setTextAlignment(Qt::AlignCenter);
        it->setBackground(QColor("#dce8f5")); // Light blue — matches #003366 theme
        it->setForeground(QColor("#003366"));
        it->setFont(QFont("Arial", 9, QFont::Bold));
        table->setItem(r, c, it);
    };

    setH(0, 0,  "Line");         table->setSpan(0, 0,  3, 1);
    setH(0, 1,  "Date");         table->setSpan(0, 1,  3, 1);
    setH(0, 2,  "ICD/PIL");      table->setSpan(0, 2,  3, 1);
    setH(0, 3,  "IC Code");      table->setSpan(0, 3,  3, 1);
    setH(0, 4,  "No. of\nItems");table->setSpan(0, 4,  3, 1);
    setH(0, 5,  "Increases");    table->setSpan(0, 5,  1, 4);
    setH(0, 9,  "Decreases");    table->setSpan(0, 9,  1, 4);
    setH(0, 13, "Inventory");    table->setSpan(0, 13, 1, 2);
    setH(0, 15, "No. of\nItems");table->setSpan(0, 15, 3, 1);

    setH(1, 5,  "Receipts");     table->setSpan(1, 5,  1, 2);
    setH(1, 7,  "Other");        table->setSpan(1, 7,  1, 2);
    setH(1, 9,  "Shipments");    table->setSpan(1, 9,  1, 2);
    setH(1, 11, "Other");        table->setSpan(1, 11, 1, 2);
    setH(1, 13, "Bal");          table->setSpan(1, 13, 1, 2);

    for (int i : {5, 7, 9, 11, 13})  setH(2, i, "U");
    for (int i : {6, 8, 10, 12, 14}) setH(2, i, "U-235");

    table->setColumnWidth(0, 40);
    table->setColumnWidth(1, 80);
    table->setColumnWidth(3, 60);
    table->setColumnWidth(4, 50);

    layout->addWidget(table);
}

// ─────────────────────────────────────────────────────────────────────────
// DATA OPERATIONS
// ─────────────────────────────────────────────────────────────────────────

void GeneralLedgerWidget::addEntry() {
    QMap<QString, QVariant> data;
    data["date"]       = dateEdit->text();
    data["ref"]        = txtRef->text();
    data["type"]       = comboType->currentText();
    data["u_weight"]    = spinElem->value();
data["u235_weight"] = spinIso->value();
data["code"]        = comboCode->currentText().isEmpty()
                      ? comboElem->currentText().left(1)  // auto-fill code from element
                      : comboCode->currentText();

    QDoubleSpinBox *spinItems = this->findChild<QDoubleSpinBox*>("spinItems");
    data["items"] = spinItems ? (int)spinItems->value() : 0;

    if (DatabaseManager::instance().addManualLedgerEntry(data)) {
        refreshData();
        emit dataChanged();
        txtRef->clear();
        spinElem->setValue(0);
        spinIso->setValue(0);
        if (spinItems) spinItems->setValue(0);
    }
}

void GeneralLedgerWidget::deleteEntry() {
    int row = table->currentRow();

    if (row < 3) {
        QMessageBox::warning(this, "Selection Error",
            "Please select a data row to delete (not the headers).");
        return;
    }

    if (DatabaseManager::instance().currentDatabaseName().contains("AIR_Training")) {
        PinDialog authDialog(this);
        if (authDialog.exec() != QDialog::Accepted) {
            qDebug() << "Zero Trust Policy: Deletion blocked.";
            return;
        }
    }

    QTableWidgetItem *item = table->item(row, 1);
    if (!item) return;

    int id = item->data(Qt::UserRole).toInt();
    if (id == 0) return;

    if (QMessageBox::question(this, "Confirm Deletion",
            "Are you sure you want to delete this ledger entry?\nThis action cannot be undone.",
            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        if (DatabaseManager::instance().deleteManualLedgerEntry(id)) {
            refreshData();
            emit dataChanged();
        } else {
            QMessageBox::critical(this, "Error", "Failed to delete entry from database.");
        }
    }
}

void GeneralLedgerWidget::refreshData() {
    table->setRowCount(3);
    balU = 0; balU235 = 0; balItems = 0; lineCounter = 1;

    QSqlQuery q = DatabaseManager::instance().getManualLedgerEntries();
    while (q.next()) {
        int r = table->rowCount();
        table->insertRow(r);

        int     dbID  = q.value("id").toInt();
        QString type  = q.value("type").toString();
        double  u     = q.value("u_weight").toDouble();
        double  u235  = q.value("u235_weight").toDouble();
        int     items = q.value("items").toInt();

        if      (type == "Receipt")                                 { balU += u; balU235 += u235; balItems += items; }
        else if (type == "Shipment")                                { balU -= u; balU235 -= u235; balItems -= items; }
        else if (type == "Other Increase")                          { balU += u; balU235 += u235; }
        else if (type == "Other Decrease" || type == "Nuclear Loss"){ balU -= u; balU235 -= u235; }
        else if (type == "PIL (Set Balance)") {
            if (lineCounter == 1 && balU == 0) { balU = u; balU235 = u235; balItems = items; }
        }

        table->setItem(r, 0, new QTableWidgetItem(QString::number(lineCounter++)));

        QTableWidgetItem *dateItem = new QTableWidgetItem(q.value("date").toString());
        dateItem->setData(Qt::UserRole, dbID);
        table->setItem(r, 1, dateItem);

        table->setItem(r, 2, new QTableWidgetItem(q.value("ref").toString()));
        table->setItem(r, 3, new QTableWidgetItem(q.value("code").toString()));

        QString displayItems = "";
        if (type == "Receipt" || type == "Shipment")
            displayItems = (items > 0 ? QString::number(items) : "");
        table->setItem(r, 4, new QTableWidgetItem(displayItems));

        for (int i = 5; i <= 12; i++)
            table->setItem(r, i, new QTableWidgetItem(""));

        if      (type == "Receipt")                                  { table->setItem(r, 5,  new QTableWidgetItem(QString::number(u)));    table->setItem(r, 6,  new QTableWidgetItem(QString::number(u235))); }
        else if (type == "Other Increase")                           { table->setItem(r, 7,  new QTableWidgetItem(QString::number(u)));    table->setItem(r, 8,  new QTableWidgetItem(QString::number(u235))); }
        else if (type == "Shipment")                                 { table->setItem(r, 9,  new QTableWidgetItem(QString::number(u)));    table->setItem(r, 10, new QTableWidgetItem(QString::number(u235))); }
        else if (type == "Other Decrease" || type == "Nuclear Loss") { table->setItem(r, 11, new QTableWidgetItem(QString::number(u)));    table->setItem(r, 12, new QTableWidgetItem(QString::number(u235))); }

        auto makeBal = [&](int col, double val) {
            QTableWidgetItem *it = new QTableWidgetItem(QString::number(val));
            it->setBackground(QColor("#e8f5e9"));
            it->setTextAlignment(Qt::AlignCenter);
            table->setItem(r, col, it);
        };
        makeBal(13, balU);
        makeBal(14, balU235);

        QTableWidgetItem *b3 = new QTableWidgetItem(QString::number(balItems));
        b3->setBackground(QColor("#e8f5e9"));
        b3->setTextAlignment(Qt::AlignCenter);
        table->setItem(r, 15, b3);
    }
}

void GeneralLedgerWidget::exportPDF() {
    if (DatabaseManager::instance().currentDatabaseName().contains("AIR_Training")) {
        PinDialog authDialog(this);
        if (authDialog.exec() != QDialog::Accepted) {
            qDebug() << "Zero Trust Policy: Export blocked.";
            return;
        }
    }

    QString fileName = QFileDialog::getSaveFileName(
        this, "Save General Ledger", "GL_Report.pdf", "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    QMap<QString, QString> header;
    header["facility"] = txtFacility->text();
    header["mba"]      = comboMBA->currentText();
    header["desc"]     = txtMatDesc->text();
    header["elemCode"] = txtElemCode->text();
    header["isoCode"]  = txtIsoCode->text();
    header["unit"]     = txtUnit->text();

    QSqlQuery data = DatabaseManager::instance().getManualLedgerEntries();

    if (ReportGenerator::generateGL_PDF(fileName, header, data)) {
        QMessageBox::information(this, "Success", "Report saved successfully.");
    } else {
        QMessageBox::critical(this, "Error", "Failed to save report.");
    }
}
