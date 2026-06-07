#include "UIHelper.hpp"
#include <iostream>
#include <iomanip>
#include <windows.h>
#include <conio.h>
#include <sstream>

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
void drawBoxTop(int x, int y, int w, const string& color) {
    moveTo(x, y);
    cout << (color.empty() ? CLR_CY : color) << "\xC9";
    for (int i = 0; i < w - 2; i++) cout << "\xCD";
    cout << "\xBB" << CLR_RS;
}

void drawBoxBot(int x, int y, int w, const string& color) {
    moveTo(x, y);
    cout << (color.empty() ? CLR_CY : color) << "\xC8";
    for (int i = 0; i < w - 2; i++) cout << "\xCD";
    cout << "\xBC" << CLR_RS;
}

void drawBoxMid(int x, int y, int w, const string& color) {
    moveTo(x, y);
    cout << (color.empty() ? CLR_CY : color) << "\xCC";
    for (int i = 0; i < w - 2; i++) cout << "\xCD";
    cout << "\xB9" << CLR_RS;
}

int getVisibleLength(const string& s) {
    int len = 0;
    for (size_t i = 0; i < s.length(); ) {
        if (s[i] == '\033') {
            i++;
            if (i < s.length() && s[i] == '[') {
                i++;
                while (i < s.length() && ((s[i] >= '0' && s[i] <= '9') || s[i] == ';' || s[i] == '?')) {
                    i++;
                }
                if (i < s.length() && (s[i] == 'm' || s[i] == 'K' || s[i] == 'J' || s[i] == 'h' || s[i] == 'l')) {
                    i++;
                }
            }
        } else {
            len++;
            i++;
        }
    }
    return len;
}

void drawBoxRow(int x, int y, int w, const string& text, const string& color) {
    moveTo(x, y);
    cout << (color.empty() ? CLR_CY : color) << "\xBA" << CLR_RS;
    int pad = w - 2 - getVisibleLength(text);
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
    try {
        while ((int)r.length() < w) r += " ";
        if ((int)r.length() > w) r = r.substr(0, w);
    } catch(const std::exception& e) {
        cerr << "Exception in padStr: " << e.what() << " w=" << w << " s.length=" << s.length() << endl;
        throw;
    }
    return r;
}

// ==================== Components ====================
int showMenu(const string& title, const vector<string>& opts, const string& sub) {
    int sel = 0, n = (int)opts.size();
    while (true) {
        cls(); hideCur();
        int sx = (getTermW() - BWIDTH) / 2;
        if (sx < 1) sx = 1;
        int sy = 2;
        drawTitle(sx, sy, BWIDTH, title);
        int currY = sy + 3;
        if (!sub.empty()) {
            size_t start = 0;
            while (start < sub.length()) {
                size_t end = sub.find('\n', start);
                string line = (end == string::npos) ? sub.substr(start) : sub.substr(start, end - start);
                drawBoxRow(sx, currY, BWIDTH, "  " + CLR_DM + line + CLR_RS);
                currY++;
                if (end == string::npos) break;
                start = end + 1;
            }
        }
        drawBoxRow(sx, currY++, BWIDTH, "");
        for (int i = 0; i < n; i++) {
            string line;
            if (i == sel)
                line = "    " + CLR_BYL + "\x10 " + CLR_BWH + opts[i] + CLR_RS;
            else
                line = "      " + CLR_GY + opts[i] + CLR_RS;
            
            int visLen = (i == sel) ? 6 + (int)opts[i].length() : 6 + (int)opts[i].length();
            int totalPad = BWIDTH - 2 - visLen;
            moveTo(sx, currY + i);
            cout << CLR_CY << "\xBA" << CLR_RS << line;
            try {
                string pad_str(totalPad < 0 ? 0 : totalPad, ' ');
                cout << pad_str;
            } catch(const std::exception& e) {
                cerr << "Exception in showMenu string pad: " << e.what() << " totalPad=" << totalPad << endl;
                throw;
            }
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

string getInput(const string& prompt, bool pwd) {
    if (pwd) return getPasswordInput(prompt);
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
            cout << (char)k;
        }
    }
    cout << endl;
    return s;
}

string getPasswordInput(const string& prompt) {
    showCur();
    cout << CLR_BCY << "  \x10 " << CLR_WH << prompt << CLR_RS;
    string s;
    while (true) {
        int k = getKey();
        if (k == K_ENTER) break;
        if (k == K_ESC) { s = ""; break; }
        if (k == K_BKSP && !s.empty()) {
            s.pop_back();
            cout << "\b \b"; // Clears the asterisk
        } else if (k >= 32 && k < 127) {
            s += (char)k;
            cout << '*'; // Masked character
        }
    }
    cout << endl;
    return s;
}

int getIntInput(const string& prompt) {
    showCur();
    int value;
    while (true) {
        cout << CLR_BCY << "  \x10 " << CLR_WH << prompt << CLR_RS;
        if (cin >> value) {
            // Success, consume any trailing characters in the buffer
            cin.ignore(10000, '\n');
            break;
        } else {
            // Failure (user typed letters instead of numbers)
            cin.clear(); // Clear the error flag
            cin.ignore(10000, '\n'); // Discard invalid input
            showMsg("Invalid input! Please enter a valid number.", "err");
        }
    }
    return value;
}

void showPaginatedTable(const string& title, const vector<string>& headers, const vector<vector<string>>& rows, int limit) {
    int totalRows = (int)rows.size();
    if (totalRows == 0) {
        showScreenHeader(title);
        showMsg("No records found.", "info");
        waitKey();
        return;
    }
    
    int totalPages = (totalRows + limit - 1) / limit;
    int page = 0;
    
    // Auto-calculate column widths based on headers and data
    vector<int> colWidths(headers.size(), 0);
    for (size_t i = 0; i < headers.size(); ++i) {
        colWidths[i] = max(colWidths[i], (int)headers[i].length());
    }
    for (const auto& row : rows) {
        for (size_t i = 0; i < row.size() && i < headers.size(); ++i) {
            colWidths[i] = max(colWidths[i], (int)row[i].length());
        }
    }
    
    while (true) {
        cls(); hideCur();
        int sx = (getTermW() - BWIDTH) / 2;
        if (sx < 1) sx = 1;
        drawTitle(sx, 1, BWIDTH, title + " (Page " + to_string(page + 1) + " of " + to_string(totalPages) + ")");
        
        int y = 4;
        
        // Print top border
        moveTo(sx, y++);
        cout << CLR_CY << "  \xC9";
        for (size_t i = 0; i < colWidths.size(); ++i) {
            for (int j = 0; j < colWidths[i] + 2; ++j) cout << "\xCD";
            if (i < colWidths.size() - 1) cout << "\xCB";
        }
        cout << "\xBB" << CLR_RS;

        // Print headers
        moveTo(sx, y++);
        cout << CLR_CY << "  \xBA" << CLR_RS << CLR_BD << CLR_BWH;
        for (size_t i = 0; i < headers.size(); ++i) {
            cout << " " << padStr(headers[i], colWidths[i]) << " " << CLR_CY << "\xBA" << CLR_RS << CLR_BD << CLR_BWH;
        }
        cout << CLR_RS;

        // Header separator
        moveTo(sx, y++);
        cout << CLR_CY << "  \xCC";
        for (size_t i = 0; i < colWidths.size(); ++i) {
            for (int j = 0; j < colWidths[i] + 2; ++j) cout << "\xCD";
            if (i < colWidths.size() - 1) cout << "\xCE";
        }
        cout << "\xB9" << CLR_RS;

        // Print rows
        int startIdx = page * limit;
        int endIdx = min(startIdx + limit, totalRows);
        for (int r = startIdx; r < endIdx; ++r) {
            moveTo(sx, y++);
            cout << CLR_CY << "  \xBA" << CLR_RS << CLR_WH;
            for (size_t i = 0; i < rows[r].size() && i < headers.size(); ++i) {
                // Status color heuristic
                string cell = rows[r][i];
                string color = CLR_WH;
                if (cell == "Active" || cell == "Completed") color = CLR_BGR;
                else if (cell == "Pending" || cell == "Pending_Verification" || cell == "Approved" || cell == "Deposit Paid") color = CLR_BYL;
                else if (cell == "Banned" || cell == "Rejected") color = CLR_BRD;
                else if (cell == "Suspended" || cell == "Photographer") color = CLR_BMG;
                else if (cell == "Admin") color = CLR_BCY;
                else if (cell == "Customer") color = CLR_BYL;

                cout << color << " " << padStr(cell, colWidths[i]) << " " << CLR_CY << "\xBA" << CLR_RS << CLR_WH;
            }
            cout << CLR_RS;
        }

        // Bottom border
        moveTo(sx, y++);
        cout << CLR_CY << "  \xC8";
        for (size_t i = 0; i < colWidths.size(); ++i) {
            for (int j = 0; j < colWidths[i] + 2; ++j) cout << "\xCD";
            if (i < colWidths.size() - 1) cout << "\xCA";
        }
        cout << "\xBC" << CLR_RS;
        
        // Print bottom instructions
        y += 2;
        moveTo(sx, y);
        cout << CLR_CY << "\xBA " << CLR_GY << "Use Left/Right (or Up/Down) to flip pages. Press Esc to exit." << CLR_RS;
        
        int k = getKey();
        if (k == K_ESC || k == K_ENTER) break;
        if (k == K_UP || k == 75) { // 75 is Left arrow
            if (page > 0) page--;
        } else if (k == K_DOWN || k == 77) { // 77 is Right arrow
            if (page < totalPages - 1) page++;
        }
    }
    showCur();
}

void showMsg(const string& text, const string& type) {
    if (type == "ok") cout << CLR_BGR << "  [OK] " << CLR_WH << text << CLR_RS << endl;
    else if (type == "err") cout << CLR_BRD << "  [!] " << CLR_WH << text << CLR_RS << endl;
    else cout << CLR_BYL << "  [i] " << CLR_WH << text << CLR_RS << endl;
}

void showField(const string& label, const string& val, int lw) {
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
