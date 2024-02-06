
#include "../include/nullproc.h"


NullProcess::Process::Process() {
    this->pid = -1;
    this->pkgName = "UNDEFINED";
}

bool NullProcess::Process::setProcByPid(pid_t procID) {
    std::ifstream cmdlineFile("/proc/" + std::to_string(procID) + "/cmdline");
    if (!cmdlineFile) {
        return false;
    }


    this->pid = procID;
    this->maps = getMaps(this->pid);


    return true;
}

bool NullProcess::Process::setProcByName(const std::string& pkg) {
    this->pkgName = pkg;
    this->pid = -1;
    namespace fs = std::filesystem;
    for(const auto &entry : fs::directory_iterator("/proc")) {
        try {
            std::ifstream cmdlineFile(entry.path() / "cmdline");
            std::string pkgNameBuffer;
            if(getline(cmdlineFile, pkgNameBuffer)) {
                pkgNameBuffer = NullUtils::removeNullChars(pkgNameBuffer);
                if(pkgNameBuffer == pkg) {
                    this->pid = std::stoi(entry.path().filename());
                    this->maps = getMaps(this->pid);
                    this->locateSymbols();
                    break;
                }
            }
        } catch(const std::invalid_argument &e) {
            //skip non pid entries
        }
    }
    return pid != -1;
}

std::vector<NullProcess::Map> NullProcess::Process::getMaps(const pid_t &pid) {


    std::vector<NullProcess::Map> maps;
    std::string path = "/proc/" + std::to_string(pid) + "/maps";

    std::ifstream file(path);
    if(!file.is_open()) {
        std::cerr << "Error opening file: " << path << "\n";
        return {};
    }
    std::string line;
    while(std::getline(file, line))
        maps.push_back(NullProcess::Process::parseMap(line));


    return maps;
}

bool
NullProcess::Process::writeProcessMemory(std::string hex, uintptr_t address) {

    std::vector<uint8_t> hexVec = NullUtils::interpretHex(std::move(hex));
    if(hexVec.empty())
        return false;

    if(ptrace(PTRACE_ATTACH, this->pid, nullptr, nullptr) == -1)
        return false;

    waitpid(this->pid, nullptr, WUNTRACED);

    uint8_t writeData[hexVec.size()];
    for(int i = 0; i < hexVec.size(); i++)
        writeData[i] = hexVec.at(i);

    if(!NullTrace::ptraceWrite(this->pid, address, writeData, hexVec.size()))
        return false;

    if(ptrace(PTRACE_DETACH, this->pid, nullptr, nullptr) == -1)
        return false;

    return true;
}

std::string NullProcess::Process::readProcessMemory(uintptr_t address, size_t len) {



    if(ptrace(PTRACE_ATTACH, this->pid, nullptr, nullptr) == -1)
        return "";
    waitpid(this->pid, nullptr, WUNTRACED);
    uint8_t readData[len];

    if(!NullTrace::ptraceRead(this->pid, address, readData, len)) {
        return "";
    }

    if(ptrace(PTRACE_DETACH, this->pid, nullptr, nullptr) == -1)
        return "";

    std::string hex = NullUtils::bytesToHex(readData, len);
    return hex;
}



bool NullProcess::Process::injectLibrary(std::string path) {
    std::cout << "[NullTrace] Starting library injection\n";
    namespace fs = std::filesystem;
    fs::path p(path);
    if(!fs::exists(p)) {
        std::cerr << "[NullTrace] File doesnt exist aborting injection\n";
        return false;
    }
    bool shouldEmuInject = false;
    int libArch = NullElf::getLibraryArch(path.c_str());
    #if defined(__x86_64__) || defined(__i386__)
        if(libArch == NullElf::EARM || libArch == NullElf::EAARCH64)
            shouldEmuInject = true;
    #endif

    std::cout << "[NullTrace] LIBC Malloc Address: " << std::hex
              << this->libc.remote_malloc << " LIBC Free Address: " << std::hex
              << this->libc.remote_free << " LIBDL Dlopen Address: " << std::hex << this->libdl.remote_dlopen
              << " LIBDL Dlsym Address: " << std::hex << this->libdl.remote_dlsym <<"\n";
    #if defined(__arm__) || defined(__aarch64__)
        if (!this->libc.remote_malloc || !this->libc.remote_free || !this->libdl.remote_dlopen || !this->libdl.remote_dlsym) {
            std::cerr << "[NullTrace] failed injection failed locating syms\n";
            return false;
        }
    #elif defined(__i386__) || defined(__x86_64__)
        if (!this->libc.remote_malloc || !this->libc.remote_free ) {
            std::cerr << "[NullTrace] failed injection failed locating syms\n";
            return false;
        }
        if(!this->libdl.remote_dlopen || !this->libdl.remote_dlsym) {
            std::cout << "[NullTrace] WARNING NO DL FUNCTIONS FOUND TRYING NATIVE BRIDGE INJECTION MAKE SURE YOU INJECT AN ARM LIBRARY !\n";
            shouldEmuInject = true;
        }
    #endif


    size_t pathSize = path.size() + 1;
    void* pathAddr = this->remoteString(path);

    if(pathAddr == nullptr) {
        std::cerr << "[NullTrace] failed allocating memory for path aborting\n";
        return false;
    }

    printf("[NullTrace] Allocated Path: %s\n", this->readProcessMemory(
            reinterpret_cast<uintptr_t>(pathAddr), pathSize).c_str() );
    uintptr_t libLR_base = 0x0;

    for(const NullProcess::Map& map : this->maps) {
        #if defined(__x86_64__) || defined(__i386__)

            if(map.pathName.find("system/lib64/libart.so") != std::string::npos || map.pathName.find("system/lib/libart.so") != std::string::npos) {
                libLR_base = map.start;
                std::cout << "libLR as : libart.so\n";
                break;
            }
            if(map.pathName.find("system/lib64/libRS.so") != std::string::npos || map.pathName.find("system/lib/libRS.so") != std::string::npos) {
                libLR_base = map.start;
                std::cout << "libLR as : libRS.so\n";
                break;
            }

        #elif defined(__arm__) || defined(__aarch64__)
            if(map.pathName.find("libart.so") != std::string::npos) {
                libLR_base = map.start;
                std::cout << "libLR as : libart.so\n";
                break;
            }
            if(map.pathName.find("libRS.so") != std::string::npos) {
                libLR_base = map.start;
                std::cout << "libLR as : libRS.so\n";
                break;
            }
        #endif
    }

    if(!libLR_base) {
        std::cerr << "[NullTrace] Not found libLR trying with libc\n";
        for(const NullProcess::Map& map : this->maps) {
            #if defined(__x86_64__) || defined(__i386__)

                if(map.pathName.find("system/lib64/libc.so") != std::string::npos || map.pathName.find("system/lib/libc.so") != std::string::npos) {
                    libLR_base = map.start;
                    std::cout << "libLR as : libc.so\n";
                    break;
                }

            #elif defined(__arm__) || defined(__aarch64__)
                if(map.pathName.find("libc.so") != std::string::npos) {
                    libLR_base = map.start;
                    std::cout << "libLR as : libc.so\n";
                    break;
                }

            #endif
        }
    }
    std::cout << "[NullTrace] libLR base: " << std::hex << libLR_base << "\n";


    if (shouldEmuInject) {
        if (!this->injectLibNB(pathAddr, libLR_base)) {
            this->call<void>(this->libc.remote_free, pathAddr);
            return false;
        }
        this->call<void>(this->libc.remote_free, pathAddr);
        return true;
    }
    else {
        void *handle = this->callR<void *>(libLR_base, this->libdl.remote_dlopen, pathAddr,
                                           RTLD_NOW | RTLD_GLOBAL);
        if (!handle) {
            std::cerr << "[NullTrace] failed injection " << std::hex << handle << "\n";
            uintptr_t errnoMsg = this->call<uintptr_t>(this->libdl.remote_dlerror);
            if(errnoMsg) {
                std::vector<uint8_t> errnoChars = NullUtils::interpretHex(
                        this->readProcessMemory(errnoMsg, 500));
                std::cerr << "[NullTrace] DlError message: ";
                for (unsigned char &errnoChar: errnoChars) {
                    if (errnoChar == 0x0)
                        break;
                    std::cerr << (char) errnoChar;
                }
                std::cerr << "\n";
            }
            this->call<void>(this->libc.remote_free, pathAddr);
            return false;
        }

        std::cout << "[NullTrace] Successful injection handle: " << std::hex << handle << "\n";

        this->call<void>(this->libc.remote_free, pathAddr);
        return true;
    }
}

bool NullProcess::Process::injectLibNB(void* pathAddr, uintptr_t libLR_base) {
    std::cout << "[NullTrace] using nativebridge to inject on emulator\n";

    if(!this->nbInfo.nativeBridgeActive) {
        std::cerr << "[NullTrace] native bridge is not initialized in this process aborting injection!\n";
        return false;
    }
    void* handle = nullptr;

    if(this->nbInfo.usesCallbacksPtr) {
        std::cout << "[NullTrace] using nbcallbacks pointer for nativebridge\n";
        if(NullUtils::getApiLevel() >= 26)
            handle = this->callR<void*>(libLR_base, reinterpret_cast<uintptr_t>(nbCallbacks.loadLibraryExt), pathAddr, RTLD_NOW | RTLD_GLOBAL, 3);
        else
            handle = this->callR<void*>(libLR_base, reinterpret_cast<uintptr_t>(nbCallbacks.loadLibrary), RTLD_GLOBAL | RTLD_NOW);
    }
    else {
        std::cout << "[NullTrace] using libnativebridge for nativebridge\n";
        if(NullUtils::getApiLevel() >= 26)
            handle = this->callR<void*>(libLR_base, libnativebridge.remote_loadLibraryExt, pathAddr, RTLD_NOW | RTLD_GLOBAL, 3);
        else
            handle = this->callR<void*>(libLR_base, libnativebridge.remote_loadLibrary, pathAddr, RTLD_GLOBAL | RTLD_NOW);
    }

    if(!handle) {
        std::cerr << "[NullTrace] failed emulator injection " << std::hex << handle << "\n";

        uintptr_t errnoMsg;

        if(this->nbInfo.usesCallbacksPtr) {
            errnoMsg = this->callR<uintptr_t>(libLR_base, reinterpret_cast<uintptr_t>(nbCallbacks.getError));
        }
        else
            errnoMsg = this->callR<uintptr_t>(libLR_base, libnativebridge.remote_getError);
        if(errnoMsg) {
            std::vector<uint8_t> errnoChars = NullUtils::interpretHex(
                    this->readProcessMemory(errnoMsg, 500));
            std::cerr << "[NullTrace] NativeBridgeGetError message: ";
            for (unsigned char &errnoChar: errnoChars) {
                if (errnoChar == 0x0)
                    break;
                std::cerr << (char) errnoChar;
            }
            std::cerr << "\n";
        }
        return false;
    }
    std::cout << "[NullTrace] Successful emulator injection handle: " << std::hex << handle << "\n";

    return true;
}

void* NullProcess::Process::remoteString(std::string str) {
    size_t strSize = str.size();

    void* strAddr = this->call<void*>(this->libc.remote_malloc, strSize + 1);
    if(strAddr == nullptr) {
        std::cerr << "[NullTrace] Failed allocating memory on target process aborting\n";
        return nullptr;
    }
    std::cout << "[NullTrace] Allocated memory for string at Address: " << std::hex << strAddr << "\n";

    uint8_t strData[strSize+1];
    for(int i = 0; i < strSize; i++) {
        strData[i] = str[i];
    }
    strData[strSize] = '\0';

    if(!this->writeProcessMemory(NullUtils::bytesToHex(strData, strSize+1),
                                 reinterpret_cast<uintptr_t>(strAddr)) ) {
        std::cerr << "[NullTrace] Failed writing string into target process aborting\n";
        this->call<void>(this->libc.remote_free, strAddr);
        return nullptr;
    }

    std::cout << "[NullTrace] Successfully written string into allocated memory\n";

    return strAddr;
}

bool NullProcess::Process::processExists(pid_t pid) {

    namespace fs = std::filesystem;
    for(const auto &entry : fs::directory_iterator("/proc")) {
        try {
            if(std::stoi(entry.path().filename()) == pid)
                return true;
        } catch (const std::invalid_argument&) {
            // Non-integer filename, skip to the next iteration
            continue;
        }
    }
    return false;
}

void NullProcess::Process::locateSymbols() {
    this->libdl.remote_dlopen                            = 0x0;
    this->libdl.remote_dlerror                           = 0x0;
    this->libdl.remote_dlsym                             = 0x0;
    this->libc.remote_malloc                             = 0x0;
    this->libc.remote_free                               = 0x0;
    this->libc.remote_mmap                               = 0x0;

    this->libnativebridge.remote_loadLibraryExt          = 0x0;
    this->libnativebridge.remote_loadLibrary             = 0x0;
    this->libnativebridge.remote_getTrampoline           = 0x0;
    this->libnativebridge.remote_getError                = 0x0;

    bool libdlinit           = false;
    bool libcinit            = false;
    bool libnativebridgeinit = false;

    #if defined(__arm__) || defined(__aarch64__)
        for(const NullProcess::Map &map : this->maps) {
            if(NullUtils::endsWith(map.pathName, "libdl.so") && !libdlinit) {
                this->libdl.remote_dlopen = map.start + NullElf::getAddrSym(map.pathName.c_str(), "dlopen");
                this->libdl.remote_dlerror = map.start + NullElf::getAddrSym(map.pathName.c_str(), "dlerror");
                this->libdl.remote_dlsym = map.start + NullElf::getAddrSym(map.pathName.c_str(), "dlsym");
                libdlinit = true;
            }
            if(NullUtils::endsWith(map.pathName, "libc.so") && !libcinit) {
                this->libc.remote_malloc = map.start + NullElf::getAddrSym(map.pathName.c_str(), "malloc");
                this->libc.remote_free = map.start + NullElf::getAddrSym(map.pathName.c_str(), "free");
                this->libc.remote_mmap = map.start + NullElf::getAddrSym(map.pathName.c_str(), "mmap");
                libcinit = true;
            }
        }
    #elif defined(__i386__) || defined(__x86_64__)

        for(const NullProcess::Map &map : this->maps) {
            if(map.pathName.find("/arm/") == std::string::npos
            && map.pathName.find("/arm64/") == std::string::npos
            && map.pathName.find("/nb/") == std::string::npos
            && NullUtils::endsWith(map.pathName, "libdl.so")
            && !libdlinit) {
                this->libdl.remote_dlopen = map.start + NullElf::getAddrSym(map.pathName.c_str(), "dlopen");
                this->libdl.remote_dlerror = map.start + NullElf::getAddrSym(map.pathName.c_str(), "dlerror");
                this->libdl.remote_dlsym = map.start + NullElf::getAddrSym(map.pathName.c_str(), "dlsym");
                libdlinit = true;
            }

            if(map.pathName.find("/arm/") == std::string::npos
               && map.pathName.find("/arm64/") == std::string::npos
               && map.pathName.find("/nb/") == std::string::npos
               && NullUtils::endsWith(map.pathName, "libc.so")
               && !libcinit) {
                this->libc.remote_malloc = map.start + NullElf::getAddrSym(map.pathName.c_str(), "malloc");
                this->libc.remote_free = map.start + NullElf::getAddrSym(map.pathName.c_str(), "free");
                this->libc.remote_mmap = map.start + NullElf::getAddrSym(map.pathName.c_str(), "mmap");
                libcinit = true;
            }

            if(map.pathName.find("/arm/") == std::string::npos
               && map.pathName.find("/arm64/") == std::string::npos
               && map.pathName.find("/nb/") == std::string::npos
               && NullUtils::endsWith(map.pathName, "libnativebridge.so")
               && !libnativebridgeinit) {
                if(NullUtils::getApiLevel() >= 26)
                    this->libnativebridge.remote_loadLibraryExt = map.start + NullElf::getAddrSym(map.pathName.c_str(), "_ZN7android26NativeBridgeLoadLibraryExtEPKciPNS_25native_bridge_namespace_tE");
                else
                    this->libnativebridge.remote_loadLibrary    = map.start + NullElf::getAddrSym(map.pathName.c_str(),"_ZN7android23NativeBridgeLoadLibraryEPKci");
                this->libnativebridge.remote_getError = map.start + NullElf::getAddrSym(map.pathName.c_str(), "_ZN7android20NativeBridgeGetErrorEv");
                this->libnativebridge.remote_getTrampoline = map.start + NullElf::getAddrSym(map.pathName.c_str(), "_ZN7android25NativeBridgeGetTrampolineEPvPKcS2_j");
                this->nbInfo.nativeBridgeActive = true;
                this->nbInfo.usesCallbacksPtr   = false;
                libnativebridgeinit = true;
            }
        }

        if(!libnativebridgeinit) {
            NullProcess::Map nbMap = this->findMap("libhoudini.so");
            if(nbMap.pathName.empty()) {
                auto nbProp = std::array<char, PROP_VALUE_MAX>();
                __system_property_get("ro.dalvik.vm.native.bridge", nbProp.data());
                std::string nbStr = {nbProp.data()};

                nbMap = this->findMap(nbStr);
            }
            if(nbMap.pathName.empty()) {
                this->nbInfo.nativeBridgeActive = false;
                this->nbInfo.usesCallbacksPtr   = false;
            }
            else {
                uintptr_t  nbCallbacksAddr = (NullElf::getAddrSym(nbMap.pathName.c_str(), "NativeBridgeItf") + nbMap.start);
                this->nbCallbacks = this->readProcessMemory<NullUtils::NativeBridgeCallbacks>(nbCallbacksAddr);
                this->nbInfo.nativeBridgeActive = true;
                this->nbInfo.usesCallbacksPtr   = true;
            }
        }

    #else
        #error Unsupported Device
    #endif
}

NullProcess::Map NullProcess::Process::parseMap(const std::string &line) {
    NullProcess::Map map;

    size_t spaces[5];
    spaces[0] = line.find(' ');
    for(int i = 1; i < 5; i++)
        spaces[i] = line.find(' ', spaces[i-1] + 1);
    map.arch = -1;

    size_t dashIndex = line.find('-');
    size_t pathIndex;
    if((pathIndex = line.find('/')) != std::string::npos) {
        map.pathName = line.substr(pathIndex);
        if(NullUtils::endsWith(map.pathName, ".so"))
            map.arch = NullElf::getLibraryArch(map.pathName.c_str());

    }
    else if((pathIndex = line.find('[')) != std::string::npos)
        map.pathName = line.substr(pathIndex);
    else
        map.pathName = "";

    map.start = std::stoull(line.substr(0, dashIndex), nullptr, 16);
    map.end = std::stoull( line.substr(dashIndex+1, spaces[0]- (dashIndex+1)), nullptr, 16 );
    map.length = map.end - map.start;
    map.perms = 0;
    if (line[spaces[0] + 1] == 'r') map.perms |= NullProcess::READ;
    if (line[spaces[0] + 2] == 'w') map.perms |= NullProcess::WRITE;
    if (line[spaces[0] + 3] == 'x') map.perms |= NullProcess::EXECUTE;

    map.offset = std::stoull(line.substr(spaces[1] + 1, spaces[2] - spaces[1] + 1), nullptr, 16);
    map.device = line.substr(spaces[2] + 1, spaces[3] - spaces[2] + 1);
    map.inode = std::stoi(line.substr(spaces[3] + 1, spaces[4] - spaces[3] + 1));
    return map;
}

NullProcess::Map NullProcess::Process::findMap(std::string mapname) {
    for(NullProcess::Map map : this->maps) {
        if(map.pathName.find(mapname) != std::string::npos  ) {
            return map;
        }
    }
    NullProcess::Map invalidMap;
    invalidMap.pathName = "";
    return invalidMap;
}
