#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include <QString>
#include <QSqlQuery>
#include <QMap> // <--- Added
#include <QTableWidget>

class ReportGenerator {
public:
    // ICR - UPDATED to accept Header Map
    static bool generateICR_PDF(const QString &filename, const QMap<QString, QString> &headerData, QSqlQuery &data);
    static bool generateICR_Excel(const QString &filename, const QString &mba, const QString &start, const QString &end, int reportNo, QSqlQuery &data);

    // LII - UPDATED to accept Header Map
    static bool generateLII_PDF(const QString &filename, const QMap<QString, QString> &headerData, QSqlQuery &data);
    
    // NLI
    static bool generateNLI_PDF(const QString &filename, const QMap<QString, QString> &headerData, QSqlQuery &data);

    // GENERAL LEDGER (Updated Signature)
    // Now accepts 'headerInfo' to pass Facility, Codes, Units from the UI
    static bool generateGL_PDF(const QString &filename, const QMap<QString, QString> &headerInfo, QSqlQuery &data);
    
    // New MBR functions
    static bool generateMBR_PDF(const QString &filename, const QMap<QString, QString> &headerData, QTableWidget *table);

private:
    // Updated Helper for ICR
    static QString generateICR_HTML(const QMap<QString, QString> &headerData, QSqlQuery &data);
    // NEW Helper for LII
    static QString generateLII_HTML(const QMap<QString, QString> &headerData, QSqlQuery &data);
    
    static QString generateNLI_HTML(const QMap<QString, QString> &headerData, QSqlQuery &data);
    
    // Updated Helper
    static QString generateGL_HTML(const QMap<QString, QString> &headerInfo, QSqlQuery &data);
    
    // New MBR HTML helper
    static QString generateMBR_HTML(const QMap<QString, QString> &headerData, QTableWidget *table);
};

#endif // REPORTGENERATOR_H
