#ifndef PINDIALOG_H
#define PINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

class PinDialog : public QDialog {
    Q_OBJECT
public:
    explicit PinDialog(QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle("Zero Trust Verification");
        setFixedSize(320, 150);
        setStyleSheet("QDialog { background-color: #f8f9fa; }");

        QVBoxLayout *layout = new QVBoxLayout(this);
        
        QLabel *lbl = new QLabel("<b>Security Verification Required</b><br>Please enter Admin PIN to authorize this action.");
        lbl->setWordWrap(true);
        lbl->setStyleSheet("color: #c0392b; font-size: 14px;");
        layout->addWidget(lbl);

        txtPin = new QLineEdit(this);
        txtPin->setEchoMode(QLineEdit::Password);
        txtPin->setPlaceholderText("Enter PIN (Hint: use 1234)");
        txtPin->setStyleSheet("padding: 5px; font-size: 14px;");
        layout->addWidget(txtPin);

        QHBoxLayout *btnLayout = new QHBoxLayout();
        QPushButton *btnVerify = new QPushButton("Verify Identity");
        btnVerify->setStyleSheet("background-color: #003366; color: white; font-weight: bold; padding: 6px;");
        
        QPushButton *btnCancel = new QPushButton("Cancel");
        btnCancel->setStyleSheet("padding: 6px;");

        btnLayout->addStretch();
        btnLayout->addWidget(btnCancel);
        btnLayout->addWidget(btnVerify);
        layout->addLayout(btnLayout);

        connect(btnVerify, &QPushButton::clicked, this, &PinDialog::checkPin);
        connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    }

private slots:
    void checkPin() {
        // For educational/training purposes, we hardcode the PIN to "1234"
        // In a real system, this would hash the input and check the users table.
        if (txtPin->text() == "1234") {
            accept(); // PIN is correct, close dialog and return Accepted
        } else {
            QMessageBox::critical(this, "Authentication Failed", "Invalid PIN. Zero Trust policy blocking action.");
            txtPin->clear();
        }
    }

private:
    QLineEdit *txtPin;
};

#endif // PINDIALOG_H
