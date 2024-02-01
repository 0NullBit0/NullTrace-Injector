#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <array>
#include <iomanip>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>


namespace NullUtils {
    std::string removeNullChars(const std::string &str);
    std::vector<uint8_t> interpretHex(std::string hex);
    std::string bytesToHex(uint8_t* bytes, size_t len);
    bool endsWith(const std::string &str, const std::string &suffix);

    template<typename T>
    uintptr_t handleArg(T* input) {
        return reinterpret_cast<uintptr_t>(input);
    }
    template<typename T>
    uintptr_t handleArg(T input) {
        return static_cast<uintptr_t>(input);
    }
}
