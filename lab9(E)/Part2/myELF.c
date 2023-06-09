#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>

#define BUFSIZE 128

typedef struct
{
    char name[BUFSIZE];
    int fd;
    void *map_start;
    Elf32_Ehdr *header;
    size_t len;
} elf_file;

typedef struct fun_desc
{
    char *name;
    void (*fun)();
} fun_desc;

elf_file *f1, *f2;
char input[BUFSIZE];
int debug_mode = 0, files_num = 0;

void quit()
{
    printf("Goodbye.");
    munmap(f1->map_start, f1->len);
    close(f1->fd);
    munmap(f2->map_start, f2->len);
    close(f2->fd);
    free(f1);
    free(f2);
    exit(EXIT_SUCCESS);
}
int load_file(elf_file *f)
{
    if ((f->fd = open(f->name, O_RDWR)) < 0)
    {
        perror("Failed to open file");
        return 0;
    }

    if ((f->len = lseek(f->fd, 0, SEEK_END)) == -1)
    {
        perror("Failed to determine file size");
        return 0;
    }

    if ((f->map_start = mmap(NULL, f->len, PROT_READ, MAP_SHARED, f->fd, 0)) == MAP_FAILED)
    {
        perror("Failed to map file into memory");
        return 0;
    }

    files_num++; // File loaded successfully
    return 1;
}
char *get_scheme(Elf32_Ehdr *header)
{
    switch (header->e_ident[EI_DATA])
    {
    case ELFDATA2LSB:
        return "2's complement, little endian\n";
        break;
    case ELFDATA2MSB:
        return "2's complement, big endian\n";
        break;
    default:
        return "Unknown\n";
        break;
    }
}
void print_elf_info(elf_file *f)
{
    Elf32_Ehdr *header = f->header;

    printf("\nMagic numbers: %x %x %x\n", header->e_ident[EI_MAG0], header->e_ident[EI_MAG1], header->e_ident[EI_MAG2]);

    // Check if the magic numbers indicate an ELF file
    if (header->e_ident[EI_MAG0] != ELFMAG0 ||
        header->e_ident[EI_MAG1] != ELFMAG1 ||
        header->e_ident[EI_MAG2] != ELFMAG2)
    {
        printf("Not a valid ELF file.\n");
        munmap(f->map_start, f->len);
        close(f->fd);
        f->fd = -1;
    }

    // Get the encoding scheme of the object file
    printf("Encoding scheme: %s", get_scheme(header));
    printf("Entry: 0x%x\n", header->e_entry);
    printf("Section header table offset: 0x%lx\n", (unsigned long)header->e_shoff);
    printf("Number of section header entries: %u\n", (unsigned int)header->e_shnum);
    printf("Size of each section header entry: %u\n", (unsigned int)header->e_shentsize);
    printf("Program header table offset: 0x%lx\n", (unsigned long)header->e_phoff);
    printf("Number of program header entries: %u\n", (unsigned int)header->e_phnum);
    printf("Size of each program header entry: %u\n", (unsigned int)header->e_phentsize);
}
void toggle_debug_mode()
{
    if (!debug_mode)
    {
        debug_mode = 1;
        printf("\n%s", "Debug flag now on.\n");
    }
    else
    {
        debug_mode = 0;
        printf("\n%s", "Debug flag now off.\n");
    }
}
void eef(elf_file *f)
{
    if (files_num == 0)
        printf("\nEnter first file name: ");
    else
        printf("\nEnter second file name: ");

    fgets(input, BUFSIZE, stdin);
    sscanf(input, "%s", f->name);
    f->name[strcspn(f->name, "\n")] = '\0';

    if (load_file(f))
    {
        f->header = (Elf32_Ehdr *)f->map_start;
        print_elf_info(f);
    }
}
void examine_elf_file()
{
    if (files_num == 0)
        eef(f1);
    else if (files_num == 1)
        eef(f2);
    else
        fprintf(stderr, "%s", "\nCannot examine more than 2 ELF files.\n");
}

char *type_to_string(int type)
{
    switch (type)
    {
    case 0:
        return "NULL";
    case 1:
        return "PROGBITS";
    case 2:
        return "SYMTAB";
    case 3:
        return "STRTAB";
    case 4:
        return "RELA";
    case 5:
        return "HASH";
    case 6:
        return "DYNAMIC";
    case 7:
        return "NOTE";
    case 8:
        return "NOBITS";
    case 9:
        return "REL";
    case 10:
        return "SHLIB";
    case 11:
        return "DYNSYM";
    case 0x6ffffffa:
        return "SUNW_move";
    case 0x6ffffffb:
        return "SUNW_COMDAT";
    case 0x6ffffffc:
        return "SUNW_syminfo";
    case 0x6ffffffd:
        return "SUNW_verdef";
    case 0x6ffffffe:
        return "SUNW_verneed";
    case 0x6fffffff:
        return "SUNW_versym";
    case 0x70000000:
        return "LOPROC";
    case 0x7fffffff:
        return "HIPROC";
    case 0x80000000:
        return "LOUSER";
    case 0xffffffff:
        return "HIUSER";
    }
    return "";
}
void psn(elf_file *f)
{
    int i;
    Elf32_Shdr *sections_table = (Elf32_Shdr *)(f->map_start + f->header->e_shoff);
    Elf32_Shdr *sections_names_raw = (Elf32_Shdr *)(sections_table + f->header->e_shstrndx);
    char *sections_names = f->map_start + sections_names_raw->sh_offset;

    printf("\nFILE %s\n", f->name);
    if (debug_mode)
    {
        printf("Sections table address: %p\n", sections_names);
        printf("Sections table entry: %p\n", sections_names_raw);
    }
    printf("%s %-20s %-12s %-12s %-12s %-12s\n",
           "[Nr]", "Name", "Address", "Offset", "Size", "Type");
    for (i = 0; i < f->header->e_shnum; i++)
    {
        Elf32_Shdr section = sections_table[i];

        printf("[%2d] %-20s %-12.08x %-12.06x %-12.06x %-12s\n",
               i, sections_names + section.sh_name, section.sh_addr, section.sh_offset, section.sh_size, type_to_string(section.sh_type));
    }
}
void print_section_names()
{
    if (files_num > 0)
    {
        psn(f1);
        if (files_num > 1)
            psn(f2);
    }
    else
        fprintf(stderr, "%s", "\nNo files loaded.\n");
}

char *get_section_idx(Elf32_Section index)
{
    switch (index)
    {
    case SHN_UNDEF:
        return "UND";
    case SHN_LORESERVE:
        return "LORESERVE";
    case SHN_HIPROC:
        return "HIPROC";
    case SHN_ABS:
        return "ABS";
    case SHN_COMMON:
        return "COMMON";
    case SHN_HIRESERVE:
        return "HIRESERVE";
    default:
        return NULL;
    }
}
Elf32_Shdr *get_table(elf_file *f, char *table_name)
{
    int i;
    Elf32_Shdr *curr_symbol, *sections_names_raw = f->map_start + f->header->e_shoff + (f->header->e_shstrndx * f->header->e_shentsize);
    char *curr_name;
    for (i = 0; i < f->header->e_shnum; i++)
    {
        curr_symbol = f->map_start + f->header->e_shoff + (i * f->header->e_shentsize);
        curr_name = f->map_start + sections_names_raw->sh_offset + curr_symbol->sh_name;
        if (strcmp(table_name, curr_name) == 0)
            return curr_symbol;
    }
    return NULL;
}
void print_table(elf_file *f, char *table_name)
{
    int i, symbols_num;
    Elf32_Sym *curr_symbol;
    Elf32_Shdr *sym_table = get_table(f, table_name),
               *strtab = get_table(f, ".strtab"),
               *shstrtab = get_table(f, ".shstrtab"),
               *section_entry;
    char *section_name, *symbol_name, *sec_idx;

    if (!sym_table)
    {
        fprintf(stderr, "\nError: Can't find symbol table '%s' offset\n", table_name);
        return;
    }

    symbols_num = sym_table->sh_size / sizeof(Elf32_Sym);

    printf("\nSymbol table '%s' contains %d entries:\n", table_name, symbols_num);
    printf("%s\t %-12s %-12s %-20s %-12s\n",
           "[Num]", "Value", "SecIdx", "SecName", "SymName");

    for (i = 0; i < symbols_num; i++)
    {
        curr_symbol = f->map_start + sym_table->sh_offset + (i * sizeof(Elf32_Sym));

        sec_idx = get_section_idx(curr_symbol->st_shndx);

        if (sec_idx)
            section_name = "";
        else
        {
            section_entry = f->map_start + f->header->e_shoff + (curr_symbol->st_shndx * f->header->e_shentsize);
            section_name = f->map_start + shstrtab->sh_offset + section_entry->sh_name;
        }

        symbol_name = f->map_start + strtab->sh_offset + curr_symbol->st_name;

        // Print data
        if (sec_idx)
            printf("[%2d]\t %-12.08x %-12s %-20s %-12s\n",
                   i, curr_symbol->st_value, sec_idx, section_name, symbol_name);
        else
            printf("[%2d]\t %-12.08x %-12d %-20s %-12s\n",
                   i, curr_symbol->st_value, curr_symbol->st_shndx, section_name, symbol_name);
    }
}
void ps(elf_file *f)
{
    printf("\nFILE %s\n", f->name);
    print_table(f, ".dynsym");
    print_table(f, ".symtab");
}

void print_symbols()
{
    if (files_num > 0)
    {
        ps(f1);
        if (files_num > 1)
            ps(f2);
    }
    else
        fprintf(stderr, "%s", "\nNo files loaded.\n");
}
void check_files() { printf("\nNot implemented yet.\n"); }
void merge_files() { printf("\nNot implemented yet.\n"); }

void print_menu(fun_desc menu[])
{
    if (debug_mode)
    {
        // TODO: Pring debug
    }

    printf("Please choose a function:\n");
    for (int i = 0; i < 7; i++)
        printf("%d-%s\n", i, menu[i].name);
    printf("Option: ");
}

int main(int argc, char **argv)
{
    int idx;
    f1 = malloc(sizeof(elf_file));
    f2 = malloc(sizeof(elf_file));

    // Initialize menu
    struct fun_desc menu[] = {{"Toggle Debug Mode", &toggle_debug_mode},
                              {"Examine ELF File", &examine_elf_file},
                              {"Print Section Names", &print_section_names},
                              {"Print Symbols", &print_symbols},
                              {"Check Files for Merge", &check_files},
                              {"Merge ELF Files", &merge_files},
                              {"Quit", &quit},
                              {NULL, NULL}};
    print_menu(menu);

    while (fgets(input, BUFSIZE, stdin))
    {
        idx = atoi(input);
        if (idx < 0 || idx > 6)
            fprintf(stderr, "%s", "\nNot within bounds.\n");
        else
            menu[idx].fun();

        printf("\n");
        print_menu(menu);
    }

    quit();
    return 0;
}