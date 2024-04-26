#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

int new_fd = -1;

void signal_handler(int sig)
{
    dup2(new_fd, STDOUT_FILENO);
}

int main(int argc, char* argv[])
{
    int hello_fd = open("test.txt", O_WRONLY | O_CREAT | O_TRUNC);
    char* child_buf = "Child hello!";
    char* parent_buf = "Parent howdy!";

    new_fd = dup(STDOUT_FILENO);
    dup2(hello_fd, STDOUT_FILENO);

    struct sigaction sa = {0};
    sa.sa_handler = &signal_handler;
    sigaction(SIGTERM, &sa, NULL);

    pid_t pid = fork();
    if (pid == 0)
    {
        execlp("echo", "echo", child_buf, NULL);
        kill(getppid(), SIGTERM);
    }
    else
    {
        sleep(5);
        kill(pid, SIGTERM);
        wait(NULL);
        dprintf(hello_fd, "%s: %d\n", parent_buf, pid);
        printf("%s: %d\n", parent_buf, pid);
    }
    
    return 0;
}