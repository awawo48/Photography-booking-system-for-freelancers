#pragma once

#include <string>

namespace Security {
    std::string hashSHA256(const std::string& input);
    std::string generateSalt(int length = 32);
    std::string hashWithSalt(const std::string& salt, const std::string& password);
}
