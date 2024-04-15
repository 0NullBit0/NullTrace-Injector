#pragma once

#include <elf.h>
#include <cstdint>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstdio>
#include <iostream>

namespace NullElf {
    constexpr int EAARCH64 = 0;
    constexpr int EARM     = 1;
    constexpr int EI386    = 2;
    constexpr int EX86_64  = 3;


    uintptr_t getAddrSym(const char* path, const char* symbol);

    int getLibraryArch(const char* path);

}

namespace NullElfUtils {


    template <typename Elf_Ehdr>
    int getArch(char* data) {
        Elf_Ehdr* ehdr = reinterpret_cast<Elf_Ehdr*>(data);
        int arch = -1;
        switch(ehdr->e_machine) {
            case EM_AARCH64:
                arch = NullElf::EAARCH64;
                break;
            case EM_ARM:
                arch = NullElf::EARM;
                break;
            case EM_386:
                arch = NullElf::EI386;
                break;
            case EM_X86_64:
                arch = NullElf::EX86_64;
                break;
            default:
                arch = -3;
                break;
        }
        return arch;
    }

    template <typename Elf_Ehdr, typename Elf_Shdr, typename Elf_Sym>
    uintptr_t searchSymbolTable(char* data, const char* symbol) {
        Elf_Ehdr* ehdr = reinterpret_cast<Elf_Ehdr*>(data);
        Elf_Shdr* shdr = reinterpret_cast<Elf_Shdr*>(data + ehdr->e_shoff);
        Elf_Shdr* sh_strtable = &shdr[ehdr->e_shstrndx];
        const char* sh_strtable_p = data + sh_strtable->sh_offset;

        Elf_Shdr* symtab_shdr = nullptr;
        const char* strtable = nullptr;
        for(int i = 0; i < ehdr->e_shnum; i++) {
            if(shdr[i].sh_type == SHT_DYNSYM) {
                symtab_shdr = &shdr[i];
                strtable = data + shdr[symtab_shdr->sh_link].sh_offset;
                break;
            }
        }
        if(symtab_shdr == nullptr) {
            std::cerr << "[NullElf] failed to find symbol table\n";
            return 0;
        }

        Elf_Sym* symbols = reinterpret_cast<Elf_Sym*>(data + symtab_shdr->sh_offset);
        int symCount = symtab_shdr->sh_size / symtab_shdr->sh_entsize;
        for(int i = 0; i < symCount; i++) {
            if(strcmp(symbol, strtable + symbols[i].st_name) == 0) {
                return (uintptr_t) (symbols[i].st_value);
            }
        }

        std::cout << "[NullElf] " << symbol <<" not found\n";

        return 0;
    }


}
