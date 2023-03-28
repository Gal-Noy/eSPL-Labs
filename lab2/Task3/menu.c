#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
        char *line = malloc(sizeof(char)*100);
	while (fprintf(stdout, "Select operation from the following menu: ") &&
			(fgets(line,100, stdin)) != NULL)
                fprintf(stdout, "%s", line);
	free(line);
        return 0;
}
