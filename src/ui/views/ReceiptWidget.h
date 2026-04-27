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
#include <QCompleter>

class ReceiptWidget : public QWidget {
    Q_OBJECT

public:
    explicit ReceiptWidget(QWidget *parent = nullptr);
    void refreshTable();
    void refreshMBAList() {}   // kept for MainWindow compatibility

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

    // Report Header Fields
    QComboBox   *comboCountry;     // Searchable IAEA country list
    QLineEdit   *txtFacility;      // Editable with placeholder
    QComboBox   *comboMBA;         // Editable + saveable
    QSpinBox    *spinReportNo;     // Number only (was QLineEdit)
    QDateEdit   *dateReport;
    QLineEdit   *txtShipper;
    QLineEdit   *txtReceiver;

    // Transaction Form Fields
    QLineEdit      *txtBatch;
    QLineEdit      *txtForm;
    QComboBox      *comboMbaTo;
    QComboBox      *comboMbaFrom;
    QComboBox      *comboCode;
    QComboBox      *comboElement;
    QSpinBox       *spinItems;
    QDoubleSpinBox *spinWeightU;
    QDoubleSpinBox *spinWeightU235;
    QDateEdit      *dateEdit;

    // Display Table
    QTableWidget *table;
};

#endif // RECEIPTWIDGET_H
