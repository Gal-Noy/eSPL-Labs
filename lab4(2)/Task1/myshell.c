#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include "LineParser.h"

void exit_program(cmdLine *pCmdLine, int code, char *error)
{
    freeCmdLines(pCmdLine);
    if (error)
    {
        perror(error);
        free(error);
    }
    exit(code);
}

void execute(cmdLine *pCmdLine, int debug)
{
    int child_pid;

    if (strcmp(pCmdLine->arguments[0], "quit") == 0)
        exit_program(pCmdLine, 1, NULL);
    else if (strcmp(pCmdLine->arguments[0], "cd") == 0)
    {
        if (chdir(pCmdLine->arguments[1]) < 0)
            exit_program(pCmdLine, 0, "chdir() error");
        exit_program(pCmdLine, 1, NULL);
    }

    child_pid = fork();

    if (child_pid == -1)
        exit_program(pCmdLine, 0, "fork() error");
    else if (child_pid == 0)
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
            exit_program(pCmdLine, 0, "execvp() error");

    if (debug)
        fprintf(stderr, "PID: %d\nExecuting command: %s\n", child_pid, pCmdLine->arguments[0]);

    if (pCmdLine->blocking)
        waitpid(child_pid, NULL, 0);
}

int main(int argc, char **argv)
{
    char curr_dir[PATH_MAX], line[2048];
    cmdLine *pCmdLine;
    int debug = 0;

    for (int i = 0; i < argc; i++)
        if (strcmp(argv[i], "-d") == 0)
            debug = 1;

    while (1)
    {
        getcwd(curr_dir, PATH_MAX);
        printf("%s : ", curr_dir);

        fgets(line, 2048, stdin);
        pCmdLine = parseCmdLines(line);

        execute(pCmdLine, debug);

        freeCmdLines(pCmdLine);
    }

    return 0;
}