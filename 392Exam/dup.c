#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <string.h>

/*
    Experiment Conclusion: dup() only copies the file description. They still point to the same open file table.
*/

int main()
{
    printf("Starting Main\n");
    int file_fd = open("arianaGrande.txt", O_WRONLY | O_TRUNC | O_CREAT, 0666);
    int new_fd = dup(file_fd);

    char str1[] = "This is a true story about all the lies\n";
    char str2[] = "You fantasized\n";
    char str3[] = "About you and I...\n";

    write(file_fd, str1, strlen(str1));
    write(new_fd, str2, strlen(str2));
    write(file_fd, str3, strlen(str3));

    return 0;
}