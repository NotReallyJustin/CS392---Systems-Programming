#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int val = 0;

void rectangle()
{
    pid_t oval = getppid();
    kill(oval, SIGCONT);
}

void circle(int i)
{
    val = 1;
}

int main()
{
    pid_t pid = fork();
    signal(SIGCONT, circle);

    if (pid == 0)
    {
        rectangle();
    }
    else
    {
        wait(NULL);
    }

    printf("val = %d\n", val);
}