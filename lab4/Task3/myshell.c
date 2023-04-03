#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
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
    int child_pid, input = -1, output = -1;
    char *arg = pCmdLine->arguments[0];

    if (strcmp(arg, "quit") == 0)
        exit_program(pCmdLine, 1, NULL);
    else if (strcmp(arg, "cd") == 0)
    {
        if (chdir(pCmdLine->arguments[1]) < 0)
            exit_program(pCmdLine, 0, "chdir() error");
        exit_program(pCmdLine, 1, NULL);
    }
    else if (strcmp(arg, "wake") == 0)
    {
        child_pid = atoi(pCmdLine->arguments[1]);
        if (kill(child_pid, SIGCONT) < 0)
            exit_program(pCmdLine, 0, "kill() error");
        printf("Process %d has been woken up\n", child_pid);
        return;
    }
    else if (strcmp(arg, "kill") == 0)
    {
        child_pid = atoi(pCmdLine->arguments[1]);
        if (kill(child_pid, SIGTERM) < 0)
            exit_program(pCmdLine, 0, "kill() error");
        printf("Process %d has been terminated\n", child_pid);
        return;
    }

    child_pid = fork();

    if (child_pid == -1)
        exit_program(pCmdLine, 0, "fork() error");
    else if (child_pid == 0)
    {
        if (pCmdLine->inputRedirect)
        {
            input = open(pCmdLine->inputRedirect, O_RDONLY);
            if (input == -1)
                exit_program(pCmdLine, 0, "open() inputRedirect error");
            if (dup2(input, STDIN_FILENO) == -1)
                exit_program(pCmdLine, 0, "dup2() inputRedirect error");
            close(input);
        }

        if (pCmdLine->outputRedirect)
        {
            output = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output == -1)
                exit_program(pCmdLine, 0, "open() outputRedirect error");
            if (dup2(output, STDOUT_FILENO) == -1)
                exit_program(pCmdLine, 0, "dup2() outputRedirect error");
            close(output);
        }

        if (execvp(arg, pCmdLine->arguments) == -1)
            exit_program(pCmdLine, 0, "execvp() error");
    }

    if (debug)
        fprintf(stderr, "PID: %d\nExecuting command: %s\n", child_pid, arg);

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