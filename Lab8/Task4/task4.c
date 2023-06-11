#include <stdio.h>

int digit_cnt(char *s)
{
    int i = 0, count = 0;
    while (s[i] != '\0')
    {
        if (s[i] >= '0' && s[i] <= '9')
            count++;
        i++;
    }
    return count;
}

int main(int argc, char *argv[])
{
    int count;

    if (argc < 2)
    {
        printf("Usage: ./digit_ctr <string>\n");
        return 1;
    }

    count = digit_cnt(argv[1]);
    printf("Number of digits: %d\n", count);

    return 0;
}