#include <iostream>
#include <fstream>
#include <sstream>
#include "Database.hpp"
#include <mysql.h>

using namespace std;

int main() {
    MYSQL* conn = connectDB();
    if (!conn) {
        cerr << "Failed to connect to DB." << endl;
        return 1;
    }

    vector<string> files = {"add_payment_notes.sql"};
    for (const string& fname : files) {
        ifstream file(fname);
        if (!file.is_open()) {
            cerr << "Failed to open " << fname << endl;
            continue;
        }

        string line, query;
        while (getline(file, line)) {
            if (line.empty() || line.substr(0, 2) == "--") continue;
            query += line + " ";
            if (line.back() == ';') {
                cout << "Executing: " << query << endl;
                if (mysql_query(conn, query.c_str())) {
                    cerr << "Error: " << mysql_error(conn) << endl;
                } else {
                    cout << "Success." << endl;
                }
                query = "";
            }
        }
    }

    mysql_close(conn);
    return 0;
}
