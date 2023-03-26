#include <stdio.h>

void PrintHex(unsigned char *buffer, int length)
{
    for (int i = 0; i < length; i++)
        printf("%02X ", buffer[i]);
}
int main(int argc, char **argv)
{
    FILE *f;
    int length;
    unsigned char *buffer;

    f = fopen(argv[1], "r");
    if (!f)
    {
        fprintf(stderr, "Error: could not open file");
        return 0;
    }

    buffer = malloc(sizeof(int) * 256);
    length = fread(buffer, sizeof(char), 256, f);
    PrintHex(buffer, length);

    fclose(f);
    free(buffer);
}