#include "TrainingWidget.h"
#include "../../db/DatabaseManager.h"
#include <QMessageBox>
#include <QFrame>
#include <QTabWidget> // <--- REQUIRED FOR THE NEW UI

TrainingWidget::TrainingWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
}

void TrainingWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // Title
    QLabel *header = new QLabel("<h2>Safeguards Training Simulator</h2>");
    header->setStyleSheet("color: #2c3e50;");
    mainLayout->addWidget(header);

    // Subtitle with Glossary Tooltips
    QLabel *sub = new QLabel("Select a training scenario to launch the simulator environment.<br>"
                             "Hover over acronyms like <span style='text-decoration: underline; color: #3498db;' title='Material Balance Area'>MBA</span> or <span style='text-decoration: underline; color: #3498db;' title='Material Unaccounted For'>MUF</span> for definitions.");
    sub->setStyleSheet("color: #7f8c8d; font-size: 11pt; margin-bottom: 10px;");
    mainLayout->addWidget(sub);

    // --- CREATE TABS FOR PROGRESSION ---
    QTabWidget *tabWidget = new QTabWidget();
    tabWidget->setStyleSheet("QTabBar::tab { padding: 10px 20px; font-weight: bold; }");

    // ==========================================
    // TAB 1: FUNDAMENTALS
    // ==========================================
    QWidget *tab1 = new QWidget();
    QGridLayout *grid1 = new QGridLayout(tab1);
    grid1->setSpacing(20);
    grid1->setAlignment(Qt::AlignTop);

    grid1->addWidget(createScenarioCard(
        "Module 1.1: The Baseline", 
        "Receive a fresh fuel shipment (<span title='Inventory Change Document'>ICD</span>) and record it in the General Ledger. Understand <span title='Material Balance Area'>MBAs</span> and <span title='Key Measurement Point'>KMPs</span>.", 
        "Beginner", "scen_baseline"), 0, 0);

    grid1->addWidget(createScenarioCard(
        "Module 1.2: The Reactor Cycle", 
        "Move fuel from the Fresh Fuel Store (FFS) to the Reactor Core (RRC), run a cycle, and log Burnup and Pu production.", 
        "Beginner", "scen_reactor"), 0, 1);

    tabWidget->addTab(tab1, "Level 1: Fundamentals");

    // ==========================================
    // TAB 2: RECONCILIATION
    // ==========================================
    QWidget *tab2 = new QWidget();
    QGridLayout *grid2 = new QGridLayout(tab2);
    grid2->setSpacing(20);
    grid2->setAlignment(Qt::AlignTop);

    grid2->addWidget(createScenarioCard(
        "Module 2.1: Shipper/Receiver Differences (SRD)", 
        "You receive UO2 powder. Local mass spectrometry indicates a weight 15g lower than the shipper's documentation. Resolve the dispute.", 
        "Intermediate", "scen_srd"), 0, 0);

    grid2->addWidget(createScenarioCard(
        "Module 2.2: Physical Inventory Taking (PIT)", 
        "End of the year. Verify item serial numbers against the Book Inventory and generate the <span title='Physical Inventory Listing'>PIL</span>.", 
        "Intermediate", "scen_pit"), 0, 1);

    grid2->addWidget(createScenarioCard(
        "Module 2.3: MUF Evaluation", 
        "The PIT reveals a +2.1kg discrepancy. Calculate the limit of error (σMUF) based on scale calibrations to check statistical significance.", 
        "Intermediate", "scen_muf"), 1, 0);

    tabWidget->addTab(tab2, "Level 2: Reconciliation");

    // ==========================================
    // TAB 3: ADVANCED SAFEGUARDS
    // ==========================================
    QWidget *tab3 = new QWidget();
    QGridLayout *grid3 = new QGridLayout(tab3);
    grid3->setSpacing(20);
    grid3->setAlignment(Qt::AlignTop);

    grid3->addWidget(createScenarioCard(
        "Module 3.1: Protracted Diversion", 
        "Analyze two years of historical ledger data to notice a systemic bias masking the slow siphoning of material.", 
        "Advanced", "scen_protracted"), 0, 0);

    grid3->addWidget(createScenarioCard(
        "Module 3.2: The Substituted Dummy", 
        "During a PIT, an irradiated fuel assembly's serial number matches, but the Cherenkov viewing device (ICVD) shows anomalies.", 
        "Advanced", "scen_dummy"), 0, 1);

    tabWidget->addTab(tab3, "Level 3: Advanced");

    mainLayout->addWidget(tabWidget);
}

QWidget* TrainingWidget::createScenarioCard(QString title, QString desc, QString difficulty, QString id) {
    QFrame *card = new QFrame;
    card->setStyleSheet("QFrame { background-color: white; border: 1px solid #ddd; border-radius: 8px; }"
                        "QFrame:hover { border: 2px solid #3498db; }");
    
    QVBoxLayout *lay = new QVBoxLayout(card);
    
    QLabel *lblTitle = new QLabel("<b>" + title + "</b>");
    lblTitle->setStyleSheet("font-size: 12pt; color: #34495e; border: none;");
    
    QLabel *lblDesc = new QLabel(desc);
    lblDesc->setWordWrap(true);
    lblDesc->setTextFormat(Qt::RichText); // Allows HTML tooltips to render correctly
    lblDesc->setStyleSheet("color: #555; border: none;");
    
    QLabel *lblDiff = new QLabel("Difficulty: " + difficulty);
    QString color = (difficulty == "Beginner" ? "green" : (difficulty == "Intermediate" ? "orange" : "red"));
    lblDiff->setStyleSheet("color: " + color + "; font-weight: bold; border: none;");

    QPushButton *btnStart = new QPushButton("Start Scenario");
    btnStart->setStyleSheet("background-color: #3498db; color: white; padding: 8px; border-radius: 4px;");
    
    // We strip the HTML tags from 'desc' for the popup box so it looks clean
    QString cleanDesc = desc;
    cleanDesc.remove(QRegExp("<[^>]*>"));

    connect(btnStart, &QPushButton::clicked, [this, id, cleanDesc](){
        launchScenario(id, cleanDesc);
    });

    lay->addWidget(lblTitle);
    lay->addWidget(lblDesc);
    lay->addWidget(lblDiff);
    lay->addStretch();
    lay->addWidget(btnStart);
    
    return card;
}

void TrainingWidget::launchScenario(QString name, QString description) {
    // 1. Ask for confirmation BEFORE switching databases
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Launch Training Simulator?", 
                                  "Are you sure you want to launch this training scenario?\n\n" + description + 
                                  "\n\nReal operational data will be hidden.",
                                  QMessageBox::Yes | QMessageBox::Cancel);

    // If the user clicks "Cancel" or the 'X' button, stop right here
    if (reply != QMessageBox::Yes) {
        return; 
    }

    // 2. Switch the Database Engine
    DatabaseManager::instance().connectToScenario(name);

    // 3. Show a quick confirmation that it worked
    QMessageBox::information(this, "Simulator Active", "You are now in TRAINING MODE.");

    // 4. Emit signal so MainWindow can refresh the Home Dashboard with fake data
    emit scenarioStarted(name);
}
