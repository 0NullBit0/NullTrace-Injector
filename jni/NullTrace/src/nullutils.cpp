
#include "../include//nullutils.h"

std::string NullUtils::removeNullChars(const std::string &str) {
    size_t nullPos = str.find('\0');
    if (nullPos != std::string::npos)
        return str.substr(0, nullPos);
    else
        return str;
}

std::vector<uint8_t> NullUtils::interpretHex(std::string hex) {
    hex.erase(std::remove_if(hex.begin(), hex.end(), ::isspace), hex.end());

    if(hex.size() % 2 != 0) {
        return {};
    }
    for(char c : hex) {
        if(!std::isxdigit(c)) {
            return {};
        }
    }

    std::vector<uint8_t> bytes;
    for(int i = 0; i < hex.size(); i += 2) {
        std::string byteStr = hex.substr(i, 2);
        uint8_t byte = static_cast<uint8_t>(std::stoul(byteStr, nullptr, 16));
        bytes.push_back(byte);
    }
    return bytes;
}

std::string NullUtils::bytesToHex(uint8_t* bytes, size_t len) {
    std::stringstream hexStream;
    for(int i = 0; i < len; i++)
        hexStream << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(bytes[i]) << " ";
    std::string outStr = hexStream.str();
    std::transform(outStr.begin(), outStr.end(), outStr.begin(), ::toupper);
    return outStr;
}

bool NullUtils::endsWith(const std::string &str, const std::string &suffix) {
    if(suffix.size() > str.size()) return false;
    return std::equal(suffix.rbegin(), suffix.rend(), str.rbegin());
}






