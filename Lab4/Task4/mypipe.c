/*Done by Yuval Raviv, ID: 206589210 and Gal Yaacov Noy, ID: 209346485*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char **argv)
{
    int pipefd[2], child_pid;
    char *message = "hello";

    if (pipe(pipefd) < 0)
    {
        perror("pipe error");
        exit(1);
    }

    child_pid = fork();

    if (child_pid < 0)
    {
        perror("fork() error");
        exit(1);
    }
    else if (child_pid == 0) // Child process
    {
        close(pipefd[0]);

        write(pipefd[1], message, sizeof(message));
        close(pipefd[1]);

        exit(0);
    }
    else // Parent process
    {
        close(pipefd[1]);

        read(pipefd[0], message, 6);
        printf("Received message: %s\n", message);
        close(pipefd[0]);

        exit(0);
    }
}