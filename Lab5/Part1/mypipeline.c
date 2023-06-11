#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    int pipefd[2], pid1, pid2;
    char *ls_args[] = {"ls", "-l", 0};
    char *tail_args[] = {"tail", "-n", "2", 0};

    if (pipe(pipefd) == -1)
    {
        perror("pipe error.");
        exit(EXIT_FAILURE);
    }

    fprintf(stderr, "(parent_process>forking...)\n");
    pid1 = fork();
    switch (pid1)
    {
    case -1:
        perror("child1 fork error.");
        exit(EXIT_FAILURE);
        break;
    case 0: // In the child1 process
        close(STDOUT_FILENO);
        dup(pipefd[1]);
        close(pipefd[1]);
        fprintf(stderr, "(child1>redirecting stdout to the write end of the pipe...)\n");
        fprintf(stderr, "(child1>going to execute cmd: ls -l)\n");
        execvp(ls_args[0], ls_args);
        exit(EXIT_FAILURE);
        break;
    default: // In the parent process
        fprintf(stderr, "(parent_process>created process with id: %d)\n", pid1);
        close(pipefd[1]);
        fprintf(stderr, "(parent_process>closing the write end of the pipe...)\n");

        fprintf(stderr, "(parent_process>forking...)\n");
        pid2 = fork();
        switch (pid2)
        {
        case -1:
            perror("child2 fork error.");
            exit(EXIT_FAILURE);
            break;
        case 0: // In the child2 process
            close(STDIN_FILENO);
            dup(pipefd[0]);
            close(pipefd[0]);
            fprintf(stderr, "(child2>redirecting stdin to the read end of the pipe...)\n");
            fprintf(stderr, "(child2>going to execute cmd: tail -n 2)\n");
            execvp(tail_args[0], tail_args);
            exit(EXIT_FAILURE);
            break;
        default: // In the parent process
            fprintf(stderr, "(parent_process>created process with id: %d)\n", pid2);
            close(pipefd[0]);
            fprintf(stderr, "(parent_process>closing the read end of the pipe...)\n");

            // Wait for child processes to terminate
            fprintf(stderr, "(parent_process>waiting for child processes to terminate...)\n");
            waitpid(pid1, NULL, 0);
            waitpid(pid2, NULL, 0);
            fprintf(stderr, "(parent_process>exiting...)\n");
        }
    }
    return 0;
}