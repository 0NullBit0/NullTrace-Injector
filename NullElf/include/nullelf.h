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
    enum Architecture{EAARCH64, EARM, EI386, EX86_64};
    enum SymbolResMode{DEFAULT, CONTAINS};

    uintptr_t getAddrSym(const char* path, const char* symbol, SymbolResMode mode = DEFAULT);

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
    uintptr_t searchSymbolTable(char* data, const char* symbol, NullElf::SymbolResMode mode) {
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
            switch(mode) {
                case NullElf::DEFAULT:
                    if (std::strcmp(symbol, strtable + symbols[i].st_name) == 0) {
                        return (uintptr_t) (symbols[i].st_value);
                    }
                    break;
                case NullElf::CONTAINS:
                    if (std::strstr(strtable + symbols[i].st_name, symbol) != nullptr) {
                        return (uintptr_t) (symbols[i].st_value);
                    }
                    break;
            }
        }

        std::cerr << "[NullElf] " << symbol <<" not found\n";

        return 0;
    }


}
