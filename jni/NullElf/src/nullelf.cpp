#include "../include/nullelf.h"



uintptr_t NullElf::getAddrSym(const char* path, const char* symbol) {
    int fd = open(path, O_RDONLY);
    if(fd < 0) {
        std::cerr << "[NullElf] failed opening file\n";
        return 0;
    }

    struct stat sb;
    if(fstat(fd, &sb) < 0) {
        std::cerr << "[NullElf] failed getting file size\n";
        return 0;
    }
    
    char* data = reinterpret_cast<char*>(mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
    if(data == MAP_FAILED) {
        std::cerr << "[NullElf] failed mapping file\n";
        return 0;
    }

    unsigned char e_ident[EI_NIDENT];

    if(read(fd, &e_ident, EI_NIDENT) != EI_NIDENT) {
        std::cerr << "[NullElf] failed to read ELF ID\n";
        if(munmap(data, sb.st_size) == -1)
            std::cerr << "[NullElf] failed to unmap memory\n";
        return 0;
    }

    close(fd);

    if(e_ident[EI_CLASS] == ELFCLASS32) {
        uintptr_t address = NullElfUtils::searchSymbolTable<Elf32_Ehdr, Elf32_Shdr, Elf32_Sym>(data, symbol);

        if(munmap(data, sb.st_size) == -1)
            std::cerr << "[NullElf] failed to unmap memory\n";

        return address;
    }
    else if(e_ident[EI_CLASS] == ELFCLASS64) {
        uintptr_t address = NullElfUtils::searchSymbolTable<Elf64_Ehdr, Elf64_Shdr, Elf64_Sym>(data, symbol);

        if(munmap(data, sb.st_size) == -1)
            std::cerr << "[NullElf] failed to unmap memory\n";

        return address;
    }
    else {
        std::cerr << "[NullElf] unknown ELF class\n";
        if(munmap(data, sb.st_size) == -1)
            std::cerr << "[NullElf] failed to unmap memory\n";
        return 0;
    }
}

int NullElf::getLibraryArch(const char* path) {
    int fd = open(path, O_RDONLY);
    if(fd < 0) {
        std::cerr << "[NullElf] failed opening file\n";
        return -1;
    }

    struct stat sb;
    if(fstat(fd, &sb) < 0) {
        std::cerr << "[NullElf] failed getting file size\n";
        return -1;
    }

    char* data = reinterpret_cast<char*>(mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0));
    if(data == MAP_FAILED) {
        std::cerr << "[NullElf] failed mapping file\n";
        return -1;
    }

    unsigned char e_ident[EI_NIDENT];

    if(read(fd, &e_ident, EI_NIDENT) != EI_NIDENT) {
        std::cerr << "[NullElf] failed to read ELF ID\n";
        if(munmap(data, sb.st_size) == -1)
            std::cerr << "[NullElf] failed to unmap memory\n";
        return 0;
    }

    close(fd);

    if(e_ident[EI_CLASS] == ELFCLASS32) {
        int arch;
        if((arch = NullElfUtils::getArch<Elf32_Ehdr>(data) ) < 0) {
            std::cerr << "[NullElf] failed to get Arch\n";
            if(munmap(data, sb.st_size) == -1)
                std::cerr << "[NullElf] failed to unmap memory\n";
            return -1;
        }
        if(munmap(data, sb.st_size) == -1)
            std::cerr << "[NullElf] failed to unmap memory\n";
        return arch;

    }
    else if(e_ident[EI_CLASS] == ELFCLASS64) {
        int arch;
        if((arch = NullElfUtils::getArch<Elf64_Ehdr>(data) ) < 0) {
            std::cerr << "[NullElf] failed to get Arch\n";
            if(munmap(data, sb.st_size) == -1)
                std::cerr << "[NullElf] failed to unmap memory\n";
            return -1;
        }
        if(munmap(data, sb.st_size) == -1)
            std::cerr << "[NullElf] failed to unmap memory\n";
        return arch;
    }
    else {
        std::cerr << "[NullElf] unknown ELF class\n";
        if(munmap(data, sb.st_size) == -1)
            std::cerr << "[NullElf] failed to unmap memory\n";
        return -1;
    }

    if(munmap(data, sb.st_size) == -1)
        std::cerr << "[NullElf] failed to unmap memory\n";
    return -1;

}


