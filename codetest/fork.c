#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

int main()
{

    for (int i = 0; i < 10; i++)
    {
        fork();
        break;
    }
    
    puts("I'm a process!");
    return 0;
}