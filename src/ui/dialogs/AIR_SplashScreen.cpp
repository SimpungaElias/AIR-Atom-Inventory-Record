#include "AIR_SplashScreen.h"
#include <QApplication>
#include <QScreen>
#include <QFrame>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QSvgRenderer>
#include <QPainter>

// ============================================================
// EXACT AIR PROGRAM COLORS
// ============================================================

static const QString S_BTN_PRIMARY =
    "QPushButton {"
    "  background-color: #0056b3;"
    "  color: white;"
    "  padding: 10px;"
    "  font-weight: bold;"
    "  border-radius: 4px;"
    "  font-size: 11pt;"
    "  min-width: 200px;"
    "}"
    "QPushButton:hover { background-color: #004494; }";

static const QString S_BTN_SECONDARY =
    "QPushButton {"
    "  background-color: transparent;"
    "  color: #0056b3;"
    "  padding: 10px;"
    "  font-weight: bold;"
    "  border-radius: 4px;"
    "  font-size: 11pt;"
    "  border: 2px solid #0056b3;"
    "  min-width: 200px;"
    "}"
    "QPushButton:hover { background-color: #0056b3; color: white; }";

static const QString S_BTN_EXIT =
    "QPushButton {"
    "  background-color: transparent;"
    "  color: #c0392b;"
    "  padding: 10px;"
    "  font-weight: bold;"
    "  border-radius: 4px;"
    "  font-size: 11pt;"
    "  border: 2px solid #c0392b;"
    "  min-width: 200px;"
    "}"
    "QPushButton:hover { background-color: #c0392b; color: white; }";

static const QString S_BTN_NAV =
    "QPushButton {"
    "  background-color: #ecf0f1;"
    "  color: #003366;"
    "  padding: 8px 20px;"
    "  font-weight: bold;"
    "  border-radius: 4px;"
    "  font-size: 10pt;"
    "  border: 1px solid #bdc3c7;"
    "  min-width: 140px;"
    "}"
    "QPushButton:hover { background-color: #003366; color: white; border-color: #003366; }";

static const QString S_BTN_OPEN =
    "QPushButton {"
    "  background-color: #ecf0f1;"
    "  color: #003366;"
    "  padding: 6px 15px;"
    "  font-weight: bold;"
    "  border-radius: 4px;"
    "  font-size: 10pt;"
    "  border: 1px solid #003366;"
    "}"
    "QPushButton:hover { background-color: #003366; color: white; }";

// ============================================================
// HELPER — open a PDF from Qt resources, with fallbacks
// ============================================================
static void openDocument(const QString &resourcePath,
                         const QString &fileName,
                         const QString &fallbackUrl)
{
    // 1. Try Qt resource (compiled into binary via .qrc)
    QFile resFile(resourcePath);
    if (resFile.exists()) {
        QString tempPath = QDir::tempPath() + QDir::separator() + fileName;

        if (QFile::exists(tempPath))
            QFile::remove(tempPath);

        if (resFile.copy(tempPath)) {
            QFile::setPermissions(tempPath,
                QFile::ReadOwner | QFile::WriteOwner |
                QFile::ReadUser  | QFile::ReadGroup  | QFile::ReadOther);

            if (QDesktopServices::openUrl(QUrl::fromLocalFile(tempPath)))
                return;
        }
    }

    // 2. Try docs/ folder next to executable
    QString exeDir = QApplication::applicationDirPath();
    QStringList localPaths = {
        exeDir + "/" + fileName,
        exeDir + "/docs/" + fileName,
        exeDir + "/../docs/" + fileName,
    };
    for (const QString &p : localPaths) {
        if (QFile::exists(p)) {
            if (QDesktopServices::openUrl(QUrl::fromLocalFile(p)))
                return;
        }
    }

    // 3. Fall back to IAEA website for official documents
    if (!fallbackUrl.isEmpty()) {
        QMessageBox::information(nullptr, "Opening Online",
            "This document is not bundled with this installation.\n"
            "Opening the official IAEA source in your web browser.");
        QDesktopServices::openUrl(QUrl(fallbackUrl));
        return;
    }

    // 4. Nothing worked
    QMessageBox::warning(nullptr, "Document Not Found",
        QString("Could not open '%1'.\n\n"
                "Place the PDF in the docs/ folder next to the AIR executable,\n"
                "or rebuild AIR with the document bundled in resources.qrc.")
        .arg(fileName));
}

// ============================================================
AIRSplashScreen::AIRSplashScreen(QWidget *parent) : QDialog(parent) {
    setWindowTitle("AIR — Atom Inventory Record");
    setModal(true);
    setFixedSize(680, 520);
    setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);

    setStyleSheet("QDialog { background-color: white; }"
                  "QLabel { background: transparent; }");

    QScreen *screen = QApplication::primaryScreen();
    if (screen)
        move(screen->geometry().center() - rect().center());

    setupUI();
}

void AIRSplashScreen::setupUI() {
    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    stack = new QStackedWidget;
    stack->setStyleSheet("background: transparent;");
    root->addWidget(stack, 1);

    buildPage0_Welcome();
    buildPage1_Enter();
    buildPage2_Resources();

    stack->setCurrentIndex(0);
}

// ============================================================
// PAGE 0 — LANDING
// ============================================================
void AIRSplashScreen::buildPage0_Welcome() {
    QWidget *page = new QWidget;
    page->setStyleSheet("background: white;");

    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(60, 40, 60, 40);
    lay->setSpacing(0);
    lay->setAlignment(Qt::AlignCenter);

    // Logo
    QLabel *logo = new QLabel;
    logo->setStyleSheet("border: none; background: transparent;");
    logo->setFixedWidth(300);
    QSvgRenderer splashRenderer(QString(":/icons/air_logo.svg"));
if (splashRenderer.isValid()) {
    qreal dpr = QApplication::primaryScreen()->devicePixelRatio();
    int w = qRound(300 * dpr);
    int h = qRound(w * splashRenderer.defaultSize().height()
                     / (double)splashRenderer.defaultSize().width());
    QPixmap px(w, h);
    px.fill(Qt::transparent);
    QPainter painter(&px);
    splashRenderer.render(&painter);
    painter.end();
    px.setDevicePixelRatio(dpr);
    logo->setPixmap(px);
} else {
    logo->setText("AIR");
    logo->setStyleSheet("color: #003366; font-size: 48pt; font-weight: bold;");
}
    logo->setAlignment(Qt::AlignCenter);
    lay->addWidget(logo);

    lay->addSpacing(20);

    // Tagline
    QLabel *tagline = new QLabel("EDUCATIONAL NUCLEAR MATERIAL\nACCOUNTING AND CONTROL SYSTEM");
    tagline->setAlignment(Qt::AlignCenter);
    tagline->setStyleSheet("color: #003366; font-size: 11pt; font-weight: bold; letter-spacing: 1px;");
    lay->addWidget(tagline);

    lay->addSpacing(30);

    // Divider
    QFrame *div = new QFrame;
    div->setFrameShape(QFrame::HLine);
    div->setStyleSheet("background-color: #003366; border: none; max-height: 2px; margin: 0 60px;");
    lay->addWidget(div);

    lay->addSpacing(28);

    // Buttons
    QVBoxLayout *btnLay = new QVBoxLayout;
    btnLay->setSpacing(12);
    btnLay->setAlignment(Qt::AlignCenter);

    QPushButton *btnEnter = new QPushButton("ENTER");
    btnEnter->setStyleSheet(S_BTN_PRIMARY);
    btnEnter->setCursor(Qt::PointingHandCursor);
    connect(btnEnter, &QPushButton::clicked, [this](){ goToPage(1); });

    QPushButton *btnRes = new QPushButton("RESOURCES");
    btnRes->setStyleSheet(S_BTN_SECONDARY);
    btnRes->setCursor(Qt::PointingHandCursor);
    connect(btnRes, &QPushButton::clicked, [this](){ goToPage(2); });

    QPushButton *btnExit = new QPushButton("EXIT");
    btnExit->setStyleSheet(S_BTN_EXIT);
    btnExit->setCursor(Qt::PointingHandCursor);
    connect(btnExit, &QPushButton::clicked, this, &QDialog::reject);

    btnLay->addWidget(btnEnter, 0, Qt::AlignCenter);
    btnLay->addWidget(btnRes,   0, Qt::AlignCenter);
    btnLay->addWidget(btnExit,  0, Qt::AlignCenter);

    lay->addLayout(btnLay);
    lay->addStretch();

    // Checkbox
    chkShow = new QCheckBox("Show this screen each time AIR is started");
    chkShow->setChecked(true);
    chkShow->setStyleSheet(
        "QCheckBox { color: #003366; font-size: 9pt; font-weight: bold; }"
        "QCheckBox::indicator { width: 14px; height: 14px; border: 1px solid #003366; border-radius: 2px; }"
        "QCheckBox::indicator:checked { background-color: #0056b3; border-color: #0056b3; }"
    );
    lay->addWidget(chkShow, 0, Qt::AlignCenter);

    stack->addWidget(page);
}

// ============================================================
// PAGE 1 — WELCOME TO AIR
// ============================================================
void AIRSplashScreen::buildPage1_Enter() {
    QWidget *page = new QWidget;
    page->setStyleSheet("background: white;");

    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(60, 40, 60, 40);
    lay->setSpacing(0);
    lay->setAlignment(Qt::AlignCenter);

    QLabel *title = new QLabel("WELCOME TO AIR");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: #003366; font-size: 20pt; font-weight: bold;");
    lay->addWidget(title);

    lay->addSpacing(6);

    QLabel *sub = new QLabel("Atom Inventory Record");
    sub->setAlignment(Qt::AlignCenter);
    sub->setStyleSheet("color: #003366; font-size: 11pt; font-weight: bold;");
    lay->addWidget(sub);

    lay->addSpacing(24);

    // Info panel
    QWidget *panel = new QWidget;
    panel->setStyleSheet(
        "QWidget {"
        "  background-color: #f9f9f9;"
        "  border: 1px solid #003366;"
        "  border-radius: 4px;"
        "}"
    );
    QVBoxLayout *panelLay = new QVBoxLayout(panel);
    panelLay->setContentsMargins(28, 22, 28, 22);
    panelLay->setSpacing(10);

    QLabel *desc = new QLabel(
        "AIR is a secure, air-gapped Nuclear Material Accountancy\n"
        "and Control system designed to support IAEA Safeguards\n"
        "reporting requirements.\n\n"
        "This system generates ICR, LII, NLI, MBR, and General\n"
        "Ledger reports in compliance with INFCIRC/153 and\n"
        "Subsidiary Arrangements (IAEA Code 10)."
    );
    desc->setAlignment(Qt::AlignCenter);
    desc->setWordWrap(true);
    desc->setStyleSheet("color: #333333; font-size: 10pt; line-height: 1.6; border: none; background: transparent;");
    panelLay->addWidget(desc);

    QFrame *sep = new QFrame;
    sep->setFrameShape(QFrame::HLine);
    sep->setStyleSheet("background-color: #003366; border: none; max-height: 1px; margin: 4px 0;");
    panelLay->addWidget(sep);

    QLabel *note = new QLabel("Click  ENTER  to proceed to login.");
    note->setAlignment(Qt::AlignCenter);
    note->setStyleSheet("color: #c0392b; font-weight: bold; font-size: 10pt; border: none; background: transparent;");
    panelLay->addWidget(note);

    lay->addWidget(panel);
    lay->addSpacing(28);

    // Nav buttons
    QHBoxLayout *navLay = new QHBoxLayout;
    navLay->setSpacing(12);
    navLay->setAlignment(Qt::AlignCenter);

    QPushButton *btnBack = new QPushButton("← Back");
    btnBack->setStyleSheet(S_BTN_NAV);
    btnBack->setCursor(Qt::PointingHandCursor);
    connect(btnBack, &QPushButton::clicked, [this](){ goToPage(0); });

    QPushButton *btnRes = new QPushButton("Resources →");
    btnRes->setStyleSheet(S_BTN_NAV);
    btnRes->setCursor(Qt::PointingHandCursor);
    connect(btnRes, &QPushButton::clicked, [this](){ goToPage(2); });

    QPushButton *btnEnter = new QPushButton("ENTER");
    btnEnter->setStyleSheet(S_BTN_PRIMARY);
    btnEnter->setCursor(Qt::PointingHandCursor);
    connect(btnEnter, &QPushButton::clicked, this, &AIRSplashScreen::onEnterClicked);

    navLay->addWidget(btnBack);
    navLay->addWidget(btnRes);
    navLay->addWidget(btnEnter);

    lay->addLayout(navLay);

    stack->addWidget(page);
}

// ============================================================
// PAGE 2 — RESOURCES
// ============================================================
void AIRSplashScreen::buildPage2_Resources() {
    QWidget *page = new QWidget;
    page->setStyleSheet("background: white;");

    QVBoxLayout *lay = new QVBoxLayout(page);
    lay->setContentsMargins(60, 36, 60, 36);
    lay->setSpacing(0);

    QLabel *title = new QLabel("Resources & Documentation");
    title->setAlignment(Qt::AlignCenter);
    title->setStyleSheet("color: #003366; font-size: 18pt; font-weight: bold;");
    lay->addWidget(title);

    lay->addSpacing(6);

    QLabel *desc = new QLabel(
        "Review the following documents before operating AIR.\n"
        "Click OPEN to view or download each document."
    );
    desc->setAlignment(Qt::AlignCenter);
    desc->setStyleSheet("color: #555555; font-size: 10pt;");
    lay->addWidget(desc);

    lay->addSpacing(20);

    // Top divider
    QFrame *div = new QFrame;
    div->setFrameShape(QFrame::HLine);
    div->setStyleSheet("background-color: #003366; border: none; max-height: 2px;");
    lay->addWidget(div);

    lay->addSpacing(16);

    // ── Document list ──────────────────────────────────────────────────────
    // resourcePath : Qt resource path — compiled into binary via resources.qrc
    // fileName     : filename used when extracted to system temp directory
    // fallbackUrl  : IAEA website URL shown when document is not bundled
    struct Doc {
        QString title;
        QString subtitle;
        QString resourcePath;
        QString fileName;
        QString fallbackUrl;
    };

    const QList<Doc> docs = {
        {
            "AIR User Manual",
            "Complete operational guide for the AIR system",
            ":/docs/AIR-User-Manual.pdf",
            "AIR-User-Manual.pdf",
            ""
        },
        {
            "INFCIRC/153",
            "Structure and Content of Agreements Between the Agency and States",
            ":/docs/INFCIRC153.pdf",
            "INFCIRC153.pdf",
            "https://www.iaea.org/publications/documents/infcircs/structure-and-content-agreements-between-agency-and-states-required-connection-treaty-non-proliferation-nuclear-weapons"
        },
        {
            "Subsidiary Arrangements — IAEA Code 10",
            "General Part: Detailed safeguards implementation procedures",
            ":/docs/Code-10.pdf",
            "Code-10.pdf",
            "https://www.iaea.org/safeguards/safeguards-legal-framework/subsidiary-arrangements"
        },
    };

    for (const Doc &doc : docs) {
        QWidget *row = new QWidget;
        row->setFixedHeight(76);
        row->setStyleSheet(
            "QWidget {"
            "  background-color: #f9f9f9;"
            "  border: 1px solid #003366;"
            "  border-radius: 4px;"
            "}"
        );

        QHBoxLayout *rowLay = new QHBoxLayout(row);
        rowLay->setContentsMargins(20, 0, 16, 0);
        rowLay->setSpacing(16);

        // Text block
        QVBoxLayout *textLay = new QVBoxLayout;
        textLay->setSpacing(2);

        QLabel *t = new QLabel(doc.title);
        t->setStyleSheet("color: #003366; font-size: 11pt; font-weight: bold; "
                         "border: none; background: transparent;");

        QLabel *s = new QLabel(doc.subtitle);
        s->setStyleSheet("color: #555555; font-size: 9pt; "
                         "border: none; background: transparent;");

        // Bundled / Online badge
        bool bundled = QFile::exists(doc.resourcePath);
        QLabel *badge = new QLabel(bundled ? "● Bundled" : "● Opens online if not found locally");
        badge->setStyleSheet(bundled
            ? "color: #27ae60; font-size: 8pt; font-weight: bold; border: none; background: transparent;"
            : "color: #e67e22; font-size: 8pt; font-weight: bold; border: none; background: transparent;");

        textLay->addWidget(t);
        textLay->addWidget(s);
        textLay->addWidget(badge);

        // OPEN button
        QString resourcePath = doc.resourcePath;
        QString fileName     = doc.fileName;
        QString fallbackUrl  = doc.fallbackUrl;

        QPushButton *btnOpen = new QPushButton("OPEN");
        btnOpen->setFixedWidth(80);
        btnOpen->setFixedHeight(36);
        btnOpen->setStyleSheet(S_BTN_OPEN);
        btnOpen->setCursor(Qt::PointingHandCursor);
        btnOpen->setToolTip(bundled
            ? "Opens bundled PDF with your system PDF viewer"
            : "Opens IAEA website if not found locally");

        connect(btnOpen, &QPushButton::clicked, [resourcePath, fileName, fallbackUrl](){
            openDocument(resourcePath, fileName, fallbackUrl);
        });

        rowLay->addLayout(textLay, 1);
        rowLay->addWidget(btnOpen, 0, Qt::AlignVCenter);

        lay->addWidget(row);
        lay->addSpacing(10);
    }

    lay->addStretch();

    // Bottom divider
    QFrame *div2 = new QFrame;
    div2->setFrameShape(QFrame::HLine);
    div2->setStyleSheet("background-color: #003366; border: none; max-height: 1px;");
    lay->addWidget(div2);

    lay->addSpacing(16);

    // Nav buttons
    QHBoxLayout *navLay = new QHBoxLayout;
    navLay->setSpacing(12);
    navLay->setAlignment(Qt::AlignCenter);

    QPushButton *btnBack = new QPushButton("← Back");
    btnBack->setStyleSheet(S_BTN_NAV);
    btnBack->setCursor(Qt::PointingHandCursor);
    connect(btnBack, &QPushButton::clicked, [this](){ goToPage(0); });

    QPushButton *btnEnter = new QPushButton("ENTER");
    btnEnter->setStyleSheet(S_BTN_PRIMARY);
    btnEnter->setCursor(Qt::PointingHandCursor);
    connect(btnEnter, &QPushButton::clicked, this, &AIRSplashScreen::onEnterClicked);

    navLay->addWidget(btnBack);
    navLay->addWidget(btnEnter);

    lay->addLayout(navLay);

    stack->addWidget(page);
}

// ============================================================
// NAVIGATION — smooth fade between pages
// ============================================================
void AIRSplashScreen::goToPage(int index) {
    QWidget *current = stack->currentWidget();
    QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(current);
    current->setGraphicsEffect(effect);

    QPropertyAnimation *fadeOut = new QPropertyAnimation(effect, "opacity");
    fadeOut->setDuration(140);
    fadeOut->setStartValue(1.0);
    fadeOut->setEndValue(0.0);

    connect(fadeOut, &QPropertyAnimation::finished, [this, index](){
        stack->currentWidget()->setGraphicsEffect(nullptr);
        stack->setCurrentIndex(index);

        QWidget *next = stack->currentWidget();
        QGraphicsOpacityEffect *inFx = new QGraphicsOpacityEffect(next);
        next->setGraphicsEffect(inFx);

        QPropertyAnimation *fadeIn = new QPropertyAnimation(inFx, "opacity");
        fadeIn->setDuration(180);
        fadeIn->setStartValue(0.0);
        fadeIn->setEndValue(1.0);
        fadeIn->start(QAbstractAnimation::DeleteWhenStopped);

        connect(fadeIn, &QPropertyAnimation::finished, [next](){
            next->setGraphicsEffect(nullptr);
        });
    });

    fadeOut->start(QAbstractAnimation::DeleteWhenStopped);
}

void AIRSplashScreen::onEnterClicked() {
    emit enterApplication();
    accept();
}
