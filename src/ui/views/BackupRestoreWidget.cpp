#include "BackupRestoreWidget.h"
#include "../../db/DatabaseManager.h"
#include <QVBoxLayout>
#include <QHeaderView>
#include <QMessageBox>
#include <QLabel>
#include <QGridLayout>

BackupRestoreWidget::BackupRestoreWidget(QWidget *parent) : QWidget(parent) {
    setupUI();
    refreshList();
}

void BackupRestoreWidget::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Header Title
    mainLayout->addWidget(new QLabel("<h2>Database Backup & Restore</h2>"));

    // "Add new record" button
    QPushButton *btnAdd = new QPushButton("+ Add new record");
    btnAdd->setStyleSheet("background-color: #ecf0f1; border: 1px solid #ccc; padding: 6px; font-weight: bold; color: #333; text-align: left;");
    connect(btnAdd, &QPushButton::clicked, this, &BackupRestoreWidget::toggleAddForm);
    mainLayout->addWidget(btnAdd);

    // Form (Hidden)
    formGroup = new QGroupBox("New Backup Details");
    formGroup->setVisible(false);
    formGroup->setStyleSheet("background-color: #f9f9f9; border: 1px solid #ccc; padding: 10px; margin-top: 5px;");
    QGridLayout *formGrid = new QGridLayout(formGroup);
    
    txtTitle = new QLineEdit; txtTitle->setPlaceholderText("Backup Title");
    txtDesc = new QLineEdit; txtDesc->setPlaceholderText("Description");
    
    formGrid->addWidget(new QLabel("Title:"), 0, 0); formGrid->addWidget(txtTitle, 0, 1);
    formGrid->addWidget(new QLabel("Description:"), 0, 2); formGrid->addWidget(txtDesc, 0, 3);
    
    QPushButton *btnSave = new QPushButton("Update (Create Backup)"); 
    btnSave->setStyleSheet("background-color: #27ae60; color: white; font-weight: bold; padding: 5px 15px;");
    connect(btnSave, &QPushButton::clicked, this, &BackupRestoreWidget::createBackup);
    
    formGrid->addWidget(btnSave, 0, 4);
    mainLayout->addWidget(formGroup);

    // Table
    table = new QTableWidget;
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels({"ID", "Title", "Description", "Date", "User"});
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setAlternatingRowColors(true);
    
    // Restore/Delete Buttons
    QHBoxLayout *actionLay = new QHBoxLayout;
    QPushButton *btnRest = new QPushButton("Restore Selected");
    btnRest->setStyleSheet("background-color: #074282; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px; border: none;");
    connect(btnRest, &QPushButton::clicked, this, &BackupRestoreWidget::restoreSelected);

    QPushButton *btnDel = new QPushButton("Delete Selected");
    btnDel->setStyleSheet("background-color: #c0392b; color: white; font-weight: bold; padding: 6px 15px; border-radius: 4px; border: none;");
    connect(btnDel, &QPushButton::clicked, this, &BackupRestoreWidget::deleteSelected);

    actionLay->addWidget(btnRest);
    actionLay->addWidget(btnDel);
    actionLay->addStretch();

    mainLayout->addWidget(table);
    mainLayout->addLayout(actionLay);
}

void BackupRestoreWidget::toggleAddForm() {
    formGroup->setVisible(!formGroup->isVisible());
}

void BackupRestoreWidget::createBackup() {
    if(txtTitle->text().isEmpty()) {
        QMessageBox::warning(this, "Validation", "Title is required.");
        return;
    }
    
    if(DatabaseManager::instance().createBackup(txtTitle->text(), txtDesc->text())) {
        QMessageBox::information(this, "Success", "Database Backup Completed Successfully.");
        txtTitle->clear(); txtDesc->clear();
        formGroup->setVisible(false);
        refreshList();
    } else {
        QMessageBox::critical(this, "Error", "Backup Failed.");
    }
}

void BackupRestoreWidget::refreshList() {
    table->setRowCount(0);
    QSqlQuery q = DatabaseManager::instance().getBackups();
    while(q.next()) {
        int row = table->rowCount();
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(q.value("id").toString()));
        table->setItem(row, 1, new QTableWidgetItem(q.value("title").toString()));
        table->setItem(row, 2, new QTableWidgetItem(q.value("description").toString()));
        table->setItem(row, 3, new QTableWidgetItem(q.value("created_date").toString()));
        table->setItem(row, 4, new QTableWidgetItem(q.value("created_by").toString()));
    }
}

void BackupRestoreWidget::restoreSelected() {
    int row = table->currentRow();
    if(row < 0) return;
    
    int id = table->item(row, 0)->text().toInt();
    QString title = table->item(row, 1)->text();
    
    if(QMessageBox::warning(this, "Confirm Restore", 
        "Are you sure you want to restore snapshot '" + title + "'?\n\nWARNING: Current data will be overwritten and the application will close.", 
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
            
        if(DatabaseManager::instance().restoreBackup(id)) {
            QMessageBox::information(this, "Success", "Database Restored Successfully.\nPlease restart the application.");
            exit(0); // Exit to ensure DB reload on next run
        } else {
            QMessageBox::critical(this, "Error", "Restore Failed.");
        }
    }
}

void BackupRestoreWidget::deleteSelected() {
    int row = table->currentRow();
    if(row < 0) return;
    int id = table->item(row, 0)->text().toInt();
    
    if(DatabaseManager::instance().deleteBackup(id)) {
        refreshList();
    }
}
