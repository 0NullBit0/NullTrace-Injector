


#include "../../NullTrace/app/include/nullproc.h"
#include "../../NullTrace/app/include/nullutils.h"


int main(int argc, char* argv[]) {

    NullProcess::Process proc;

    while(!proc.setProcByName("com.nullbit.pocinject")) {

    }
    printf("Found target process\n");

    /*
    proc.writeProcessMemory("1B FF", 0x1);
    proc.readProcessMemory(0xFFF, 5);
    proc.call<int>(0xFFFF, 1,2,3);
    */

    if(proc.injectLibrary("/data/local/tmp/libhacklib.so")) {
        printf("Successfully injected library\n");
    }

    return 0;
}

