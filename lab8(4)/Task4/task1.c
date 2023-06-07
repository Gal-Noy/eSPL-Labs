#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INSIZE 128

char input[INSIZE];

typedef struct
{
    char debug_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
    char display_mode;
} state;

typedef struct fun_desc
{
    char *name;
    void (*fun)(state *s);
} fun_desc;

void toggle_debug_mode(state *s)
{
    if (s->debug_mode == '0')
    {
        s->debug_mode = '1';
        printf("\n%s", "Debug flag now on.\n");
    }
    else
    {
        s->debug_mode = '0';
        printf("\n%s", "Debug flag now off.\n");
    }
}
void set_file_name(state *s)
{
    char name[INSIZE];

    printf("\nEnter file name: ");
    fgets(input, INSIZE, stdin);
    sscanf(input, "%s", name);

    name[strcspn(name, "\n")] = '\0';
    strcpy(s->file_name, name);

    if (s->debug_mode == '1')
        fprintf(stderr, "\nDebug: file name set to '%s'.", s->file_name);
}
void set_unit_size(state *s)
{
    int u_size;

    printf("\nEnter unit size: ");
    fgets(input, INSIZE, stdin);
    sscanf(input, "%d", &u_size);

    if (u_size != 1 && u_size != 2 && u_size != 4)
    {
        fprintf(stderr, "%s", "Invalid unit size.");
        return;
    }

    s->unit_size = u_size;

    if (s->debug_mode == '1')
        fprintf(stderr, "Debug: set size to %d\n", s->unit_size);
}
void load_into_memory(state *s)
{
    FILE *file;
    unsigned int location;
    size_t length;

    if (strcmp(s->file_name, "") == 0)
    {
        fprintf(stderr, "%s", "Error: File name is empty.\n");
        return;
    }

    file = fopen(s->file_name, "r+");
    if (!file)
    {
        fprintf(stderr, "Error: Failed to open file '%s' for reading.\n", s->file_name);
        return;
    }

    printf("Enter: <location> <length>: ");
    fgets(input, INSIZE, stdin);
    sscanf(input, "%x %d", &location, &length);

    if (s->debug_mode == '1')
    {
        printf("\nFile Name: %s\n", s->file_name);
        printf("Location: 0x%X\n", location);
        printf("Length: %d\n", length);
    }

    fseek(file, location, SEEK_SET);
    fread(s->mem_buf, s->unit_size, length, file);
    s->mem_count = s->unit_size * length;

    printf("\nLoaded %d units into memory.\n", length);
    fclose(file);
}
void toggle_display_mode(state *s)
{
    if (s->display_mode == 'd')
    {
        s->display_mode = 'x';
        printf("\n%s", "Display flag now on, hexadecimal representation.\n");
    }
    else
    {
        s->display_mode = 'd';
        printf("\n%s", "Display flag now off, decimal representation.\n");
    }
}
void print_memory(void *buff, int u, state *s)
{
    int u_size = s->unit_size;
    char display_mode = s->display_mode;

    void *end = buff + (u_size * u);
    static char *hex_formats[] = {"%#hhx\n", "%#hx\n", "No such unit", "%#x\n"};
    static char *dec_formats[] = {"%#hhd\n", "%#hd\n", "No such unit", "%#d\n"};

    if (display_mode == 'd')
        printf("Decimal\n=======\n");
    else
        printf("Hexadecimal\n=======\n");

    while (buff < end)
    {
        if (display_mode == 'd') // Decimal
            printf(dec_formats[u_size - 1], *(int *)buff);
        else // Hexadecimal
            printf(hex_formats[u_size - 1], *(int *)buff);

        buff += u_size;
    }
}
void memory_display(state *s)
{
    int u;
    unsigned int address;

    printf("Enter: <address> <units>: ");
    fgets(input, INSIZE, stdin);
    sscanf(input, "%x %d", &address, &u);

    if (address == 0)
        print_memory(&(s->mem_buf), u, s);
    else
        print_memory(&address, u, s);
}
void save_into_file(state *s)
{
    FILE *file;
    unsigned int source_address, target_location;
    size_t length;

    if (strcmp(s->file_name, "") == 0)
    {
        printf("Error: File name is empty.\n");
        return;
    }

    file = fopen(s->file_name, "r+b");
    if (!file)
    {
        printf("Error: Failed to open file '%s' for writing.\n", s->file_name);
        return;
    }

    printf("Enter <source-address> <target-location> <length>: ");
    fgets(input, INSIZE, stdin);
    sscanf(input, "%x %x %d", &source_address, &target_location, &length);

    if (s->debug_mode == '1')
    {
        printf("\nFile name: %s\n", s->file_name);
        printf("Source address: 0x%X\n", source_address);
        printf("Target location: 0x%X\n", target_location);
        printf("Length: %d\n", length);
    }

    fseek(file, target_location, SEEK_SET);

    if (source_address == 0)
        fwrite(&s->mem_buf, s->unit_size, length, file);
    else
        fwrite(&source_address, s->unit_size, length, file);

    printf("Wrote %d units into file\n", length);
    fclose(file);
}
void memory_modify(state *s)
{
    unsigned int location, val;

    printf("Enter <location> <val>: ");
    fgets(input, INSIZE, stdin);
    sscanf(input, "%x %x", &location, &val);

    if (s->debug_mode == '1')
    {
        printf("\nLocation: %X\n", location);
        printf("Val: 0x%X\n", val);
    }

    memcpy(&s->mem_buf[location], &val, s->unit_size);
}
void quit(state *s)
{
    if (s->debug_mode == '1')
        fprintf(stderr, "%s", "Debug: quitting.");
    free(s);
    exit(EXIT_SUCCESS);
}
void print_menu(fun_desc menu[], state *s)
{
    if (s->debug_mode == '1')
    {
        fprintf(stderr, "%s", "Debug Information:\n");
        fprintf(stderr, "Unit Size: %d\n", s->unit_size);
        fprintf(stderr, "File Name: %s\n", s->file_name);
        fprintf(stderr, "Memory Count: %d\n", s->mem_count);
        if (s->display_mode == 'd')
            fprintf(stderr, "%s", "Display Mode: Decimal\n");
        else
            fprintf(stderr, "%s", "Display Mode: Hexadecimal\n");
    }

    printf("\nPlease choose a function:\n");
    for (int i = 0; i < 9; i++)
        printf("%d-%s\n", i, menu[i].name);
    printf("Option: ");
}

int main(int argc, char **argv)
{
    int idx;

    // Initialize menu
    struct fun_desc menu[] = {{"Toggle Debug Mode", &toggle_debug_mode},
                              {"Set File Name", &set_file_name},
                              {"Set Unit Size", &set_unit_size},
                              {"Load Into Memory", &load_into_memory},
                              {"Toggle Display Mode", &toggle_display_mode},
                              {"Memory Display", &memory_display},
                              {"Save Into File", &save_into_file},
                              {"Memory Modify", &memory_modify},
                              {"Quit", &quit},
                              {NULL, NULL}};

    // Initialize state
    state *s = malloc(sizeof(state));
    s->debug_mode = '0';
    s->file_name[0] = '\0';
    s->unit_size = 1;
    s->mem_buf[0] = '\0';
    s->mem_count = (size_t)0;
    s->display_mode = 'd'; // Decimal

    print_menu(menu, s);

    while (fgets(input, INSIZE, stdin))
    {
        idx = atoi(input);
        if (idx < 0 || idx > 8)
            fprintf(stderr, "%s", "\nNot within bounds.\n");
        else
            menu[idx].fun(s);

        printf("\n");
        print_menu(menu, s);
    }

    quit(s);
    return 0;
}
