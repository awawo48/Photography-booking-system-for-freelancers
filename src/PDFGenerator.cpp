#include "PDFGenerator.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>

using namespace std;

string getTimestamp() {
    time_t now = time(0);
    tm t; localtime_s(&t, &now);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &t);
    return string(buf);
}

string getDateStamp() {
    time_t now = time(0);
    tm t; localtime_s(&t, &now);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y%m%d_%H%M%S", &t);
    return string(buf);
}

bool exportPDF(const string& filename, const string& title,
               const vector<string>& headers, const vector<vector<string>>& rows,
               const vector<string>& summaryLines) {
    ofstream f(filename, ios::binary);
    if (!f.is_open()) return false;

    // Simple PDF 1.4 generation
    vector<long> offsets;
    auto writeObj = [&](int id, const string& content) {
        offsets.push_back((long)f.tellp());
        f << id << " 0 obj\n" << content << "\nendobj\n";
    };

    f << "%PDF-1.4\n";

    // Build page content stream
    ostringstream cs;
    float pageW = 595, pageH = 842; // A4
    float marginL = 40, marginR = 40, marginT = 60;
    float usableW = pageW - marginL - marginR;
    float y = pageH - marginT;

    // Title
    cs << "BT\n/F1 16 Tf\n" << marginL << " " << y << " Td\n(" << title << ") Tj\nET\n";
    y -= 20;
    cs << "BT\n/F2 9 Tf\n" << marginL << " " << y << " Td\n(Generated: " << getTimestamp() << ") Tj\nET\n";
    y -= 25;

    // Divider line
    cs << marginL << " " << y << " m " << (pageW - marginR) << " " << y << " l S\n";
    y -= 20;

    int nCols = (int)headers.size();
    float colW = usableW / nCols;
    float rowH = 16;

    // Header background
    cs << "0.85 0.92 0.97 rg\n";
    cs << marginL << " " << (y - 3) << " " << usableW << " " << rowH << " re f\n";
    cs << "0 0 0 rg\n";

    // Header text
    cs << "BT\n/F1 9 Tf\n";
    for (int i = 0; i < nCols; i++) {
        cs << (marginL + i * colW + 4) << " " << (y + 2) << " Td\n(" << headers[i] << ") Tj\n";
        if (i < nCols - 1) cs << (-(marginL + i * colW + 4)) << " " << (-(y + 2)) << " Td\n";
    }
    cs << "ET\n";
    y -= rowH;

    // Table grid - header bottom line
    cs << "0.7 0.7 0.7 RG\n";
    cs << marginL << " " << (y + rowH - 3) << " m " << (marginL + usableW) << " " << (y + rowH - 3) << " l S\n";

    // Data rows
    bool alt = false;
    for (auto& row : rows) {
        if (y < 80) break; // page overflow guard
        if (alt) {
            cs << "0.95 0.95 0.95 rg\n";
            cs << marginL << " " << (y - 3) << " " << usableW << " " << rowH << " re f\n";
            cs << "0 0 0 rg\n";
        }
        cs << "BT\n/F2 8 Tf\n";
        for (int i = 0; i < nCols; i++) {
            string cell = (i < (int)row.size()) ? row[i] : "";
            // Escape PDF special chars
            string safe = "";
            for (char c : cell) {
                if (c == '(' || c == ')' || c == '\\') safe += '\\';
                safe += c;
            }
            cs << (marginL + i * colW + 4) << " " << (y + 2) << " Td\n(" << safe << ") Tj\n";
            if (i < nCols - 1) cs << (-(marginL + i * colW + 4)) << " " << (-(y + 2)) << " Td\n";
        }
        cs << "ET\n";
        // Row line
        cs << marginL << " " << (y - 3) << " m " << (marginL + usableW) << " " << (y - 3) << " l S\n";
        y -= rowH;
        alt = !alt;
    }

    // Column vertical lines
    for (int i = 0; i <= nCols; i++) {
        float x = marginL + i * colW;
        cs << x << " " << (pageH - marginT - 45 - 3) << " m " << x << " " << (y + rowH - 3) << " l S\n";
    }

    // Summary section
    if (!summaryLines.empty()) {
        y -= 15;
        cs << marginL << " " << y << " m " << (pageW - marginR) << " " << y << " l S\n";
        y -= 18;
        cs << "BT\n/F1 10 Tf\n" << marginL << " " << y << " Td\n(Summary) Tj\nET\n";
        y -= 16;
        for (auto& sl : summaryLines) {
            if (y < 40) break;
            string safe = "";
            for (char c : sl) { if (c == '(' || c == ')' || c == '\\') safe += '\\'; safe += c; }
            cs << "BT\n/F2 9 Tf\n" << marginL << " " << y << " Td\n(" << safe << ") Tj\nET\n";
            y -= 14;
        }
    }

    // Footer
    cs << "BT\n/F2 7 Tf\n" << marginL << " 25 Td\n(Photography Booking System - Auto-generated Report) Tj\nET\n";

    string stream = cs.str();

    // Write PDF objects
    writeObj(1, "<< /Type /Catalog /Pages 2 0 R >>");
    writeObj(2, "<< /Type /Pages /Kids [3 0 R] /Count 1 >>");
    writeObj(3, "<< /Type /Page /Parent 2 0 R /MediaBox [0 0 595 842] "
               "/Contents 6 0 R /Resources << /Font << /F1 4 0 R /F2 5 0 R >> >> >>");
    writeObj(4, "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica-Bold >>");
    writeObj(5, "<< /Type /Font /Subtype /Type1 /BaseFont /Helvetica >>");

    offsets.push_back((long)f.tellp());
    f << "6 0 obj\n<< /Length " << stream.size() << " >>\nstream\n" << stream << "endstream\nendobj\n";

    long xrefPos = (long)f.tellp();
    f << "xref\n0 7\n0000000000 65535 f \n";
    for (int i = 0; i < (int)offsets.size(); i++) {
        char b[32]; sprintf_s(b, "%010ld", offsets[i]);
        f << b << " 00000 n \n";
    }
    f << "trailer\n<< /Size 7 /Root 1 0 R >>\nstartxref\n" << xrefPos << "\n%%EOF\n";
    f.close();
    return true;
}
