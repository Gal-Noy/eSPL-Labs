#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <linux/limits.h>
#include "LineParser.h"

void execute(cmdLine *pCmdLine)
{
    if (strcmp(pCmdLine->arguments[0], "quit") == 0)
    {
        freeCmdLines(pCmdLine);
        exit(1);
    }
    if (execv(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
    {
        perror("execv() error");
        freeCmdLines(pCmdLine);
        _exit(0);
    }
}

int main(int argc, char **argv)
{
    char curr_dir[PATH_MAX], line[2048];
    cmdLine *pCmdLine;

    getcwd(curr_dir, PATH_MAX);

    while (1)
    {
        printf("%s : ", curr_dir);

        fgets(line, 2048, stdin);
        pCmdLine = parseCmdLines(line);

        execute(pCmdLine);

        freeCmdLines(pCmdLine);
    }

    return 0;
}