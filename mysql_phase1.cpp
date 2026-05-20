#include <iostream>
#include <string>
#include <iomanip>
#include <vector>
#include <mysql.h>
#include <windows.h>
#include <conio.h>
#include <sstream>
#include <fstream>
#include <ctime>

using namespace std;

// ==================== ANSI Colors ====================
const string CLR_RS = "\033[0m", CLR_BD = "\033[1m", CLR_DM = "\033[2m";
const string CLR_CY = "\033[36m", CLR_GR = "\033[32m", CLR_YL = "\033[33m", CLR_RD = "\033[31m";
const string CLR_MG = "\033[35m", CLR_WH = "\033[97m", CLR_GY = "\033[90m";
const string CLR_BCY = "\033[1;36m", CLR_BGR = "\033[1;32m", CLR_BYL = "\033[1;33m";
const string CLR_BRD = "\033[1;31m", CLR_BWH = "\033[1;97m", CLR_BMG = "\033[1;35m";

// ==================== Key Codes ====================
const int K_UP = 1000, K_DOWN = 1001, K_ENTER = 13, K_ESC = 27, K_BKSP = 8;

// ==================== Layout ====================
const int BWIDTH = 62;

// ==================== Terminal Control ====================
void enableAnsi() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD m = 0; GetConsoleMode(h, &m);
    SetConsoleMode(h, m | 0x0004);
}

void cls() { cout << "\033[2J\033[H" << flush; }
void moveTo(int x, int y) { cout << "\033[" << y << ";" << x << "H" << flush; }
void hideCur() { cout << "\033[?25l" << flush; }
void showCur() { cout << "\033[?25h" << flush; }

int getTermW() {
    CONSOLE_SCREEN_BUFFER_INFO i;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &i);
    return i.srWindow.Right - i.srWindow.Left + 1;
}

int getKey() {
    int c = _getch();
    if (c == 0 || c == 224) {
        c = _getch();
        if (c == 72) return K_UP;
        if (c == 80) return K_DOWN;
        return -1;
    }
    return c;
}

// ==================== Drawing ====================
void drawBoxTop(int x, int y, int w, const string& color = "") {
    moveTo(x, y);
    cout << (color.empty() ? CLR_CY : color) << "\xC9";
    for (int i = 0; i < w - 2; i++) cout << "\xCD";
    cout << "\xBB" << CLR_RS;
}

void drawBoxBot(int x, int y, int w, const string& color = "") {
    moveTo(x, y);
    cout << (color.empty() ? CLR_CY : color) << "\xC8";
    for (int i = 0; i < w - 2; i++) cout << "\xCD";
    cout << "\xBC" << CLR_RS;
}

void drawBoxMid(int x, int y, int w, const string& color = "") {
    moveTo(x, y);
    cout << (color.empty() ? CLR_CY : color) << "\xCC";
    for (int i = 0; i < w - 2; i++) cout << "\xCD";
    cout << "\xB9" << CLR_RS;
}

void drawBoxRow(int x, int y, int w, const string& text, const string& color = "") {
    moveTo(x, y);
    cout << (color.empty() ? CLR_CY : color) << "\xBA" << CLR_RS;
    int pad = w - 2 - (int)text.length();
    if (pad < 0) pad = 0;
    cout << text;
    for (int i = 0; i < pad; i++) cout << " ";
    cout << (color.empty() ? CLR_CY : color) << "\xBA" << CLR_RS;
}

void drawTitle(int x, int y, int w, const string& title) {
    int pad = w - 2 - (int)title.length();
    int left = pad / 2, right = pad - left;
    string row = "";
    for (int i = 0; i < left; i++) row += " ";
    row += title;
    for (int i = 0; i < right; i++) row += " ";
    drawBoxTop(x, y, w);
    moveTo(x, y + 1);
    cout << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH << row << CLR_RS << CLR_CY << "\xBA" << CLR_RS;
    drawBoxMid(x, y + 2, w);
}

string padStr(const string& s, int w) {
    string r = s;
    while ((int)r.length() < w) r += " ";
    if ((int)r.length() > w) r = r.substr(0, w);
    return r;
}

// ==================== Components ====================
int showMenu(const string& title, const vector<string>& opts, const string& sub = "") {
    int sel = 0, n = (int)opts.size();
    while (true) {
        cls(); hideCur();
        int sx = (getTermW() - BWIDTH) / 2;
        if (sx < 1) sx = 1;
        int sy = 2;
        drawTitle(sx, sy, BWIDTH, title);
        int currY = sy + 3;
        if (!sub.empty()) {
            drawBoxRow(sx, currY, BWIDTH, "  " + CLR_DM + sub + CLR_RS);
            currY++;
        }
        drawBoxRow(sx, currY++, BWIDTH, "");
        for (int i = 0; i < n; i++) {
            string line;
            if (i == sel)
                line = "    " + CLR_BYL + "\x10 " + CLR_BWH + opts[i] + CLR_RS;
            else
                line = "      " + CLR_GY + opts[i] + CLR_RS;
            // Manual padding for ANSI (visible length differs from string length)
            int visLen = (i == sel) ? 6 + (int)opts[i].length() : 6 + (int)opts[i].length();
            int totalPad = BWIDTH - 2 - visLen;
            moveTo(sx, currY + i);
            cout << CLR_CY << "\xBA" << CLR_RS << line;
            for (int p = 0; p < totalPad; p++) cout << " ";
            cout << CLR_CY << "\xBA" << CLR_RS;
        }
        currY += n;
        drawBoxRow(sx, currY++, BWIDTH, "");
        drawBoxMid(sx, currY++, BWIDTH);
        string hint = "  " + CLR_GY + "Up/Down: Navigate   Enter: Select   Esc: Back" + CLR_RS;
        moveTo(sx, currY);
        cout << CLR_CY << "\xBA" << CLR_RS << hint;
        int hpad = BWIDTH - 2 - 47;
        for (int p = 0; p < hpad; p++) cout << " ";
        cout << CLR_CY << "\xBA" << CLR_RS;
        currY++;
        drawBoxBot(sx, currY, BWIDTH);

        int k = getKey();
        if (k == K_UP) sel = (sel - 1 + n) % n;
        else if (k == K_DOWN) sel = (sel + 1) % n;
        else if (k == K_ENTER) { showCur(); return sel; }
        else if (k == K_ESC) { showCur(); return -1; }
    }
}

string getInput(const string& prompt, bool pwd = false) {
    showCur();
    cout << CLR_BCY << "  \x10 " << CLR_WH << prompt << CLR_RS;
    string s;
    while (true) {
        int k = getKey();
        if (k == K_ENTER) break;
        if (k == K_ESC) { s = ""; break; }
        if (k == K_BKSP && !s.empty()) {
            s.pop_back();
            cout << "\b \b";
        } else if (k >= 32 && k < 127) {
            s += (char)k;
            cout << (pwd ? '*' : (char)k);
        }
    }
    cout << endl;
    return s;
}

void showMsg(const string& text, const string& type = "ok") {
    if (type == "ok") cout << CLR_BGR << "  [OK] " << CLR_WH << text << CLR_RS << endl;
    else if (type == "err") cout << CLR_BRD << "  [!] " << CLR_WH << text << CLR_RS << endl;
    else cout << CLR_BYL << "  [i] " << CLR_WH << text << CLR_RS << endl;
}

void showField(const string& label, const string& val, int lw = 18) {
    cout << CLR_GY << "    " << left << setw(lw) << label << CLR_RS << CLR_WH << val << CLR_RS << endl;
}

void showDivider() {
    cout << CLR_CY << "  ";
    for (int i = 0; i < BWIDTH - 4; i++) cout << "\xC4";
    cout << CLR_RS << endl;
}

void waitKey() {
    cout << endl << CLR_GY << "  Press any key to continue..." << CLR_RS;
    _getch();
}

void showScreenHeader(const string& title) {
    cls();
    int sx = (getTermW() - BWIDTH) / 2;
    if (sx < 1) sx = 1;
    drawTitle(sx, 1, BWIDTH, title);
    cout << "\n\n\n\n";
}

// ==================== Database ====================
MYSQL* connectDB() {
    MYSQL* conn = mysql_init(0);
    if (!conn) { cerr << "MySQL init failed!" << endl; return nullptr; }
    my_bool ssl_e = 0, ssl_v = 0;
    mysql_options(conn, MYSQL_OPT_SSL_ENFORCE, &ssl_e);
    mysql_options(conn, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &ssl_v);
    MYSQL* r = mysql_real_connect(conn, "localhost", "root", "", "photography_db", 3306, nullptr, 0);
    if (!r) { cerr << "DB Connection Failed: " << mysql_error(conn) << endl; }
    return r ? conn : nullptr;
}

// ==================== Forward Declarations ====================
void adminDashboard(MYSQL* conn, int userId);
void customerDashboard(MYSQL* conn, int userId);
void photographerDashboard(MYSQL* conn, int userId);

// ==================== PDF Export Helper ====================
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

// ==================== Auth Screens ====================
void registerUser(MYSQL* conn) {
    showScreenHeader("REGISTRATION");
    string name = getInput("Enter Name: ");
    string email = getInput("Enter Email: ");
    string password = getInput("Enter Password: ", true);
    string phone = getInput("Enter Phone Number: ");

    cout << endl;
    vector<string> roles = {"Customer", "Photographer"};
    cout << CLR_BYL << "  Select Role:" << CLR_RS << endl;
    cout << CLR_GY << "    [1] Customer" << endl;
    cout << "    [2] Photographer" << CLR_RS << endl;
    string rc = getInput("Enter choice (1/2): ");
    string role = (rc == "2") ? "Photographer" : "Customer";
    string status = (role == "Photographer") ? "Pending" : "Active";

    if (name.empty() || email.empty() || password.empty()) {
        showMsg("Registration cancelled.", "err");
        waitKey(); return;
    }

    string q = "INSERT INTO USERS (name, email, password, phone_no, role, account_status) VALUES ('" +
               name + "', '" + email + "', '" + password + "', '" + phone + "', '" + role + "', '" + status + "')";
    if (mysql_query(conn, q.c_str()))
        showMsg(string("Registration Failed: ") + mysql_error(conn), "err");
    else
        showMsg("Registration Successful! " + string(role == "Photographer" ? "Please wait for Admin approval." : ""), "ok");
    waitKey();
}

int loginUser(MYSQL* conn) {
    showScreenHeader("LOGIN");
    string email = getInput("Enter Email: ");
    string password = getInput("Enter Password: ", true);

    if (email.empty() || password.empty()) { return -1; }

    string q = "SELECT user_id, role, account_status FROM USERS WHERE email = '" + email + "' AND password = '" + password + "'";
    if (mysql_query(conn, q.c_str())) {
        showMsg(string("Login Failed: ") + mysql_error(conn), "err");
        waitKey(); return -1;
    }
    MYSQL_RES* res = mysql_store_result(conn);
    if (!res) { showMsg("Query error", "err"); waitKey(); return -1; }

    MYSQL_ROW row = mysql_fetch_row(res);
    if (row) {
        int uid = stoi(row[0]);
        string role = row[1], status = row[2];
        for (char& c : role) c = tolower(c);
        if (role == "photographer") role = "Photographer";
        else if (role == "customer") role = "Customer";
        else if (role == "admin") role = "Admin";

        if (role == "Photographer" && status == "Pending") {
            showMsg("Account pending admin approval.", "err");
            mysql_free_result(res); waitKey(); return -1;
        }
        showMsg("Login Successful! Logged in as " + role + ".", "ok");
        mysql_free_result(res);
        Sleep(800);
        if (role == "Admin") adminDashboard(conn, uid);
        else if (role == "Photographer") photographerDashboard(conn, uid);
        else if (role == "Customer") customerDashboard(conn, uid);
        return uid;
    } else {
        showMsg("Invalid email or password!", "err");
        mysql_free_result(res); waitKey(); return -1;
    }
}

// ==================== Admin Dashboard ====================
void adminDashboard(MYSQL* conn, int userId) {
    vector<string> opts = {"User Management", "Booking Monitoring", "Dispute Management", "Payment Management", "Review Management", "Business Reports", "Back / Logout"};
    while (true) {
        // Fetch total revenue & admin commission for dashboard display
        string dashSub = "";
        string dq = "SELECT COALESCE(SUM(amount), 0) FROM PAYMENTS";
        if (!mysql_query(conn, dq.c_str())) {
            MYSQL_RES* dres = mysql_store_result(conn);
            if (dres) {
                MYSQL_ROW dr = mysql_fetch_row(dres);
                if (dr && dr[0]) {
                    double totalRev = 0;
                    try { totalRev = stod(dr[0]); } catch(...) {}
                    double adminComm = totalRev * 0.05;
                    ostringstream oss;
                    oss << fixed << setprecision(2);
                    oss << "Revenue: RM " << totalRev << "  |  Your Commission (5%): RM " << adminComm;
                    dashSub = oss.str();
                }
                mysql_free_result(dres);
            }
        }
        int c = showMenu("ADMIN DASHBOARD  (ID: " + to_string(userId) + ")", opts, dashSub);
        if (c == 6 || c == -1) break;

        // --- User Management ---
        if (c == 0) {
            vector<string> uopts = {"Verify Pending Photographers", "Ban / Suspend a User", "View All Users", "Delete User", "Search User", "Back"};
            int uc = showMenu("USER MANAGEMENT", uopts);
            if (uc == 0) {
                showScreenHeader("VERIFY PENDING PHOTOGRAPHERS");
                string q = "SELECT user_id, name, email FROM USERS WHERE role = 'Photographer' AND account_status = 'Pending'";
                if (mysql_query(conn, q.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); }
                else {
                    MYSQL_RES* res = mysql_store_result(conn);
                    if (!res || mysql_num_rows(res) == 0) {
                        showMsg("No pending photographers.", "info");
                        if (res) mysql_free_result(res);
                    } else {
                        MYSQL_ROW row;
                        while ((row = mysql_fetch_row(res))) {
                            showDivider();
                            showField("User ID", row[0]);
                            showField("Name", row[1]);
                            showField("Email", row[2]);
                        }
                        showDivider();
                        mysql_free_result(res);
                        cout << endl;
                        string tid = getInput("Enter User ID to action: ");
                        string act = getInput("Enter Action (Active/Rejected): ");
                        string uq = "UPDATE USERS SET account_status = '" + act + "' WHERE user_id = " + tid;
                        if (mysql_query(conn, uq.c_str())) showMsg(string("Error: ") + mysql_error(conn), "err");
                        else showMsg("User " + tid + " status updated to '" + act + "'.", "ok");
                    }
                }
                waitKey();
            } else if (uc == 1) {
                vector<string> banopts = {"Ban a User", "Suspend a User", "Cancel Ban / Reactivate", "Back"};
                while (true) {
                    int bc = showMenu("BAN / SUSPEND MANAGEMENT", banopts, "Manage user account statuses");
                    if (bc == 3 || bc == -1) break;

                    showScreenHeader(bc == 0 ? "BAN USER" : (bc == 1 ? "SUSPEND USER" : "CANCEL BAN / REACTIVATE"));

                    // Build query based on action context
                    string q;
                    if (bc == 2)
                        q = "SELECT user_id, name, email, role, account_status FROM USERS WHERE account_status IN ('Banned','Suspended')";
                    else
                        q = "SELECT user_id, name, email, role, account_status FROM USERS WHERE role != 'Admin'";

                    if (mysql_query(conn, q.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                    MYSQL_RES* res = mysql_store_result(conn);
                    if (!res || mysql_num_rows(res) == 0) {
                        showMsg(bc == 2 ? "No banned/suspended users found." : "No users found.", "info");
                        if (res) mysql_free_result(res); waitKey(); continue;
                    }

                    // Column widths: ID(4) | Name(18) | Email(26) | Role(14) | Status(10)
                    int cID=4, cName=18, cEmail=26, cRole=14, cStatus=10;

                    // Table top border
                    cout << CLR_CY << "  \xC9";
                    for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cStatus+2;i++) cout<<"\xCD";
                    cout << "\xBB" << CLR_RS << endl;

                    // Header row
                    cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("ID", cID+1)
                         << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Name", cName+1)
                         << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Email", cEmail+1)
                         << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Role", cRole+1)
                         << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Status", cStatus+1)
                         << CLR_CY << "\xBA" << CLR_RS << endl;

                    // Header-body separator
                    cout << CLR_CY << "  \xCC";
                    for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cStatus+2;i++) cout<<"\xCD";
                    cout << "\xB9" << CLR_RS << endl;

                    // Data rows
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        string sid = row[0] ? row[0] : "";
                        string sname = row[1] ? row[1] : "";
                        string semail = row[2] ? row[2] : "";
                        string srole = row[3] ? row[3] : "";
                        string sstatus = row[4] ? row[4] : "";

                        // Color-code status
                        string statusColor = CLR_WH;
                        if (sstatus == "Active") statusColor = CLR_BGR;
                        else if (sstatus == "Pending") statusColor = CLR_BYL;
                        else if (sstatus == "Banned") statusColor = CLR_BRD;
                        else if (sstatus == "Suspended") statusColor = CLR_BMG;

                        cout << CLR_CY << "  \xBA" << CLR_RS
                             << CLR_WH << " " << padStr(sid, cID+1)
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(sname, cName+1)
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(semail, cEmail+1)
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(srole, cRole+1)
                             << CLR_CY << "\xBA" << CLR_RS
                             << statusColor << " " << padStr(sstatus, cStatus+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS << endl;
                    }

                    // Table bottom border
                    cout << CLR_CY << "  \xC8";
                    for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cStatus+2;i++) cout<<"\xCD";
                    cout << "\xBC" << CLR_RS << endl;

                    mysql_free_result(res);
                    cout << endl;

                    // Action input
                    string tid = getInput("Enter User ID (0 to cancel): ");
                    if (tid == "0" || tid.empty()) { waitKey(); continue; }

                    string newStatus, actionLabel;
                    if (bc == 0) { newStatus = "Banned"; actionLabel = "banned"; }
                    else if (bc == 1) { newStatus = "Suspended"; actionLabel = "suspended"; }
                    else { newStatus = "Active"; actionLabel = "reactivated"; }

                    string uq = "UPDATE USERS SET account_status = '" + newStatus + "' WHERE user_id = " + tid;
                    if (mysql_query(conn, uq.c_str())) showMsg(string("Error: ") + mysql_error(conn), "err");
                    else showMsg("User " + tid + " has been " + actionLabel + ".", "ok");
                    waitKey();
                }
            }

            // --- View All Users ---
            else if (uc == 2) {
                showScreenHeader("ALL REGISTERED USERS");
                string q = "SELECT user_id, name, email, phone_no, role, account_status FROM USERS ORDER BY user_id ASC";
                if (mysql_query(conn, q.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); }
                else {
                    MYSQL_RES* res = mysql_store_result(conn);
                    if (!res || mysql_num_rows(res) == 0) {
                        showMsg("No users found.", "info");
                        if (res) mysql_free_result(res);
                    } else {
                        int cID=4, cName=18, cEmail=26, cPhone=14, cRole=14, cSt=10;
                        cout << CLR_CY << "  \xC9";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cPhone+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                        cout << "\xBB" << CLR_RS << endl;

                        cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("ID", cID+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Name", cName+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Email", cEmail+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Phone", cPhone+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Role", cRole+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Status", cSt+1)
                             << CLR_CY << "\xBA" << CLR_RS << endl;

                        cout << CLR_CY << "  \xCC";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cPhone+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                        cout << "\xB9" << CLR_RS << endl;

                        MYSQL_ROW row;
                        while ((row = mysql_fetch_row(res))) {
                            string sid = row[0]?row[0]:"", sn = row[1]?row[1]:"";
                            string se = row[2]?row[2]:"", sp = row[3]?row[3]:"";
                            string sr = row[4]?row[4]:"", ss = row[5]?row[5]:"";
                            string sc = CLR_WH, rc = CLR_WH;
                            if (ss=="Active") sc=CLR_BGR; else if (ss=="Pending") sc=CLR_BYL;
                            else if (ss=="Banned") sc=CLR_BRD; else if (ss=="Suspended") sc=CLR_BMG;
                            if (sr=="Admin") rc=CLR_BCY; else if (sr=="Photographer") rc=CLR_BMG;
                            else if (sr=="Customer") rc=CLR_BYL;
                            cout << CLR_CY << "  \xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(sid,cID+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(sn,cName+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(se,cEmail+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(sp,cPhone+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << rc << " " << padStr(sr,cRole+1) << CLR_RS
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << sc << " " << padStr(ss,cSt+1) << CLR_RS
                                 << CLR_CY << "\xBA" << CLR_RS << endl;
                        }
                        cout << CLR_CY << "  \xC8";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cPhone+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                        cout << "\xBC" << CLR_RS << endl;
                        mysql_free_result(res);
                    }
                }
                waitKey();
            }

            // --- Delete User ---
            else if (uc == 3) {
                showScreenHeader("DELETE USER");
                string q = "SELECT user_id, name, email, role, account_status FROM USERS WHERE role != 'Admin' ORDER BY user_id";
                if (mysql_query(conn, q.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); }
                else {
                    MYSQL_RES* res = mysql_store_result(conn);
                    if (!res || mysql_num_rows(res) == 0) {
                        showMsg("No users found.", "info");
                        if (res) mysql_free_result(res);
                    } else {
                        int cID=4, cName=18, cEmail=26, cRole=14, cSt=10;
                        cout << CLR_CY << "  \xC9";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                        cout << "\xBB" << CLR_RS << endl;
                        cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("ID",cID+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Name",cName+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Email",cEmail+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Role",cRole+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Status",cSt+1) << CLR_CY << "\xBA" << CLR_RS << endl;
                        cout << CLR_CY << "  \xCC";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                        cout << "\xB9" << CLR_RS << endl;
                        MYSQL_ROW row;
                        while ((row = mysql_fetch_row(res))) {
                            string sid=row[0]?row[0]:"", sn=row[1]?row[1]:"";
                            string se=row[2]?row[2]:"", sr=row[3]?row[3]:"", ss=row[4]?row[4]:"";
                            cout << CLR_CY << "  \xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(sid,cID+1) << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(sn,cName+1) << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(se,cEmail+1) << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(sr,cRole+1) << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(ss,cSt+1)
                                 << CLR_CY << "\xBA" << CLR_RS << endl;
                        }
                        cout << CLR_CY << "  \xC8";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                        cout << "\xBC" << CLR_RS << endl;
                        mysql_free_result(res);
                        cout << endl;
                        string tid = getInput("Enter User ID to delete (0 to cancel): ");
                        if (tid != "0" && !tid.empty()) {
                            string confirm = getInput("Type 'yes' to confirm delete user " + tid + ": ");
                            if (confirm == "yes") {
                                // Delete related data first (FK constraints)
                                mysql_query(conn, ("DELETE FROM REVIEWS WHERE booking_id IN (SELECT booking_id FROM BOOKINGS WHERE user_id=" + tid + ")").c_str());
                                mysql_query(conn, ("DELETE FROM PAYMENTS WHERE booking_id IN (SELECT booking_id FROM BOOKINGS WHERE user_id=" + tid + ")").c_str());
                                mysql_query(conn, ("DELETE FROM REPORTS WHERE user_id=" + tid).c_str());
                                mysql_query(conn, ("DELETE FROM BOOKINGS WHERE user_id=" + tid).c_str());
                                mysql_query(conn, ("DELETE FROM SCHEDULES WHERE user_id=" + tid).c_str());
                                mysql_query(conn, ("DELETE FROM PACKAGES WHERE user_id=" + tid).c_str());
                                mysql_query(conn, ("DELETE FROM PORTFOLIOS WHERE user_id=" + tid).c_str());
                                string dq = "DELETE FROM USERS WHERE user_id = " + tid + " AND role != 'Admin'";
                                if (mysql_query(conn, dq.c_str())) showMsg(string("Error: ") + mysql_error(conn), "err");
                                else showMsg("User " + tid + " deleted successfully.", "ok");
                            } else {
                                showMsg("Delete cancelled.", "info");
                            }
                        }
                    }
                }
                waitKey();
            }

            // --- Search User ---
            else if (uc == 4) {
                showScreenHeader("SEARCH USER");
                string keyword = getInput("Enter name or email to search: ");
                if (keyword.empty()) { showMsg("Search cancelled.", "info"); waitKey(); }
                else {
                    string q = "SELECT user_id, name, email, phone_no, role, account_status FROM USERS "
                               "WHERE name LIKE '%" + keyword + "%' OR email LIKE '%" + keyword + "%' ORDER BY user_id";
                    if (mysql_query(conn, q.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); }
                    else {
                        MYSQL_RES* res = mysql_store_result(conn);
                        if (!res || mysql_num_rows(res) == 0) {
                            showMsg("No users matched '" + keyword + "'.", "info");
                            if (res) mysql_free_result(res);
                        } else {
                            int cID=4, cName=18, cEmail=26, cPhone=14, cRole=14, cSt=10;
                            cout << CLR_CY << "  \xC9";
                            for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCB";
                            for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCB";
                            for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCB";
                            for(int i=0;i<cPhone+2;i++) cout<<"\xCD"; cout<<"\xCB";
                            for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCB";
                            for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                            cout << "\xBB" << CLR_RS << endl;
                            cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                                 << " " << padStr("ID",cID+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                                 << " " << padStr("Name",cName+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                                 << " " << padStr("Email",cEmail+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                                 << " " << padStr("Phone",cPhone+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                                 << " " << padStr("Role",cRole+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                                 << " " << padStr("Status",cSt+1) << CLR_CY << "\xBA" << CLR_RS << endl;
                            cout << CLR_CY << "  \xCC";
                            for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCE";
                            for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCE";
                            for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCE";
                            for(int i=0;i<cPhone+2;i++) cout<<"\xCD"; cout<<"\xCE";
                            for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCE";
                            for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                            cout << "\xB9" << CLR_RS << endl;
                            MYSQL_ROW row;
                            while ((row = mysql_fetch_row(res))) {
                                string sid=row[0]?row[0]:"", sn=row[1]?row[1]:"";
                                string se=row[2]?row[2]:"", sp=row[3]?row[3]:"";
                                string sr=row[4]?row[4]:"", ss=row[5]?row[5]:"";
                                string sc=CLR_WH;
                                if (ss=="Active") sc=CLR_BGR; else if (ss=="Pending") sc=CLR_BYL;
                                else if (ss=="Banned") sc=CLR_BRD; else if (ss=="Suspended") sc=CLR_BMG;
                                cout << CLR_CY << "  \xBA" << CLR_RS
                                     << CLR_WH << " " << padStr(sid,cID+1) << CLR_CY << "\xBA" << CLR_RS
                                     << CLR_WH << " " << padStr(sn,cName+1) << CLR_CY << "\xBA" << CLR_RS
                                     << CLR_WH << " " << padStr(se,cEmail+1) << CLR_CY << "\xBA" << CLR_RS
                                     << CLR_WH << " " << padStr(sp,cPhone+1) << CLR_CY << "\xBA" << CLR_RS
                                     << CLR_WH << " " << padStr(sr,cRole+1) << CLR_CY << "\xBA" << CLR_RS
                                     << sc << " " << padStr(ss,cSt+1) << CLR_RS
                                     << CLR_CY << "\xBA" << CLR_RS << endl;
                            }
                            cout << CLR_CY << "  \xC8";
                            for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCA";
                            for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCA";
                            for(int i=0;i<cEmail+2;i++) cout<<"\xCD"; cout<<"\xCA";
                            for(int i=0;i<cPhone+2;i++) cout<<"\xCD"; cout<<"\xCA";
                            for(int i=0;i<cRole+2;i++) cout<<"\xCD"; cout<<"\xCA";
                            for(int i=0;i<cSt+2;i++) cout<<"\xCD";
                            cout << "\xBC" << CLR_RS << endl;
                            mysql_free_result(res);
                        }
                    }
                    waitKey();
                }
            }

        // --- Booking Monitoring ---
        } else if (c == 1) {
            vector<string> bmopts = {"View All Bookings", "Filter by Status", "Update Booking Status", "Cancel Booking", "Back"};
            while (true) {
                int bmc = showMenu("BOOKING MONITORING", bmopts, "Track and manage all bookings");
                if (bmc == 4 || bmc == -1) break;

                // --- Update Booking Status ---
                if (bmc == 2) {
                    showScreenHeader("UPDATE BOOKING STATUS");
                    string bq = "SELECT b.booking_id, c.name, p.package_name, b.booking_date, b.job_status "
                                "FROM BOOKINGS b JOIN USERS c ON b.user_id = c.user_id "
                                "JOIN PACKAGES p ON b.package_id = p.package_id ORDER BY b.booking_id";
                    if (mysql_query(conn, bq.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                    MYSQL_RES* bres = mysql_store_result(conn);
                    if (!bres || mysql_num_rows(bres) == 0) {
                        showMsg("No bookings found.", "info");
                        if (bres) mysql_free_result(bres); waitKey(); continue;
                    }
                    int cBI=4, cCu=16, cPk=18, cDt=12, cSs=13;
                    cout << CLR_CY << "  \xC9";
                    for(int i=0;i<cBI+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cSs+2;i++) cout<<"\xCD";
                    cout << "\xBB" << CLR_RS << endl;
                    cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("ID",cBI+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Customer",cCu+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Package",cPk+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Date",cDt+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Status",cSs+1) << CLR_CY << "\xBA" << CLR_RS << endl;
                    cout << CLR_CY << "  \xCC";
                    for(int i=0;i<cBI+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cSs+2;i++) cout<<"\xCD";
                    cout << "\xB9" << CLR_RS << endl;
                    MYSQL_ROW br;
                    while ((br = mysql_fetch_row(bres))) {
                        string sc = CLR_WH;
                        string ss = br[4]?br[4]:"";
                        if (ss=="Completed") sc=CLR_BGR; else if (ss=="Rejected") sc=CLR_BRD;
                        else if (ss=="Pending"||ss=="Approved"||ss=="Deposit Paid") sc=CLR_BYL;
                        cout << CLR_CY << "  \xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[0]?br[0]:"",cBI+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[1]?br[1]:"",cCu+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[2]?br[2]:"",cPk+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[3]?br[3]:"",cDt+1) << CLR_CY << "\xBA" << CLR_RS
                             << sc << " " << padStr(ss,cSs+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS << endl;
                    }
                    cout << CLR_CY << "  \xC8";
                    for(int i=0;i<cBI+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cSs+2;i++) cout<<"\xCD";
                    cout << "\xBC" << CLR_RS << endl;
                    mysql_free_result(bres);
                    cout << endl;
                    string bid = getInput("Enter Booking ID to update (0 to cancel): ");
                    if (bid != "0" && !bid.empty()) {
                        cout << endl;
                        cout << CLR_BWH << "    Select new status:" << CLR_RS << endl;
                        cout << CLR_BYL << "      [1] Pending" << CLR_RS << endl;
                        cout << CLR_BYL << "      [2] Approved" << CLR_RS << endl;
                        cout << CLR_BYL << "      [3] Deposit Paid" << CLR_RS << endl;
                        cout << CLR_BGR << "      [4] Completed" << CLR_RS << endl;
                        cout << CLR_BRD << "      [5] Rejected" << CLR_RS << endl;
                        string sc = getInput("Enter choice (1-5): ");
                        string ns = "";
                        if (sc=="1") ns="Pending"; else if (sc=="2") ns="Approved";
                        else if (sc=="3") ns="Deposit Paid"; else if (sc=="4") ns="Completed";
                        else if (sc=="5") ns="Rejected";
                        if (!ns.empty()) {
                            string uq = "UPDATE BOOKINGS SET job_status = '" + ns + "' WHERE booking_id = " + bid;
                            if (mysql_query(conn, uq.c_str())) showMsg(string("Error: ") + mysql_error(conn), "err");
                            else showMsg("Booking #" + bid + " status updated to '" + ns + "'.", "ok");
                        } else showMsg("Invalid choice.", "err");
                    }
                    waitKey(); continue;
                }

                // --- Cancel Booking ---
                if (bmc == 3) {
                    showScreenHeader("CANCEL / DELETE BOOKING");
                    string bq = "SELECT b.booking_id, c.name, p.package_name, b.booking_date, b.job_status "
                                "FROM BOOKINGS b JOIN USERS c ON b.user_id = c.user_id "
                                "JOIN PACKAGES p ON b.package_id = p.package_id ORDER BY b.booking_id";
                    if (mysql_query(conn, bq.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                    MYSQL_RES* bres = mysql_store_result(conn);
                    if (!bres || mysql_num_rows(bres) == 0) {
                        showMsg("No bookings found.", "info");
                        if (bres) mysql_free_result(bres); waitKey(); continue;
                    }
                    int cBI=4, cCu=16, cPk=18, cDt=12, cSs=13;
                    cout << CLR_CY << "  \xC9";
                    for(int i=0;i<cBI+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cSs+2;i++) cout<<"\xCD";
                    cout << "\xBB" << CLR_RS << endl;
                    cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("ID",cBI+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Customer",cCu+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Package",cPk+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Date",cDt+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Status",cSs+1) << CLR_CY << "\xBA" << CLR_RS << endl;
                    cout << CLR_CY << "  \xCC";
                    for(int i=0;i<cBI+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cSs+2;i++) cout<<"\xCD";
                    cout << "\xB9" << CLR_RS << endl;
                    MYSQL_ROW br;
                    while ((br = mysql_fetch_row(bres))) {
                        cout << CLR_CY << "  \xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[0]?br[0]:"",cBI+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[1]?br[1]:"",cCu+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[2]?br[2]:"",cPk+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[3]?br[3]:"",cDt+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(br[4]?br[4]:"",cSs+1)
                             << CLR_CY << "\xBA" << CLR_RS << endl;
                    }
                    cout << CLR_CY << "  \xC8";
                    for(int i=0;i<cBI+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cSs+2;i++) cout<<"\xCD";
                    cout << "\xBC" << CLR_RS << endl;
                    mysql_free_result(bres);
                    cout << endl;
                    string bid = getInput("Enter Booking ID to delete (0 to cancel): ");
                    if (bid != "0" && !bid.empty()) {
                        string confirm = getInput("Type 'yes' to confirm delete booking #" + bid + ": ");
                        if (confirm == "yes") {
                            mysql_query(conn, ("DELETE FROM REVIEWS WHERE booking_id=" + bid).c_str());
                            mysql_query(conn, ("DELETE FROM PAYMENTS WHERE booking_id=" + bid).c_str());
                            mysql_query(conn, ("DELETE FROM REPORTS WHERE booking_id=" + bid).c_str());
                            string dq = "DELETE FROM BOOKINGS WHERE booking_id = " + bid;
                            if (mysql_query(conn, dq.c_str())) showMsg(string("Error: ") + mysql_error(conn), "err");
                            else showMsg("Booking #" + bid + " deleted successfully.", "ok");
                        } else showMsg("Delete cancelled.", "info");
                    }
                    waitKey(); continue;
                }

                string statusFilter = "";
                if (bmc == 1) {
                    vector<string> fopts = {"Pending", "Approved", "Deposit Paid", "Completed", "Rejected", "Back"};
                    int fc = showMenu("FILTER BY STATUS", fopts);
                    if (fc == 5 || fc == -1) continue;
                    statusFilter = fopts[fc];
                }

                showScreenHeader(statusFilter.empty() ? "ALL BOOKINGS (MASTER LIST)" : ("BOOKINGS: " + statusFilter));

                string q = "SELECT b.booking_id, c.name AS customer_name, p.package_name, "
                           "b.booking_date, b.job_status, p.price "
                           "FROM BOOKINGS b JOIN USERS c ON b.user_id = c.user_id "
                           "JOIN PACKAGES p ON b.package_id = p.package_id";
                if (!statusFilter.empty())
                    q += " WHERE b.job_status = '" + statusFilter + "'";
                q += " ORDER BY b.booking_id ASC";

                if (mysql_query(conn, q.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                MYSQL_RES* res = mysql_store_result(conn);
                if (!res || mysql_num_rows(res) == 0) {
                    showMsg("No bookings found.", "info");
                    if (res) mysql_free_result(res); waitKey(); continue;
                }

                // Column widths: ID(4) | Customer(16) | Package(18) | Date(12) | Status(13) | Price(10)
                int cID=4, cCust=16, cPkg=18, cDate=12, cStat=13, cPrice=10;
                int tableW = 1 + (cID+2) + 1 + (cCust+2) + 1 + (cPkg+2) + 1 + (cDate+2) + 1 + (cStat+2) + 1 + (cPrice+2) + 1;

                // Table top border
                cout << CLR_CY << "  ";
                cout << "\xC9";
                for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cCust+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cStat+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cPrice+2;i++) cout<<"\xCD";
                cout << "\xBB" << CLR_RS << endl;

                // Header row
                cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("ID", cID+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Customer", cCust+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Package", cPkg+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Date", cDate+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Status", cStat+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Price(RM)", cPrice+1)
                     << CLR_CY << "\xBA" << CLR_RS << endl;

                // Header-body separator
                cout << CLR_CY << "  ";
                cout << "\xCC";
                for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cCust+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cStat+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cPrice+2;i++) cout<<"\xCD";
                cout << "\xB9" << CLR_RS << endl;

                // Data rows
                MYSQL_ROW row;
                int totalBookings = 0;
                double totalValue = 0.0;
                int cntPending=0, cntApproved=0, cntDeposit=0, cntCompleted=0, cntRejected=0;
                while ((row = mysql_fetch_row(res))) {
                    totalBookings++;
                    string sid = row[0] ? row[0] : "";
                    string sname = row[1] ? row[1] : "";
                    string spkg = row[2] ? row[2] : "";
                    string sdate = row[3] ? row[3] : "";
                    string sstatus = row[4] ? row[4] : "";
                    string sprice = row[5] ? row[5] : "0";
                    try { totalValue += stod(sprice); } catch(...) {}

                    if (sstatus == "Pending") cntPending++;
                    else if (sstatus == "Approved") cntApproved++;
                    else if (sstatus == "Deposit Paid") cntDeposit++;
                    else if (sstatus == "Completed") cntCompleted++;
                    else if (sstatus == "Rejected") cntRejected++;

                    // Color-code status
                    string statusColor = CLR_WH;
                    if (sstatus == "Completed") statusColor = CLR_BGR;
                    else if (sstatus == "Pending" || sstatus == "Approved" || sstatus == "Deposit Paid") statusColor = CLR_BYL;
                    else if (sstatus == "Rejected") statusColor = CLR_BRD;

                    cout << CLR_CY << "  \xBA" << CLR_RS
                         << CLR_WH << " " << padStr(sid, cID+1)
                         << CLR_CY << "\xBA" << CLR_RS
                         << CLR_WH << " " << padStr(sname, cCust+1)
                         << CLR_CY << "\xBA" << CLR_RS
                         << CLR_WH << " " << padStr(spkg, cPkg+1)
                         << CLR_CY << "\xBA" << CLR_RS
                         << CLR_WH << " " << padStr(sdate, cDate+1)
                         << CLR_CY << "\xBA" << CLR_RS
                         << statusColor << " " << padStr(sstatus, cStat+1) << CLR_RS
                         << CLR_CY << "\xBA" << CLR_RS
                         << CLR_WH << " " << padStr(sprice, cPrice+1)
                         << CLR_CY << "\xBA" << CLR_RS << endl;
                }

                // Table bottom border
                cout << CLR_CY << "  ";
                cout << "\xC8";
                for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cCust+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cStat+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cPrice+2;i++) cout<<"\xCD";
                cout << "\xBC" << CLR_RS << endl;

                mysql_free_result(res);

                // Summary stats
                cout << endl;
                showDivider();
                cout << CLR_BD << CLR_BWH << "    SUMMARY" << CLR_RS << endl;
                showDivider();

                ostringstream oss;
                oss << fixed << setprecision(2) << totalValue;
                double adminCut = totalValue * 0.05;
                ostringstream ossAdmin;
                ossAdmin << fixed << setprecision(2) << adminCut;

                showField("Total Bookings", to_string(totalBookings));
                showField("Total Value", string("RM ") + oss.str());
                showField("Admin Commission (5%)", string("RM ") + ossAdmin.str());
                cout << endl;
                cout << CLR_BYL << "    Pending: " << cntPending << CLR_RS
                     << CLR_GY << "  |  " << CLR_RS
                     << CLR_BYL << "Approved: " << cntApproved << CLR_RS
                     << CLR_GY << "  |  " << CLR_RS
                     << CLR_BYL << "Deposit: " << cntDeposit << CLR_RS
                     << CLR_GY << "  |  " << CLR_RS
                     << CLR_BGR << "Done: " << cntCompleted << CLR_RS
                     << CLR_GY << "  |  " << CLR_RS
                     << CLR_BRD << "Rejected: " << cntRejected << CLR_RS << endl;
                showDivider();

                waitKey();
            }

        // --- Dispute Management ---
        } else if (c == 2) {
            vector<string> dmopts = {"View Pending Disputes", "View All Disputes", "View Resolved", "View Dismissed", "Back"};
            while (true) {
                int dmc = showMenu("DISPUTE MANAGEMENT", dmopts, "Review and resolve customer reports");
                if (dmc == 4 || dmc == -1) break;

                string filterClause = "";
                string headerTitle = "ALL DISPUTE REPORTS";
                if (dmc == 0) { filterClause = " WHERE r.admin_action = 'Pending'"; headerTitle = "PENDING DISPUTES"; }
                else if (dmc == 2) { filterClause = " WHERE r.admin_action = 'Resolved'"; headerTitle = "RESOLVED DISPUTES"; }
                else if (dmc == 3) { filterClause = " WHERE r.admin_action = 'Dismissed'"; headerTitle = "DISMISSED DISPUTES"; }

                showScreenHeader(headerTitle);

                string q = "SELECT r.report_id, u.name AS reporter, r.booking_id, "
                           "r.admin_action, LEFT(r.description, 30) AS short_desc "
                           "FROM REPORTS r JOIN USERS u ON r.user_id = u.user_id" + filterClause +
                           " ORDER BY r.report_id ASC";

                if (mysql_query(conn, q.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                MYSQL_RES* res = mysql_store_result(conn);
                if (!res || mysql_num_rows(res) == 0) {
                    showMsg("No disputes found.", "info");
                    if (res) mysql_free_result(res); waitKey(); continue;
                }

                // Column widths: ID(4) | Reporter(16) | Booking(8) | Action(10) | Description(30)
                int cID=4, cReporter=16, cBooking=8, cAction=10, cDesc=30;

                // Table top border
                cout << CLR_CY << "  \xC9";
                for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cReporter+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cBooking+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cAction+2;i++) cout<<"\xCD"; cout<<"\xCB";
                for(int i=0;i<cDesc+2;i++) cout<<"\xCD";
                cout << "\xBB" << CLR_RS << endl;

                // Header row
                cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("ID", cID+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Reporter", cReporter+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Booking", cBooking+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Action", cAction+1)
                     << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                     << " " << padStr("Description", cDesc+1)
                     << CLR_CY << "\xBA" << CLR_RS << endl;

                // Header-body separator
                cout << CLR_CY << "  \xCC";
                for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cReporter+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cBooking+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cAction+2;i++) cout<<"\xCD"; cout<<"\xCE";
                for(int i=0;i<cDesc+2;i++) cout<<"\xCD";
                cout << "\xB9" << CLR_RS << endl;

                // Data rows
                MYSQL_ROW row;
                int totalReports = 0, cntPending=0, cntResolved=0, cntDismissed=0;
                while ((row = mysql_fetch_row(res))) {
                    totalReports++;
                    string sid = row[0] ? row[0] : "";
                    string sreporter = row[1] ? row[1] : "";
                    string sbooking = row[2] ? row[2] : "";
                    string saction = row[3] ? row[3] : "";
                    string sdesc = row[4] ? row[4] : "";

                    if (saction == "Pending") cntPending++;
                    else if (saction == "Resolved") cntResolved++;
                    else if (saction == "Dismissed") cntDismissed++;

                    // Color-code action status
                    string actionColor = CLR_WH;
                    if (saction == "Pending") actionColor = CLR_BYL;
                    else if (saction == "Resolved") actionColor = CLR_BGR;
                    else if (saction == "Dismissed") actionColor = CLR_BRD;

                    cout << CLR_CY << "  \xBA" << CLR_RS
                         << CLR_WH << " " << padStr(sid, cID+1)
                         << CLR_CY << "\xBA" << CLR_RS
                         << CLR_WH << " " << padStr(sreporter, cReporter+1)
                         << CLR_CY << "\xBA" << CLR_RS
                         << CLR_WH << " " << padStr(sbooking, cBooking+1)
                         << CLR_CY << "\xBA" << CLR_RS
                         << actionColor << " " << padStr(saction, cAction+1) << CLR_RS
                         << CLR_CY << "\xBA" << CLR_RS
                         << CLR_GY << " " << padStr(sdesc, cDesc+1)
                         << CLR_CY << "\xBA" << CLR_RS << endl;
                }

                // Table bottom border
                cout << CLR_CY << "  \xC8";
                for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cReporter+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cBooking+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cAction+2;i++) cout<<"\xCD"; cout<<"\xCA";
                for(int i=0;i<cDesc+2;i++) cout<<"\xCD";
                cout << "\xBC" << CLR_RS << endl;

                mysql_free_result(res);

                // Summary stats
                cout << endl;
                showDivider();
                cout << CLR_BD << CLR_BWH << "    SUMMARY" << CLR_RS << endl;
                showDivider();
                showField("Total Reports", to_string(totalReports));
                cout << CLR_BYL << "    Pending: " << cntPending << CLR_RS
                     << CLR_GY << "  |  " << CLR_RS
                     << CLR_BGR << "Resolved: " << cntResolved << CLR_RS
                     << CLR_GY << "  |  " << CLR_RS
                     << CLR_BRD << "Dismissed: " << cntDismissed << CLR_RS << endl;
                showDivider();
                cout << endl;

                // Detail view & action
                string rid = getInput("Enter Report ID to view details (0 to go back): ");
                if (rid == "0" || rid.empty()) { continue; }

                // Fetch full report details
                string dq = "SELECT r.report_id, u.name, u.email, r.booking_id, "
                            "p.package_name, b.booking_date, r.description, r.admin_action "
                            "FROM REPORTS r "
                            "JOIN USERS u ON r.user_id = u.user_id "
                            "JOIN BOOKINGS b ON r.booking_id = b.booking_id "
                            "JOIN PACKAGES p ON b.package_id = p.package_id "
                            "WHERE r.report_id = " + rid;
                if (mysql_query(conn, dq.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                MYSQL_RES* dres = mysql_store_result(conn);
                if (!dres || mysql_num_rows(dres) == 0) {
                    showMsg("Report not found.", "err");
                    if (dres) mysql_free_result(dres); waitKey(); continue;
                }

                MYSQL_ROW dr = mysql_fetch_row(dres);
                cout << endl;
                showDivider();
                cout << CLR_BD << CLR_BCY << "    REPORT DETAILS  #" << rid << CLR_RS << endl;
                showDivider();
                showField("Report ID", dr[0] ? dr[0] : "");
                showField("Reporter", dr[1] ? dr[1] : "");
                showField("Reporter Email", dr[2] ? dr[2] : "");
                showField("Booking ID", dr[3] ? dr[3] : "");
                showField("Package", dr[4] ? dr[4] : "");
                showField("Booking Date", dr[5] ? dr[5] : "");
                showField("Current Action", dr[7] ? dr[7] : "");
                showDivider();
                cout << CLR_BYL << "    Description:" << CLR_RS << endl;
                cout << CLR_WH << "    " << (dr[6] ? dr[6] : "") << CLR_RS << endl;
                showDivider();

                string currentAction = dr[7] ? dr[7] : "";
                mysql_free_result(dres);

                if (currentAction != "Pending") {
                    showMsg("This report has already been " + currentAction + ".", "info");
                    waitKey(); continue;
                }

                // Action selection
                cout << endl;
                vector<string> actopts = {"Resolve", "Dismiss", "Cancel"};
                cout << CLR_BWH << "    Choose action:" << CLR_RS << endl;
                cout << CLR_BGR << "      [1] Resolve" << CLR_RS << "  -  Issue acknowledged, action taken" << endl;
                cout << CLR_BRD << "      [2] Dismiss" << CLR_RS << "  -  Report invalid or no action needed" << endl;
                cout << CLR_GY << "      [3] Cancel" << CLR_RS << "   -  Go back without action" << endl;
                cout << endl;
                string choice = getInput("Enter choice (1/2/3): ");

                if (choice == "1" || choice == "2") {
                    string newAction = (choice == "1") ? "Resolved" : "Dismissed";
                    string uq = "UPDATE REPORTS SET admin_action = '" + newAction + "' WHERE report_id = " + rid;
                    if (mysql_query(conn, uq.c_str())) showMsg(string("Error: ") + mysql_error(conn), "err");
                    else showMsg("Report #" + rid + " has been marked as '" + newAction + "'.", "ok");
                }
                waitKey();
            }

        // --- Payment Management ---
        } else if (c == 3) {
            vector<string> pmopts = {"View All Payments", "Delete Payment", "Back"};
            while (true) {
                int pmc = showMenu("PAYMENT MANAGEMENT", pmopts, "Manage and review payment records");
                if (pmc == 2 || pmc == -1) break;

                if (pmc == 0) {
                    showScreenHeader("ALL PAYMENT RECORDS");
                    string q = "SELECT pay.payment_id, c.name AS customer, p.package_name, "
                               "pay.payment_type, pay.payment_method, pay.payment_date, pay.amount "
                               "FROM PAYMENTS pay "
                               "JOIN BOOKINGS b ON pay.booking_id = b.booking_id "
                               "JOIN USERS c ON b.user_id = c.user_id "
                               "JOIN PACKAGES p ON b.package_id = p.package_id "
                               "ORDER BY pay.payment_date DESC";
                    if (mysql_query(conn, q.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                    MYSQL_RES* res = mysql_store_result(conn);
                    if (!res || mysql_num_rows(res) == 0) {
                        showMsg("No payment records found.", "info");
                        if (res) mysql_free_result(res); waitKey(); continue;
                    }
                    int cPI=4, cCu=14, cPk=16, cTy=14, cMe=14, cDt=12, cAm=10;
                    cout << CLR_CY << "  \xC9";
                    for(int i=0;i<cPI+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cTy+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cMe+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cAm+2;i++) cout<<"\xCD";
                    cout << "\xBB" << CLR_RS << endl;
                    cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("ID",cPI+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Customer",cCu+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Package",cPk+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Type",cTy+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Method",cMe+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Date",cDt+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Amt(RM)",cAm+1) << CLR_CY << "\xBA" << CLR_RS << endl;
                    cout << CLR_CY << "  \xCC";
                    for(int i=0;i<cPI+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cTy+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cMe+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cAm+2;i++) cout<<"\xCD";
                    cout << "\xB9" << CLR_RS << endl;
                    MYSQL_ROW row; double totalAmt = 0; int totalPay = 0;
                    while ((row = mysql_fetch_row(res))) {
                        totalPay++;
                        string amt = row[6]?row[6]:"0";
                        try { totalAmt += stod(amt); } catch(...) {}
                        cout << CLR_CY << "  \xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[0]?row[0]:"",cPI+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[1]?row[1]:"",cCu+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[2]?row[2]:"",cPk+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_BYL << " " << padStr(row[3]?row[3]:"",cTy+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[4]?row[4]:"",cMe+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[5]?row[5]:"",cDt+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_BGR << " " << padStr(amt,cAm+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS << endl;
                    }
                    cout << CLR_CY << "  \xC8";
                    for(int i=0;i<cPI+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cTy+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cMe+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cAm+2;i++) cout<<"\xCD";
                    cout << "\xBC" << CLR_RS << endl;
                    mysql_free_result(res);
                    cout << endl;
                    showDivider();
                    ostringstream oss; oss << fixed << setprecision(2) << totalAmt;
                    showField("Total Payments", to_string(totalPay));
                    showField("Total Amount", string("RM ") + oss.str());
                    showDivider();
                    waitKey();

                } else if (pmc == 1) {
                    showScreenHeader("DELETE PAYMENT");
                    string q = "SELECT pay.payment_id, c.name, pay.payment_type, pay.amount, pay.payment_date "
                               "FROM PAYMENTS pay JOIN BOOKINGS b ON pay.booking_id = b.booking_id "
                               "JOIN USERS c ON b.user_id = c.user_id ORDER BY pay.payment_id";
                    if (mysql_query(conn, q.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                    MYSQL_RES* res = mysql_store_result(conn);
                    if (!res || mysql_num_rows(res) == 0) {
                        showMsg("No payments found.", "info");
                        if (res) mysql_free_result(res); waitKey(); continue;
                    }
                    int cPI=4, cCu=16, cTy=14, cAm=10, cDt=12;
                    cout << CLR_CY << "  \xC9";
                    for(int i=0;i<cPI+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cTy+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cAm+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD";
                    cout << "\xBB" << CLR_RS << endl;
                    cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("ID",cPI+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Customer",cCu+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Type",cTy+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Amt(RM)",cAm+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Date",cDt+1) << CLR_CY << "\xBA" << CLR_RS << endl;
                    cout << CLR_CY << "  \xCC";
                    for(int i=0;i<cPI+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cTy+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cAm+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD";
                    cout << "\xB9" << CLR_RS << endl;
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        cout << CLR_CY << "  \xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[0]?row[0]:"",cPI+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[1]?row[1]:"",cCu+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[2]?row[2]:"",cTy+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_BGR << " " << padStr(row[3]?row[3]:"",cAm+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[4]?row[4]:"",cDt+1)
                             << CLR_CY << "\xBA" << CLR_RS << endl;
                    }
                    cout << CLR_CY << "  \xC8";
                    for(int i=0;i<cPI+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cTy+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cAm+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cDt+2;i++) cout<<"\xCD";
                    cout << "\xBC" << CLR_RS << endl;
                    mysql_free_result(res);
                    cout << endl;
                    string pid = getInput("Enter Payment ID to delete (0 to cancel): ");
                    if (pid != "0" && !pid.empty()) {
                        string confirm = getInput("Type 'yes' to confirm delete payment #" + pid + ": ");
                        if (confirm == "yes") {
                            string dq = "DELETE FROM PAYMENTS WHERE payment_id = " + pid;
                            if (mysql_query(conn, dq.c_str())) showMsg(string("Error: ") + mysql_error(conn), "err");
                            else showMsg("Payment #" + pid + " deleted successfully.", "ok");
                        } else showMsg("Delete cancelled.", "info");
                    }
                    waitKey();
                }
            }

        // --- Review Management ---
        } else if (c == 4) {
            vector<string> rmopts = {"View All Reviews", "Delete Review", "Back"};
            while (true) {
                int rmc = showMenu("REVIEW MANAGEMENT", rmopts, "Moderate customer reviews");
                if (rmc == 2 || rmc == -1) break;

                if (rmc == 0) {
                    showScreenHeader("ALL CUSTOMER REVIEWS");
                    string q = "SELECT rv.review_id, c.name AS customer, p.package_name, "
                               "rv.rating, rv.comment "
                               "FROM REVIEWS rv "
                               "JOIN BOOKINGS b ON rv.booking_id = b.booking_id "
                               "JOIN USERS c ON b.user_id = c.user_id "
                               "JOIN PACKAGES p ON b.package_id = p.package_id "
                               "ORDER BY rv.review_id DESC";
                    if (mysql_query(conn, q.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                    MYSQL_RES* res = mysql_store_result(conn);
                    if (!res || mysql_num_rows(res) == 0) {
                        showMsg("No reviews found.", "info");
                        if (res) mysql_free_result(res); waitKey(); continue;
                    }
                    int cRI=4, cCu=16, cPk=18, cRt=6, cCm=30;
                    cout << CLR_CY << "  \xC9";
                    for(int i=0;i<cRI+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cRt+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cCm+2;i++) cout<<"\xCD";
                    cout << "\xBB" << CLR_RS << endl;
                    cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("ID",cRI+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Customer",cCu+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Package",cPk+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Rating",cRt+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Comment",cCm+1) << CLR_CY << "\xBA" << CLR_RS << endl;
                    cout << CLR_CY << "  \xCC";
                    for(int i=0;i<cRI+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cRt+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cCm+2;i++) cout<<"\xCD";
                    cout << "\xB9" << CLR_RS << endl;
                    MYSQL_ROW row; int totalReviews = 0; double totalRating = 0;
                    while ((row = mysql_fetch_row(res))) {
                        totalReviews++;
                        string rating = row[3]?row[3]:"0";
                        try { totalRating += stod(rating); } catch(...) {}
                        string stars = "";
                        int r = 0; try { r = stoi(rating); } catch(...) {}
                        for (int i=0;i<r;i++) stars += "\xDB";
                        for (int i=r;i<5;i++) stars += "\xB0";
                        string comment = row[4]?row[4]:"";
                        if ((int)comment.length() > cCm) comment = comment.substr(0, cCm-3) + "...";
                        string ratingColor = (r >= 4) ? CLR_BGR : ((r >= 3) ? CLR_BYL : CLR_BRD);
                        cout << CLR_CY << "  \xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[0]?row[0]:"",cRI+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[1]?row[1]:"",cCu+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[2]?row[2]:"",cPk+1) << CLR_CY << "\xBA" << CLR_RS
                             << ratingColor << " " << padStr(stars,cRt+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_GY << " " << padStr(comment,cCm+1)
                             << CLR_CY << "\xBA" << CLR_RS << endl;
                    }
                    cout << CLR_CY << "  \xC8";
                    for(int i=0;i<cRI+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cRt+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cCm+2;i++) cout<<"\xCD";
                    cout << "\xBC" << CLR_RS << endl;
                    mysql_free_result(res);
                    cout << endl;
                    showDivider();
                    showField("Total Reviews", to_string(totalReviews));
                    if (totalReviews > 0) {
                        ostringstream oss; oss << fixed << setprecision(1) << (totalRating / totalReviews);
                        showField("Average Rating", oss.str() + " / 5.0");
                    }
                    showDivider();
                    waitKey();

                } else if (rmc == 1) {
                    showScreenHeader("DELETE REVIEW");
                    string q = "SELECT rv.review_id, c.name, p.package_name, rv.rating, LEFT(rv.comment, 40) "
                               "FROM REVIEWS rv JOIN BOOKINGS b ON rv.booking_id = b.booking_id "
                               "JOIN USERS c ON b.user_id = c.user_id "
                               "JOIN PACKAGES p ON b.package_id = p.package_id ORDER BY rv.review_id";
                    if (mysql_query(conn, q.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                    MYSQL_RES* res = mysql_store_result(conn);
                    if (!res || mysql_num_rows(res) == 0) {
                        showMsg("No reviews found.", "info");
                        if (res) mysql_free_result(res); waitKey(); continue;
                    }
                    int cRI=4, cCu=16, cPk=18, cRt=6, cCm=30;
                    cout << CLR_CY << "  \xC9";
                    for(int i=0;i<cRI+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cRt+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cCm+2;i++) cout<<"\xCD";
                    cout << "\xBB" << CLR_RS << endl;
                    cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("ID",cRI+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Customer",cCu+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Package",cPk+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Rating",cRt+1) << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Comment",cCm+1) << CLR_CY << "\xBA" << CLR_RS << endl;
                    cout << CLR_CY << "  \xCC";
                    for(int i=0;i<cRI+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cRt+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cCm+2;i++) cout<<"\xCD";
                    cout << "\xB9" << CLR_RS << endl;
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        cout << CLR_CY << "  \xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[0]?row[0]:"",cRI+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[1]?row[1]:"",cCu+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[2]?row[2]:"",cPk+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(row[3]?row[3]:"",cRt+1) << CLR_CY << "\xBA" << CLR_RS
                             << CLR_GY << " " << padStr(row[4]?row[4]:"",cCm+1)
                             << CLR_CY << "\xBA" << CLR_RS << endl;
                    }
                    cout << CLR_CY << "  \xC8";
                    for(int i=0;i<cRI+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cCu+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cPk+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cRt+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cCm+2;i++) cout<<"\xCD";
                    cout << "\xBC" << CLR_RS << endl;
                    mysql_free_result(res);
                    cout << endl;
                    string rid = getInput("Enter Review ID to delete (0 to cancel): ");
                    if (rid != "0" && !rid.empty()) {
                        string confirm = getInput("Type 'yes' to confirm delete review #" + rid + ": ");
                        if (confirm == "yes") {
                            string dq = "DELETE FROM REVIEWS WHERE review_id = " + rid;
                            if (mysql_query(conn, dq.c_str())) showMsg(string("Error: ") + mysql_error(conn), "err");
                            else showMsg("Review #" + rid + " deleted successfully.", "ok");
                        } else showMsg("Delete cancelled.", "info");
                    }
                    waitKey();
                }
            }

        // --- Business Reports ---
        } else if (c == 5) {
            vector<string> bropts = {"Revenue Report", "Activity Report", "Photographer Performance", "Export Revenue to PDF", "Export Activity to PDF", "Back"};
            while (true) {
                int brc = showMenu("BUSINESS REPORTS", bropts, "Generate and export business analytics");
                if (brc == 5 || brc == -1) break;

                // ===== Revenue Report =====
                if (brc == 0 || brc == 3) {
                    string rq = "SELECT u.name AS photographer, p.package_name, "
                                "pay.payment_type, pay.payment_method, pay.payment_date, pay.amount "
                                "FROM PAYMENTS pay "
                                "JOIN BOOKINGS b ON pay.booking_id = b.booking_id "
                                "JOIN PACKAGES p ON b.package_id = p.package_id "
                                "JOIN USERS u ON p.user_id = u.user_id "
                                "ORDER BY pay.payment_date DESC";
                    if (mysql_query(conn, rq.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                    MYSQL_RES* res = mysql_store_result(conn);
                    if (!res || mysql_num_rows(res) == 0) {
                        showMsg("No payment records found.", "info");
                        if (res) mysql_free_result(res); waitKey(); continue;
                    }

                    // Collect data
                    vector<vector<string>> pdfRows;
                    double totalRev = 0;
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        vector<string> r;
                        for (int i = 0; i < 6; i++) r.push_back(row[i] ? row[i] : "");
                        try { totalRev += stod(r[5]); } catch(...) {}
                        pdfRows.push_back(r);
                    }
                    mysql_free_result(res);

                    if (brc == 0) {
                        // Display table on screen
                        showScreenHeader("REVENUE REPORT");
                        int cPh=16, cPkg=18, cType=14, cMethod=14, cDate=12, cAmt=10;

                        cout << CLR_CY << "  \xC9";
                        for(int i=0;i<cPh+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cType+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cMethod+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cAmt+2;i++) cout<<"\xCD";
                        cout << "\xBB" << CLR_RS << endl;

                        cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Photographer", cPh+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Package", cPkg+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Type", cType+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Method", cMethod+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Date", cDate+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Amount(RM)", cAmt+1)
                             << CLR_CY << "\xBA" << CLR_RS << endl;

                        cout << CLR_CY << "  \xCC";
                        for(int i=0;i<cPh+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cType+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cMethod+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cAmt+2;i++) cout<<"\xCD";
                        cout << "\xB9" << CLR_RS << endl;

                        for (auto& r : pdfRows) {
                            string amtColor = CLR_BGR;
                            cout << CLR_CY << "  \xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[0], cPh+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[1], cPkg+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_BYL << " " << padStr(r[2], cType+1) << CLR_RS
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[3], cMethod+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[4], cDate+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << amtColor << " " << padStr(r[5], cAmt+1) << CLR_RS
                                 << CLR_CY << "\xBA" << CLR_RS << endl;
                        }

                        cout << CLR_CY << "  \xC8";
                        for(int i=0;i<cPh+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cType+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cMethod+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cAmt+2;i++) cout<<"\xCD";
                        cout << "\xBC" << CLR_RS << endl;

                        cout << endl;
                        showDivider();
                        ostringstream oss; oss << fixed << setprecision(2) << totalRev;
                        double adminCut = totalRev * 0.05;
                        ostringstream ossAdmin; ossAdmin << fixed << setprecision(2) << adminCut;
                        showField("Total Revenue", string("RM ") + oss.str());
                        showField("Admin Commission (5%)", string("RM ") + ossAdmin.str());
                        showField("Transactions", to_string(pdfRows.size()));
                        showDivider();
                        waitKey();
                    } else {
                        // Export to PDF
                        CreateDirectoryA("Revenue Report", NULL);
                        string fname = "Revenue Report\\Revenue_Report_" + getDateStamp() + ".pdf";
                        vector<string> hdrs = {"Photographer", "Package", "Type", "Method", "Date", "Amount(RM)"};
                        ostringstream oss; oss << fixed << setprecision(2) << totalRev;
                        double adminCutPdf = totalRev * 0.05;
                        ostringstream ossAdminPdf; ossAdminPdf << fixed << setprecision(2) << adminCutPdf;
                        vector<string> summary = {
                            "Total Revenue: RM " + oss.str(),
                            "Admin Commission (5%): RM " + ossAdminPdf.str(),
                            "Total Transactions: " + to_string(pdfRows.size()),
                            "Report Date: " + getTimestamp()
                        };
                        if (exportPDF(fname, "Revenue Report - Photography Booking System", hdrs, pdfRows, summary))
                            showMsg("PDF exported: " + fname, "ok");
                        else
                            showMsg("Failed to export PDF.", "err");
                        waitKey();
                    }

                // ===== Activity Report =====
                } else if (brc == 1 || brc == 4) {
                    string aq = "SELECT b.booking_id, c.name AS customer, u2.name AS photographer, "
                                "p.package_name, b.booking_date, b.job_status "
                                "FROM BOOKINGS b "
                                "JOIN USERS c ON b.user_id = c.user_id "
                                "JOIN PACKAGES p ON b.package_id = p.package_id "
                                "JOIN USERS u2 ON p.user_id = u2.user_id "
                                "ORDER BY b.booking_date DESC";
                    if (mysql_query(conn, aq.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                    MYSQL_RES* res = mysql_store_result(conn);
                    if (!res || mysql_num_rows(res) == 0) {
                        showMsg("No booking activity found.", "info");
                        if (res) mysql_free_result(res); waitKey(); continue;
                    }

                    vector<vector<string>> pdfRows;
                    int cntP=0, cntA=0, cntD=0, cntC=0, cntR=0;
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        vector<string> r;
                        for (int i = 0; i < 6; i++) r.push_back(row[i] ? row[i] : "");
                        string st = r[5];
                        if (st == "Pending") cntP++;
                        else if (st == "Approved") cntA++;
                        else if (st == "Deposit Paid") cntD++;
                        else if (st == "Completed") cntC++;
                        else if (st == "Rejected") cntR++;
                        pdfRows.push_back(r);
                    }
                    mysql_free_result(res);

                    if (brc == 1) {
                        showScreenHeader("ACTIVITY REPORT");
                        int cID=4, cCust=14, cPhot=14, cPkg=16, cDate=12, cStat=13;

                        cout << CLR_CY << "  \xC9";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cCust+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cPhot+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCB";
                        for(int i=0;i<cStat+2;i++) cout<<"\xCD";
                        cout << "\xBB" << CLR_RS << endl;

                        cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("ID", cID+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Customer", cCust+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Photographer", cPhot+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Package", cPkg+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Date", cDate+1)
                             << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                             << " " << padStr("Status", cStat+1)
                             << CLR_CY << "\xBA" << CLR_RS << endl;

                        cout << CLR_CY << "  \xCC";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cCust+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cPhot+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCE";
                        for(int i=0;i<cStat+2;i++) cout<<"\xCD";
                        cout << "\xB9" << CLR_RS << endl;

                        for (auto& r : pdfRows) {
                            string sc = CLR_WH;
                            if (r[5] == "Completed") sc = CLR_BGR;
                            else if (r[5] == "Pending" || r[5] == "Approved" || r[5] == "Deposit Paid") sc = CLR_BYL;
                            else if (r[5] == "Rejected") sc = CLR_BRD;

                            cout << CLR_CY << "  \xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[0], cID+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[1], cCust+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[2], cPhot+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[3], cPkg+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << CLR_WH << " " << padStr(r[4], cDate+1)
                                 << CLR_CY << "\xBA" << CLR_RS
                                 << sc << " " << padStr(r[5], cStat+1) << CLR_RS
                                 << CLR_CY << "\xBA" << CLR_RS << endl;
                        }

                        cout << CLR_CY << "  \xC8";
                        for(int i=0;i<cID+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cCust+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cPhot+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cPkg+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cDate+2;i++) cout<<"\xCD"; cout<<"\xCA";
                        for(int i=0;i<cStat+2;i++) cout<<"\xCD";
                        cout << "\xBC" << CLR_RS << endl;

                        cout << endl;
                        showDivider();
                        cout << CLR_BD << CLR_BWH << "    BOOKING SUMMARY" << CLR_RS << endl;
                        showDivider();
                        showField("Total Bookings", to_string(pdfRows.size()));
                        cout << CLR_BYL << "    Pending: " << cntP << CLR_RS
                             << CLR_GY << " | " << CLR_RS
                             << CLR_BYL << "Approved: " << cntA << CLR_RS
                             << CLR_GY << " | " << CLR_RS
                             << CLR_BYL << "Deposit: " << cntD << CLR_RS
                             << CLR_GY << " | " << CLR_RS
                             << CLR_BGR << "Done: " << cntC << CLR_RS
                             << CLR_GY << " | " << CLR_RS
                             << CLR_BRD << "Rejected: " << cntR << CLR_RS << endl;
                        showDivider();
                        waitKey();
                    } else {
                        CreateDirectoryA("Activity Report", NULL);
                        string fname = "Activity Report\\Activity_Report_" + getDateStamp() + ".pdf";
                        vector<string> hdrs = {"ID", "Customer", "Photographer", "Package", "Date", "Status"};
                        vector<string> summary = {
                            "Total Bookings: " + to_string(pdfRows.size()),
                            "Pending: " + to_string(cntP) + "  Approved: " + to_string(cntA) + "  Deposit Paid: " + to_string(cntD),
                            "Completed: " + to_string(cntC) + "  Rejected: " + to_string(cntR),
                            "Report Date: " + getTimestamp()
                        };
                        if (exportPDF(fname, "Activity Report - Photography Booking System", hdrs, pdfRows, summary))
                            showMsg("PDF exported: " + fname, "ok");
                        else
                            showMsg("Failed to export PDF.", "err");
                        waitKey();
                    }

                // ===== Photographer Performance =====
                } else if (brc == 2) {
                    showScreenHeader("PHOTOGRAPHER PERFORMANCE");
                    string pq = "SELECT u.name, COUNT(b.booking_id) AS total_jobs, "
                                "COALESCE(SUM(pay.amount), 0) AS total_earned "
                                "FROM USERS u "
                                "LEFT JOIN PACKAGES p ON u.user_id = p.user_id "
                                "LEFT JOIN BOOKINGS b ON p.package_id = b.package_id AND b.job_status = 'Completed' "
                                "LEFT JOIN PAYMENTS pay ON b.booking_id = pay.booking_id "
                                "WHERE u.role = 'Photographer' AND u.account_status = 'Active' "
                                "GROUP BY u.user_id, u.name ORDER BY total_earned DESC";
                    if (mysql_query(conn, pq.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
                    MYSQL_RES* res = mysql_store_result(conn);
                    if (!res || mysql_num_rows(res) == 0) {
                        showMsg("No photographer data found.", "info");
                        if (res) mysql_free_result(res); waitKey(); continue;
                    }

                    int cRank=5, cName=20, cJobs=10, cEarned=14;

                    cout << CLR_CY << "  \xC9";
                    for(int i=0;i<cRank+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cJobs+2;i++) cout<<"\xCD"; cout<<"\xCB";
                    for(int i=0;i<cEarned+2;i++) cout<<"\xCD";
                    cout << "\xBB" << CLR_RS << endl;

                    cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Rank", cRank+1)
                         << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Photographer", cName+1)
                         << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Jobs Done", cJobs+1)
                         << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH
                         << " " << padStr("Earned (RM)", cEarned+1)
                         << CLR_CY << "\xBA" << CLR_RS << endl;

                    cout << CLR_CY << "  \xCC";
                    for(int i=0;i<cRank+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cJobs+2;i++) cout<<"\xCD"; cout<<"\xCE";
                    for(int i=0;i<cEarned+2;i++) cout<<"\xCD";
                    cout << "\xB9" << CLR_RS << endl;

                    MYSQL_ROW row; int rank = 1;
                    while ((row = mysql_fetch_row(res))) {
                        string srank = "#" + to_string(rank++);
                        string sname = row[0] ? row[0] : "";
                        string sjobs = row[1] ? row[1] : "0";
                        string searned = row[2] ? row[2] : "0.00";

                        string rankColor = (rank <= 2) ? CLR_BYL : ((rank <= 3) ? CLR_WH : CLR_GY);

                        cout << CLR_CY << "  \xBA" << CLR_RS
                             << rankColor << " " << padStr(srank, cRank+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_WH << " " << padStr(sname, cName+1)
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_BCY << " " << padStr(sjobs, cJobs+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS
                             << CLR_BGR << " " << padStr(searned, cEarned+1) << CLR_RS
                             << CLR_CY << "\xBA" << CLR_RS << endl;
                    }

                    cout << CLR_CY << "  \xC8";
                    for(int i=0;i<cRank+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cName+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cJobs+2;i++) cout<<"\xCD"; cout<<"\xCA";
                    for(int i=0;i<cEarned+2;i++) cout<<"\xCD";
                    cout << "\xBC" << CLR_RS << endl;

                    mysql_free_result(res);
                    waitKey();
                }
            }
        }
    }
}

// ==================== Customer Dashboard ====================
void customerDashboard(MYSQL* conn, int userId) {
    vector<string> opts = {"Browse Available Packages", "Make a Booking", "Make Payment", "Leave a Review", "Back / Logout"};
    while (true) {
        int c = showMenu("CUSTOMER DASHBOARD  (ID: " + to_string(userId) + ")", opts);
        if (c == 4 || c == -1) break;

        // --- Browse Packages ---
        if (c == 0) {
            showScreenHeader("AVAILABLE PACKAGES");
            string q = "SELECT p.package_id, u.name AS photographer_name, p.package_name, "
                       "p.category, p.price, p.details FROM PACKAGES p JOIN USERS u ON p.user_id = u.user_id";
            if (mysql_query(conn, q.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); }
            else {
                MYSQL_RES* res = mysql_store_result(conn);
                if (res) {
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(res))) {
                        showDivider();
                        showField("Package ID", row[0]);
                        showField("Photographer", row[1]);
                        showField("Package Name", row[2]);
                        showField("Category", row[3]);
                        showField("Price", string("RM ") + row[4]);
                        showField("Details", row[5]);
                    }
                    showDivider();
                    mysql_free_result(res);
                }
            }
            waitKey();

        // --- Make Booking ---
        } else if (c == 1) {
            showScreenHeader("MAKE A BOOKING");
            string pkg_id = getInput("Enter Package ID to book: ");
            string bdate = getInput("Enter Booking Date (YYYY-MM-DD): ");
            if (pkg_id.empty() || bdate.empty()) { showMsg("Cancelled.", "info"); waitKey(); continue; }

            string fq = "SELECT user_id FROM PACKAGES WHERE package_id = " + pkg_id;
            if (mysql_query(conn, fq.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
            MYSQL_RES* res = mysql_store_result(conn);
            if (!res || mysql_num_rows(res) == 0) {
                showMsg("Package not found!", "err");
                if (res) mysql_free_result(res); waitKey(); continue;
            }
            MYSQL_ROW pkgRow = mysql_fetch_row(res);
            int photographer_id = stoi(pkgRow[0]);
            mysql_free_result(res);

            string cq = "SELECT status FROM SCHEDULES WHERE user_id = " + to_string(photographer_id) +
                        " AND target_date = '" + bdate + "' AND status = 'Blocked'";
            if (mysql_query(conn, cq.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
            MYSQL_RES* sr = mysql_store_result(conn);
            if (sr && mysql_num_rows(sr) > 0) {
                showMsg("Photographer not available on " + bdate + ".", "err");
            } else {
                string iq = "INSERT INTO BOOKINGS (user_id, package_id, booking_date, job_status) VALUES (" +
                            to_string(userId) + ", " + pkg_id + ", '" + bdate + "', 'Pending')";
                if (mysql_query(conn, iq.c_str())) showMsg(string("Booking Failed: ") + mysql_error(conn), "err");
                else showMsg("Booking placed successfully! Status: Pending.", "ok");
            }
            if (sr) mysql_free_result(sr);
            waitKey();

        // --- Make Payment ---
        } else if (c == 2) {
            showScreenHeader("MAKE PAYMENT");
            string q = "SELECT b.booking_id, p.package_name, b.booking_date, b.job_status "
                       "FROM BOOKINGS b JOIN PACKAGES p ON b.package_id = p.package_id "
                       "WHERE b.user_id = " + to_string(userId) + " AND b.job_status = 'Approved'";
            if (mysql_query(conn, q.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
            MYSQL_RES* res = mysql_store_result(conn);
            if (!res || mysql_num_rows(res) == 0) {
                showMsg("No approved bookings for payment.", "info");
                if (res) mysql_free_result(res); waitKey(); continue;
            }
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                showDivider();
                showField("Booking ID", row[0]);
                showField("Package", row[1]);
                showField("Booking Date", row[2]);
                showField("Status", row[3]);
            }
            showDivider();
            mysql_free_result(res);
            cout << endl;
            string bid = getInput("Enter Booking ID to pay: ");
            string amt = getInput("Enter Deposit Amount (RM): ");
            string pm = getInput("Enter Payment Method: ");
            string pd = getInput("Enter Payment Date (YYYY-MM-DD): ");

            string iq = "INSERT INTO PAYMENTS (booking_id, amount, payment_type, payment_method, payment_date) "
                        "VALUES (" + bid + ", " + amt + ", 'Deposit', '" + pm + "', '" + pd + "')";
            if (mysql_query(conn, iq.c_str())) { showMsg(string("Payment Failed: ") + mysql_error(conn), "err"); }
            else {
                string uq = "UPDATE BOOKINGS SET job_status = 'Deposit Paid' WHERE booking_id = " + bid;
                if (mysql_query(conn, uq.c_str())) showMsg(string("Error: ") + mysql_error(conn), "err");
                else showMsg("Payment recorded! Status updated to 'Deposit Paid'.", "ok");
            }
            waitKey();

        // --- Leave Review ---
        } else if (c == 3) {
            showScreenHeader("LEAVE A REVIEW");
            string q = "SELECT b.booking_id, p.package_name, b.booking_date "
                       "FROM BOOKINGS b JOIN PACKAGES p ON b.package_id = p.package_id "
                       "WHERE b.user_id = " + to_string(userId) + " AND b.job_status = 'Completed'";
            if (mysql_query(conn, q.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
            MYSQL_RES* res = mysql_store_result(conn);
            if (!res || mysql_num_rows(res) == 0) {
                showMsg("No completed bookings for review.", "info");
                if (res) mysql_free_result(res); waitKey(); continue;
            }
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                showDivider();
                showField("Booking ID", row[0]);
                showField("Package", row[1]);
                showField("Booking Date", row[2]);
            }
            showDivider();
            mysql_free_result(res);
            cout << endl;
            string bid = getInput("Enter Booking ID to review: ");
            string rating = getInput("Enter Rating (1-5): ");
            int r = 0;
            try { r = stoi(rating); } catch (...) {}
            if (r < 1 || r > 5) { showMsg("Invalid rating (1-5).", "err"); waitKey(); continue; }
            string comment = getInput("Enter Comment: ");
            string iq = "INSERT INTO REVIEWS (booking_id, rating, comment) VALUES (" + bid + ", " + rating + ", '" + comment + "')";
            if (mysql_query(conn, iq.c_str())) showMsg(string("Review Failed: ") + mysql_error(conn), "err");
            else showMsg("Review submitted! Thank you!", "ok");
            waitKey();
        }
    }
}

// ==================== Photographer Dashboard ====================
void photographerDashboard(MYSQL* conn, int userId) {
    vector<string> opts = {"Manage Profile/Portfolio", "Manage Packages", "Manage Schedule", "Booking Management", "Back / Logout"};
    while (true) {
        int c = showMenu("PHOTOGRAPHER DASHBOARD  (ID: " + to_string(userId) + ")", opts);
        if (c == 4 || c == -1) break;

        if (c == 0) {
            showScreenHeader("ADD PORTFOLIO ITEM");
            string title = getInput("Enter Title: ");
            string link = getInput("Enter Media Link: ");
            string date = getInput("Enter Upload Date (YYYY-MM-DD): ");
            if (title.empty()) { showMsg("Cancelled.", "info"); waitKey(); continue; }
            string q = "INSERT INTO PORTFOLIOS (user_id, title, media_link, upload_date) VALUES (" +
                       to_string(userId) + ", '" + title + "', '" + link + "', '" + date + "')";
            if (mysql_query(conn, q.c_str())) showMsg(string("Error: ") + mysql_error(conn), "err");
            else showMsg("Portfolio added!", "ok");
            waitKey();

        } else if (c == 1) {
            showScreenHeader("ADD PACKAGE");
            string pname = getInput("Enter Package Name: ");
            string cat = getInput("Enter Category: ");
            string price = getInput("Enter Price (RM): ");
            string details = getInput("Enter Details: ");
            if (pname.empty()) { showMsg("Cancelled.", "info"); waitKey(); continue; }
            string q = "INSERT INTO PACKAGES (user_id, package_name, category, price, details) VALUES (" +
                       to_string(userId) + ", '" + pname + "', '" + cat + "', " + price + ", '" + details + "')";
            if (mysql_query(conn, q.c_str())) showMsg(string("Error: ") + mysql_error(conn), "err");
            else showMsg("Package added!", "ok");
            waitKey();

        } else if (c == 2) {
            showScreenHeader("MANAGE SCHEDULE");
            string tdate = getInput("Enter Target Date (YYYY-MM-DD): ");
            string status = getInput("Enter Status (Available/Blocked): ");
            if (tdate.empty()) { showMsg("Cancelled.", "info"); waitKey(); continue; }
            string q = "INSERT INTO SCHEDULES (user_id, target_date, status) VALUES (" +
                       to_string(userId) + ", '" + tdate + "', '" + status + "')";
            if (mysql_query(conn, q.c_str())) showMsg(string("Error: ") + mysql_error(conn), "err");
            else showMsg("Schedule updated!", "ok");
            waitKey();

        } else if (c == 3) {
            showScreenHeader("PENDING BOOKINGS");
            string q = "SELECT b.booking_id, c.name AS customer_name, p.package_name, b.booking_date, b.job_status "
                       "FROM BOOKINGS b JOIN PACKAGES p ON b.package_id = p.package_id "
                       "JOIN USERS c ON b.user_id = c.user_id "
                       "WHERE p.user_id = " + to_string(userId) + " AND b.job_status = 'Pending'";
            if (mysql_query(conn, q.c_str())) { showMsg(string("Error: ") + mysql_error(conn), "err"); waitKey(); continue; }
            MYSQL_RES* res = mysql_store_result(conn);
            if (!res || mysql_num_rows(res) == 0) {
                showMsg("No pending bookings.", "info");
                if (res) mysql_free_result(res); waitKey(); continue;
            }
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))) {
                showDivider();
                showField("Booking ID", row[0]);
                showField("Customer", row[1]);
                showField("Package", row[2]);
                showField("Date", row[3]);
                showField("Status", row[4]);
            }
            showDivider();
            mysql_free_result(res);
            cout << endl;
            string bid = getInput("Enter Booking ID (0 to cancel): ");
            if (bid != "0" && !bid.empty()) {
                string act = getInput("Enter Action (Approved/Rejected): ");
                string uq = "UPDATE BOOKINGS SET job_status = '" + act + "' WHERE booking_id = " + bid;
                if (mysql_query(conn, uq.c_str())) showMsg(string("Error: ") + mysql_error(conn), "err");
                else showMsg("Booking updated!", "ok");
            }
            waitKey();
        }
    }
}

// ==================== Splash Screen ====================
void showSplash() {
    cls(); hideCur();
    int sx = (getTermW() - BWIDTH) / 2;
    if (sx < 1) sx = 1;
    int sy = 3;

    drawBoxTop(sx, sy, BWIDTH, CLR_CY);
    drawBoxRow(sx, sy + 1, BWIDTH, "");
    string t1 = "P H O T O G R A P H Y";
    string t2 = "B O O K I N G   S Y S T E M";
    int p1 = (BWIDTH - 2 - (int)t1.length()) / 2;
    int p2 = (BWIDTH - 2 - (int)t2.length()) / 2;
    string r1 = "", r2 = "";
    for (int i = 0; i < p1; i++) r1 += " ";
    r1 += CLR_BCY + t1 + CLR_RS;
    for (int i = 0; i < p2; i++) r2 += " ";
    r2 += CLR_BWH + t2 + CLR_RS;

    // Draw with visible padding
    moveTo(sx, sy + 2);
    cout << CLR_CY << "\xBA" << CLR_RS << r1;
    int vl1 = p1 + (int)t1.length();
    for (int i = vl1; i < BWIDTH - 2; i++) cout << " ";
    cout << CLR_CY << "\xBA" << CLR_RS;

    moveTo(sx, sy + 3);
    cout << CLR_CY << "\xBA" << CLR_RS << r2;
    int vl2 = p2 + (int)t2.length();
    for (int i = vl2; i < BWIDTH - 2; i++) cout << " ";
    cout << CLR_CY << "\xBA" << CLR_RS;

    drawBoxRow(sx, sy + 4, BWIDTH, "");
    drawBoxMid(sx, sy + 5, BWIDTH);

    // Progress bar animation
    int barW = BWIDTH - 12;
    for (int p = 0; p <= barW; p++) {
        moveTo(sx, sy + 6);
        cout << CLR_CY << "\xBA" << CLR_RS << "  " << CLR_GY << "Loading ";
        for (int i = 0; i < p; i++) cout << CLR_CY << "\xDB" << CLR_RS;
        for (int i = p; i < barW; i++) cout << CLR_GY << "\xB0";
        cout << CLR_RS << " " << CLR_CY << "\xBA" << CLR_RS;
        Sleep(25);
    }
    drawBoxRow(sx, sy + 7, BWIDTH, "");
    drawBoxBot(sx, sy + 8, BWIDTH);

    Sleep(300);
    showCur();
}

// ==================== Main ====================
int main() {
    enableAnsi();
    showSplash();

    MYSQL* conn = connectDB();
    if (!conn) {
        showMsg("Database connection failed!", "err");
        waitKey();
        return 1;
    }
    showMsg("Database Connected!", "ok");
    Sleep(500);

    vector<string> mainOpts = {"Login", "Register", "Exit"};
    while (true) {
        int c = showMenu("PHOTOGRAPHY BOOKING SYSTEM", mainOpts, "Welcome! Please select an option.");
        if (c == 0) loginUser(conn);
        else if (c == 1) registerUser(conn);
        else if (c == 2 || c == -1) {
            cls();
            showMsg("Goodbye! Thank you for using the system.", "info");
            break;
        }
    }

    mysql_close(conn);
    return 0;
}

