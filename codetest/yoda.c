#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

void process_array(int num_process, int textbook_size)
{

    int start_index = 0;
    int end_index = textbook_size;

    for (int i = 0; i < num_process; i++)
    {
        pid_t pid = fork();

        if (pid == 0)
        {
            start_index = (textbook_size / num_process) * i;
            end_index = (textbook_size / num_process) * (i + 1);
            break;
        }
        else
        {
            wait(NULL);
        }
    }

    // Search for spell
    printf("%d %d\n", start_index, end_index);
}

int main()
{
    process_array(10, 100);
    return 0;
}