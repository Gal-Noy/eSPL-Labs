#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <linux/limits.h>
#include "LineParser.h"

#define LINE_SIZE 2048

int debug = 0;

void exit_program(cmdLine *pCmdLine, int code, char *error, int use_exit)
{
    if (error)
        perror(error);
    freeCmdLines(pCmdLine);
    if (use_exit)
        exit(code);
    else
        _exit(code);
}
void change_directory(cmdLine *pCmdLine)
{
    if (chdir(pCmdLine->arguments[1]) < 0)
        exit_program(pCmdLine, 1, "chdir() error", 0);
}
void io_process(cmdLine *pCmdLine, const char *fd, int flags, int stdfd, char *error)
{
    if (fd)
    {
        int res = open(fd, flags, 0644);
        if (res == -1)
        {
            close(res);
            exit_program(pCmdLine, 1, error, 0);
        }
        if (dup2(res, stdfd) == -1)
        {
            close(res);
            exit_program(pCmdLine, 1, error, 0);
        }
        close(res);
    }
}
int special_commands_process(cmdLine *pCmdLine)
{
    char *arg = pCmdLine->arguments[0];

    if (strcmp(arg, "quit") == 0)
        exit_program(pCmdLine, 0, NULL, 0);
    else if (strcmp(arg, "cd") == 0)
    {
        change_directory(pCmdLine);
        return 1;
    }
    return 0;
}
void execute(cmdLine *pCmdLine)
{
    int child_pid;
    char *arg = pCmdLine->arguments[0];

    if (special_commands_process(pCmdLine))
        return;

    child_pid = fork();

    switch (child_pid)
    {
    case -1:
        exit_program(pCmdLine, 1, "fork() error", 0);
        break;
    case 0:
        if (pCmdLine->inputRedirect)
            io_process(pCmdLine, pCmdLine->inputRedirect, O_RDONLY, STDIN_FILENO, "open() inputRedirect error");
        if (pCmdLine->outputRedirect)
            io_process(pCmdLine, pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, STDOUT_FILENO, "open() outputRedirect error");
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
            exit_program(pCmdLine, 1, "execvp() error", 1);
        break;
    default:
        if (debug)
            fprintf(stderr, "PID: %d\nExecuting command: %s\n", child_pid, arg);

        if (pCmdLine->blocking)
            waitpid(child_pid, NULL, 0);
    }
}

int main(int argc, char **argv)
{
    char line[LINE_SIZE], curr_dir[PATH_MAX];
    cmdLine *pCmdLine;

    for (int i = 0; i < argc; i++)
        if (strcmp(argv[i], "-d") == 0)
            debug = 1;

    while (1)
    {
        getcwd(curr_dir, PATH_MAX);
        printf("%s : ", curr_dir);

        fgets(line, LINE_SIZE, stdin);
        pCmdLine = parseCmdLines(line);
        if (!pCmdLine)
            continue;

        execute(pCmdLine);

        freeCmdLines(pCmdLine);
    }

    return 0;
}