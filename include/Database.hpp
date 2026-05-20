#pragma once

#include <mysql.h>
#include <string>
#include <vector>

// ==================== Database ====================
MYSQL* connectDB();

class DBHelper {
public:
    static std::vector<std::vector<std::string>> executeQuery(MYSQL* conn, const std::string& query, const std::vector<std::string>& params = {});
    static int executeUpdate(MYSQL* conn, const std::string& query, const std::vector<std::string>& params = {});
};
