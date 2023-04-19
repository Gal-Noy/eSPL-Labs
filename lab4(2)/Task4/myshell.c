/*Done by Yuval Raviv, ID: 206589210 and Gal Yaacov Noy, ID: 209346485*/

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

/* Function to exit the program, free memory and either exit or _exit the program */
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

/* Function to change directory */
void change_directory(cmdLine *pCmdLine)
{
    if (chdir(pCmdLine->arguments[1]) < 0)
        exit_program(pCmdLine, 1, "chdir() error", 0);
}

/* Function to send signals to a process */
void signal_process(cmdLine *pCmdLine, int signal)
{
    int child_pid = atoi(pCmdLine->arguments[1]);
    if (kill(child_pid, signal) < 0)
    {
        perror("kill() error");
        return;
    }
    printf("Process %d has been %s.\n", child_pid, signal == SIGTSTP ? "suspended" : signal == SIGCONT ? "woken up"
                                                                                                       : "terminated");
}

/* Function to redirect input/output of a process */
void io_process(cmdLine *pCmdLine, const char *fd, int flags, int stdfd, char *error)
{
    if (fd)
    {
        int res = open(fd, flags, 0644); // Open file with specified flags and mode

        if (res == -1)
        {
            close(res);
            exit_program(pCmdLine, 1, error, 0);
        }
        if (dup2(res, stdfd) == -1) // // Redirect standard input or output to the opened file
        {
            close(res);
            exit_program(pCmdLine, 1, error, 0);
        }
        close(res);
    }
}

/* Function to handle special commands like cd, quit, kill, suspend, and wake */
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
    else if (strcmp(arg, "suspend") == 0)
    {
        signal_process(pCmdLine, SIGTSTP);
        return 1;
    }
    else if (strcmp(arg, "wake") == 0)
    {
        signal_process(pCmdLine, SIGCONT);
        return 1;
    }
    else if (strcmp(arg, "kill") == 0)
    {
        signal_process(pCmdLine, SIGINT);
        return 1;
    }
    return 0;
}

/* Function to execute a command */
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
    case 0: // Child process
        if (pCmdLine->inputRedirect)
            io_process(pCmdLine, pCmdLine->inputRedirect, O_RDONLY, STDIN_FILENO, "open() inputRedirect error");
        if (pCmdLine->outputRedirect)
            io_process(pCmdLine, pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, STDOUT_FILENO, "open() outputRedirect error");
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1)
            exit_program(pCmdLine, 1, "execvp() error", 1);
        break;
    default: // Parent process
        if (debug)
            fprintf(stderr, "PID: %d\nExecuting command: %s\n", child_pid, arg);

        if (pCmdLine->blocking) // Wait for child processes to finish
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
        // Print current directory
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