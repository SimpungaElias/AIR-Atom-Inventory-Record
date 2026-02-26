#ifndef GENERALLEDGERWIDGET_H
#define GENERALLEDGERWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QDoubleSpinBox>
#include <QDateEdit>
#include <QVBoxLayout>
#include <QPushButton>

class GeneralLedgerWidget : public QWidget {
    Q_OBJECT

public:
    explicit GeneralLedgerWidget(QWidget *parent = nullptr);
    void refreshData();

    // --- THIS IS THE FIX ---
signals:
    void dataChanged(); 
    // -----------------------

private slots:
    void addEntry();
    void exportPDF();
    void deleteEntry();

private:
    void setupUI();
    void setupReportHeader(QVBoxLayout *layout);
    void setupInputForm(QVBoxLayout *layout);
    void setupComplexTable(QVBoxLayout *layout);

    // Report Header Fields
    QLineEdit *txtFacility;
    QComboBox *comboMBA;
    QLineEdit *txtMatDesc;
    QLineEdit *txtElemCode;
    QLineEdit *txtIsoCode;
    QLineEdit *txtUnit;

    // Transaction Entry Fields
    QDateEdit *dateEdit;
    QLineEdit *txtRef;
    QComboBox *comboCode, *comboType; 
    QDoubleSpinBox *spinElem, *spinIso;
    
    // Display
    QTableWidget *table;
    int lineCounter;
    
    // Running Balances
    double balU;
    double balU235;
    int balItems;
};

#endif // GENERALLEDGERWIDGET_H
