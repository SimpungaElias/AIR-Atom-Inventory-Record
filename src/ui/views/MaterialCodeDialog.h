#ifndef MATERIALCODEDIALOG_H
#define MATERIALCODEDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>

class MaterialCodeDialog : public QDialog {
    Q_OBJECT

public:
    explicit MaterialCodeDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("Material Description Code Selector (Label 430)");
        resize(600, 400);
        setupUI();
    }

    QString getSelectedCode() const {
        return comboPhysical->currentData().toString() +
               comboChemical->currentData().toString() +
               comboContainment->currentData().toString() +
               comboQuality->currentData().toString();
    }

    QString getSelectedDescription() const {
        // Returns a human-readable summary
        return QString("%1, %2, %3, %4")
                .arg(comboPhysical->currentText())
                .arg(comboChemical->currentText())
                .arg(comboContainment->currentText())
                .arg(comboQuality->currentText());
    }

private:
    QComboBox *comboPhysical;
    QComboBox *comboChemical;
    QComboBox *comboContainment;
    QComboBox *comboQuality;
    QLabel *lblPreview;

    void setupUI() {
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        QGridLayout *grid = new QGridLayout();

        // 1. Physical Form
        QGroupBox *grpPhys = new QGroupBox("1. Physical Form");
        QVBoxLayout *layPhys = new QVBoxLayout(grpPhys);
        comboPhysical = new QComboBox();
        populatePhysical(comboPhysical);
        layPhys->addWidget(comboPhysical);
        grid->addWidget(grpPhys, 0, 0);

        // 2. Chemical Form
        QGroupBox *grpChem = new QGroupBox("2. Chemical Form");
        QVBoxLayout *layChem = new QVBoxLayout(grpChem);
        comboChemical = new QComboBox();
        populateChemical(comboChemical);
        layChem->addWidget(comboChemical);
        grid->addWidget(grpChem, 0, 1);

        // 3. Containment
        QGroupBox *grpCont = new QGroupBox("3. Containment");
        QVBoxLayout *layCont = new QVBoxLayout(grpCont);
        comboContainment = new QComboBox();
        populateContainment(comboContainment);
        layCont->addWidget(comboContainment);
        grid->addWidget(grpCont, 1, 0);

        // 4. Irradiation/Quality
        QGroupBox *grpQual = new QGroupBox("4. Irradiation Status & Quality");
        QVBoxLayout *layQual = new QVBoxLayout(grpQual);
        comboQuality = new QComboBox();
        populateQuality(comboQuality);
        layQual->addWidget(comboQuality);
        grid->addWidget(grpQual, 1, 1);

        mainLayout->addLayout(grid);

        // Preview Section
        QHBoxLayout *previewLayout = new QHBoxLayout();
        previewLayout->addWidget(new QLabel("<b>Selected Code:</b>"));
        lblPreview = new QLabel("----");
        lblPreview->setStyleSheet("font-size: 18px; font-weight: bold; color: #003366;");
        previewLayout->addWidget(lblPreview);
        previewLayout->addStretch();
        mainLayout->addLayout(previewLayout);

        // Buttons
        QHBoxLayout *btnLayout = new QHBoxLayout();
        QPushButton *btnOk = new QPushButton("Select Code");
        btnOk->setStyleSheet("background-color: #0056b3; color: white; padding: 8px;");
        connect(btnOk, &QPushButton::clicked, this, &QDialog::accept);
        
        QPushButton *btnCancel = new QPushButton("Cancel");
        connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);

        btnLayout->addStretch();
        btnLayout->addWidget(btnCancel);
        btnLayout->addWidget(btnOk);
        mainLayout->addLayout(btnLayout);

        // Connect updates
        connect(comboPhysical, &QComboBox::currentTextChanged, this, &MaterialCodeDialog::updatePreview);
        connect(comboChemical, &QComboBox::currentTextChanged, this, &MaterialCodeDialog::updatePreview);
        connect(comboContainment, &QComboBox::currentTextChanged, this, &MaterialCodeDialog::updatePreview);
        connect(comboQuality, &QComboBox::currentTextChanged, this, &MaterialCodeDialog::updatePreview);
        
        updatePreview();
    }

    void updatePreview() {
        lblPreview->setText(getSelectedCode());
    }

    // --- DATA POPULATION (Based on your images) ---
    void populatePhysical(QComboBox *c) {
        c->addItem("Powders (F)", "F");
        c->addItem("Liquids (N)", "N");
        c->addItem("Fuel elements (B)", "B");
        c->addItem("Fuel components (D)", "D");
        c->addItem("Powder, ceramic (G)", "G");
        c->addItem("Ceramics (J)", "J");
        c->addItem("Solids, other (Ø)", "Ø");
        c->addItem("Residues, scrap (R)", "R");
        c->addItem("Waste, solid (T)", "T");
        c->addItem("Waste, liquid (U)", "U");
        c->addItem("Sealed sources (Q)", "Q");
        c->addItem("Small samples (V)", "V");
    }

    void populateChemical(QComboBox *c) {
        c->addItem("Dioxide (Q)", "Q");
        c->addItem("Nitrate (J)", "J");
        c->addItem("Elemental (Metal) (D)", "D");
        c->addItem("Hexafluoride (G)", "G");
        c->addItem("Trioxide (T)", "T");
        c->addItem("Oxide U3O8 (U)", "U");
        c->addItem("Carbide (W)", "W");
        c->addItem("Other oxides (R)", "R");
        c->addItem("Alloys (Al) (3)", "3");
        c->addItem("Miscellaneous (Ø)", "Ø");
    }

    void populateContainment(QComboBox *c) {
        c->addItem("Uncontained (1)", "1");
        c->addItem("Fuel units (2)", "2");
        c->addItem("Flask (3)", "3");
        c->addItem("Vessel, calibrated (5)", "5");
        c->addItem("Birdcage (8)", "8");
        c->addItem("Bottle/Can <0.5L (A)", "A");
        c->addItem("Bottle/Can 0.5-1L (E)", "E");
        c->addItem("Bottle/Can 1-5L (G)", "G");
        c->addItem("Can 5-10L (H)", "H");
        c->addItem("Drum 20-50L (L)", "L");
        c->addItem("Drum 50-100L (M)", "M");
        c->addItem("Drum 100-200L (N)", "N");
        c->addItem("Drum 200-500L (Q)", "Q");
    }

    void populateQuality(QComboBox *c) {
        // Non-Irradiated
        c->addItem("Fresh Fuel (F)", "F");
        c->addItem("Pure, Stable (B)", "B");
        c->addItem("Pure (C)", "C");
        c->addItem("Heterogeneous (D)", "D");
        c->addItem("Variable (E)", "E");
        // Irradiated
        c->addItem("Irradiated Fuel (G)", "G");
        c->addItem("Irr. Pure (K)", "K");
        c->addItem("Irr. Heterogeneous (L)", "L");
    }
};

#endif // MATERIALCODEDIALOG_H
