#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>

#define INSIZE 128

char input[INSIZE], file_name[INSIZE];
int debug_mode = 0, fd = -1;
void *map_start;
Elf32_Ehdr *header;
size_t length;

typedef struct fun_desc
{
    char *name;
    void (*fun)();
} fun_desc;

int load_file(char *file_name)
{
    int fd;

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
char *get_scheme()
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
void print_elf_info()
{
    printf("\nMagic numbers: %x %x %x\n", header->e_ident[EI_MAG0], header->e_ident[EI_MAG1], header->e_ident[EI_MAG2]);

    // Check if the magic numbers indicate an ELF file
    if (header->e_ident[EI_MAG0] != ELFMAG0 ||
        header->e_ident[EI_MAG1] != ELFMAG1 ||
        header->e_ident[EI_MAG2] != ELFMAG2)
    {
        printf("Not a valid ELF file.\n");
        munmap(map_start, length);
        close(fd);
        fd = -1;
    }

    // Get the encoding scheme of the object file
    printf("Encoding scheme: %s", get_scheme());
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
        printf("\n%s", "Debug flag now on.");
    }
    else
    {
        debug_mode = 0;
        printf("\n%s", "Debug flag now off.");
    }
}
void examine_elf_file()
{
    printf("\nEnter file name: ");

    fgets(input, INSIZE, stdin);
    sscanf(input, "%s", file_name);
    file_name[strcspn(file_name, "\n")] = '\0';

    fd = load_file(file_name);
    header = (Elf32_Ehdr *)map_start;

    print_elf_info();
}
void print_section_names() {}
void print_symbols() {}
void check_files() {}
void merge_files() {}
void quit()
{
    printf("Goodbye.");
    munmap(map_start, length);
    close(fd);
    exit(EXIT_SUCCESS);
}

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

    while (fgets(input, INSIZE, stdin))
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