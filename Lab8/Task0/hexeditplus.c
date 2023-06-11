#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LINESIZE 128

char line[LINESIZE];

typedef struct
{
    char debug_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
    /*Any additional fields you deem necessary*/
} state;

typedef struct fun_desc
{
    char *name;
    void (*fun)(state *s); // TODO: change signature
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

    printf("\nEnter file name: ");
    if (fgets(line, LINESIZE, stdin) == NULL)
    {
        fprintf(stderr, "%s", "Invalid file name.");
        return;
    }
    if (line[0] == '\n')
    {
        fprintf(stderr, "%s", "Invalid file name.");
        return;
    }

    line[strcspn(line, "\n")] = '\0';
    strcpy(s->file_name, line);

    if (s->debug_mode == '1')
        fprintf(stderr, "Debug: file name set to '%s'\n", s->file_name);
}
void set_unit_size(state *s)
{
    int u_size;

    printf("\nEnter unit size: ");
    fgets(line, LINESIZE, stdin);

    if (line[0] == '\n')
    {
        fprintf(stderr, "%s", "Invalid unit size.");
        return;
    }

    u_size = atoi(line);
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
    fprintf(stderr, "%s", "Not Implemented yet.");
}
void toggle_display_mode(state *s)
{
    fprintf(stderr, "%s", "Not Implemented yet.");
}
void memory_display(state *s)
{
    fprintf(stderr, "%s", "Not Implemented yet.");
}
void save_into_file(state *s)
{
    fprintf(stderr, "%s", "Not Implemented yet.");
}
void memory_modify(state *s)
{
    fprintf(stderr, "%s", "Not Implemented yet.");
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
    s->mem_count = 0;

    print_menu(menu, s);

    while ((fgets(line, LINESIZE, stdin)) != NULL)
    {
        idx = atoi(line);
        if (idx < 0 || idx > 8)
            fprintf(stderr, "%s", "Not within bounds.\n");

        menu[idx].fun(s);
        // virus_list = menu[idx - 1].fun(virus_list, file_name);

        // if (!virus_list)
        // {
        //     if (idx == 5)
        //         printf("Goodbye.\n");
        //     goto end_program;
        // }

        printf("\n");
        print_menu(menu, s);
    }

    quit(s);
    return 0;
}
