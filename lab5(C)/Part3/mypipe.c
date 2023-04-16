#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
    int pipefd[2], pid;
    char *message = "hello";

    if (pipe(pipefd) == -1)
    {
        perror("pipe error");
        exit(0);
    }

    pid = fork();

    if (pid < 0)
    {
        perror("fork() error");
        exit(0);
    }
    else if (pid == 0)
    {
        close(pipefd[0]);

        write(pipefd[1], message, 6);
        close(pipefd[1]);

        exit(1);
    }
    else
    {
        close(pipefd[1]);

        read(pipefd[0], message, 6);
        printf("Received message: %s\n", message);
        close(pipefd[0]);

        exit(1);
    }
}