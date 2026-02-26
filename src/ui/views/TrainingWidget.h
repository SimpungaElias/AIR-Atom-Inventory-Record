#ifndef TRAININGWIDGET_H
#define TRAININGWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QGridLayout>

class TrainingWidget : public QWidget {
    Q_OBJECT

public:
    explicit TrainingWidget(QWidget *parent = nullptr);

signals:
    void scenarioStarted(QString name); // Signal to tell MainWindow to refresh UI

private slots:
    void launchScenario(QString name, QString description);

private:
    void setupUI();
    QWidget* createScenarioCard(QString title, QString desc, QString difficulty, QString id);
};

#endif // TRAININGWIDGET_H
