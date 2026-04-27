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
#include <QCompleter>

class LIIWidget : public QWidget {
    Q_OBJECT
public:
    explicit LIIWidget(QWidget *parent = nullptr);
    void loadData();
private slots:
    void addItem();
    void exportPDF();
    void deleteItem();
    void openMaterialCodeSelector();
private:
    void setupUI();
    void setupReportHeader(QVBoxLayout *layout);
    void setupInputForm(QVBoxLayout *layout);
    void setupTable(QVBoxLayout *layout);
    QComboBox    *comboCountry;   // searchable IAEA list
    QLineEdit    *txtFacility;
    QComboBox    *comboMBA;
    QDateEdit    *dateReport;
    QSpinBox     *spinReportNo;   // number only
    QComboBox    *comboKMP;
    QLineEdit    *txtPosition;
    QLineEdit    *txtBatch;
    QLineEdit    *txtMaterialCode;
    QDoubleSpinBox *spinElem, *spinFissile, *spinPu, *spinBurnup;
    QTableWidget *table;
    int itemCounter;
};
#endif
