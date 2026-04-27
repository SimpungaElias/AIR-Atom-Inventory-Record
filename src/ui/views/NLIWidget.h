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
#include <QCompleter>

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
    QComboBox    *comboCountry;   // searchable IAEA list
    QLineEdit    *txtFacility;
    QComboBox    *comboMBA;
    QDateEdit    *dateReport;
    QSpinBox     *spinReportNo;   // number only (was QLineEdit)
    QLineEdit    *txtBatch;
    QSpinBox     *spinItems;
    QComboBox    *comboCode;
    QLineEdit    *txtUElemCode, *txtUIsoCode, *txtPElemCode;
    QDoubleSpinBox *spinUWeight, *spinUIsoWeight, *spinPWeight;
    QTableWidget *table;
    int lineCounter;
};
#endif
