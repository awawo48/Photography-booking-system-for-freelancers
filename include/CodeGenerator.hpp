#ifndef CODE_GENERATOR_HPP
#define CODE_GENERATOR_HPP

#include <string>
#include <mysql.h>

class CodeGenerator {
public:
    // Generates a unique business key (e.g., BKG-202606-001)
    // - prefix: e.g., "BKG", "PKG", "RPT"
    // - tableName: the table to check against (e.g., "BOOKINGS")
    // - columnName: the column holding the code (e.g., "booking_code")
    // - useYearMonth: if true, appends YYYYMM (e.g., BKG-202606-001)
    // - useDay: if true, appends DD (e.g., RPT-20260601-001) - requires useYearMonth=true
    static std::string generate(MYSQL* conn, const std::string& prefix, const std::string& tableName, const std::string& columnName, bool useYearMonth = false, bool useDay = false);
};

#endif // CODE_GENERATOR_HPP
