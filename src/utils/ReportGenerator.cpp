#include "ReportGenerator.h"
#include <QPrinter>
#include <QTextDocument>
#include <QFile>
#include <QTextStream>
#include <QVariant>
#include <QDate>
#include <QPageSize>
#include <QPageLayout>
#include <QDebug>

// =============================================================================
// 1. INVENTORY CHANGE REPORT (ICR)
// =============================================================================

// PDF Version (Uses QMap for Headers)
bool ReportGenerator::generateICR_PDF(const QString &filename, const QMap<QString, QString> &headerData, QSqlQuery &data) {
    QString html = generateICR_HTML(headerData, data);
    
    QTextDocument document;
    document.setHtml(html);
    
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize::A4);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setOutputFileName(filename);

    document.print(&printer);
    return true;
}

// Excel Version (Uses old signature with int reportNo - Fixes your Linker Error)
bool ReportGenerator::generateICR_Excel(const QString &filename, const QString &mba, const QString &start, const QString &end, int reportNo, QSqlQuery &data) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) return false;

    QTextStream out(&file);
    out << "Inventory Change Report (ICR)\n";
    out << "MBA:," << mba << "\n";
    out << "Period:," << start << " to " << end << "\n";
    out << "Report No:," << reportNo << "\n\n";
    
    out << "Date,Batch,Code,Items,Element,Increase U,Decrease U\n";

    while(data.next()) {
        out << data.value("record_date").toString() << ",";
        out << data.value("batch_number").toString() << ",";
        out << data.value("change_type").toString() << ",";
        out << data.value("items_count").toInt() << ",";
        out << data.value("element").toString() << ",";
        out << data.value("increase_u").toDouble() << ",";
        out << data.value("decrease_u").toDouble() << "\n";
    }
    file.close();
    return true;
}

// HTML Helper (Uses QMap)
QString ReportGenerator::generateICR_HTML(const QMap<QString, QString> &headerData, QSqlQuery &data) {
    QString html = "<html><head><style>"
                   "body { font-family: Helvetica; font-size: 10pt; }"
                   "table { width: 100%; border-collapse: collapse; margin-top: 10px; }"
                   "th, td { border: 1px solid black; padding: 5px; text-align: center; vertical-align: middle; font-size: 9pt; }"
                   "th { background-color: #f0f0f0; font-weight: bold; }"
                   ".meta-table { width: 100%; border: none; margin-bottom: 20px; font-weight: bold; }"
                   ".meta-table td { border: none; text-align: left; padding: 2px; }"
                   ".footer-table { width: 100%; border: none; margin-top: 40px; }"
                   ".footer-table td { border: none; text-align: left; padding: 5px; font-size: 10pt; }"
                   "h2 { text-align: center; font-size: 16pt; text-decoration: underline; margin-bottom: 5px; }"
                   "</style></head><body>";

    html += "<h2>Inventory Change Document</h2>";
    
    html += "<table class='meta-table'>"
            "<tr>"
            "<td width='15%'>COUNTRY:</td><td width='35%'>" + headerData["country"] + "</td>"
            "<td width='15%'>DATE:</td><td width='35%'>" + headerData["date"] + "</td>"
            "</tr>"
            "<tr>"
            "<td>FACILITY:</td><td>" + headerData["facility"] + "</td>"
            "<td>REPORT NO:</td><td>" + headerData["reportNo"] + "</td>"
            "</tr>"
            "<tr>"
            "<td>MBA:</td><td>" + headerData["mba"] + "</td>"
            "<td></td><td></td>"
            "</tr>"
            "</table>";

    html += "<table><thead>"
            "<tr>"
            "<th colspan='4' style='border:none;'></th>"
            "<th colspan='4'>Uranium</th>"
            "<th colspan='2'>Plutonium</th>"
            "</tr>"
            "<tr>"
            "<th>Line</th><th>Batch Identity</th><th>No. of Items</th><th>Inventory<br>Change Code</th>"
            "<th>Element<br>Code</th><th>Isotope<br>Code</th><th>Element<br>weight(g)</th><th>Isotope<br>weight(g)</th>"
            "<th>Element<br>Code</th><th>Element<br>Weight(g)</th>"
            "</tr></thead><tbody>";

    int line = 1;
    double totalItems = 0;
    double sumU_Elem = 0, sumU_Iso = 0, sumPu = 0;

    while(data.next()) {
        QString batch = data.value("batch_number").toString();
        int items = data.value("items_count").toInt();
        if(items == 0) items = 1;
        QString code = data.value("change_type").toString();
        QString rawEl = data.value("element").toString(); 
        QString elCode = rawEl.left(1); 

        double wInc = data.value("increase_u").toDouble();
        double wDec = data.value("decrease_u").toDouble();
        double wElem = (wInc > 0) ? wInc : wDec;
        
        double wIso = data.value("weight_u235").toDouble();
        
        totalItems += items;

        html += "<tr>";
        html += "<td>" + QString::number(line++) + "</td>";
        html += "<td>" + batch + "</td>";
        html += "<td>" + QString::number(items) + "</td>";
        html += "<td>" + code + "</td>";

        if(elCode != "P") {
            sumU_Elem += wElem; sumU_Iso += wIso;
            html += "<td>" + elCode + "</td><td>G</td>";
            html += "<td>" + QString::number(wElem, 'f', 0) + "</td>";
            html += "<td>" + QString::number(wIso, 'f', 0) + "</td>";
            html += "<td></td><td></td>";
        } else {
            sumPu += wElem;
            html += "<td></td><td></td><td></td><td></td>";
            html += "<td>P</td><td>" + QString::number(wElem, 'f', 0) + "</td>";
        }
        html += "</tr>";
    }

    html += "<tr style='font-weight:bold; background-color:#f9f9f9;'>"
            "<td></td><td>Totals</td><td>" + QString::number(totalItems) + "</td>"
            "<td></td><td></td><td></td>"
            "<td>" + QString::number(sumU_Elem, 'f', 0) + "</td>"
            "<td>" + QString::number(sumU_Iso, 'f', 0) + "</td>"
            "<td></td><td>" + QString::number(sumPu, 'f', 0) + "</td>"
            "</tr>";

    html += "</tbody></table>";

    html += "<table class='footer-table'>"
            "<tr>"
            "<td width='50%'><b>Shipper-Receiver Difference</b></td>"
            "<td width='50%'><b>Shipment Date:</b> Start</td>"
            "</tr>"
            "<tr>"
            "<td>Date Measured: _________________</td>"
            "<td><b>Receiving Date:</b> " + headerData["date"] + "</td>"
            "</tr>"
            "<tr>"
            "<td>Shipper: " + headerData["shipper"] + "</td>"
            "<td>Receiver: " + headerData["receiver"] + "</td>"
            "</tr>"
            "<tr>"
            "<td>Signature: ______________________</td>"
            "<td>Signature: ______________________</td>"
            "</tr>"
            "</table>";

    html += "</body></html>";
    return html;
}

// =============================================================================
// 4. LIST OF INVENTORY ITEMS (LII)
// =============================================================================

bool ReportGenerator::generateLII_PDF(const QString &filename, const QMap<QString, QString> &headerData, QSqlQuery &data) {
    QString html = generateLII_HTML(headerData, data);
    
    QTextDocument document;
    document.setHtml(html);
    
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize::A4);
    printer.setPageOrientation(QPageLayout::Landscape);
    printer.setOutputFileName(filename);

    document.print(&printer);
    return true;
}

QString ReportGenerator::generateLII_HTML(const QMap<QString, QString> &headerData, QSqlQuery &data) {
    QString html = "<html><head><style>"
                   "body { font-family: Helvetica; font-size: 8pt; }"
                   "table { width: 100%; border-collapse: collapse; margin-top: 10px; }"
                   "th, td { border: 1px solid black; padding: 4px; text-align: center; vertical-align: middle; }"
                   "th { background-color: #f0f0f0; font-weight: bold; }"
                   ".meta-table { width: 100%; border: none; margin-bottom: 10px; }"
                   ".meta-table td { border: none; text-align: left; padding: 2px; font-size: 10pt; }"
                   "h2 { text-align: center; margin-bottom: 5px; text-decoration: underline; }"
                   "</style></head><body>";

    html += "<h2>LIST OF INVENTORY ITEMS (LII)</h2>";
    
    html += "<table class='meta-table'>"
            "<tr><td width='15%'><b>COUNTRY:</b></td><td width='35%'>" + headerData["country"] + "</td>"
            "<td width='15%'><b>DATE:</b></td><td width='35%'>" + headerData["date"] + "</td></tr>"
            "<tr><td><b>FACILITY:</b></td><td>" + headerData["facility"] + "</td>"
            "<td><b>REPORT NO:</b></td><td>" + headerData["reportNo"] + "</td></tr>"
            "<tr><td><b>MBA:</b></td><td>" + headerData["mba"] + "</td><td></td><td></td></tr>"
            "</table>";

    html += "<table><thead>"
            "<tr>"
            "<th colspan='2'>Location</th>"
            "<th colspan='2'>Identification</th>"
            "<th rowspan='2'>Material<br>Description</th>"
            "<th colspan='2'>Uranium</th>"
            "<th rowspan='2'>Plutonium<br>(g)</th>"
            "<th rowspan='2'>Thorium<br>(g)</th>"
            "<th colspan='2'>Nuclear</th>"
            "<th colspan='2'>Irradiated fuel</th>"
            "</tr>"
            "<tr>"
            "<th>KMP</th><th>Position</th>"
            "<th>Item</th><th>Batch</th>"
            "<th>Element (g)</th><th>Fissile (g)</th>"
            "<th>Loss (g)</th><th>Production (g)</th>"
            "<th>Burnup</th><th>Cooling</th>"
            "</tr></thead><tbody>";

    // Loop through query
    while(data.next()) {
        // MATCH THESE KEYS WITH LIIWidget::exportPDF temp table
        QString kmp = data.value("kmp").toString();
        QString pos = data.value("position").toString(); 
        QString batch = data.value("batch").toString();
        QString desc = data.value("desc").toString();
        double u = data.value("weight_elem").toDouble();
        double u235 = data.value("weight_fissile").toDouble();
        double pu = data.value("weight_pu").toDouble();
        double bu = data.value("burnup").toDouble();

        html += "<tr>";
        html += "<td>" + kmp + "</td>";
        html += "<td>" + pos + "</td>";
        
        // FIX: Display "1" for Item count in PDF
        html += "<td>1</td>";
        
        html += "<td>" + batch + "</td>";
        html += "<td>" + desc + "</td>";
        html += "<td>" + (u > 0 ? QString::number(u, 'f', 1) : "0.0") + "</td>";
        html += "<td>" + (u235 > 0 ? QString::number(u235, 'f', 1) : "0.0") + "</td>";
        html += "<td>" + (pu > 0 ? QString::number(pu, 'f', 1) : "0.0") + "</td>";
        html += "<td>0.00</td>";
        html += "<td>0.0</td><td>0.0</td>"; 
        html += "<td>" + QString::number(bu) + "</td><td>-</td>";
        html += "</tr>";
    }
    html += "</tbody></table></body></html>";
    return html;
}

// =============================================================================
// 6. NUCLEAR LOSS ITEMS (NLI)
// =============================================================================

bool ReportGenerator::generateNLI_PDF(const QString &filename, const QMap<QString, QString> &headerData, QSqlQuery &data) {
    // 1. Generate the HTML content using the helper function
    QString html = generateNLI_HTML(headerData, data);
    
    // 2. Set up the document
    QTextDocument document;
    document.setHtml(html);
    
    // 3. Configure Printer
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize::A4);
    printer.setPageOrientation(QPageLayout::Landscape); // Landscape fits the wide table better
    printer.setOutputFileName(filename);

    // 4. Print
    document.print(&printer);
    return true;
}

QString ReportGenerator::generateNLI_HTML(const QMap<QString, QString> &headerData, QSqlQuery &data) {
    QString html = "<html><head><style>"
                   "body { font-family: Helvetica; font-size: 10pt; }"
                   "table { width: 100%; border-collapse: collapse; margin-top: 15px; }"
                   "th, td { border: 1px solid black; padding: 6px; text-align: center; font-size: 9pt; vertical-align: middle; }"
                   "th { background-color: #f0f0f0; font-weight: bold; }"
                   // Header Styles
                   ".title-section { text-align: center; margin-bottom: 25px; }"
                   "h1 { margin: 0; font-size: 18pt; text-decoration: underline; font-weight: bold; }"
                   "h2 { margin: 5px 0 0 0; font-size: 12pt; font-weight: normal; }"
                   // Metadata Table Styles
                   ".meta-table { width: 100%; border: none; margin-bottom: 20px; }"
                   ".meta-table td { border: none; text-align: left; padding: 4px; font-size: 10pt; font-weight: bold; }"
                   "</style></head><body>";

    // 1. Title Section
    html += "<div class='title-section'>"
            "<h1>Nuclear Loss Items</h1>"
            "</div>";

    // 2. Metadata Section
    html += "<table class='meta-table'>"
            "<tr>"
            "<td width='15%'>COUNTRY:</td><td width='35%'>" + headerData["country"] + "</td>"
            "<td width='15%'>DATE:</td><td width='35%'>" + headerData["date"] + "</td>"
            "</tr>"
            "<tr>"
            "<td>FACILITY:</td><td>" + headerData["facility"] + "</td>"
            "<td>REPORT NO:</td><td>" + headerData["reportNo"] + "</td>"
            "</tr>"
            "<tr>"
            "<td>MBA:</td><td>" + headerData["mba"] + "</td>"
            "<td></td><td></td>"
            "</tr>"
            "</table>";

    // 3. Main Data Table
    html += "<table><thead>"
            // Row 1: Category Headers
            "<tr>"
            // colspan=4 covers: Line, Batch, Items, Code
            "<th colspan='4' style='border:none; border-bottom:1px solid black; background-color:white;'></th>" 
            "<th colspan='4'>Uranium</th>"
            "<th colspan='2'>Plutonium</th>"
            "</tr>"
            // Row 2: Column Headers (Split Line and Batch)
            "<tr>"
            "<th width='5%'>Line</th>"
            "<th width='12%'>Batch Identity</th>"
            "<th width='8%'>No. of<br>Items</th>"
            "<th width='8%'>Inventory<br>Change Code</th>"
            // Uranium
            "<th width='8%'>Element<br>Code</th>"
            "<th width='8%'>Isotope<br>Code</th>"
            "<th width='12%'>Element<br>weight (g)</th>"
            "<th width='12%'>Isotope<br>weight (g)</th>"
            // Plutonium
            "<th width='8%'>Element<br>Code</th>"
            "<th width='12%'>Element<br>Weight (g)</th>"
            "</tr></thead><tbody>";

    int line = 1;
    double totItems = 0;
    double totUWt = 0, totUIso = 0, totPWt = 0;

    while(data.next()) {
        QString batch = data.value("batch").toString();
        int items = data.value("items").toInt();
        QString code = data.value("code").toString();
        
        QString u_elem = data.value("u_elem_code").toString();
        QString u_iso = data.value("u_iso_code").toString();
        double u_wt = data.value("u_weight").toDouble();
        double u_iso_wt = data.value("u_iso_weight").toDouble();
        
        QString p_elem = data.value("p_elem_code").toString();
        double p_wt = data.value("p_weight").toDouble();

        totItems += items;
        totUWt += u_wt;
        totUIso += u_iso_wt;
        totPWt += p_wt;

        html += "<tr>";
        // Separate columns for Line and Batch
        html += "<td>" + QString::number(line++) + "</td>";
        html += "<td>" + batch + "</td>";
        html += "<td>" + QString::number(items) + "</td>";
        html += "<td>" + code + "</td>";
        
        // Uranium Data
        if(u_wt > 0 || !u_elem.isEmpty()) {
            html += "<td>" + u_elem + "</td>";
            html += "<td>" + u_iso + "</td>";
            html += "<td>" + QString::number(u_wt, 'f', 0) + "</td>";
            html += "<td>" + QString::number(u_iso_wt, 'f', 0) + "</td>";
        } else {
            html += "<td>-</td><td>-</td><td>-</td><td>-</td>";
        }

        // Plutonium Data
        if(p_wt > 0 || !p_elem.isEmpty()) {
            html += "<td>" + p_elem + "</td>";
            html += "<td>" + QString::number(p_wt, 'f', 0) + "</td>";
        } else {
            html += "<td>-</td><td>-</td>";
        }
        
        html += "</tr>";
    }

    // 4. Totals Row
    html += "<tr style='font-weight:bold; background-color:#f9f9f9;'>"
            "<td colspan='2' style='text-align:right'>Totals</td>" // Span Line+Batch
            "<td>" + QString::number(totItems) + "</td>"
            "<td></td>" // Code
            "<td></td>" // U Elem
            "<td></td>" // U Iso
            "<td>" + QString::number(totUWt, 'f', 0) + "</td>"
            "<td>" + QString::number(totUIso, 'f', 0) + "</td>"
            "<td></td>" // P Elem
            "<td>" + QString::number(totPWt, 'f', 0) + "</td>"
            "</tr>";

    html += "</tbody></table>";

    // FOOTER REMOVED as requested

    html += "</body></html>";
    return html;
}


// =============================================================================
// 5. GENERAL LEDGER (GL) - FIXED LOGIC
// =============================================================================

bool ReportGenerator::generateGL_PDF(const QString &filename, const QMap<QString, QString> &headerInfo, QSqlQuery &data) {
    QString html = generateGL_HTML(headerInfo, data);
    
    QTextDocument document;
    document.setHtml(html);
    
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize::A4);
    printer.setPageOrientation(QPageLayout::Landscape); 
    printer.setOutputFileName(filename);

    document.print(&printer);
    return true;
}

QString ReportGenerator::generateGL_HTML(const QMap<QString, QString> &headerInfo, QSqlQuery &data) {
    QString html = "<html><head><style>"
                   "body { font-family: Helvetica; font-size: 9pt; }"
                   "table { width: 100%; border-collapse: collapse; margin-top: 15px; }"
                   "th, td { border: 1px solid black; padding: 4px; text-align: center; font-size: 8pt; }"
                   "th { background-color: #f0f0f0; font-weight: bold; }"
                   "h2 { text-align: center; margin-bottom: 20px; }"
                   ".meta-table { width: 100%; border: none; margin-bottom: 5px; }"
                   ".meta-table td { border: none; text-align: left; padding: 5px; font-size: 10pt; font-weight: bold; }"
                   "</style></head><body>";

    html += "<h2>General Ledger</h2>";

    html += "<table class='meta-table'>"
            "<tr>"
            "<td width='60%'>Facility: " + headerInfo["facility"] + "</td>"
            "<td width='40%'>MBA: " + headerInfo["mba"] + "</td>"
            "</tr>"
            "<tr>"
            "<td colspan='2'>Material Description: " + headerInfo["desc"] + "</td>"
            "</tr>"
            "<tr>"
            "<td colspan='2'>"
            "Element Code: " + headerInfo["elemCode"] + "&nbsp;&nbsp;&nbsp;&nbsp;"
            "Isotope Code: " + headerInfo["isoCode"] + "&nbsp;&nbsp;&nbsp;&nbsp;"
            "Unit: " + headerInfo["unit"] +
            "</td>"
            "</tr>"
            "</table>";

    html += "<table><thead>"
            "<tr>"
            "<th rowspan='3'>Line</th>"
            "<th rowspan='3'>Date</th>"
            "<th rowspan='3'>ICD/PIL</th>"
            "<th rowspan='3'>IC Code</th>"
            "<th rowspan='3'>No. of<br>Items</th>"
            "<th colspan='4'>Increases</th>"
            "<th colspan='4'>Decreases</th>"
            "<th colspan='2'>Inventory</th>"
            "<th rowspan='3'>No. of<br>items</th>"
            "</tr>"
            "<tr>"
            "<th colspan='2'>Receipts</th><th colspan='2'>Other</th>"
            "<th colspan='2'>Shipments</th><th colspan='2'>Other</th>"
            "<th colspan='2'></th>" 
            "</tr>"
            "<tr>"
            "<th>U</th><th>U-235</th><th>U</th><th>U-235</th>"
            "<th>U</th><th>U-235</th><th>U</th><th>U-235</th>"
            "<th>U</th><th>U-235</th>"
            "</tr></thead><tbody>";

    int line = 1;
    double runU = 0, runU235 = 0;
    int runItems = 0;

    while(data.next()) {
        QString date = data.value("date").toString();
        QString ref = data.value("ref").toString();
        QString code = data.value("code").toString();
        QString type = data.value("type").toString();
        
        double u = data.value("u_weight").toDouble();
        double u235 = data.value("u235_weight").toDouble();
        int items = data.value("items").toInt();

        // --- FIXED REPORT LOGIC ---
        if(type == "Receipt") {
            // Receipt: Increases Items
            runU += u; runU235 += u235; runItems += items;
        } 
        else if (type == "Shipment") {
            // Shipment: Decreases Items
            runU -= u; runU235 -= u235; runItems -= items;
        }
        else if (type == "Other Increase") {
            runU += u; runU235 += u235;
        }
        else if (type == "Other Decrease" || type == "Nuclear Loss") {
            // Nuclear Loss: Decreases Weight ONLY. Items stay SAME.
            runU -= u; runU235 -= u235;
        }
        else if (type == "PIL (Set Balance)") {
            if(line == 1 && runU == 0) {
                runU = u; runU235 = u235; runItems = items;
            }
        }

        html += "<tr>";
        html += "<td>" + QString::number(line++) + "</td>";
        html += "<td>" + date + "</td>";
        html += "<td>" + ref + "</td>";
        html += "<td>" + code + "</td>";
        
        QString displayItems = "";
        if(type == "Receipt" || type == "Shipment") {
             displayItems = (items > 0 ? QString::number(items) : "");
        }
        html += "<td>" + displayItems + "</td>";

        QString rU="", r235="", oU="", o235="", sU="", s235="", odU="", od235="";
        
        if(type == "Receipt") {
            rU = QString::number(u); r235 = QString::number(u235);
        } else if (type == "Other Increase") {
            oU = QString::number(u); o235 = QString::number(u235);
        } else if (type == "Shipment") {
            sU = QString::number(u); s235 = QString::number(u235);
        } else { 
            // Nuclear Loss / Other Decrease
            odU = QString::number(u); od235 = QString::number(u235);
        }

        html += "<td>" + rU + "</td><td>" + r235 + "</td>";
        html += "<td>" + oU + "</td><td>" + o235 + "</td>";
        html += "<td>" + sU + "</td><td>" + s235 + "</td>";
        html += "<td>" + odU + "</td><td>" + od235 + "</td>";

        html += "<td style='background-color:#e8f5e9'>" + QString::number(runU) + "</td>";
        html += "<td style='background-color:#e8f5e9'>" + QString::number(runU235) + "</td>";
        html += "<td style='background-color:#e8f5e9'>" + QString::number(runItems) + "</td>";
        html += "</tr>";
    }

    html += "</tbody></table></body></html>";
    return html;
}


// =============================================================================
// MATERIAL BALANCE REPORT (MBR)
// =============================================================================

bool ReportGenerator::generateMBR_PDF(const QString &filename, const QMap<QString, QString> &headerData, QTableWidget *table) {
    QString html = generateMBR_HTML(headerData, table);
    
    QTextDocument document;
    document.setHtml(html);
    
    QPrinter printer(QPrinter::PrinterResolution);
    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setPageSize(QPageSize::A4);
    printer.setPageOrientation(QPageLayout::Landscape); // MBR is wide
    printer.setOutputFileName(filename);

    document.print(&printer);
    return true;
}

QString ReportGenerator::generateMBR_HTML(const QMap<QString, QString> &headerData, QTableWidget *table) {
    QString html = "<html><head><style>"
                   "body { font-family: Helvetica; font-size: 10pt; }"
                   "h1 { text-align: center; margin-bottom: 20px; }"
                   ".header-table { width: 100%; border: none; margin-bottom: 20px; font-weight: bold; }"
                   ".header-table td { border: none; padding: 4px; }"
                   ".data-table { width: 100%; border-collapse: collapse; margin-top: 10px; }"
                   ".data-table th, .data-table td { border: 1px solid black; padding: 5px; text-align: center; font-size: 9pt; }"
                   ".data-table th { background-color: #f0f0f0; }"
                   "</style></head><body>";

    html += "<h1>Material Balance Report</h1>";

    // Recreate the header layout from the image
    html += "<table class='header-table'>"
            "<tr>"
            "<td width='60%'>Country: " + headerData["country"] + "</td>"
            "<td width='40%'>Reporting Period From: " + headerData["periodFrom"] + "&nbsp;&nbsp;To: " + headerData["periodTo"] + "</td>"
            "</tr>"
            "<tr>"
            "<td>Facility: " + headerData["facility"] + "</td>"
            "<td>Report No. " + headerData["reportNo"] + "</td>"
            "</tr>"
            "<tr>"
            "<td>Material Balance Area: " + headerData["mba"] + "</td>"
            "<td></td>"
            "</tr>"
            "</table>";

    // Table Header matching the image's merged cells
    html += "<table class='data-table'>"
            "<thead>"
            "<tr>"
            "<th rowspan='2'>Entry No.</th>"
            "<th rowspan='2'>Continuation</th>"
            "<th rowspan='2'>Entry Name</th>"
            "<th colspan='5'>Accountancy Data</th>"
            "<th rowspan='2'>Report No</th>"
            "</tr>"
            "<tr>"
            "<th>Element</th>"
            "<th>Weight of Element</th>"
            "<th>Unit Kg/g</th>"
            "<th>Weight Of Fissile Isotopes<br>(Uranium Only) (G)</th>"
            "<th>Isotope Code</th>"
            "</tr>"
            "</thead><tbody>";

    // Table Data
    for (int i = 0; i < table->rowCount(); ++i) {
        html += "<tr>";
        for (int j = 0; j < table->columnCount(); ++j) {
            QTableWidgetItem *item = table->item(i, j);
            html += "<td>" + (item ? item->text() : "") + "</td>";
        }
        html += "</tr>";
    }

    html += "</tbody></table></body></html>";
    return html;
}
