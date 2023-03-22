#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *map(char *array, int array_length, char (*f)(char))
{
  char *mapped_array = (char *)(malloc(array_length * sizeof(char)));
  /* TODO: Complete during task 2.a */
  for (int i = 0; i < array_length; i++)
    mapped_array[i] = f(array[i]);
  return mapped_array;
}

char my_get(char c)
{
  return fgetc(stdin);
}
char cprt(char c)
{
  if (c >= 0x20 && c <= 0x7E)
    printf("%c\n", c);
  else
    printf(".\n");
  return c;
}
char encrypt(char c)
{
  return (c >= 0x20 && c <= 0x7E) ? c + 1 : c;
}
char decrypt(char c)
{
  return (c >= 0x20 && c <= 0x7E) ? c - 1 : c;
}
char xprt(char c)
{
  if (c >= 0x20 && c <= 0x7E)
    printf("%x\n", c);
  else
    printf(".\n");
  return c;
}

int main(int argc, char **argv)
{
  typedef struct fun_desc
  {
    char *name;
    char (*fun)(char);
  } fun_desc;

  char *carray, *line;
  int idx;

  carray = malloc(sizeof(char) * 5);
  struct fun_desc menu[] = {{"Get string", &my_get}, {"Print string", &cprt}, {"Print hex", &xprt}, {"Encrypt", &encrypt}, {"Decrypt", &decrypt}, {NULL, NULL}};

  printf("Please choose a function:\n");
  for (int i = 0; i < 5; i++)
    printf("%d) %s\n", i, menu[i].name);
  printf("Option: ");

  line = malloc(sizeof(char) * 3);
  while ((fgets(line, sizeof(line), stdin)) != NULL)
  {
    idx = *line - '0';
    if (idx < 0 || idx > 4)
    {
      printf("Not within bounds.\n");
      free(line);
      free(carray);
      exit(0);
    }
    else
      printf("Within bounds.\n");

    carray = map(carray, 5, menu[idx].fun);

    printf("DONE.\n");

    printf("\nPlease choose a function:\n");
    for (int i = 0; i < 5; i++)
      printf("%d) %s\n", i, menu[i].name);
    printf("Option: ");
  }

  free(line);
  free(carray);
  return 0;
}
