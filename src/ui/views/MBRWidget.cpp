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
#include <QPushButton>
#include <QSettings>

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

static const QString BTN_PRIMARY =
    "QPushButton {"
    "  background-color: #0056b3; color: white; font-weight: bold;"
    "  padding: 8px 16px; border-radius: 4px; border: none; font-size: 10pt;"
    "}"
    "QPushButton:hover { background-color: #004494; }";

static const QString BTN_DARK =
    "QPushButton {"
    "  background-color: #074282; color: white; font-weight: bold;"
    "  padding: 8px 16px; border-radius: 4px; border: none; font-size: 10pt;"
    "}"
    "QPushButton:hover { background-color: #003366; }";

static const QString BTN_DANGER =
    "QPushButton {"
    "  background-color: #c0392b; color: white; font-weight: bold;"
    "  padding: 8px 16px; border-radius: 4px; border: none; font-size: 10pt;"
    "}"
    "QPushButton:hover { background-color: #a93226; }";

static const QString BTN_NEUTRAL =
    "QPushButton {"
    "  background-color: #ecf0f1; color: #003366; font-weight: bold;"
    "  padding: 8px 12px; border-radius: 4px; border: 1px solid #003366; font-size: 10pt;"
    "}"
    "QPushButton:hover { background-color: #003366; color: white; }";

// ── Full IAEA country list (code + name) ──────────────────────────────────
static const QList<QPair<QString,QString>> COUNTRY_LIST = {
    {"AF","Afghanistan"},{"AL","Albania"},{"DZ","Algeria"},{"AR","Argentina"},
    {"AM","Armenia"},{"AU","Australia"},{"AT","Austria"},{"AZ","Azerbaijan"},
    {"BD","Bangladesh"},{"BY","Belarus"},{"BE","Belgium"},{"BZ","Belize"},
    {"BR","Brazil"},{"BG","Bulgaria"},{"CM","Cameroon"},{"CA","Canada"},
    {"CL","Chile"},{"CN","China"},{"CO","Colombia"},{"HR","Croatia"},
    {"CU","Cuba"},{"CZ","Czech Republic"},{"DK","Denmark"},{"EC","Ecuador"},
    {"EG","Egypt"},{"ET","Ethiopia"},{"FI","Finland"},{"FR","France"},
    {"GE","Georgia"},{"DE","Germany"},{"GH","Ghana"},{"GR","Greece"},
    {"HU","Hungary"},{"IN","India"},{"ID","Indonesia"},{"IR","Iran"},
    {"IQ","Iraq"},{"IE","Ireland"},{"IL","Israel"},{"IT","Italy"},
    {"JP","Japan"},{"JO","Jordan"},{"KZ","Kazakhstan"},{"KE","Kenya"},
    {"KR","Republic of Korea"},{"KW","Kuwait"},{"KG","Kyrgyzstan"},
    {"LA","Laos"},{"LB","Lebanon"},{"LY","Libya"},{"LT","Lithuania"},
    {"MX","Mexico"},{"MA","Morocco"},{"MM","Myanmar"},{"NL","Netherlands"},
    {"NZ","New Zealand"},{"NG","Nigeria"},{"NO","Norway"},{"PK","Pakistan"},
    {"PE","Peru"},{"PH","Philippines"},{"PL","Poland"},{"PT","Portugal"},
    {"QA","Qatar"},{"RO","Romania"},{"RU","Russia"}, {"RW","Rwanda"},{"SA","Saudi Arabia"},
    {"SN","Senegal"},{"RS","Serbia"},{"SK","Slovakia"},{"SI","Slovenia"},
    {"ZA","South Africa"},{"ES","Spain"},{"LK","Sri Lanka"},{"SD","Sudan"},
    {"SE","Sweden"},{"CH","Switzerland"},{"SY","Syria"},{"TW","Taiwan"},
    {"TJ","Tajikistan"},{"TH","Thailand"},{"TN","Tunisia"},{"TR","Turkey"},
    {"TM","Turkmenistan"},{"UA","Ukraine"},{"AE","United Arab Emirates"},
    {"GB","United Kingdom"},{"US","United States"},{"UZ","Uzbekistan"},
    {"VE","Venezuela"},{"VN","Vietnam"},{"YE","Yemen"},{"ZW","Zimbabwe"},
};

// ─────────────────────────────────────────────────────────────────────────

MBRWidget::MBRWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
}

void MBRWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 15, 15, 15);
    mainLayout->setSpacing(16);

    QLabel *title = new QLabel("<h2>Material Balance Report (MBR)</h2>");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: #003366; font-weight: bold; padding: 6px 0px;");
    mainLayout->addWidget(title);

    setupHeader();
    setupInputForm();
    setupTable();
    loadData();
}

void MBRWidget::setupHeader() {
    QGroupBox *headerGrp = new QGroupBox("Report Information");
    headerGrp->setStyleSheet(GB_STYLE);

    QGridLayout *grid = new QGridLayout(headerGrp);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(10);
    grid->setContentsMargins(10, 8, 10, 10);

    // ── Country: searchable combo with full IAEA list ──
    grid->addWidget(new QLabel("Country:"), 0, 0);
    comboCountry = new QComboBox();
    comboCountry->setEditable(true);
    comboCountry->setInsertPolicy(QComboBox::NoInsert);
    comboCountry->completer()->setCompletionMode(QCompleter::PopupCompletion);
    comboCountry->completer()->setFilterMode(Qt::MatchContains);
    for (auto &p : COUNTRY_LIST)
        comboCountry->addItem(p.first + " — " + p.second, p.first);
    // Default to Austria as example
    comboCountry->setCurrentIndex(comboCountry->findData("AT"));
    grid->addWidget(comboCountry, 0, 1);

    grid->addWidget(new QLabel("Reporting Period From:"), 0, 2);
    dateFrom = new QDateEdit(QDate::currentDate().addMonths(-1));
    dateFrom->setCalendarPopup(true);
    dateFrom->setDisplayFormat("yyyy-MM-dd");
    grid->addWidget(dateFrom, 0, 3);

    grid->addWidget(new QLabel("To:"), 0, 4);
    dateTo = new QDateEdit(QDate::currentDate());
    dateTo->setCalendarPopup(true);
    dateTo->setDisplayFormat("yyyy-MM-dd");
    grid->addWidget(dateTo, 0, 5);

    // ── Facility: editable with placeholder ──
    grid->addWidget(new QLabel("Facility:"), 1, 0);
    txtFacility = new QLineEdit();
    txtFacility->setPlaceholderText("e.g. Compton Research Reactor");
    txtFacility->setText("Compton Research Reactor");
    grid->addWidget(txtFacility, 1, 1, 1, 3);

    // ── Report No: spinbox (numbers only) ──
    grid->addWidget(new QLabel("Report No:"), 1, 4);
    spinReportNo = new QSpinBox();
    spinReportNo->setRange(1, 99999);
    spinReportNo->setValue(1);
    grid->addWidget(spinReportNo, 1, 5);

    // ── MBA: editable combo — user can type new ones, saved for reuse ──
    grid->addWidget(new QLabel("Material Balance Area:"), 2, 0);

    QHBoxLayout *mbaLay = new QHBoxLayout;
    mbaLay->setSpacing(6);

    comboMBA = new QComboBox();
    comboMBA->setEditable(true);
    comboMBA->setInsertPolicy(QComboBox::NoInsert); // We handle insert manually
    // Load default MBAs
    comboMBA->addItems({"CRRF", "EULE", "EXT", "DKNZ"});
    // Load any user-saved MBAs from QSettings
    QSettings settings;
    QStringList savedMBAs = settings.value("MBR/savedMBAs").toStringList();
    for (const QString &m : savedMBAs)
        if (comboMBA->findText(m) < 0) // avoid duplicates
            comboMBA->addItem(m);

    // Save MBA button
    QPushButton *btnSaveMBA = new QPushButton("Save MBA");
    btnSaveMBA->setStyleSheet(BTN_NEUTRAL);
    btnSaveMBA->setFixedHeight(30);
    btnSaveMBA->setToolTip("Save current MBA text for future use");
    btnSaveMBA->setCursor(Qt::PointingHandCursor);
    connect(btnSaveMBA, &QPushButton::clicked, [this](){
        QString text = comboMBA->currentText().trimmed();
        if (text.isEmpty()) return;
        if (comboMBA->findText(text) < 0) {
            comboMBA->addItem(text);
            // Persist to QSettings
            QSettings s;
            QStringList saved = s.value("MBR/savedMBAs").toStringList();
            if (!saved.contains(text)) {
                saved << text;
                s.setValue("MBR/savedMBAs", saved);
            }
            QMessageBox::information(this, "MBA Saved",
                QString("'%1' has been saved and will be available in future sessions.").arg(text));
        } else {
            QMessageBox::information(this, "MBA Exists",
                QString("'%1' is already in the list.").arg(text));
        }
    });

    mbaLay->addWidget(comboMBA, 1);
    mbaLay->addWidget(btnSaveMBA);
    grid->addLayout(mbaLay, 2, 1, 1, 3);

    static_cast<QVBoxLayout*>(layout())->addWidget(headerGrp);
}

void MBRWidget::setupInputForm() {
    QGroupBox *grp = new QGroupBox("Add Accountancy Entry");
    grp->setStyleSheet(GB_STYLE);

    QGridLayout *grid = new QGridLayout(grp);
    grid->setHorizontalSpacing(12);
    grid->setVerticalSpacing(10);
    grid->setContentsMargins(10, 8, 10, 10);

    // Row 0: Entry Name | Element | Weight
    grid->addWidget(new QLabel("Entry Name:"), 0, 0);
    comboEntryName = new QComboBox();
    comboEntryName->addItems({"PB", "RD", "LN", "SF", "SD", "BA", "PE", "NP", "MF"});
    grid->addWidget(comboEntryName, 0, 1);

    grid->addWidget(new QLabel("Element:"), 0, 2);
    comboElement = new QComboBox();
    comboElement->addItems({"E — Enriched U", "N — Natural U", "D — Depleted U",
                             "P — Plutonium",  "T — Thorium"});
    grid->addWidget(comboElement, 0, 3);

    grid->addWidget(new QLabel("Weight of Element:"), 0, 4);
    spinWeight = new QDoubleSpinBox();
    spinWeight->setRange(-99999999, 99999999);
    spinWeight->setDecimals(2);
    grid->addWidget(spinWeight, 0, 5);

    // Row 1: Unit | Fissile Weight | Isotope Code (selectable)
    grid->addWidget(new QLabel("Unit (Kg/g):"), 1, 0);
    comboUnit = new QComboBox();
    comboUnit->addItems({"G", "KG"});
    grid->addWidget(comboUnit, 1, 1);

    grid->addWidget(new QLabel("Fissile Weight (g):"), 1, 2);
    spinFissile = new QDoubleSpinBox();
    spinFissile->setRange(-99999999, 99999999);
    spinFissile->setDecimals(2);
    grid->addWidget(spinFissile, 1, 3);

    // Isotope code — selectable like Element, matching IAEA codes
    grid->addWidget(new QLabel("Isotope Code:"), 1, 4);
    comboIsotopeCode = new QComboBox();
    comboIsotopeCode->addItems({
        "G — U-235 (Enriched)",
        "H — U-233",
        "C — U-235 (Natural/Depleted)",
        "F — Pu-239",
        "J — Pu-241",
        "N — Np-237",
        "T — Th-232",
        "X — Other"
    });
    grid->addWidget(comboIsotopeCode, 1, 5);

    // Row 2: Continuation | Report No (entry-level) | Buttons
    grid->addWidget(new QLabel("Continuation:"), 2, 0);
    txtContinuation = new QLineEdit();
    txtContinuation->setPlaceholderText("e.g. 1, 2 ...");
    grid->addWidget(txtContinuation, 2, 1);

    grid->addWidget(new QLabel("Entry Report No:"), 2, 2);
    spinEntryReportNo = new QSpinBox();
    spinEntryReportNo->setRange(1, 99999);
    spinEntryReportNo->setValue(1);
    grid->addWidget(spinEntryReportNo, 2, 3);

    // Buttons
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(8);
    btnLayout->addStretch();

    QPushButton *btnAdd = new QPushButton("Add Entry");
    btnAdd->setStyleSheet(BTN_PRIMARY);
    btnAdd->setMinimumHeight(32);
    connect(btnAdd, &QPushButton::clicked, this, &MBRWidget::addEntry);

    btnExport = new QPushButton("Export PDF");
    btnExport->setStyleSheet(BTN_DARK);
    btnExport->setMinimumHeight(32);
    connect(btnExport, &QPushButton::clicked, this, &MBRWidget::exportPDF);

    QPushButton *btnDel = new QPushButton("Delete Selected");
    btnDel->setStyleSheet(BTN_DANGER);
    btnDel->setMinimumHeight(32);
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
        "Weight Of Fissile\nIsotopes\n(Uranium Only)\n(G)",
        "Isotope Code", "Report\nNo"
    });

    table->verticalHeader()->setVisible(false);
    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setStyleSheet(
        "QHeaderView::section { background-color: #f0f0f0; font-weight: bold;"
        "  border: 1px solid #ccc; padding: 4px; color: #003366; }"
        "QTableWidget { border: 1px solid #003366; gridline-color: #e0e0e0; }"
    );

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

// ─────────────────────────────────────────────────────────────────────────
// DATA OPERATIONS
// ─────────────────────────────────────────────────────────────────────────

void MBRWidget::addEntry() {
    // Extract just the code letter from the combo (e.g. "E — Enriched U" → "E")
    QString elemCode    = comboElement->currentText().left(1);
    QString isotopeCode = comboIsotopeCode->currentText().left(1);

    QMap<QString, QVariant> data;
    data["continuation"] = txtContinuation->text();
    data["entry_name"]   = comboEntryName->currentText();
    data["element"]      = elemCode;
    data["weight"]       = spinWeight->value();
    data["unit"]         = comboUnit->currentText();
    data["fissile"]      = spinFissile->value();
    data["isotope"]      = isotopeCode;
    data["report_no"]    = QString::number(spinEntryReportNo->value());

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

    if (DatabaseManager::instance().currentDatabaseName().contains("AIR_Training")) {
        PinDialog authDialog(this);
        if (authDialog.exec() != QDialog::Accepted) {
            qDebug() << "Zero Trust Policy: Deletion blocked.";
            return;
        }
    }

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
    while (q.next()) {
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
    if (table->rowCount() == 0) {
        QMessageBox::warning(this, "Export Error", "The list is empty. Add entries first.");
        return;
    }

    if (DatabaseManager::instance().currentDatabaseName().contains("AIR_Training")) {
        PinDialog authDialog(this);
        if (authDialog.exec() != QDialog::Accepted) {
            qDebug() << "Zero Trust Policy: Export blocked.";
            return;
        }
    }

    QString fileName = QFileDialog::getSaveFileName(
        this, "Save MBR", "MBR_Report.pdf", "PDF Files (*.pdf)");
    if (fileName.isEmpty()) return;

    // Extract country code from combo (e.g. "AT — Austria" → "AT")
    QString countryCode = comboCountry->currentData().toString();
    if (countryCode.isEmpty())
        countryCode = comboCountry->currentText().left(2).toUpper();

    QMap<QString, QString> headerData;
    headerData["country"]    = countryCode;
    headerData["facility"]   = txtFacility->text();
    headerData["mba"]        = comboMBA->currentText();
    headerData["periodFrom"] = dateFrom->text();
    headerData["periodTo"]   = dateTo->text();
    headerData["reportNo"]   = QString::number(spinReportNo->value());

    if (ReportGenerator::generateMBR_PDF(fileName, headerData, table)) {
        QMessageBox::information(this, "Success", "MBR PDF generated successfully.");
    } else {
        QMessageBox::critical(this, "Error", "Failed to generate PDF.");
    }
}
