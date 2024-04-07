#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>

int main()
{
    printf("Starting Main\n");
    int file_fd = open("test.txt", O_WRONLY | O_TRUNC | O_CREAT, 0666);

    dup2(file_fd, 1);
    pid_t child_pid = fork();

    if (child_pid != 0)
    {
        wait(NULL);
        printf("In Parent\n");
    }
    else printf("In Child\n");
    printf("Ending Main: %d\n", child_pid);
}