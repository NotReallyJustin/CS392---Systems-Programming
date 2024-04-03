/*
Justin Chen
I pledge my honor that I have abided by the Stevens Honor System.
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define TRUE 1
#define FALSE 0
#define WRITE_END 1
#define READ_END 0

/*
    Overarching design idea:
    Parent spawns a child that will run $sort. That child will spawn a child that runs $ls

    For reference, see the illustration I drew here: https://imgur.com/a/UXULPc7
*/

/**
 * Creates a child that will execute the $ls command.
 * @param ls_path Path to run $ls on
*/
void create_ls_child(char* ls_path)
{
    // Create a pipe, and error check to see pipe actually worked
    int fd[2];

    if (pipe(fd) == -1)
    {
        fprintf(stderr, "Error: Pipe failed for ls child pipe. %s.", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Fork a child
    pid_t pid = fork();

    // If you're the child:
    if (pid == 0)
    {
        // Close the read end of pipe since we will not be reading from anything
        close(fd[READ_END]);

        // Now, chuck the write end of the pipe as your new "stdout". Also we'll error check
        if (dup2(fd[WRITE_END], 1) == -1)
        {
            fprintf(stderr, "Error. Duplicating pipe failed: %s.", strerror(errno));
            exit(EXIT_FAILURE);
        }

        close(fd[WRITE_END]);           // We don't need you anymore

        // Run the ls -ai exec(). We know that stdout gets preserved.
        execlp("ls", "ls", "-ai", ls_path, NULL);
        
        // If we reach here (and the exe didn't just immediately terminate) something went wrong
        fprintf(stderr, "Error: exec() failed. %s.\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        // If I'm the sort child (in this relative case, the parent), close the write end of the fd
        close(fd[WRITE_END]);

        // Now, chuck the read end of the pipe as your new "stdin". Also we'll error check
        if (dup2(fd[READ_END], 0) == -1)
        {
            fprintf(stderr, "Error. Duplicating pipe failed: %s.", strerror(errno));
            exit(EXIT_FAILURE);
        }

        close(fd[READ_END]);                // Useless file descriptor table entry

        // Wait for the child process to finish
        int child_status_code;

        if (waitpid(pid, &child_status_code, 0) == -1)
        {
            fprintf(stderr, "Error: waitpid() failed. %s.\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        // Great! We're done with everything the ls child process needs to do!
        // Now, head back to down to the code for the sort child process where we'll be execlp-ing this output
    }
}

/**
 * Given a string, count the number of newlines.
 * @param str String to count newline of
 * @param str_size Size of string (maybe even in bytes), since 1 char == 1 byte
 * @returns Number of newlines
*/
int count_newline(char* str, int str_size)
{
    int num_newline = 0;

    for (int i = 0; i < str_size; i++)
    {
        if (str[i] == '\n')
        {
            num_newline += 1;
        }
    }

    return num_newline;
}

/**
 * Print everything in stdin
 * Or more accurately, puts everything in file descriptor 0 into file descriptor 1
 * @returns Number of lines printed.
*/
int print_stdin()
{
    // Tracker for # lines printed
    int num_lines_printed = 0;

    // We will have 1024 bytes per "read buffer"
    char message[1024 + 1];
    int num_bytes;

    // Some code from Shudong's textbook
    // Loop through everything possibly in stdin, and print that out
    while (num_bytes = read(0, message, 1024))
    {
        if (num_bytes > 0)
        {
            message[num_bytes] = '\0';      // Add null terminator because printing is weird in syscalls

            // printf("%d\n", !(num_bytes = read(0, message, 1024)));
            // message[num_bytes] = '\0';
            // write(1, message, num_bytes);            // Write to stdout

            // Count the number of lines printed and increment that.
            // Since $sort will add \n to everything printed, # new lines == # lines printed
            num_lines_printed += count_newline(message, num_bytes);

            write(1, message, num_bytes);
        }
        else
        {
            break;
        }
    }
    
    return num_lines_printed;
}

/**
 * Creates a child that will sort the output pipe for the $ls command
 * @param ls_path The path to run $ls on down the line
*/
void create_sort_child(char* ls_path)
{
    // Create a pipe, and error check
    int fd[2];

    if (pipe(fd) == -1)
    {
        fprintf(stderr, "Error: Pipe failed for sort child pipe. %s.", strerror(errno));
        exit(EXIT_FAILURE);
    }

    // Fork a child
    pid_t pid = fork();

    // If you're the child:
    if (pid == 0)
    {
        // Close the read end of pipe
        close(fd[READ_END]);

        // Then, set the write end of the pipe to be stdout for the main program
        if (dup2(fd[WRITE_END], 1) == -1)
        {
            fprintf(stderr, "Error. Duplicating pipe failed: %s.", strerror(errno));
            exit(EXIT_FAILURE);
        }

        close(fd[WRITE_END]);       // No need for this anymore

        // Spawn a new child to $ls and run it.
        create_ls_child(ls_path);

        // 🛑 From the $sort manpage: DO NOT PASS IN ANY ARGUMENT TO $sort
        // WHEN YOU DO THAT, SORT WILL DIRECTLY READ FROM STDIN
        // THERE'S NO REASON TO DO ANYTHING ON YOUR OWN

        // Run the sort Bash command using exec(). The good thing is stdout gets preserved (which we like)
        execlp("sort", "sort", NULL);     // It's always funny to see it called a "Sentinel" since that's a DND feat
        
        // If we reach here (and the exe didn't just immediately terminate) something went wrong
        fprintf(stderr, "Error: exec() failed. %s.\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else
    {
        // If you're the parent (main program), close the write end of the pipe.
        close(fd[WRITE_END]);

        // Then, set the read end of the pipe to be the stdin for the main program
        if (dup2(fd[READ_END], 0) == -1)
        {
            fprintf(stderr, "Error. Duplicating pipe failed: %s.", strerror(errno));
            exit(EXIT_FAILURE);
        }
        
        close(fd[READ_END]);        // No need for this anymore

        // Wait for the child process to finish
        int child_status_code;

        if (waitpid(pid, &child_status_code, 0) == -1)
        {
            fprintf(stderr, "Error: waitpid() failed. %s.\n", strerror(errno));
            exit(EXIT_FAILURE);
        }

        // Now, read from stdin and print it out
        // Each file has its own line. Hence, the # of lines printed == # files printed
        int num_files = print_stdin();
        printf("Total files: %d\n", num_files);
    }
}

/**
 * Runs the stat command to check whether or not a given file is a directory with read access
 * @param pathname The pathname to check whether or not it's a directory with read access
 * @throw Throws an error and prints to stderr if any of the three happens:
 * Running stat fails, the $pathname is not a directory, or the user does not have read access
*/
void check_stat(char* pathname)
{
    // Runs the stat syscall to get a JSON of the file information
    struct stat file_info;
    int stat_status = stat(pathname, &file_info);

    // Check if stat fails
    if (stat_status == -1)
    {
        // fprintf(stderr, "Running stat on %s failed: %s.", pathname, strerror(errno));
        fprintf(stderr, "Permission denied. %s cannot be read.", pathname);
        exit(EXIT_FAILURE);
    }

    // Check whether something is a directory
    if (S_ISDIR(file_info.st_mode) == 0)
    {
        fprintf(stderr, "The first argument has to be a directory.");
        exit(EXIT_FAILURE);
    }

    // Check whether a user has read permission to the directory
    // Check: can we use access()
    if (access(pathname, R_OK) == -1)
    {
        fprintf(stderr, "Permission denied. %s cannot be read.", pathname);
        exit(EXIT_FAILURE);
    }
}

/**
 * The main process/entrypoint of $sl.c
*/
int main(int argc, char** argv)
{
    // Do we need to handle ~ or blank $sl?
    if (argc != 2)
    {
        fprintf(stderr, "The first argument has to be a directory.");
        exit(EXIT_FAILURE);
    }

    // Check whether file is a readable dir
    check_stat(argv[1]);

    // Begin executing $sl by spawning necessary children
    create_sort_child(argv[1]);
    return EXIT_SUCCESS;
}