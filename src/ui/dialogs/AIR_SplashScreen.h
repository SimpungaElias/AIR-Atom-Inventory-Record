#ifndef AIR_SPLASHSCREEN_H
#define AIR_SPLASHSCREEN_H

#include <QDialog>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QFile>
#include <QSettings>

class AIRSplashScreen : public QDialog {
    Q_OBJECT

public:
    explicit AIRSplashScreen(QWidget *parent = nullptr);
    bool shouldShowOnStartup() const { return chkShow->isChecked(); }

signals:
    void enterApplication();

private slots:
    void goToPage(int index);
    void onEnterClicked();

private:
    void setupUI();
    void buildPage0_Welcome();
    void buildPage1_Enter();
    void buildPage2_Resources();

    QStackedWidget *stack;
    QCheckBox      *chkShow;
};

#endif // AIR_SPLASHSCREEN_H
