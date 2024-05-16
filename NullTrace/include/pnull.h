#pragma once

#include <cstdio>
#include <sys/ptrace.h>
#include <asm/ptrace.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <sys/user.h>
#include <cstring>
#include <elf.h>
#include <signal.h>
#include <iostream>
#include <iomanip>
#include <cstring>

namespace NullTrace {
#if defined(__aarch64__) || defined(__x86_64__) || defined(__i386__)
#define regs_s user_regs_struct
#elif defined(__arm__)
#define regs_s user_regs
#endif
    constexpr unsigned long WORDSIZE  = sizeof(long);
    constexpr unsigned long CPSRTMASK = (1u << 5);

    bool ptraceWrite(pid_t pid, uintptr_t addr, uint8_t* data, size_t len);
    bool ptraceRead  (pid_t pid, uintptr_t addr, uint8_t* data, size_t len);
    uintptr_t ptraceRemoteCall(pid_t pid, uintptr_t addr, uintptr_t* argv, size_t argc, uintptr_t retAddr = 0);
    bool ptraceGetRegs(pid_t pid, regs_s &regs);
    bool ptraceSetRegs(pid_t pid, regs_s &regs);
}