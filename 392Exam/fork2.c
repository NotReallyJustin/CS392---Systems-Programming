#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>

int main()
{
    printf("Starting Main\n");
    int file_fd = open("test.txt", O_WRONLY | O_TRUNC | O_CREAT, 0666);
    int new_fd = dup(1);                // Duplicates STDOUT in file descriptor table

    dup2(file_fd, 1);
    pid_t child_pid = fork();

    if (child_pid != 0)
    {
        wait(NULL);
        printf("In Parent\n");
    }
    else
    {
        // Chucks open file table entry for STDOUT to file descriptor[1]
        dup2(new_fd, 1);

        // Remember that printf uses file descriptor[1]
        printf("In Child\n");
    }

    printf("Ending Main: %d\n", child_pid);
}