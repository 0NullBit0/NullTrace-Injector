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

    if(SELinuxMode == 0) std::cout << "SELinux already permissive\n";
    else if(SELinuxMode == 1) {
        std::cout << "SELinux enforcing trying to set permissive\n";
        NullUtils::SELINUX_SetEnforce(0);
        SELinuxMode = NullUtils::SELINUX_GetEnforce();
        if(SELinuxMode == 0) std::cout << "Successfully set SELinux permissive\n";
        else std::cerr << "Failed setting SELinux permissive\n";
    }
    else std::cerr << "Couldn't determine SELinux status\n";

    std::cout << "Starting Injection\n";

    if(proc.injectLibrary(lib)) {
        std::cout << "Successfully injected library\n";
    }
    if(SELinuxMode == 0) {
        std::cout << "Resetting SELinux to enforcing\n";
        NullUtils::SELINUX_SetEnforce(1);
        SELinuxMode = NullUtils::SELINUX_GetEnforce();
        if(SELinuxMode == 0) std::cerr << "Failed setting back SELinux to enforcing\n";
    }
    return 0;
}


bool handle_args(int argc, char* argv[], std::string &pkg, std::string &lib) {
    bool hasPkg = false;
    bool hasLib = false;

    for(int i = 0; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-p") {
            if(i + 1 < argc) {
                pkg = argv[++i];
                hasPkg = true;
            }
            else {
                std::cerr << "-p option requires the package name.\n";
                return false;
            }
        }
        else if (arg == "-lib") {
            if(i + 1 < argc) {
                lib = argv[++i];
                hasLib = true;
            }
            else {
                std::cerr << "-lib option requires the library path.\n";
                return false;
            }
        }
    }
    if(!hasLib || !hasPkg) {
        std::cerr << "-lib and -p options are required aborting.\n";
        return false;
    }

    return true;
}
