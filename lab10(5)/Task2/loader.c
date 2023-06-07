#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>

void *map_start;
Elf32_Ehdr *header;

extern int startup(int argc, char **argv, void (*start)());

char *get_flags(Elf32_Word flags)
{
    switch (flags)
    {
    case 0:
        return "";
    case 1:
        return "E";
    case 2:
        return "W";
    case 3:
        return "WE";
    case 4:
        return "R";
    case 5:
        return "RE";
    case 6:
        return "RW";
    case 7:
        return "RWE";
    }
    return "ERROR";
}
int get_prot_flags(Elf32_Word flags)
{
    int prot_flags = 0;

    if (flags & PF_R)
        prot_flags |= PROT_READ;

    if (flags & PF_W)
        prot_flags |= PROT_WRITE;

    if (flags & PF_X)
        prot_flags |= PROT_EXEC;

    return prot_flags;
}
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
    printf("%-12s %#-12x %#-12x %#-12x %#-12x %#-12x %-12s %-12d %-12d %#-12x\n",
           ptype_name(phdr->p_type),
           phdr->p_offset,
           phdr->p_vaddr,
           phdr->p_paddr,
           phdr->p_filesz,
           phdr->p_memsz,
           get_flags(phdr->p_flags),
           get_prot_flags(phdr->p_flags),
           MAP_SHARED,
           phdr->p_align);
}
void load_phdr(Elf32_Phdr *phdr, int fd)
{
    void *address, *vaddress;
    int offset, padding;

    if (phdr->p_type == PT_LOAD)
    {
        vaddress = (void *)(phdr->p_vaddr & 0xfffff000);
        offset = phdr->p_offset & 0xfffff000;
        padding = phdr->p_vaddr & 0xfff;
        address = mmap(vaddress, phdr->p_memsz + padding, get_prot_flags(phdr->p_flags), MAP_FIXED | MAP_PRIVATE, fd, offset);
        if (address == MAP_FAILED)
        {
            perror("mmap failed");
            exit(EXIT_FAILURE);
        }
        print_phdr_info(phdr, 0);
    }
    // if (phdr->p_type != PT_LOAD)
    //     return;
    // void *vadd = (void *)(phdr->p_vaddr & 0xfffff000);
    // int offset = phdr->p_offset & 0xfffff000;
    // int padding = phdr->p_vaddr & 0xfff;
    // int convertedFlag = get_prot_flags(phdr->p_flags);
    // void *temp;
    // if ((temp = mmap(vadd, phdr->p_memsz + padding, convertedFlag, MAP_FIXED | MAP_PRIVATE, fd, offset)) == MAP_FAILED)
    // {
    //     perror("mmap failed1");
    //     exit(-4);
    // }
    // print_phdr_info(phdr, 0);
}
int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg)
{
    Elf32_Phdr *phdr = (Elf32_Phdr *)(map_start + header->e_phoff);

    // Print readelf-l
    printf("%-12s %-12s %-12s %-12s %-12s %-12s %-12s %-12s %-12s %-12s\n",
           "Type", "Offset", "VirtAddr", "PhysAddr", "FileSiz", "MemSiz", "Flg", "ProtFlg", "MapFlg", "Align");

    for (int i = 0; i < header->e_phnum; i++)
        func(map_start + header->e_phoff + (i * header->e_phentsize), arg);

    return 0;
}
int load_file(char *file_name)
{
    int fd, length;

    if ((fd = open(file_name, O_RDWR)) < 0)
    {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    if ((length = lseek(fd, 0, SEEK_END)) == -1)
    {
        perror("Failed to determine file size");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if ((map_start = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0)) == MAP_FAILED)
    {
        perror("Failed to map file into memory");
        close(fd);
        exit(EXIT_FAILURE);
    }

    return fd;
}
int main(int argc, char *argv[])
{
    int fd;
    if (argc < 2)
    {
        printf("Usage: %s <elf_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    fd = load_file(argv[1]);
    header = (Elf32_Ehdr *)map_start;

    foreach_phdr(map_start, load_phdr, fd);
    startup(argc - 1, argv + 1, (void *)(header->e_entry));

    return 0;
}