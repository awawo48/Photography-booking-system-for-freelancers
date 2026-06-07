#include "CodeGenerator.hpp"
#include "Database.hpp"
#include <iomanip>
#include <sstream>
#include <ctime>

using namespace std;

string CodeGenerator::generate(MYSQL* conn, const string& prefix, const string& tableName, const string& columnName, bool useYearMonth, bool useDay) {
    string dateStr = "";
    if (useYearMonth) {
        time_t t = time(nullptr);
        tm* now = localtime(&t);
        stringstream ss;
        ss << (now->tm_year + 1900)
           << setfill('0') << setw(2) << (now->tm_mon + 1);
        if (useDay) {
            ss << setfill('0') << setw(2) << now->tm_mday;
        }
        dateStr = ss.str();
    }

    // Build prefix block
    string prefixBlock = prefix;
    if (!dateStr.empty()) {
        prefixBlock += "-" + dateStr;
    }
    
    // Find the latest record matching this prefix
    string q = "SELECT " + columnName + " FROM " + tableName + " WHERE " + columnName + " LIKE '" + prefixBlock + "-%' ORDER BY " + columnName + " DESC LIMIT 1";
    auto results = DBHelper::executeQuery(conn, q, {});
    
    int nextId = 1;
    if (!results.empty()) {
        string lastCode = results[0][0];
        // Parse the number after the last hyphen
        size_t lastDash = lastCode.find_last_of('-');
        if (lastDash != string::npos) {
            string numStr = lastCode.substr(lastDash + 1);
            try {
                nextId = stoi(numStr) + 1;
            } catch(...) {
                nextId = 1; // Fallback
            }
        }
    }

    stringstream ss;
    ss << prefixBlock << "-" << setfill('0') << setw(3) << nextId;
    return ss.str();
}
