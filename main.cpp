#include "NullTrace/include/nullproc.h"
#include "NullTrace/include/nullutils.h"

bool handle_args(int argc, char* argv[], std::string &pkg, std::string &lib);

int main(int argc, char* argv[]) {
    std::string pkg;
    std::string lib;

    if(!handle_args(argc, argv, pkg, lib)) return -1;

    NullProcess::Process proc;

    while(!proc.setProcByName(pkg)) {
        sleep(1);
        std::cout << "Waiting for process: " << pkg.c_str() << std::endl;
    }

    std::cout << "Found target process\n";

    int SELinuxMode = NullUtils::SELINUX_GetEnforce();
    bool changed = false;
    if (SELinuxMode == 0) std::cout << "SELinux already permissive\n";
    else if (SELinuxMode == 1) {
        std::cout << "SELinux enforcing trying to set permissive\n";
        NullUtils::SELINUX_SetEnforce(0);
        if (NullUtils::SELINUX_GetEnforce() == 0) {
            changed = true;
            std::cout << "Successfully set SELinux permissive\n";
        }
        else std::cerr << "Failed setting SELinux permissive\n";
    } else std::cerr << "Couldn't determine SELinux status\n";

    std::cout << "Starting Injection\n";

    if(proc.injectLibrary(lib)) {
        std::cout << "Successfully injected library\n";
        if(pkg == "zygote" || pkg == "zygote64") {
            std::cout << "[INFO] INJECTED INTO ZYGOTE MAKE SURE YOU CHECK IF YOUR LIB IS IN THE RIGHT PROCESS\n";
            std::cout << "[INFO] START OR RESTART YOUR GAME IF ITS ALREADY OPENED\n";
        }
    }
    if(changed) {
        std::cout << "Resetting SELinux to old mode\n";
        NullUtils::SELINUX_SetEnforce(SELinuxMode);
        if (NullUtils::SELINUX_GetEnforce() != SELinuxMode)
            std::cerr << "Failed setting back SELinux mode\n";
    }
    return 0;
}


bool handle_args(int argc, char* argv[], std::string &pkg, std::string &lib) {
    bool hasPkg = false;
    bool hasLib = false;
    bool zygoteinjection = false;
    for(int i = 0; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-p" || arg == "--p") {
            if(i + 1 < argc) {
                pkg = argv[++i];
                hasPkg = true;
            }
            else {
                std::cerr << "-p option requires the package name.\n";
                return false;
            }
        }
        else if (arg == "-lib" || arg == "--lib") {
            if(i + 1 < argc) {
                lib = argv[++i];
                hasLib = true;
            }
            else {
                std::cerr << "-lib option requires the library path.\n";
                return false;
            }
        }
        else if(arg == "-zyi" || arg == "--zyi") {
            if(i + 1 < argc) {
                zygoteinjection = true;
#if defined(__arm__) || defined(__i386__)
                pkg = "zygote";
#elif defined(__aarch64__) || defined(__x86_64__)
                pkg = "zygote64";
#endif
            }
        }
    }
    if(!hasLib || (!hasPkg && !zygoteinjection)) {
        std::cerr << "-lib and -p or -zyi options are required aborting.\n";
        return false;
    }
    if(zygoteinjection && hasPkg) {
        std::cerr << "do not use -p and -zyi together aborting.\n";
        return false;
    }

    return true;
}
