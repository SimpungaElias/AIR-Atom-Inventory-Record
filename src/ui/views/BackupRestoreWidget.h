#ifndef BACKUPRESTOREWIDGET_H
#define BACKUPRESTOREWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QLineEdit>
#include <QGroupBox>
#include <QPushButton>

class BackupRestoreWidget : public QWidget {
    Q_OBJECT
public:
    explicit BackupRestoreWidget(QWidget *parent = nullptr);

private slots:
    void toggleAddForm();
    void createBackup();
    void restoreSelected();
    void deleteSelected();

private:
    void setupUI();
    void refreshList();
    
    QTableWidget *table;
    QGroupBox *formGroup;
    QLineEdit *txtTitle, *txtDesc;
};

#endif // BACKUPRESTOREWIDGET_H
