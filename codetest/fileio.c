#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>

void write_yoda(int fd, const void* buf, size_t count)
{
    char* buffer = (char*) buf;

    while (write(fd, buffer, count) != count)
    {

    }
}

int main()
{
    int new_file = open("./fileio.txt", O_WRONLY);

    char* to_write = "Good morning America!";
    write_yoda(new_file, to_write, strlen(to_write));
    return 0;
}