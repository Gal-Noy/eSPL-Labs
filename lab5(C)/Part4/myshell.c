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
    for (int i = 0; i < HISTLEN; i++)
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
    if (history[newest])
        free(history[newest]);

    history[newest] = command;
    newest = (newest + 1) % HISTLEN;

    if (entries < HISTLEN)
        entries++;
    else
        oldest = (oldest + 1) % HISTLEN;
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
    if (pCmdLine->inputRedirect)
    {
        strcat(command, " < ");
        strcat(command, pCmdLine->inputRedirect);
    }
    if (pCmdLine->outputRedirect)
    {
        strcat(command, " > ");
        strcat(command, pCmdLine->outputRedirect);
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
void updateProcessStatus(process *process_list, int pid, int status)
{
    process *curr = process_list;
    while (curr && curr->pid != pid) // Search for desired process
        curr = curr->next;
    if (curr)
        curr->status = status;
}
void updateProcessList(process **process_list)
{
    int res, status = RUNNING;
    process *curr = *process_list;

    while (curr)
    {
        res = waitpid(curr->pid, &status, WNOHANG);
        if (res)
        {
            if (WIFSTOPPED(status))
                curr->status = SUSPENDED;
            else if (WIFCONTINUED(status))
                curr->status = RUNNING;
            else if (WIFSIGNALED(status))
                curr->status = TERMINATED;
        }
        curr = curr->next;
    }
}
void printProcessList(process **process_list)
{
    updateProcessList(process_list);

    process *curr, *tmp, *prev = NULL;
    char *curr_command, *status_str;
    curr = *process_list;

    printf("%-20s%-20s%s\n", "PID", "Command", "Status");
    while (curr)
    {
        curr_command = get_command(curr->cmd);
        switch (curr->status)
        {
        case RUNNING:
            status_str = "Running";
            break;
        case SUSPENDED:
            status_str = "Suspended";
            break;
        case TERMINATED:
            status_str = "Terminated";
            break;
        }
        printf("%-20d%-20s%s\n", curr->pid, curr_command, status_str);

        free(curr_command);

        if (curr->status == TERMINATED)
        {
            // Delete the process from the list
            tmp = curr;
            curr = curr->next;
            freeCmdLines(tmp->cmd);
            free(tmp);
            if (prev)
                prev->next = curr;
            else // Update process_list if the first process is terminated
                *process_list = curr;
        }
        else
        {
            prev = curr;
            curr = curr->next;
        }
    }
}
void exit_program(cmdLine *pCmdLine, int code, char *error, int use_exit)
{
    if (error)
        perror(error);

    // Free memory
    free_history();
    freeProcessList(process_list);
    if (pCmdLine)
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
void signal_process(cmdLine *pCmdLine, int signal)
{
    int pid = atoi(pCmdLine->arguments[1]);

    if (kill(pid, signal) < 0)
    {
        perror("kill() error");
        return;
    }

    switch (signal)
    {
    case SIGTSTP:
        updateProcessStatus(process_list, pid, SUSPENDED);
        printf("Process %d has been suspended.\n", pid);
        break;
    case SIGCONT:
        updateProcessStatus(process_list, pid, RUNNING);
        printf("Process %d has been woken up.\n", pid);
        break;
    case SIGINT:
        updateProcessStatus(process_list, pid, TERMINATED);
        printf("Process %d has been terminated.\n", pid);
        break;
    }
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
void pipe_process(cmdLine *cmd1, cmdLine *cmd2)
{
    int pid1, pid2, pipefd[2];

    add_to_history(get_command(cmd1));
    add_to_history(get_command(cmd2));

    cmd1->next = NULL; // Separate between the commands

    if (cmd1->outputRedirect || cmd2->inputRedirect)
        exit_program(cmd1, 1, "Illegal redirecting error", 0);

    if (pipe(pipefd) < 0)
        exit_program(cmd1, 1, "pipe error", 0);

    pid1 = fork();
    switch (pid1)
    {
    case -1:
        exit_program(cmd1, 1, "child1 fork error", 0);
        break;
    case 0: // Child1 process
        close(STDOUT_FILENO);
        dup(pipefd[1]);
        close(pipefd[1]);
        io_process(cmd1, cmd1->inputRedirect, O_RDONLY, STDIN_FILENO, "open() inputRedirect error");
        if (execvp(cmd1->arguments[0], cmd1->arguments) < 0)
            exit_program(cmd1, 1, "execvp() error", 1);
        break;
    default:
        close(pipefd[1]);
        pid2 = fork();
        switch (pid2)
        {
        case -1:
            exit_program(cmd2, 0, "child2 fork error", 0);
            break;
        case 0: // Child2 process
            close(STDIN_FILENO);
            dup(pipefd[0]);
            close(pipefd[0]);
            io_process(cmd2, cmd2->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, STDOUT_FILENO, "open() outputRedirect error");
            if (execvp(cmd2->arguments[0], cmd2->arguments) < 0)
                exit_program(cmd2, 1, "execvp() error", 1);
            break;
        default: // Parent process
            close(pipefd[0]);

            if (debug)
            {
                fprintf(stderr, "PID: %d\nExecuting command: %s\n", pid1, cmd1->arguments[0]);
                fprintf(stderr, "PID: %d\nExecuting command: %s\n", pid2, cmd2->arguments[0]);
            }

            addProcess(&process_list, cmd1, pid1);
            addProcess(&process_list, cmd2, pid2);

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
        exit_program(pCmdLine, 0, NULL, 0);
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
        freeCmdLines(pCmdLine); // Not a process
        return 1;
    }
    return 0;
}
cmdLine *history_retrieve(int n)
{
    if (!entries)
    {
        fprintf(stderr, "No commands in history\n");
        return NULL;
    }
    else
    {
        if (n >= 0 && n <= entries)
        {
            int idx = n == 0 ? (newest - 1 + HISTLEN) % HISTLEN : (newest - entries + n - 1 + HISTLEN) % HISTLEN;
            return parseCmdLines(history[idx]);
        }
        else
        {
            fprintf(stderr, "Invalid history index\n");
            return NULL;
        }
    }
}

void execute(cmdLine *pCmdLine)
{
    int child_pid;
    char *curr_command, *arg = pCmdLine->arguments[0];

    if (history_command(pCmdLine))
        return;
    if (arg[0] == '!') // Should retrieve history command
    {
        int n = arg[1] == '!' ? 0 : atoi(arg + 1);
        freeCmdLines(pCmdLine); // Not a process
        if (!(pCmdLine = history_retrieve(n)))
            return;

        // Print retrieved command
        curr_command = get_command(pCmdLine);
        printf("%s\n", curr_command);
        free(curr_command);
    }

    if (special_commands_process(pCmdLine))
    {
        add_to_history(get_command(pCmdLine));
        freeCmdLines(pCmdLine); // Not a process
        return;
    }

    if (pCmdLine->next)
        pipe_process(pCmdLine, pCmdLine->next);
    else
    {
        add_to_history(get_command(pCmdLine));

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
            if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) < 0)
                exit_program(pCmdLine, 1, "execvp() error", 1);
            break;
        default: // Parent process
            if (debug)
                fprintf(stderr, "PID: %d\nExecuting command: %s\n", child_pid, pCmdLine->arguments[0]);

            addProcess(&process_list, pCmdLine, child_pid);

            if (pCmdLine->blocking)
                waitpid(child_pid, NULL, 0);
        }
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
        // Print directory
        getcwd(curr_dir, PATH_MAX);
        printf("%s : ", curr_dir);

        // Parse command
        fgets(line, LINE_SIZE, stdin);
        if (!(pCmdLine = parseCmdLines(line)))
            continue;

        // Process command
        execute(pCmdLine);
    }

    return 0;
}