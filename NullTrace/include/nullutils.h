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
        bool(*initialize)(const void* runtime_cbs, const char* private_dir, const char* instruction_set);

        void*(*loadLibrary)(const char* libpath, int flag);

        void*(*getTrampoline)(void* handle, const char* name, const char* shorty, uint32_t len);

        bool(*isSupported)(const char* libpath);
        const void*(*getAppEnv)(const char* instruction_set);
        bool(*isCompatibleWith)(uint32_t bridge_version);
        void*(*getSignalHandler)(int signal);
        int(*unloadLibrary)(void* handle);
        const char* (*getError)();
        bool(*isPathSupported)(const char* library_path);
        bool(*initAnonymousNamespace)(const char* public_ns_sonames, const char* anon_ns_library_path);
        void*(*createNamespace)(const char* name, const char* ld_library_path, const char* default_library_path, uint64_t type, const char* permitted_when_isolated_path, void* parent_ns);
        bool(*linkNamespaces)(void* from, void* to, const char* shared_libs_sonames);

        void *(*loadLibraryExt)(const char *libpath, int flag, void *ns);
        void*(*getVendorNamespace)();
        void*(*getExportedNamespace)(const char* name);
        void(*preZygoteFork)();
        void*(*getTrampolineWithJNICallType)(void* handle, const char* name, const char* shorty, uint32_t len, int32_t jni_call_type);


        void*(*getTrampolineForFunctionPointer)(const void* method, const char* shorty, uint32_t len, int32_t jni_call_type);

    };

    std::string removeNullChars(const std::string &str);
    std::vector<uint8_t> interpretHex(std::string hex);
    std::string bytesToHex(uint8_t* bytes, size_t len);
    bool endsWith(const std::string &str, const std::string &suffix);
    int getApiLevel();
    void SELINUX_SetEnforce(int mode);
    int SELINUX_GetEnforce();

    template<typename T>
    uintptr_t handleArg(T* input) {
        return reinterpret_cast<uintptr_t>(input);
    }
    template<typename T>
    uintptr_t handleArg(T input) {
        return static_cast<uintptr_t>(input);
    }
}
