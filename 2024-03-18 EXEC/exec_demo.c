#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char* argv[])
{
    pid_t pid;

    if (argc < 2)
    {
        printf("Usage: ./execdemo arg1 [arg2 ....]\n");
        exit(EXIT_FAILURE);
    }

    // Remember, from here on out, there will be a child process that runs the rest of the code here.
    // pid == 0 means you are the child
    pid = fork();
    if (pid == 0)
    {
        // Remember - you are importing a while new command
        // You are literally dumping the /bin/echo code onto exec_demo.c
        // That also means that since /bin/echo will exit(EXIT_SUCCESS) when it finishes running, execv()
        // will exit(EXIT_SUCCESS) if it's good
        execv("/bin/echo", argv);

        // That means if you get to this step, something done goofed up
        fprintf(stderr, "execv() failed to run.\n");
        exit(EXIT_FAILURE);
    }

    printf("Success!\n");
    exit(EXIT_SUCCESS);
}