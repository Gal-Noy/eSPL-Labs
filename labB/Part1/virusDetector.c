#include <stdio.h>
#include <stdlib.h>

typedef struct virus
{
    unsigned short SigSize;
    char virusName[16];
    unsigned char *sig;
} virus;

typedef struct link link;

struct link
{
    link *nextVirus;
    virus *vir;
};

typedef struct fun_desc
{
    char *name;
    link *(*fun)(link *virus_list, FILE *file);
} fun_desc;

void PrintHex(unsigned char *buffer, int length)
{
    for (int i = 0; i < length; i++)
        printf("%02X ", buffer[i]);
}

virus *readVirus(FILE *file)
{
    virus *v;
    v = malloc(sizeof(struct virus));

    if (fread(v, 1, 18, file) != 0)
    {
        v->sig = malloc(v->SigSize);
        fread(v->sig, 1, v->SigSize, file);
    }

    return v;
}

void printVirus(virus *virus, FILE *output)
{
    fprintf(output, "Virus name: %s\n", virus->virusName);
    fprintf(output, "Virus size: %d\n", virus->SigSize);
    fprintf(output, "signature:\n");
    PrintHex(virus->sig, virus->SigSize);
    fprintf(output, "\n\n");
}

void free_virus(virus *v)
{
    if (v)
    {
        if (v->sig)
            free(v->sig);
        free(v);
    }
}
void list_free(link *virus_list)
{
    link *curr = virus_list;

    if (curr)
    {
        list_free(curr->nextVirus);
        free_virus(curr->vir);
        free(curr);
    }
    return;
}

void list_print(link *virus_list, FILE *file)
{
    link *curr = virus_list;
    while (curr)
    {
        printVirus(curr->vir, file);
        curr = curr->nextVirus;
    }
    list_free(curr);
}

link *list_append(link *virus_list, virus *data)
{
    if (virus_list)
    {
        virus_list->nextVirus = list_append(virus_list->nextVirus, data);
        return virus_list;
    }
    else
    {
        link *data_link = malloc(sizeof(link));
        data_link->vir = data;
        return data_link;
    }
}

int getSize(FILE *file)
{
    int length;

    fseek(file, 0, SEEK_END);
    length = ftell(file);
    rewind(file);

    return length;
}

link *load_signatures(link *virus_list, FILE *file)
{
    char *fileName = NULL, buf[BUFSIZ], buffer[4];
    int file_size, bytes;
    link *curr = NULL;
    virus *v = NULL;

    printf("Enter signature file name: ");
    fgets(buf, sizeof(buf), stdin);
    sscanf(buf, "%ms", &fileName);

    file = fopen(fileName, "rb");
    free(fileName);

    if (!file)
    {
        printf("Error: could not open file");
        return NULL;
    }

    file_size = getSize(file);
    fread(&buffer, 1, 4, file);
    bytes = 4;

    while (bytes < file_size)
    {
        v = readVirus(file);
        curr = list_append(curr, v);
        bytes += 18 + v->SigSize;
    }

    fclose(file);

    return curr;
}

link *print_signatures(link *virus_list, FILE *file)
{
    list_print(virus_list, stdout);
    return virus_list;
}

link *detect_viruses(link *virus_list, FILE *file)
{
    return virus_list;
}

link *fix_file(link *virus_list, FILE *file)
{
    return virus_list;
}

link *quit(link *virus_list, FILE *file)
{
    return virus_list;
}

int main(int argc, char **argv)
{
    FILE *file = NULL;
    link *virus_list = NULL;
    int idx;
    char *line;

    struct fun_desc menu[] = {{"Load signatures", &load_signatures}, {"Print signatures", &print_signatures}, {"Detect viruses", &detect_viruses}, {"Fix file", &fix_file}, {"Quit", &quit}, {NULL, NULL}};

    printf("Please choose a function:\n");
    for (int i = 0; i < 5; i++)
        printf("%d) %s\n", i + 1, menu[i].name);
    printf("Option: ");

    line = malloc(sizeof(char) * 256);

    while ((fgets(line, sizeof(line), stdin)) != NULL)
    {
        idx = atoi(line);

        if (idx < 1 || idx > 5)
        {
            fprintf(stderr, "Not within bounds.\n");
            goto end_program;
        }

        printf("Within bounds.\n");

        virus_list = menu[idx - 1].fun(virus_list, file);
        if (!virus_list)
            goto end_program;

        printf("DONE.\n");

        printf("\nPlease choose a function:\n");
        for (int i = 0; i < 5; i++)
            printf("%d) %s\n", i + 1, menu[i].name);
        printf("Option: ");
    }

end_program:
    if (virus_list)
        list_free(virus_list);
    if (line)
        free(line);
    if (file)
        fclose(file);
    return 0;
}