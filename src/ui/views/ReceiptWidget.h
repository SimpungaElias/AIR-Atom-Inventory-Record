#ifndef RECEIPTWIDGET_H
#define RECEIPTWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

class ReceiptWidget : public QWidget {
    Q_OBJECT

public:
    explicit ReceiptWidget(QWidget *parent = nullptr);
    
    // Public methods for external refresh
    void refreshTable();
    void refreshMBAList(); // <--- Added this

signals:
    void dataChanged(); 

private slots:
    void saveReceipt();
    void exportPDF();
    void deleteSelected();

private:
    void setupUI();
    void setupReportHeader(QVBoxLayout *layout);
    void setupInputForm(QVBoxLayout *layout);
    void setupTable(QVBoxLayout *layout);

    // Report Header/Footer Fields
    QLineEdit *txtCountry;
    QLineEdit *txtFacility;
    QLineEdit *txtReportNo;
    QLineEdit *txtMBA; // Added MBA field
    QLineEdit *txtShipper;
    QLineEdit *txtReceiver;
    QDateEdit *dateReport;

    // Form Fields
    QLineEdit *txtBatch, *txtForm;
    QComboBox *comboMbaTo, *comboMbaFrom, *comboCode, *comboElement;
    QSpinBox *spinItems;
    QDoubleSpinBox *spinWeightU, *spinWeightU235;
    QDateEdit *dateEdit;

    // Display Table
    QTableWidget *table;
};

#endif // RECEIPTWIDGET_H
