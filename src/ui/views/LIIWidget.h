#ifndef LIIWIDGET_H
#define LIIWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTableWidget>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QDateEdit>

class LIIWidget : public QWidget {
    Q_OBJECT

public:
    explicit LIIWidget(QWidget *parent = nullptr);
    void loadData(); 

private slots:
    void addItem();
    void exportPDF();
    void deleteItem();
    void openMaterialCodeSelector(); // <--- NEW SLOT

private:
    void setupUI();
    void setupReportHeader(QVBoxLayout *layout);
    void setupInputForm(QVBoxLayout *layout);
    void setupTable(QVBoxLayout *layout);

    // Header Inputs
    QLineEdit *txtCountry;
    QLineEdit *txtFacility;
    QLineEdit *txtMBA;
    QDateEdit *dateReport;
    QSpinBox *spinReportNo;

    // Item Inputs
    QComboBox *comboKMP;
    QLineEdit *txtPosition;
    QLineEdit *txtBatch;
    QLineEdit *txtMaterialCode; // <--- RENAMED from txtDesc for clarity
    QDoubleSpinBox *spinElem;
    QDoubleSpinBox *spinFissile;
    QDoubleSpinBox *spinPu;
    QDoubleSpinBox *spinBurnup;

    QTableWidget *table;
    int itemCounter;
};

#endif // LIIWIDGET_H
