/* $Id: count-words.c 858 2010-02-21 10:26:22Z tolpin $ */

#include <stdio.h>
#include <string.h>
#include <malloc.h>

/* return string "word" if the count is 1 or "words" otherwise */
char *words(int count)
{
  char *words = "words";
  char *new_words = malloc(sizeof(char)*5);
  for (int i = 0; i < 5; i++)
    new_words[i] = words[i];

  if (count == 1)
    	new_words[strlen(words)-1] = '\0';

  return new_words;
}

/* print a message reportint the number of words */
int print_word_count(char **argv)
{
  int count = 0;
  char **a = argv;
  while (*(a++))
    ++count;
  char *wordss = words(count);
  printf("The sentence contains %d %s.\n", count, wordss);
  
  return count;
}

/* print the number of words in the command line and return the number as the exit code */
int main(int argc, char **argv)
{
  print_word_count(argv + 1);
  return 0;
}
