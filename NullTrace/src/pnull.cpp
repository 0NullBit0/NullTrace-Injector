#include "pnull.h"


bool NullTrace::ptraceWrite(pid_t pid, uintptr_t addr, const uint8_t* data, size_t len) {
    long buffer = 0;
    size_t remain = len % NullTrace::WORDSIZE;
    size_t amount = len / NullTrace::WORDSIZE;

    for(int i = 0; i < amount; i++, buffer = 0) {
        for(int j = 0; j < NullTrace::WORDSIZE; j++)
            buffer |= ((long) data[j + i * NullTrace::WORDSIZE]) << (NullTrace::BYTESIZE * (j + i * NullTrace::WORDSIZE));


        if(ptrace(PTRACE_POKEDATA, pid, i * NullTrace::WORDSIZE + addr, buffer) == -1) {
            return false;
        }
    }
    if(remain > 0) {
        size_t leftToWord = NullTrace::WORDSIZE - remain;
        uint8_t oldAfterRemain[leftToWord];
        if(!NullTrace::ptraceRead(pid, amount * NullTrace::WORDSIZE + remain + addr, oldAfterRemain, leftToWord))
            return false;

        buffer = 0;

        for(int i = 0; i < remain; i++)
            buffer |= ((long)data[i + amount * NullTrace::WORDSIZE]) << (NullTrace::BYTESIZE * i);
        for(int i = 0; i < leftToWord; i++)
            buffer |= ((long)oldAfterRemain[i]) << (NullTrace::BYTESIZE * (remain + i));


        if(ptrace(PTRACE_POKEDATA, pid, amount * NullTrace::WORDSIZE + addr, buffer) == -1) {
            return false;
        }
    }

    return true;
}

bool NullTrace::ptraceRead(pid_t pid, uintptr_t addr, uint8_t* data, size_t len) {

    long buffer = 0;
    size_t remain = len % NullTrace::WORDSIZE;
    size_t amount = len / NullTrace::WORDSIZE;

    for(int i = 0; i < amount; i++, buffer = 0) {
        errno = 0;
        buffer = ptrace(PTRACE_PEEKDATA, pid, i * NullTrace::WORDSIZE + addr, NULL);
        if(errno) {
            std::cerr << "[NullTrace] Encountered an error: " << strerror(errno) << "\n";
            return false;
        }
        for(int j = 0; j < NullTrace::WORDSIZE; j++) {
            data[j + NullTrace::WORDSIZE * i] = (buffer >> NullTrace::BYTESIZE * j) & 0xFF;
        }
    }
    if(remain > 0) {
        errno = 0;
        buffer = ptrace(PTRACE_PEEKDATA, pid, amount * NullTrace::WORDSIZE + addr, NULL);
        if(errno) {
            std::cerr << "[NullTrace] Encountered an error: " << strerror(errno) << "\n";
            return false;
        }
        for(int i = 0; i < remain; i++)  {
            data[i + NullTrace::WORDSIZE * amount] = (buffer >> NullTrace::BYTESIZE * i) & 0xFF;
        }
    }
    return true;
}




uintptr_t NullTrace::ptraceRemoteCall(pid_t pid, uintptr_t addr, uintptr_t* argv, size_t argc, uintptr_t retAddr) {
    struct regs_s regs{}, oldRegs{};

    if(!NullTrace::ptraceGetRegs(pid, regs)) {
        std::cerr << "[NullTrace] failed remote calling couldnt get regs\n";
        return 0;
    }

    std::memcpy(&oldRegs, &regs, sizeof(regs));
#if defined(__arm__)
        for(int i = 0; (i < argc) && (i < 4); i++) {
            regs.uregs[i] = argv[i];
        }

        if(argc > 4) {
            regs.ARM_sp -= sizeof(uintptr_t) * (argc - 4);
            uintptr_t stack = regs.ARM_sp;
            for(int i = 4; i < argc; i++) {
                uintptr_t arg = argv[i];
                if(!ptraceWrite(pid, stack, (uint8_t*)&arg, sizeof(uintptr_t))) {

                    return 0;
                }
                stack += sizeof(uintptr_t);
            }
        }

        regs.ARM_pc = addr;
        // handeling arm/thumb mode
        if(regs.ARM_pc & 1) {
            regs.ARM_pc &= (~1u);
            regs.ARM_cpsr |= CPSRTMASK;
        } else {
            regs.ARM_cpsr &= ~CPSRTMASK;
        }
        regs.ARM_lr = retAddr; //lr register
#elif defined(__aarch64__)
        for(int i = 0; (i < argc) && (i < 8); i++) {
            regs.regs[i] = argv[i];
        }
        if(argc > 8) {
            regs.sp -= sizeof(uintptr_t) * (argc - 8);
            uintptr_t stack = regs.sp;
            for(int i = 8; i < argc; i++) {
                uintptr_t arg = argv[i];
                if(!ptraceWrite(pid, stack, (uint8_t*)&arg, sizeof(uintptr_t))) {

                    return 0;
                }
                stack += sizeof(uintptr_t);
            }
        }

        regs.pc = addr;
        regs.regs[30] = retAddr; //lr register
#elif defined(__i386__)
        regs.esp -=  sizeof(uintptr_t) * argc;
        uintptr_t stack = regs.esp;
        for(int i = 0; i < argc; i++) {
            uintptr_t arg = argv[i];
            if(!ptraceWrite(pid, stack, (uint8_t*)&arg, sizeof(uintptr_t))) {

                return 0;
            }
            stack += sizeof(uintptr_t);
        }

        uintptr_t lr = retAddr;
        regs.esp -= sizeof(uintptr_t);
        if(!ptraceWrite(pid, regs.esp, (uint8_t*)&lr, sizeof(uintptr_t))) {

            return 0;
        }
        regs.eip = addr;
#elif defined(__x86_64__) // credits: @reveny on Github
        //Alignment
        uintptr_t space = sizeof(uintptr_t);
        if(argc > 6) {
            space += sizeof(uintptr_t) * (argc - 6);
        }
        while( ((regs.rsp - space - 8) & 0xF) != 0 ) {
            regs.rsp--;
        }
        for(int i = 0; (i < argc) && (i < 6); i++) {
            uintptr_t arg = argv[i];
            switch (i) {
                case 0: regs.rdi = arg; break;
                case 1: regs.rsi = arg; break;
                case 2: regs.rdx = arg; break;
                case 3: regs.rcx = arg; break;
                case 4: regs.r8 = arg; break;
                case 5: regs.r9 = arg; break;
            }
        }

        if(argc > 6) {
            regs.rsp -= sizeof(uintptr_t) * (argc - 6);
            uintptr_t stack = regs.rsp;
            for(int i = 6; i < argc; i++) {
                uintptr_t arg = argv[i];
                if(!ptraceWrite(pid, stack, (uint8_t*)&arg, sizeof(uintptr_t))) {

                    return 0;
                }
                stack += sizeof(uintptr_t);
            }
        }

        uintptr_t lr = retAddr;
        regs.rsp -= sizeof(uintptr_t);
        if(!ptraceWrite(pid, regs.rsp, (uint8_t*)&lr, sizeof(uintptr_t))) {
            std::cerr << "[NullTrace] failed remote calling couldnt write\n";
            return 0;
        }


        regs.rip = addr;
        regs.rax = 0;
        regs.orig_rax = 0;
    #else
        #error Unsupported Device
#endif

    if(!NullTrace::ptraceSetRegs(pid, regs)) {
        std::cerr << "[NullTrace] failed remote calling couldnt set regs\n";
        return 0;
    }



    if(ptrace(PTRACE_CONT, pid, NULL, NULL) == -1) {
        std::cerr << "[NullTrace] failed remote calling couldnt PTRACE_CONTINUE\n";
        return 0;
    }

    int status;
    waitpid(pid, &status, WUNTRACED);

    while(WSTOPSIG(status) != SIGSEGV) {
        if(ptrace(PTRACE_CONT, pid, NULL, NULL) == -1) {
            std::cerr << "[NullTrace] failed remote calling couldnt PTRACE_CONTINUE\n";
            return 0;
        }
        waitpid(pid, &status, WUNTRACED);
    }

    if(!NullTrace::ptraceGetRegs(pid, regs)) {
        std::cerr << "[NullTrace] failed remote calling couldnt get regs\n";
        return 0;
    }


    if(!NullTrace::ptraceSetRegs(pid, oldRegs)) {
        std::cerr << "[NullTrace] failed remote calling couldnt set regs\n";
        return 0;
    }

#if defined(__arm__)
        return regs.ARM_r0;
#elif defined(__aarch64__)
        return regs.regs[0];
#elif defined(__i386__)
        return regs.eax;
#elif defined(__x86_64__)
        return regs.rax;
#endif
}


bool NullTrace::ptraceGetRegs(pid_t pid, regs_s &regs) {
#if defined(__LP64__)
        struct iovec iov{};
        iov.iov_base = &regs;
        iov.iov_len = sizeof(regs);
        if(ptrace(PTRACE_GETREGSET, pid, NT_PRSTATUS, &iov) == -1) {

            return false;
        }
        return true;
#else
        if(ptrace(PTRACE_GETREGS, pid, nullptr, &regs) == -1) {

            return false;
        }
        return true;
#endif
}

bool NullTrace::ptraceSetRegs(pid_t pid, regs_s &regs) {
#if defined(__LP64__)
        struct iovec iov{};
        iov.iov_base = &regs;
        iov.iov_len = sizeof(regs);
        if (ptrace(PTRACE_SETREGSET, pid, NT_PRSTATUS, &iov) == -1) {

            return false;
        }
        return true;
#else
        if(ptrace(PTRACE_SETREGS, pid, nullptr, &regs) == -1) {

            return false;
        }
        return true;
#endif
}