#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

int global_var = 0;

int main()
{
    pid_t pid = fork();
    if (pid == 0)
    {
        global_var = 40;
    }
    else
    {
        printf("Global Variable: %d\n", global_var);
    }

    return 0;
}