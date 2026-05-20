#pragma once

#include <string>
#include <vector>

std::string getTimestamp();
std::string getDateStamp();

bool exportPDF(const std::string& filename, const std::string& title,
               const std::vector<std::string>& headers, const std::vector<std::vector<std::string>>& rows,
               const std::vector<std::string>& summaryLines);
