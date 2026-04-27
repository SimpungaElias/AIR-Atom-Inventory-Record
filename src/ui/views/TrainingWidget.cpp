#include "TrainingWidget.h"
#include "../../db/DatabaseManager.h"
#include <QMessageBox>
#include <QFrame>
#include <QTabWidget>
#include <QRegularExpression>
#include <QScrollArea>

// ── Consistent with all other AIR widgets ────────────────────────────────
static const QString CARD_STYLE =
    "QFrame {"
    "  background-color: white;"
    "  border: 1px solid #003366;"
    "  border-radius: 6px;"
    "}"
    "QFrame:hover {"
    "  border: 2px solid #0056b3;"
    "  background-color: #f0f8ff;"
    "}";

static const QString BTN_START =
    "QPushButton {"
    "  background-color: #0056b3;"
    "  color: white;"
    "  font-weight: bold;"
    "  padding: 8px 16px;"
    "  border-radius: 4px;"
    "  border: none;"
    "  font-size: 10pt;"
    "}"
    "QPushButton:hover { background-color: #004494; }"
    "QPushButton:pressed { background-color: #003366; }";

static const QString BTN_DISABLED =
    "QPushButton {"
    "  background-color: #bdc3c7;"
    "  color: #7f8c8d;"
    "  font-weight: bold;"
    "  padding: 8px 16px;"
    "  border-radius: 4px;"
    "  border: none;"
    "  font-size: 10pt;"
    "}";

TrainingWidget::TrainingWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
}

void TrainingWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 15, 20, 15);
    mainLayout->setSpacing(10);

    // ── Header ────────────────────────────────────────────────────────────
    QLabel *header = new QLabel("<h2>Safeguards Training Simulator</h2>");
    header->setStyleSheet("color: #003366; font-weight: bold;");
    mainLayout->addWidget(header);

    // ── Info Banner ───────────────────────────────────────────────────────
    QWidget *banner = new QWidget;
    banner->setStyleSheet(
        "QWidget {"
        "  background-color: #f0f8ff;"
        "  border: 1px solid #003366;"
        "  border-radius: 4px;"
        "  padding: 2px;"
        "}"
    );
    QHBoxLayout *bannerLay = new QHBoxLayout(banner);
    bannerLay->setContentsMargins(14, 10, 14, 10);

    QLabel *bannerIcon = new QLabel("ℹ");
    bannerIcon->setStyleSheet("color: #003366; font-size: 16pt; font-weight: bold; border: none; background: transparent;");
    bannerIcon->setFixedWidth(24);

    QLabel *bannerText = new QLabel(
        "<b>Training Mode</b> — All scenarios run in an isolated database. "
        "Your real operational data is never affected. "
        "Scenarios are designed for students, new inspectors, and early-career NMAC practitioners. "
    );
    bannerText->setWordWrap(true);
    bannerText->setStyleSheet("color: #003366; font-size: 9pt; border: none; background: transparent;");

    bannerLay->addWidget(bannerIcon);
    bannerLay->addWidget(bannerText, 1);
    mainLayout->addWidget(banner);

    // ── Tab Widget ────────────────────────────────────────────────────────
    QTabWidget *tabWidget = new QTabWidget();
    tabWidget->setStyleSheet(
        "QTabWidget::pane { border: 1px solid #003366; border-radius: 4px; }"
        "QTabBar::tab {"
        "  padding: 8px 20px;"
        "  font-weight: bold;"
        "  color: #003366;"
        "  border: 1px solid #ccc;"
        "  border-bottom: none;"
        "  border-radius: 4px 4px 0 0;"
        "  background-color: #ecf0f1;"
        "  margin-right: 2px;"
        "}"
        "QTabBar::tab:selected {"
        "  background-color: #003366;"
        "  color: white;"
        "}"
        "QTabBar::tab:hover:!selected {"
        "  background-color: #dce8f5;"
        "}"
    );

    // ── TAB 1: FUNDAMENTALS ───────────────────────────────────────────────
    QWidget *tab1 = new QWidget();
    tab1->setStyleSheet("background-color: white;");
    QVBoxLayout *tab1Lay = new QVBoxLayout(tab1);
    tab1Lay->setContentsMargins(12, 12, 12, 12);
    tab1Lay->setSpacing(8);

    QLabel *tab1Desc = new QLabel(
        "<b>Level 1 — Fundamentals</b>: Learn the core concepts of Nuclear Material Accountancy. "
        "No prior experience required. These modules introduce the IAEA reporting framework, "
        "the role of MBAs, KMPs, and how inventory change documents are created and recorded."
    );
    tab1Desc->setWordWrap(true);
    tab1Desc->setStyleSheet("color: #555; font-size: 9pt; padding: 4px 0; background: transparent;");
    tab1Lay->addWidget(tab1Desc);

    QGridLayout *grid1 = new QGridLayout;
    grid1->setSpacing(16);
    grid1->setAlignment(Qt::AlignTop);

    grid1->addWidget(createScenarioCard(
        "Module 1.1 — The Baseline Receipt",
        "You receive a fresh fuel shipment from an external supplier. "
        "Using the accompanying Inventory Change Document (ICD), record the receipt "
        "in the General Ledger and update the Material Balance Area inventory.",
        {"Record a Receipt transaction in the ICR module",
         "Understand the structure of an Inventory Change Document (ICD)",
         "Identify the correct MBA and KMP for the material",
         "Verify the General Ledger balance after entry"},
        "Beginner", "30 min", "None", "scen_baseline"
    ), 0, 0);

    grid1->addWidget(createScenarioCard(
        "Module 1.2 — The Reactor Cycle",
        "A fuel assembly is transferred from the Fresh Fuel Store (FFS KMP) to the "
        "Reactor Core (RRC KMP) for irradiation. Log the internal transfer, "
        "record burnup after the cycle, and account for Pu-239 production.",
        {"Record an internal material transfer between KMPs",
         "Understand how nuclear transformations are reported",
         "Log burnup values using the LII module",
         "Account for Pu production as a Nuclear Increase"},
        "Beginner", "45 min", "Module 1.1", "scen_reactor"
    ), 0, 1);

    tab1Lay->addLayout(grid1);
    tab1Lay->addStretch();
    tabWidget->addTab(tab1, "  Level 1: Fundamentals  ");

    // ── TAB 2: RECONCILIATION ─────────────────────────────────────────────
    // Wrapped in a scroll area because 3 cards can exceed visible height
    QScrollArea *scroll2 = new QScrollArea();
    scroll2->setWidgetResizable(true);
    scroll2->setFrameShape(QFrame::NoFrame);
    scroll2->setStyleSheet("QScrollArea { background-color: white; border: none; }");

    QWidget *tab2 = new QWidget();
    tab2->setStyleSheet("background-color: white;");
    QVBoxLayout *tab2Lay = new QVBoxLayout(tab2);
    tab2Lay->setContentsMargins(12, 12, 12, 12);
    tab2Lay->setSpacing(8);

    QLabel *tab2Desc = new QLabel(
        "<b>Level 2 — Reconciliation</b>: Develop skills in identifying and resolving "
        "inventory discrepancies. These modules cover the Physical Inventory Taking (PIT) "
        "process, Shipper/Receiver Differences (SRD), and Material Unaccounted For (MUF) evaluation — "
        "core competencies for any NMAC practitioner."
    );
    tab2Desc->setWordWrap(true);
    tab2Desc->setStyleSheet("color: #555; font-size: 9pt; padding: 4px 0; background: transparent;");
    tab2Lay->addWidget(tab2Desc);

    QGridLayout *grid2 = new QGridLayout;
    grid2->setSpacing(16);
    grid2->setAlignment(Qt::AlignTop);

    grid2->addWidget(createScenarioCard(
        "Module 2.1 — Shipper/Receiver Differences (SRD)",
        "A shipment of UO2 powder arrives. Your facility's measurement indicates "
        "a weight 15g below the shipper's declared value. "
        "Follow the IAEA protocol for resolving a Shipper/Receiver Difference.",
        {"Identify and document a measurement discrepancy",
         "Apply the SRD resolution procedure per INFCIRC/153",
         "Determine whether the difference is within acceptable limits",
         "Generate and review the ICR with the correct change code"},
        "Intermediate", "60 min", "Level 1 complete", "scen_srd"
    ), 0, 0);

    grid2->addWidget(createScenarioCard(
        "Module 2.2 — Physical Inventory Taking (PIT)",
        "It is end-of-year. Conduct a full Physical Inventory Taking at your facility. "
        "Verify every item's serial number and weight against the book inventory "
        "and generate the Physical Inventory Listing (PIL / LII).",
        {"Conduct a systematic item-by-item physical count",
         "Compare physical inventory to book inventory",
         "Record all items in the LII module",
         "Generate and review the PIL report for IAEA submission"},
        "Intermediate", "75 min", "Module 2.1", "scen_pit"
    ), 0, 1);

    grid2->addWidget(createScenarioCard(
        "Module 2.3 — MUF Evaluation",
        "Your PIT reveals a +2.1 kg unexplained discrepancy. "
        "Calculate the Material Unaccounted For (MUF) and its uncertainty (σMUF) "
        "based on measurement uncertainties to determine statistical significance.",
        {"Calculate MUF = Physical Inventory − Book Inventory",
         "Identify contributions to σMUF from scale calibrations",
         "Apply the D-statistic test to evaluate significance",
         "Prepare the Material Balance Report (MBR)"},
        "Intermediate", "90 min", "Module 2.2", "scen_muf"
    ), 1, 0);

    // Empty placeholder card to balance the 2-column grid
    QWidget *placeholder = new QWidget();
    placeholder->setStyleSheet("background: transparent; border: none;");
    grid2->addWidget(placeholder, 1, 1);

    tab2Lay->addLayout(grid2);
    tab2Lay->addStretch();

    scroll2->setWidget(tab2);
    tabWidget->addTab(scroll2, "  Level 2: Reconciliation  ");

    // ── TAB 3: ADVANCED ───────────────────────────────────────────────────
    QWidget *tab3 = new QWidget();
    tab3->setStyleSheet("background-color: white;");
    QVBoxLayout *tab3Lay = new QVBoxLayout(tab3);
    tab3Lay->setContentsMargins(12, 12, 12, 12);
    tab3Lay->setSpacing(8);

    QLabel *tab3Desc = new QLabel(
        "<b>Level 3 — Advanced Safeguards</b>: These scenarios simulate realistic diversion "
        "attempts and anomaly detection. They are designed for inspectors and senior "
        "accountancy officers who need to recognize subtle indicators of material misuse. "
        "The tamper-evident audit log and hash chain verification are central to these exercises."
    );
    tab3Desc->setWordWrap(true);
    tab3Desc->setStyleSheet("color: #555; font-size: 9pt; padding: 4px 0; background: transparent;");
    tab3Lay->addWidget(tab3Desc);

    QGridLayout *grid3 = new QGridLayout;
    grid3->setSpacing(16);
    grid3->setAlignment(Qt::AlignTop);

    grid3->addWidget(createScenarioCard(
        "Module 3.1 — Protracted Diversion Detection",
        "Two years of ledger data contain a systemic bias — small, consistent "
        "underreporting of element weights. Analyze the audit trail and use "
        "the tamper-evident hash chain to identify the point of first anomaly.",
        {"Review multi-year General Ledger trends for statistical bias",
         "Use the Home Dashboard tamper-detection view to locate hash breaks",
         "Calculate cumulative MUF over the reporting period",
         "Draft a findings summary for the Safeguards Officer"},
        "Advanced", "120 min", "All Level 2 modules", "scen_protracted"
    ), 0, 0);

    grid3->addWidget(createScenarioCard(
        "Module 3.2 — The Substituted Dummy Assembly",
        "During a PIT, an irradiated fuel assembly's serial number matches the ledger, "
        "but visual inspection via the Cherenkov Viewing Device (ICVD) shows anomalies. "
        "The LII entry and hash chain tell a different story from the physical evidence.",
        {"Cross-reference LII entries against physical inspection findings",
         "Identify the discrepancy between declared and observed radiation signature",
         "Verify audit log integrity using the hash chain",
         "Apply the IAEA's item-level verification protocol"},
        "Advanced", "120 min", "All Level 2 modules", "scen_dummy"
    ), 0, 1);

    tab3Lay->addLayout(grid3);
    tab3Lay->addStretch();
    tabWidget->addTab(tab3, "  Level 3: Advanced  ");

    // ── TAB 4: REFERENCE ─────────────────────────────────────────────────
    QWidget *tab4 = new QWidget();
    tab4->setStyleSheet("background-color: white;");
    QVBoxLayout *tab4Lay = new QVBoxLayout(tab4);
    tab4Lay->setContentsMargins(16, 16, 16, 16);
    tab4Lay->setSpacing(12);

    QLabel *refTitle = new QLabel("<b>Quick Reference — NMAC Concepts & IAEA Terminology</b>");
    refTitle->setStyleSheet("color: #003366; font-size: 11pt;");
    tab4Lay->addWidget(refTitle);

    struct GlossaryItem { QString term; QString definition; };
    QList<GlossaryItem> glossary = {
        {"MBA — Material Balance Area",
         "A defined area in which the quantity of nuclear material can be determined, "
         "between which transfers are measured. Accountancy is maintained within each MBA."},
        {"KMP — Key Measurement Point",
         "A location where nuclear material appears in a form allowing it to be measured. "
         "Examples: FFS (Fresh Fuel Store), RRC (Reactor Core), LOF (Loaded Fuel)."},
        {"MUF — Material Unaccounted For",
         "MUF = Physical Inventory − (Previous Inventory + Increases − Decreases). "
         "A non-zero MUF triggers an investigation. It is compared to σMUF for significance."},
        {"ICD — Inventory Change Document",
         "A document recording a change in the quantity or form of nuclear material "
         "within or between MBAs. Basis for ICR (Inventory Change Report) submissions."},
        {"ICR — Inventory Change Report",
         "An IAEA report submitted to document all nuclear material inventory changes "
         "during a reporting period. Includes receipts, shipments, nuclear losses, and production."},
        {"PIL — Physical Inventory Listing",
         "A complete list of all nuclear material items present in an MBA at the time "
         "of a Physical Inventory Taking (PIT). Also known as LII in this system."},
        {"SRD — Shipper/Receiver Difference",
         "A discrepancy between the nuclear material quantity declared by the shipper "
         "and the quantity measured by the receiver. Must be reported if above threshold."},
        {"MBR — Material Balance Report",
         "A periodic report summarizing all inventory changes and the resulting material "
         "balance. Required for IAEA safeguards verification under INFCIRC/153."},
        {"σMUF — Uncertainty of MUF",
         "The combined standard uncertainty of the MUF value, calculated from measurement "
         "uncertainties. Used to determine whether a MUF is statistically significant."},
        {"INFCIRC/153",
         "The IAEA document 'The Structure and Content of Agreements between the Agency "
         "and States Required in Connection with the Treaty on the Non-Proliferation of "
         "Nuclear Weapons.' The legal basis for safeguards agreements."},
    };

    for (auto &item : glossary) {
        QWidget *row = new QWidget;
        row->setStyleSheet(
            "QWidget { background-color: #f9f9f9; border: 1px solid #003366;"
            "  border-radius: 4px; }"
        );
        QHBoxLayout *rowLay = new QHBoxLayout(row);
        rowLay->setContentsMargins(14, 10, 14, 10);
        rowLay->setSpacing(16);

        QLabel *term = new QLabel(item.term);
        term->setFixedWidth(220);
        term->setWordWrap(true);
        term->setStyleSheet("color: #003366; font-weight: bold; font-size: 9pt;"
                            "border: none; background: transparent;");

        QLabel *def = new QLabel(item.definition);
        def->setWordWrap(true);
        def->setStyleSheet("color: #333; font-size: 9pt; border: none; background: transparent;");

        rowLay->addWidget(term);

        QFrame *divider = new QFrame;
        divider->setFrameShape(QFrame::VLine);
        divider->setStyleSheet("color: #003366; background-color: #003366; max-width: 1px;");
        rowLay->addWidget(divider);

        rowLay->addWidget(def, 1);
        tab4Lay->addWidget(row);
    }

    tab4Lay->addStretch();
    tabWidget->addTab(tab4, "  Reference Glossary  ");

    mainLayout->addWidget(tabWidget, 1);
}

QWidget* TrainingWidget::createScenarioCard(
    const QString &title,
    const QString &desc,
    const QStringList &objectives,
    const QString &difficulty,
    const QString &duration,
    const QString &prereq,
    const QString &id)
{
    QFrame *card = new QFrame;
    card->setStyleSheet(CARD_STYLE);
    card->setMinimumHeight(280);
    card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    QVBoxLayout *lay = new QVBoxLayout(card);
    lay->setContentsMargins(16, 14, 16, 14);
    lay->setSpacing(8);

    // ── Title ──
    QLabel *lblTitle = new QLabel(title);
    lblTitle->setWordWrap(true);
    lblTitle->setStyleSheet("font-size: 11pt; font-weight: bold; color: #003366; border: none; background: transparent;");
    lay->addWidget(lblTitle);

    // ── Difficulty + Duration badges ──
    QHBoxLayout *badgeLay = new QHBoxLayout;
    badgeLay->setSpacing(8);

    QString diffColor = (difficulty == "Beginner") ? "#27ae60"
                      : (difficulty == "Intermediate") ? "#e67e22" : "#c0392b";
    QString diffBg    = (difficulty == "Beginner") ? "#eafaf1"
                      : (difficulty == "Intermediate") ? "#fef5e7" : "#fdedec";

    QLabel *lblDiff = new QLabel("● " + difficulty);
    lblDiff->setStyleSheet(QString("color: %1; background-color: %2; font-size: 8pt; font-weight: bold;"
                                   "border: 1px solid %1; border-radius: 3px; padding: 2px 8px;")
                           .arg(diffColor, diffBg));

    QLabel *lblDur = new QLabel("⏱ " + duration);
    lblDur->setStyleSheet("color: #555; background-color: #f0f0f0; font-size: 8pt;"
                          "border: 1px solid #ccc; border-radius: 3px; padding: 2px 8px;");

    QLabel *lblPre = new QLabel("Prerequisites: " + prereq);
    lblPre->setStyleSheet("color: #777; font-size: 8pt; border: none; background: transparent;");

    badgeLay->addWidget(lblDiff);
    badgeLay->addWidget(lblDur);
    badgeLay->addStretch();
    lay->addLayout(badgeLay);
    lay->addWidget(lblPre);

    // ── Separator ──
    QFrame *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background-color: #dce8f5; border: none; max-height: 1px;");
    lay->addWidget(sep);

    // ── Description ──
    QLabel *lblDesc = new QLabel(desc);
    lblDesc->setWordWrap(true);
    lblDesc->setStyleSheet("color: #444; font-size: 9pt; border: none; background: transparent;");
    lay->addWidget(lblDesc);

    // ── Learning Objectives ──
    QLabel *objTitle = new QLabel("Learning Objectives:");
    objTitle->setStyleSheet("color: #003366; font-size: 8pt; font-weight: bold;"
                            "border: none; background: transparent; margin-top: 4px;");
    lay->addWidget(objTitle);

    for (const QString &obj : objectives) {
        QLabel *objItem = new QLabel("  ✓  " + obj);
        objItem->setWordWrap(true);
        objItem->setStyleSheet("color: #555; font-size: 8pt; border: none; background: transparent;");
        lay->addWidget(objItem);
    }

    lay->addStretch();

    // ── Start Button ──
    QPushButton *btnStart = new QPushButton("Start Scenario");
    btnStart->setStyleSheet(BTN_START);
    btnStart->setCursor(Qt::PointingHandCursor);

    QString cleanDesc = desc;
    cleanDesc.remove(QRegularExpression("<[^>]*>"));

    connect(btnStart, &QPushButton::clicked, [this, id, cleanDesc](){
        launchScenario(id, cleanDesc);
    });

    lay->addWidget(btnStart);

    return card;
}

void TrainingWidget::launchScenario(QString name, QString description) {
    QMessageBox confirmBox(this);
    confirmBox.setWindowTitle("Launch Training Scenario");
    confirmBox.setIcon(QMessageBox::Question);
    confirmBox.setText("<b>Launch Training Simulator?</b>");
    confirmBox.setInformativeText(
        description + "\n\n"
        "The simulator will load an isolated training database.\n"
        "Your real operational data will not be affected."
    );
    confirmBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    confirmBox.setDefaultButton(QMessageBox::Yes);
    confirmBox.button(QMessageBox::Yes)->setText("Launch Scenario");
    confirmBox.button(QMessageBox::Cancel)->setText("Cancel");

    if (confirmBox.exec() != QMessageBox::Yes) return;

    DatabaseManager::instance().connectToScenario(name);

    QMessageBox info(this);
    info.setWindowTitle("Training Mode Active");
    info.setIcon(QMessageBox::Information);
    info.setText("<b>Training Mode is now active.</b>");
    info.setInformativeText(
        "You are working in an isolated training environment.\n\n"
        "• All data entry is saved to the training database only.\n"
        "• The audit log and tamper detection are fully active.\n"
        "• Use  Exit Simulation  in the top bar to return to operational mode."
    );
    info.exec();

    emit scenarioStarted(name);
}
