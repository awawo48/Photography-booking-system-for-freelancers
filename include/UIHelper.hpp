#pragma once

#include <string>
#include <vector>

// ==================== ANSI Colors ====================
extern const std::string CLR_RS, CLR_BD, CLR_DM;
extern const std::string CLR_CY, CLR_GR, CLR_YL, CLR_RD;
extern const std::string CLR_MG, CLR_WH, CLR_GY;
extern const std::string CLR_BCY, CLR_BGR, CLR_BYL;
extern const std::string CLR_BRD, CLR_BWH, CLR_BMG;

// ==================== Key Codes ====================
extern const int K_UP, K_DOWN, K_ENTER, K_ESC, K_BKSP;

// ==================== Layout ====================
extern const int BWIDTH;

// ==================== Terminal Control ====================
void enableAnsi();
void cls();
void moveTo(int x, int y);
void hideCur();
void showCur();
int getTermW();
int getKey();

// ==================== Drawing ====================
void drawBoxTop(int x, int y, int w, const std::string& color = "");
void drawBoxBot(int x, int y, int w, const std::string& color = "");
void drawBoxMid(int x, int y, int w, const std::string& color = "");
void drawBoxRow(int x, int y, int w, const std::string& text, const std::string& color = "");
void drawTitle(int x, int y, int w, const std::string& title);
std::string padStr(const std::string& s, int w);

// ==================== Components / Widgets ====================
int showMenu(const std::string& title, const std::vector<std::string>& opts, const std::string& sub = "");
std::string getInput(const std::string& prompt, bool pwd = false);
std::string getPasswordInput(const std::string& prompt);
int getIntInput(const std::string& prompt);
void showPaginatedTable(const std::string& title, const std::vector<std::string>& headers, const std::vector<std::vector<std::string>>& rows, int limit = 10);
void showMsg(const std::string& text, const std::string& type = "ok");
void showField(const std::string& label, const std::string& val, int lw = 18);
void showDivider();
void waitKey();
void showScreenHeader(const std::string& title);
void showSplash();
