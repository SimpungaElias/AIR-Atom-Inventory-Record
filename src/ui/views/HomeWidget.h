#ifndef HOMEWIDGET_H
#define HOMEWIDGET_H

#include <QWidget>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QLabel>

class HomeWidget : public QWidget {
    Q_OBJECT

public:
    explicit HomeWidget(QWidget *parent = nullptr);
    void refreshData();

private:
    void setupUI();
    void setupGLPreview(QVBoxLayout *layout);
    QTableWidget *tableMBR;
    QTableWidget *glTable;
    // Helper state
    double balU;
    double balU235;
    int balItems;
    int lineCounter;
};

#endif // HOMEWIDGET_H
