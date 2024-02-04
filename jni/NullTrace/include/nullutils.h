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
#include <sys/system_properties.h>


namespace NullUtils {

    struct NativeBridgeCallbacks {
        uint32_t version;
        void *initialize;

        void*(*loadLibrary)(const char *libpath, int flag);

        void*(*getTrampoline)(void *handle, const char *name, const char *shorty, uint32_t len);

        void* isSupported;
        void* getAppEnv;
        void* isCompatibleWith;
        void* getSignalHandler;
        void* unloadLibrary;
        void* (*getError)();
        void* isPathSupported;
        void* initAnonymousNamespace;
        void* createNamespace;
        void* linkNamespaces;

        void *(*loadLibraryExt)(const char *libpath, int flag, void *ns);
    };

    std::string removeNullChars(const std::string &str);
    std::vector<uint8_t> interpretHex(std::string hex);
    std::string bytesToHex(uint8_t* bytes, size_t len);
    bool endsWith(const std::string &str, const std::string &suffix);
    int getApiLevel();

    template<typename T>
    uintptr_t handleArg(T* input) {
        return reinterpret_cast<uintptr_t>(input);
    }
    template<typename T>
    uintptr_t handleArg(T input) {
        return static_cast<uintptr_t>(input);
    }
}
