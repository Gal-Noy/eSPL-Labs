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

void print_directory()
{
    char curr_dir[PATH_MAX];
    getcwd(curr_dir, PATH_MAX);
    printf("%s : ", curr_dir);
}
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
        exit_program(pCmdLine, 0, "chdir() error", 0);
}
void signal_process(cmdLine *pCmdLine, int signal)
{
    int child_pid = atoi(pCmdLine->arguments[1]);
    if (kill(child_pid, signal) < 0)
        exit_program(pCmdLine, 0, "kill() error", 0);
    printf("Process %d has been %s.\n", child_pid, signal == SIGTSTP ? "suspended" : signal == SIGCONT ? "woken up"
                                                                                                       : "terminated");
}
void io_process(cmdLine *pCmdLine, const char *fd, int flags, int stdfd, char *error)
{
    if (fd)
    {
        int res = open(fd, flags, 0644);
        if (res == -1)
            exit_program(pCmdLine, 0, error, 0);
        if (dup2(res, stdfd) == -1)
            exit_program(pCmdLine, 0, error, 0);
        close(res);
    }
}

void handle_special_commands(cmdLine *pCmdLine)
{
    char *arg = pCmdLine->arguments[0];

    if (strcmp(arg, "quit") == 0)
        exit_program(pCmdLine, 1, NULL, 0);
    else if (strcmp(arg, "cd") == 0)
    {
        change_directory(pCmdLine);
        return;
    }
    else if (strcmp(arg, "suspend") == 0)
    {
        signal_process(pCmdLine, SIGTSTP);
        return;
    }
    else if (strcmp(arg, "wake") == 0)
    {
        signal_process(pCmdLine, SIGCONT);
        return;
    }
    else if (strcmp(arg, "kill") == 0)
    {
        signal_process(pCmdLine, SIGINT);
        return;
    }
}

void handle_pipe(cmdLine *pCmdLine)
{
    int pid1, pid2, pipefd[2];
    if (pipe(pipefd) < 0)
        exit_program(pCmdLine, 0, "pipe error", 0);

    pid1 = fork();
    switch (pid1)
    {
    case -1:
        exit_program(pCmdLine, 0, "child1 fork error", 0);
        break;
    case 0:
        close(STDOUT_FILENO);
        dup(pipefd[1]);
        close(pipefd[0]);
        close(pipefd[1]);
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
            exit_program(pCmdLine, 0, "execvp() error", 1);
        break;
    default:
        close(pipefd[1]);
    }

    pid2 = fork();
    switch (pid2)
    {
    case -1:
        exit_program(pCmdLine, 0, "child2 fork error", 0);
        break;
    case 0:
        close(STDIN_FILENO);
        dup(pipefd[0]);
        close(pipefd[0]);
        close(pipefd[1]);
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
            exit_program(pCmdLine, 0, "execvp() error", 1);
        break;
    default:
        close(pipefd[0]);
    }

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);
}

void execute(cmdLine *pCmdLine)
{
    int child_pid;

    handle_special_commands(pCmdLine);

    if (pCmdLine->next)
        handle_pipe(pCmdLine);
    else
    {
        child_pid = fork();
        switch (child_pid)
        {
        case -1:
            exit_program(pCmdLine, 0, "fork() error", 0);
            break;
        case 0:
            io_process(pCmdLine, pCmdLine->inputRedirect, O_RDONLY, STDIN_FILENO, "open() inputRedirect error");
            io_process(pCmdLine, pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, STDOUT_FILENO, "open() outputRedirect error");
            if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
                exit_program(pCmdLine, 0, "execvp() error", 1);
            break;
        }

        if (debug)
            fprintf(stderr, "PID: %d\nExecuting command: %s\n", child_pid, pCmdLine->arguments[0]);

        if (pCmdLine->blocking)
            waitpid(child_pid, NULL, 0);
    }
}

int main(int argc, char **argv)
{
    char line[LINE_SIZE];
    cmdLine *pCmdLine;

    for (int i = 0; i < argc; i++)
        if (strcmp(argv[i], "-d") == 0)
            debug = 1;

    while (1)
    {
        print_directory();

        fgets(line, LINE_SIZE, stdin);
        pCmdLine = parseCmdLines(line);
        if (!pCmdLine)
            continue;

        execute(pCmdLine);

        freeCmdLines(pCmdLine);
    }

    return 0;
}