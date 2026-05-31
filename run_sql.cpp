#include <iostream>
#include <fstream>
#include <sstream>
#include "Database.hpp"

using namespace std;

int main() {
    MYSQL* conn = connectDB();
    if (!conn) {
        cout << "Failed to connect" << endl;
        return 1;
    }
    cout << "Connected. Running migration..." << endl;
    
    int result = DBHelper::executeUpdate(conn, "ALTER TABLE USERS ADD COLUMN salt VARCHAR(64) DEFAULT NULL AFTER password", {});
    if (result >= 0) {
        cout << "Migration successful!" << endl;
    } else {
        cout << "Migration failed or already applied. Note: checking error." << endl;
    }
    
    mysql_close(conn);
    return 0;
}
