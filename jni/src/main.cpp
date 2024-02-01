
#include "../NullTrace/include/nullproc.h"
#include "../NullTrace/include/nullutils.h"

int main(int argc, char* argv[]) {

    std::string p;
    std::string lib;
    int hasargs = 0;
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];

        if (arg == "-p") {
            if (i + 1 < argc) {
                p = argv[++i];
                hasargs += 1;
            } else {
                std::cerr << "-p option requires one argument." << std::endl;
                return -1;
            }
        } else if (arg == "-lib") {
            if (i + 1 < argc) {
                lib = argv[++i];
                hasargs += 1;
            } else {
                std::cerr << "-lib option requires one argument." << std::endl;
                return -1;
            }
        }
    }
    if(hasargs < 2) {
        std::cerr << "-lib and -p options are required quitting" << std::endl;
        return -1;
    }

    NullProcess::Process proc;

    while(!proc.setProcByName(p)) {
        sleep(1);
        printf("Waiting for process: %s\n", p.c_str());
    }

    printf("Found target process\n");

    printf("Starting Injection\n");

    if(proc.injectLibrary(lib)) {
        printf("Successfully injected library\n");
    }

    return 0;
}

