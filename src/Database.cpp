#include "Database.hpp"
#include <iostream>
#include <cstring>
#include <memory>
#include <stdexcept>

using namespace std;

MYSQL* connectDB() {
    MYSQL* conn = mysql_init(0);
    if (!conn) {
        cerr << "MySQL init failed!" << endl;
        return nullptr;
    }
    my_bool ssl_e = 0, ssl_v = 0;
    my_bool reconnect = 1;
    mysql_options(conn, MYSQL_OPT_SSL_ENFORCE, &ssl_e);
    mysql_options(conn, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &ssl_v);
    mysql_options(conn, MYSQL_OPT_RECONNECT, &reconnect);
    MYSQL* r = mysql_real_connect(conn, "localhost", "root", "", "photography_db", 3306, nullptr, 0);
    if (!r) {
        cerr << "DB Connection Failed: " << mysql_error(conn) << endl;
    }
    return r ? conn : nullptr;
}

void printError(MYSQL_STMT* stmt) {
    cerr << "Stmt Error: " << mysql_stmt_error(stmt) << endl;
}

static bool ensureConnected(MYSQL* conn) {
    if (mysql_ping(conn) != 0) {
        cerr << "DB connection lost and auto-reconnect failed: " << mysql_error(conn) << endl;
        return false;
    }
    return true;
}

// Custom deleters
struct StmtDeleter {
    void operator()(MYSQL_STMT* stmt) const {
        if (stmt) mysql_stmt_close(stmt);
    }
};

struct ResDeleter {
    void operator()(MYSQL_RES* res) const {
        if (res) mysql_free_result(res);
    }
};

vector<vector<string>> DBHelper::executeQuery(MYSQL* conn, const string& query, const vector<string>& params) {
    vector<vector<string>> results;
    if (!ensureConnected(conn)) return results;
    
    unique_ptr<MYSQL_STMT, StmtDeleter> stmt(mysql_stmt_init(conn));
    if (!stmt) return results;

    if (mysql_stmt_prepare(stmt.get(), query.c_str(), query.length())) {
        printError(stmt.get());
        return results;
    }

    vector<MYSQL_BIND> bindParams(params.size());
    if (!params.empty()) {
        for (size_t i = 0; i < params.size(); ++i) {
            memset(&bindParams[i], 0, sizeof(MYSQL_BIND));
            bindParams[i].buffer_type = MYSQL_TYPE_STRING;
            bindParams[i].buffer = (char*)params[i].c_str();
            bindParams[i].buffer_length = params[i].length();
        }
        if (mysql_stmt_bind_param(stmt.get(), bindParams.data())) {
            printError(stmt.get());
            return results;
        }
    }

    if (mysql_stmt_execute(stmt.get())) {
        printError(stmt.get());
        return results;
    }

    unique_ptr<MYSQL_RES, ResDeleter> meta(mysql_stmt_result_metadata(stmt.get()));
    if (meta) {
        int num_fields = mysql_num_fields(meta.get());
        vector<MYSQL_BIND> bindResults(num_fields);
        vector<vector<char>> buffers(num_fields, vector<char>(1024));
        vector<unsigned long> lengths(num_fields);
        vector<my_bool> is_null(num_fields);
        vector<my_bool> error(num_fields);

        for (int i = 0; i < num_fields; ++i) {
            memset(&bindResults[i], 0, sizeof(MYSQL_BIND));
            bindResults[i].buffer_type = MYSQL_TYPE_STRING;
            bindResults[i].buffer = buffers[i].data();
            bindResults[i].buffer_length = buffers[i].size();
            bindResults[i].length = &lengths[i];
            bindResults[i].is_null = &is_null[i];
            bindResults[i].error = &error[i];
        }

        if (mysql_stmt_bind_result(stmt.get(), bindResults.data())) {
            printError(stmt.get());
        } else {
            while (!mysql_stmt_fetch(stmt.get())) {
                vector<string> row;
                try {
                    for (int i = 0; i < num_fields; ++i) {
                        if (is_null[i]) {
                            row.push_back("");
                        } else {
                            size_t actual_len = lengths[i];
                            if (actual_len > buffers[i].size()) {
                                actual_len = buffers[i].size(); // Prevent std::length_error!
                            }
                            row.push_back(string(buffers[i].data(), actual_len));
                        }
                    }
                    results.push_back(row);
                } catch (const std::exception& e) {
                    cerr << "Exception in query row processing: " << e.what() << endl;
                    throw; // The unique_ptr will ensure mysql_stmt_close is still called!
                }
            }
        }
    }
    return results;
}

int DBHelper::executeUpdate(MYSQL* conn, const string& query, const vector<string>& params) {
    if (!ensureConnected(conn)) return -1;

    unique_ptr<MYSQL_STMT, StmtDeleter> stmt(mysql_stmt_init(conn));
    if (!stmt) return -1;

    if (mysql_stmt_prepare(stmt.get(), query.c_str(), query.length())) {
        printError(stmt.get());
        return -1;
    }

    vector<MYSQL_BIND> bindParams(params.size());
    if (!params.empty()) {
        for (size_t i = 0; i < params.size(); ++i) {
            memset(&bindParams[i], 0, sizeof(MYSQL_BIND));
            bindParams[i].buffer_type = MYSQL_TYPE_STRING;
            bindParams[i].buffer = (char*)params[i].c_str();
            bindParams[i].buffer_length = params[i].length();
        }
        if (mysql_stmt_bind_param(stmt.get(), bindParams.data())) {
            printError(stmt.get());
            return -1;
        }
    }

    if (mysql_stmt_execute(stmt.get())) {
        printError(stmt.get());
        return -1;
    }

    int affected_rows = mysql_stmt_affected_rows(stmt.get());
    return affected_rows;
}
