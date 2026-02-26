#ifndef MBRWIDGET_H
#define MBRWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>

class MBRWidget : public QWidget {
    Q_OBJECT
public:
    explicit MBRWidget(QWidget *parent = nullptr);
    void loadData();

// --- CRITICAL SECTION: THIS MUST EXIST ---
signals:
    void dataChanged(); 
// ---------------------------------------

private slots:
    void addEntry();       
    void deleteEntry();    
    void exportPDF();

private:
    void setupUI();
    void setupHeader();
    void setupInputForm(); 
    void setupTable();

    // Report Header Fields
    QLineEdit *txtCountry;
    QLineEdit *txtFacility;
    QComboBox *comboMBA;
    QDateEdit *dateFrom;
    QDateEdit *dateTo;
    QLineEdit *txtReportNo;

    // Entry Form Fields
    QLineEdit *txtContinuation;
    QComboBox *comboEntryName;
    QComboBox *comboElement;
    QDoubleSpinBox *spinWeight;
    QComboBox *comboUnit;
    QDoubleSpinBox *spinFissile;
    QLineEdit *txtIsotopeCode;
    QLineEdit *txtEntryReportNo;

    // Table and Buttons
    QTableWidget *table;
    QPushButton *btnExport;
};

#endif // MBRWIDGET_H
