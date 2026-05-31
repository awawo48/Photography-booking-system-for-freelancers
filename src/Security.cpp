#include "Security.hpp"
#include <windows.h>
#include <bcrypt.h>
#include <vector>
#include <random>

namespace Security {

std::string hashSHA256(const std::string& input) {
    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_HASH_HANDLE hHash = NULL;
    DWORD cbData = 0, cbHash = 0, cbHashObject = 0;
    std::vector<BYTE> pbHashObject, pbHash;

    if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0) >= 0) {
        if (BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbHashObject, sizeof(DWORD), &cbData, 0) >= 0) {
            pbHashObject.resize(cbHashObject);
            if (BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PBYTE)&cbHash, sizeof(DWORD), &cbData, 0) >= 0) {
                pbHash.resize(cbHash);
                if (BCryptCreateHash(hAlg, &hHash, pbHashObject.data(), cbHashObject, NULL, 0, 0) >= 0) {
                    if (BCryptHashData(hHash, (PBYTE)input.c_str(), input.length(), 0) >= 0) {
                        BCryptFinishHash(hHash, pbHash.data(), cbHash, 0);
                    }
                    BCryptDestroyHash(hHash);
                }
            }
        }
        BCryptCloseAlgorithmProvider(hAlg, 0);
    }

    if (pbHash.empty()) return "";
    std::string hexStr;
    const char hexChars[] = "0123456789abcdef";
    for (BYTE b : pbHash) {
        hexStr += hexChars[(b & 0xF0) >> 4];
        hexStr += hexChars[b & 0x0F];
    }
    return hexStr;
}

std::string generateSalt(int length) {
    std::vector<BYTE> buffer(length);
    if (BCryptGenRandom(NULL, buffer.data(), length, BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0) {
        // Fallback to std::random_device if BCryptGenRandom fails (unlikely, but good practice)
        std::random_device rd;
        for (int i = 0; i < length; ++i) {
            buffer[i] = static_cast<BYTE>(rd() & 0xFF);
        }
    }
    
    std::string hexStr;
    const char hexChars[] = "0123456789abcdef";
    for (BYTE b : buffer) {
        hexStr += hexChars[(b & 0xF0) >> 4];
        hexStr += hexChars[b & 0x0F];
    }
    return hexStr;
}

std::string hashWithSalt(const std::string& salt, const std::string& password) {
    return hashSHA256(salt + password);
}

}
