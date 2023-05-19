#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINESIZE 256

int debug = 0, unit_size = 1;
char *file_name, line[LINESIZE];

typedef struct fun_desc
{
    char *name;
    void (*fun)(char *input); // TODO: change signature
} fun_desc;

void quit(char *error)
{
    if (file_name)
        free(file_name);

    if (error)
    {
        fprintf(stderr, "%s", error);
        exit(EXIT_FAILURE);
    }
    exit(EXIT_SUCCESS);
}
void toggle_debug_mode(char *unused)
{
    debug = 1;
    printf("\n%s", "Debug mode toggled.");
}
void set_file_name(char *unused)
{
    while (1)
    {
        printf("\n%s", "Enter file name: ");
        fgets(line, LINESIZE, stdin);
        if (line[0] == '\n')
            fprintf(stderr, "%s", "Invalid file name.");
        else
            break;
    }

    file_name = malloc(strlen(line) + 1);
    line[strlen(line) - 1] = '\0';
    strcpy(file_name, line);
    printf("File name set to \"%s\".", file_name);
}
void set_unit_size(char *unused)
{
    int u_size;

    while (1)
    {
        printf("\n%s", "Enter unit size: ");
        fgets(line, LINESIZE, stdin);

        if (line[0] == '\n')
        {
            fprintf(stderr, "%s", "Invalid unit size.");
            continue;
        }

        u_size = atoi(line);
        if (u_size != 1 && u_size != 2 && u_size != 4)
            fprintf(stderr, "%s", "Invalid unit size.");
        else
            break;
    }

    unit_size = u_size;
    printf("Unit size set to %d.", unit_size);
}
void load_into_memory(char *unused) {}
void toggle_display_mode(char *unused) {}
void memory_display(char *unused) {}
void save_into_file(char *unused) {}
void memory_modify(char *unused) {}

void print_menu(fun_desc menu[])
{
    printf("Please choose a function:\n");
    for (int i = 0; i < 9; i++)
        printf("%d-%s\n", i, menu[i].name);
    printf("Option: ");
}

int main(int argc, char **argv)
{
    int idx;

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

    print_menu(menu);

    while ((fgets(line, LINESIZE, stdin)) != NULL)
    {
        idx = atoi(line);
        if (idx < 0 || idx > 8)
            quit("Not within bounds.\n");

        menu[idx].fun(line);
        // virus_list = menu[idx - 1].fun(virus_list, file_name);

        // if (!virus_list)
        // {
        //     if (idx == 5)
        //         printf("Goodbye.\n");
        //     goto end_program;
        // }

        printf("\n\n");
        print_menu(menu);
    }

    quit(NULL);
    return 0;
}
