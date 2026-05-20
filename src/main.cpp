#include "Database.hpp"
#include "UIHelper.hpp"
#include "Dashboard/Auth.hpp"
#include <vector>
#include <string>
#include <windows.h>

using namespace std;

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
