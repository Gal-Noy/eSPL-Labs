#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>

const char *ptype_name(Elf32_Word p_type)
{
    switch (p_type)
    {
    case PT_NULL:
        return "NULL";
    case PT_LOAD:
        return "LOAD";
    case PT_DYNAMIC:
        return "DYNAMIC";
    case PT_INTERP:
        return "INTERP";
    case PT_NOTE:
        return "NOTE";
    case PT_SHLIB:
        return "SHLIB";
    case PT_PHDR:
        return "PHDR";
    case PT_TLS:
        return "TLS";
    case PT_NUM:
        return "NUM";
    case PT_LOOS:
        return "LOOS";
    case PT_GNU_EH_FRAME:
        return "GNU_EH_FRAME";
    case PT_GNU_STACK:
        return "GNU_STACK";
    case PT_GNU_RELRO:
        return "GNU_RELRO";
    case PT_HIOS:
        return "HIOS";
    case PT_LOPROC:
        return "LOPROC";
    case PT_HIPROC:
        return "HIPROC";
    }
    return "";
}
void print_phdr_info(Elf32_Phdr *phdr, int phdr_num)
{
    char flags[6] = {' ', ' ', ' ', ' ', ' ', '\0'};
    int prot_flags = 0, map_flags = MAP_SHARED;

    if (phdr->p_flags & PF_R)
    {
        flags[0] = 'R';
        prot_flags |= PROT_READ;
    }
    if (phdr->p_flags & PF_W)
    {
        flags[2] = 'W';
        prot_flags |= PROT_WRITE;
    }
    if (phdr->p_flags & PF_X)
    {
        flags[4] = 'E';
        prot_flags |= PROT_EXEC;
    }
    if (phdr->p_flags & MAP_SHARED)
        map_flags |= MAP_SHARED;

    printf("%-12s %#-12x %#-12x %#-12x %#-12x %#-12x %-12s %-12d %-12d %#-12x\n",
           ptype_name(phdr->p_type),
           phdr->p_offset,
           phdr->p_vaddr,
           phdr->p_paddr,
           phdr->p_filesz,
           phdr->p_memsz,
           flags,
           prot_flags,
           map_flags,
           phdr->p_align);
}

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg)
{
    Elf32_Ehdr *header = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + header->e_phoff);

    for (int i = 0; i < header->e_phnum; i++)
        func(&phdr[i], i);

    return 0;
}

int main(int argc, char *argv[])
{
    char *file_name;
    void *map_start;
    size_t length;

    if (argc < 2)
    {
        printf("Usage: %s <elf_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    file_name = argv[1];
    int fd = open(file_name, O_RDONLY);
    if (fd == -1)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    length = lseek(fd, 0, SEEK_END);
    if (length == -1)
    {
        perror("Failed to determine file size");
        close(fd);
        exit(EXIT_FAILURE);
    }

    map_start = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
    if (map_start == MAP_FAILED)
    {
        perror("Failed to map file into memory");
        close(fd);
        return 1;
    }

    // Print readelf-l
    printf("%-12s %-12s %-12s %-12s %-12s %-12s %-12s %-12s %-12s %-12s\n",
           "Type",
           "Offset",
           "VirtAddr",
           "PhysAddr",
           "FileSiz",
           "MemSiz",
           "Flg",
           "ProtFlg",
           "MapFlg",
           "Align");
    foreach_phdr(map_start, &print_phdr_info, 0);

    munmap(map_start, length);
    close(fd);
    return 0;
}