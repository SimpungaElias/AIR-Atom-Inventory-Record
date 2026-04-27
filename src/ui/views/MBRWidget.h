#ifndef MBRWIDGET_H
#define MBRWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QGroupBox>
#include <QCompleter>

class MBRWidget : public QWidget {
    Q_OBJECT
public:
    explicit MBRWidget(QWidget *parent = nullptr);
    void loadData();

signals:
    void dataChanged();

private slots:
    void addEntry();
    void deleteEntry();
    void exportPDF();

private:
    void setupUI();
    void setupHeader();
    void setupInputForm();
    void setupTable();

    // ── Report Header Fields ──────────────────────────────────────────────
    QComboBox   *comboCountry;      // Full IAEA country list, searchable
    QLineEdit   *txtFacility;       // Editable with placeholder
    QComboBox   *comboMBA;          // Editable + saveable
    QDateEdit   *dateFrom;
    QDateEdit   *dateTo;
    QSpinBox    *spinReportNo;      // Number only (was QLineEdit)

    // ── Entry Form Fields ─────────────────────────────────────────────────
    QLineEdit   *txtContinuation;
    QComboBox   *comboEntryName;
    QComboBox   *comboElement;
    QDoubleSpinBox *spinWeight;
    QComboBox   *comboUnit;
    QDoubleSpinBox *spinFissile;
    QComboBox   *comboIsotopeCode;  // Selectable (was QLineEdit)
    QSpinBox    *spinEntryReportNo; // Number only (was QLineEdit)

    // ── Table and Buttons ─────────────────────────────────────────────────
    QTableWidget *table;
    QPushButton  *btnExport;
};

#endif // MBRWIDGET_H
