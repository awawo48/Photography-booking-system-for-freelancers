#include "Database.hpp"
#include <iostream>
#include <cstring>

using namespace std;

MYSQL* connectDB() {
    MYSQL* conn = mysql_init(0);
    if (!conn) {
        cerr << "MySQL init failed!" << endl;
        return nullptr;
    }
    my_bool ssl_e = 0, ssl_v = 0;
    mysql_options(conn, MYSQL_OPT_SSL_ENFORCE, &ssl_e);
    mysql_options(conn, MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &ssl_v);
    MYSQL* r = mysql_real_connect(conn, "localhost", "root", "", "photography_db", 3306, nullptr, 0);
    if (!r) {
        cerr << "DB Connection Failed: " << mysql_error(conn) << endl;
    }
    return r ? conn : nullptr;
}

void printError(MYSQL_STMT* stmt) {
    cerr << "Stmt Error: " << mysql_stmt_error(stmt) << endl;
}

vector<vector<string>> DBHelper::executeQuery(MYSQL* conn, const string& query, const vector<string>& params) {
    vector<vector<string>> results;
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) return results;

    if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
        printError(stmt);
        mysql_stmt_close(stmt);
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
        if (mysql_stmt_bind_param(stmt, bindParams.data())) {
            printError(stmt);
            mysql_stmt_close(stmt);
            return results;
        }
    }

    if (mysql_stmt_execute(stmt)) {
        printError(stmt);
        mysql_stmt_close(stmt);
        return results;
    }

    MYSQL_RES* meta = mysql_stmt_result_metadata(stmt);
    if (meta) {
        int num_fields = mysql_num_fields(meta);
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

        if (mysql_stmt_bind_result(stmt, bindResults.data())) {
            printError(stmt);
        } else {
            while (!mysql_stmt_fetch(stmt)) {
                vector<string> row;
                for (int i = 0; i < num_fields; ++i) {
                    if (is_null[i]) {
                        row.push_back("");
                    } else {
                        try {
                            size_t actual_len = lengths[i];
                            if (actual_len > buffers[i].size()) {
                                actual_len = buffers[i].size(); // Prevent std::length_error!
                            }
                            row.push_back(string(buffers[i].data(), actual_len));
                        } catch (const std::exception& e) {
                            cerr << "Exception in row.push_back: " << e.what() << " lengths[" << i << "]=" << lengths[i] << endl;
                            throw;
                        }
                    }
                }
                results.push_back(row);
            }
        }
        mysql_free_result(meta);
    }
    mysql_stmt_close(stmt);
    return results;
}

int DBHelper::executeUpdate(MYSQL* conn, const string& query, const vector<string>& params) {
    MYSQL_STMT* stmt = mysql_stmt_init(conn);
    if (!stmt) return -1;

    if (mysql_stmt_prepare(stmt, query.c_str(), query.length())) {
        printError(stmt);
        mysql_stmt_close(stmt);
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
        if (mysql_stmt_bind_param(stmt, bindParams.data())) {
            printError(stmt);
            mysql_stmt_close(stmt);
            return -1;
        }
    }

    if (mysql_stmt_execute(stmt)) {
        printError(stmt);
        mysql_stmt_close(stmt);
        return -1;
    }

    int affected = mysql_stmt_affected_rows(stmt);
    mysql_stmt_close(stmt);
    return affected;
}
