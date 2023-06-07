#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <elf.h>

void print_phdr_addr(Elf32_Phdr *phdr, int phdr_num)
{
    printf("Program header number %d at address %p\n", phdr_num, phdr);
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

    foreach_phdr(map_start, &print_phdr_addr, 0);

    munmap(map_start, length);
    close(fd);
    return 0;
}