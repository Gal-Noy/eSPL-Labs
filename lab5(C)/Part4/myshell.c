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
#define HISTLEN 20
#define TERMINATED -1
#define RUNNING 1
#define SUSPENDED 0

typedef struct process
{
    cmdLine *cmd;         /* the parsed command line*/
    pid_t pid;            /* the process id that is running the command*/
    int status;           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next; /* next process in chain */
} process;

process *process_list = NULL;
char *history[HISTLEN];
int oldest = 0, newest = 0, entries = 0, debug = 0;

void free_history()
{
    for (int i = 0; i < entries; i++)
        free(history[i]);
}
void print_history()
{
    int i, j;
    printf("Command History:\n");
    for (i = 0, j = oldest; i < entries; i++)
    {
        printf("%d: %s\n", i + 1, history[j]);
        j = (j + 1) % HISTLEN;
    }
}
void add_to_history(char *command)
{
    if (entries < HISTLEN)
    {
        history[newest] = strdup(command);
        newest = (newest + 1) % HISTLEN;
        entries++;
    }
    else
    {
        free(history[oldest]);
        history[oldest] = strdup(command);
        oldest = (oldest + 1) % HISTLEN;
    }
    free(command);
}
char *get_command(cmdLine *pCmdLine)
{
    char *command = (char *)malloc(sizeof(char) * LINE_SIZE);
    command[0] = '\0';

    strcat(command, pCmdLine->arguments[0]);
    for (int i = 1; i < pCmdLine->argCount; i++)
    {
        strcat(command, " ");
        strcat(command, pCmdLine->arguments[i]);
    }
    return command;
}
void freeProcessList(process *process_list)
{
    process *curr, *next;
    curr = process_list;
    while (curr)
    {
        if (curr->cmd)
            freeCmdLines(curr->cmd);
        next = curr->next;
        free(curr);
        curr = next;
    }
}
void addProcess(process **process_list, cmdLine *cmd, pid_t pid)
{
    process *new_process = (process *)malloc(sizeof(process));
    new_process->cmd = cmd;
    new_process->pid = pid;
    new_process->status = RUNNING;
    new_process->next = *process_list;
    *process_list = new_process;
}
void updateProcessList(process **process_list)
{
    int status, res;
    process *curr = *process_list;

    while (curr)
    {
        res = waitpid(curr->pid, &status, WNOHANG);
        if (res < 0)
        {
            // process doesn't exist
            curr = curr->next;
            continue;
        }
        if (WIFSTOPPED(status))
            curr->status = SUSPENDED;
        else if (WIFCONTINUED(status))
            curr->status = RUNNING;
        else if (WIFSIGNALED(status))
            curr->status = TERMINATED;

        curr = curr->next;
    }
}
void updateProcessStatus(process *process_list, int pid, int status)
{
    process *curr = process_list;
    while (curr)
    {
        if (curr->pid == pid)
        {
            curr->status = status;
            break;
        }
        curr = curr->next;
    }
}
void printProcessList(process **process_list)
{
    updateProcessList(process_list);

    process *curr, *tmp, *prev = NULL;
    curr = *process_list;

    printf("PID\tStatus\tCommand\n");
    while (curr)
    {
        printf("%d\t", curr->pid);
        for (int i = 0; i < curr->cmd->argCount; i++)
            printf("%s ", curr->cmd->arguments[i]);
        printf("\t");

        switch (curr->status)
        {
        case RUNNING:
            printf("Running\n");
            break;
        case SUSPENDED:
            printf("Suspended\n");
            break;
        case TERMINATED:
            printf("Terminated\n");

            // Delete the process from the list
            if (!prev)
                *process_list = curr->next;
            else
                prev->next = curr->next;

            tmp = curr;
            curr = curr->next;
            freeCmdLines(tmp->cmd);
            free(tmp);
            continue;
        default:
            break;
        }
        prev = curr;
        curr = curr->next;
    }
}

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
    free_history();
    if (pCmdLine)
        freeCmdLines(pCmdLine);
    freeProcessList(process_list);
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
    const char *status;
    int pid = atoi(pCmdLine->arguments[1]);

    if (kill(pid, signal) < 0)
        exit_program(pCmdLine, 0, "kill() error", 0);

    switch (signal)
    {
    case SIGTSTP:
        status = "suspended";
        break;
    case SIGCONT:
        status = "woken up";
        break;
    default:
        status = "terminated";
        break;
    }

    updateProcessStatus(process_list, pid, signal == SIGTSTP ? SUSPENDED : signal == SIGCONT ? RUNNING
                                                                                             : TERMINATED);
    printf("Process %d has been %s.\n", pid, status);
}
void io_process(cmdLine *pCmdLine, const char *fd, int flags, int stdfd, char *error)
{
    if (fd)
    {
        int res = open(fd, flags, 0644);
        if (res < 0)
            exit_program(pCmdLine, 0, error, 0);
        if (dup2(res, stdfd) < 0)
            exit_program(pCmdLine, 0, error, 0);
        close(res);
    }
}
void pipe_process(cmdLine *pCmdLine)
{
    if (pCmdLine->outputRedirect || pCmdLine->next->inputRedirect)
        exit_program(pCmdLine, 0, "Illegal redirecting error", 0);

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
        close(pipefd[1]);

        io_process(pCmdLine, pCmdLine->inputRedirect, O_RDONLY, STDIN_FILENO, "open() inputRedirect error");
        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) < 0)
            exit_program(pCmdLine, 0, "execvp() error", 1);
        break;
    default:
        addProcess(&process_list, pCmdLine, pid1); // add process to the list
        close(pipefd[1]);
        pid2 = fork();
        switch (pid2)
        {
        case -1:
            exit_program(pCmdLine->next, 0, "child2 fork error", 0);
            break;
        case 0:
            close(STDIN_FILENO);
            dup(pipefd[0]);
            close(pipefd[0]);
            io_process(pCmdLine->next, pCmdLine->next->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, STDOUT_FILENO, "open() outputRedirect error");
            if (execvp(pCmdLine->next->arguments[0], pCmdLine->next->arguments) < 0)
                exit_program(pCmdLine->next, 0, "execvp() error", 1);
            break;
        default:
            addProcess(&process_list, pCmdLine->next, pid2); // add process to the list
            close(pipefd[0]);
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
        }
    }
}
int special_commands_process(cmdLine *pCmdLine)
{
    char *arg = pCmdLine->arguments[0];
    int special = 0;

    if (strcmp(arg, "quit") == 0)
        exit_program(pCmdLine, 1, NULL, 0);
    else if (strcmp(arg, "cd") == 0)
    {
        change_directory(pCmdLine);
        special = 1;
    }
    else if (strcmp(arg, "suspend") == 0)
    {
        signal_process(pCmdLine, SIGTSTP);
        special = 1;
    }
    else if (strcmp(arg, "wake") == 0)
    {
        signal_process(pCmdLine, SIGCONT);
        special = 1;
    }
    else if (strcmp(arg, "kill") == 0)
    {
        signal_process(pCmdLine, SIGINT);
        special = 1;
    }
    else if (strcmp(arg, "procs") == 0)
    {
        printProcessList(&process_list);
        special = 1;
    }

    return special;
}
int history_command(cmdLine *pCmdLine)
{
    if (strcmp(pCmdLine->arguments[0], "history") == 0)
    {
        print_history();
        freeCmdLines(pCmdLine);
        return 1;
    }
    return 0;
}
int history_retrieve(cmdLine *pCmdLine)
{
    char *arg, *nth_command;
    arg = pCmdLine->arguments[0];

    if (arg[0] == '!')
    {
        if (!entries)
        {
            fprintf(stderr, "No commands in history\n");
            return 0;
        }
        else
        {
            int n = arg[1] == '!' ? 1 : atoi(arg + 1);
            if (n >= 1 && n <= entries)
            {
                nth_command = history[(newest - n + HISTLEN) % HISTLEN];
                // add_to_history(nth_command);
                pCmdLine = parseCmdLines(nth_command);
                printf("%s", nth_command);
            }
            else
            {
                fprintf(stderr, "Invalid history index\n");
                return 0;
            }
        }
    }
    return 1;
}

void execute(cmdLine *pCmdLine)
{
    int child_pid;
    char *arg = pCmdLine->arguments[0];

    if (history_command(pCmdLine))
        return;
    // if (!history_retrieve(pCmdLine))
    //     return;
    if (special_commands_process(pCmdLine))
    {
        add_to_history(get_command(pCmdLine));
        freeCmdLines(pCmdLine);
        return;
    }
    if (pCmdLine->next)
        pipe_process(pCmdLine);
    else
    {
        add_to_history(get_command(pCmdLine));
        child_pid = fork();
        switch (child_pid)
        {
        case -1:
            exit_program(pCmdLine, 0, "fork() error", 0);
            break;
        case 0:
            io_process(pCmdLine, pCmdLine->inputRedirect, O_RDONLY, STDIN_FILENO, "open() inputRedirect error");
            io_process(pCmdLine, pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, STDOUT_FILENO, "open() outputRedirect error");
            if (execvp(arg, pCmdLine->arguments) < 0)
                exit_program(pCmdLine, 0, "execvp() error", 1);
            break;
        }

        if (debug)
            fprintf(stderr, "PID: %d\nExecuting command: %s\n", child_pid, arg);

        addProcess(&process_list, pCmdLine, child_pid); // Add process to the list

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

        // freeCmdLines(pCmdLine);
    }

    return 0;
}