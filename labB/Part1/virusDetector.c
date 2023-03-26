#include <stdio.h>
#include <stdlib.h>

void PrintHex(unsigned char *buffer, int length)
{
    for (int i = 0; i < length; i++)
        printf("%02X ", buffer[i]);
}

typedef struct virus
{
    unsigned short SigSize;
    char virusName[16];
    unsigned char *sig;
} virus;

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
    // PrintHex(virus->sig, virus->SigSize);
    fprintf(output, "\n\n");
}

int main(int argc, char **argv)
{
    FILE *file;
    virus *v;

    file = fopen(argv[1], "r");
    if (!file)
    {
        fprintf(stderr, "Error: could not open file");
        return 0;
    }

    v = readVirus(file);

    printVirus(v, stdout);
    free(v->sig);
    free(v);

    fclose(file);
    return 0;
}