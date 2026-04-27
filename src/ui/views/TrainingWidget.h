#ifndef TRAININGWIDGET_H
#define TRAININGWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGridLayout>
#include <QFrame>
#include <QStringList>

class TrainingWidget : public QWidget {
    Q_OBJECT

public:
    explicit TrainingWidget(QWidget *parent = nullptr);

signals:
    void scenarioStarted(QString name);

private slots:
    void launchScenario(QString name, QString description);

private:
    void setupUI();

    // Updated signature: includes objectives, duration, prerequisites
    QWidget* createScenarioCard(
        const QString &title,
        const QString &desc,
        const QStringList &objectives,
        const QString &difficulty,
        const QString &duration,
        const QString &prereq,
        const QString &id
    );
};

#endif // TRAININGWIDGET_H
