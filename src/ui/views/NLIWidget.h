#ifndef NLIWIDGET_H
#define NLIWIDGET_H

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

class NLIWidget : public QWidget {
    Q_OBJECT

public:
    explicit NLIWidget(QWidget *parent = nullptr);
    void loadData();

private slots:
    void addEntry();
    void exportPDF();
    void deleteEntry();

private:
    void setupUI();
    void setupReportHeader(QVBoxLayout *layout);
    void setupInputForm(QVBoxLayout *layout);
    void setupTable(QVBoxLayout *layout);
    

    // Report Header Inputs
    QLineEdit *txtCountry;
    QLineEdit *txtFacility;
    QLineEdit *txtMBA;
    QDateEdit *dateReport;
    QLineEdit *txtReportNo;

    // Transaction Inputs
    QLineEdit *txtBatch;
    QSpinBox *spinItems;
    QComboBox *comboCode;
    
    // Uranium Inputs
    QLineEdit *txtUElemCode;
    QLineEdit *txtUIsoCode;
    QDoubleSpinBox *spinUWeight;
    QDoubleSpinBox *spinUIsoWeight;

    // Plutonium Inputs
    QLineEdit *txtPElemCode;
    QDoubleSpinBox *spinPWeight;

    // Display
    QTableWidget *table;
    int lineCounter;
};

#endif // NLIWIDGET_H
