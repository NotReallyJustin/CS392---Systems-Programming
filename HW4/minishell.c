/*
    Justin Chen
    I pledge my honor that I have abided by the Stevens Honor System.
*/

// Defining colors
#define BLUE "\x1b[34;1m"
#define DEFAULT "\x1b[0m"

// IDK why, but my linter complains when I declare `struct sigaction`. This shuts it up.
// @see https://stackoverflow.com/questions/6491019/struct-sigaction-incomplete-error
#define _XOPEN_SOURCE 700

// Importing Necessary Libs
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // File I/O
#include <string.h>
#include <dirent.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <pwd.h>
#include <fcntl.h>
#include <errno.h>

// Signal Handler
// When it starts, it's set to 0 since we don't see a SIGINT
volatile sig_atomic_t interrupted = 0;

/**
 * Signal handler for SIGINT
 * @param sig The signal number, but we won't really use this
*/
void sig_handler(int sig)
{
    interrupted = 1;
}

/**
 * Counts the number of occurances of $tracking_char in $str.
 * @param str The string to count the number of occurances of
 * @param tracking_char The character to count the number of occurances of
 * @returns The number of occurances of $tracking_char in $str
*/
int num_chars_in_str(char* str, char tracking_char)
{
    int num_count = 0;

    for (size_t i = 0; i < strlen(str); i++)
    {
        if (str[i] == tracking_char)
        {
            num_count += 1;
        }
    }

    return num_count;
}

/**
 * Counts the number of occurances of $tracking_char in $str.
 * @param str The string to count the number of occurances of
 * @param tracking_char The character to count the number of occurances of
 * @returns The number of occurances of $tracking_char in the beginning and end of $str
*/
int num_chars_start_end(char* str, char tracking_char)
{
    int num_chars = 0;
    int length_str = strlen(str);

    // Count start
    for (int i = 0; i < length_str; i++)
    {
        if (str[i] == tracking_char)
        {
            num_chars += 1;
        }
        else
        {
            break;
        }
    }

    // Edge Case: If the entire string is just $tracking_char, exit here because we don't want to double count with the tails
    if (num_chars == length_str)
    {
        return num_chars;
    }

    // Count end
    for (int i = length_str - 1; i >= 0; i--)
    {
        if (str[i] == tracking_char)
        {
            num_chars += 1;
        }
        else
        {
            break;
        }
    }

    return num_chars;
}

/**
 * Checks if a string is a valid number
 * @returns 1 if it is a valid number, 0 otherwise
*/
int is_number(char* str)
{
    for (size_t i = 0; i < strlen(str); i++)
    {
        // 0 is ASCII 48 and 9 is ASCII 57
        if (str[i] >= '0' && str[i] <= '9')
        {
        }
        else
        {
            return 0;
        }
    }

    return 1;
}

/**
 * Comparison function for two strings. This is intended to be used in qsort.
 * These strings should be numbers
 * @param vp1 1st void pointer
 * @param vp2 2nd void pointer
 * @returns < 0 is str1 < str2, 0 if str1 == str2, and 1 if str1 > str2
*/
int compare(const void* vp1, const void* vp2) 
{
   // Cast and reference the two strings because we can't dereference void*
    char* str1 = *((char**)(vp1));
    char* str2 = *((char**)(vp2));

    return atoi(str1) - atoi(str2);
}


/**
 * Gather all PIDs in /proc/
 * @param pid_arr {Outbound} Pointer to a string array that will contain a list of sorted (numeric) PIDs. You're responsible for free()-ing this.
 * @param pid_arr_size {Outbound} Pointer to an int that will store the size of the pid_arr (which is just the number of numeric directories)
 * @returns 0 if successful, -1 if unsuccessful. This already throws an error, so just return when you get the exit code
*/
int get_all_pids(char*** pid_arr, int* pid_arr_size)
{
    // ⭐ In our first pass, let's just loop through and count how many numeric directories we have.
    int num_numeric_dir = 0;

    DIR* dir_object = opendir("/proc/");

    // 🚨 Error checking: Make sure we can actually open /proc/
    if (dir_object == NULL)
    {
        fprintf(stderr, "Error: Cannot open /proc/. %s.\n", strerror(errno));
        return -1;
    }
    
    // Start reading. curr file will contain a dirent object for each file in the directory
    struct dirent* curr_file = readdir(dir_object);

    while (curr_file != NULL)
    {
        // @see https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
        // A directory will return a d_type of 4
        
        if (curr_file->d_type == 4)
        {
            // Check to make sure that it's a number
            if (is_number(curr_file->d_name))
            {
                num_numeric_dir += 1;
            }
        }
        // Continue reading the new dirent
        curr_file = readdir(dir_object);
    }

    closedir(dir_object);

    // ⭐ In our second pass, we're going to add all of the files to file_arr
    // Malloc the file_arr
    *pid_arr = malloc(sizeof(char*) * num_numeric_dir);

    // 🚨 Error checking for malloc.
    if (*pid_arr == NULL)
    {
        fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
        return -1;
    }

    int curr_idx = 0;               // The current file_arr index we're chucking our numeric directories in

    dir_object = opendir("/proc/");
    curr_file = readdir(dir_object);

    while (curr_file != NULL)
    {
        // @see https://www.gnu.org/software/libc/manual/html_node/Directory-Entries.html
        
        // A directory will return a d_type of 4
        if (curr_file->d_type == 4)
        {
            // Check to make sure that it's a number
            if (is_number(curr_file->d_name))
            {
                (*pid_arr)[curr_idx] = strdup(curr_file->d_name);
                curr_idx += 1;
            }
        }
        // Continue reading the new dirent
        curr_file = readdir(dir_object);
    }
    closedir(dir_object);

    // Now that we're done, it's qsort time
    qsort(*pid_arr, num_numeric_dir, sizeof(char*), compare);

    // The size of the pid array == The number of numeric directories that exists in /proc/ (which is actually the # of pids)
    *pid_arr_size = num_numeric_dir;
    
    return 0;
}

/**
 * Returns the owner of the current file specified in $path.
 * @param path The path of the file
 * @param owner_buffer {Outbound} Buffer to store the owner of the file. YOU ARE RESPONSIBLE FOR FREEING THIS.
 * @returns 0 if success, -1 if fail
*/
int get_owner(char* path, char** owner_buffer)
{
    // Declare a buffer to store the output of stat()
    struct stat buffer;

    // Make a system call to find the stat
    // 🚨 Error checking: Make sure that our stat command works
    if (stat(path, &buffer) == -1)
    {
        fprintf(stderr, "Error: stat() failed. %s.\n", strerror(errno));
        return -1;
    }

    uid_t uid = buffer.st_uid;

    // Next, we're going into the /etc/passwd directory to find information about this uid
    // @see https://linux.die.net/man/3/getpwuid
    struct passwd* pwd_db = getpwuid(uid);

    // 🚨 Error checking whether we can access the password directory
    if (pwd_db == NULL)
    {
        fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
        return -1;
    }

    /*
        Lengthy description of what is happening:
        getpwuid is a system library function. Whenever a library function returns a pointer, that is automatically sus because you
        don't know what's going to happen with that memory later down the line. In getpwuid's case, the pointer space gets 
        freed when other syscalls are invoked. Hence, unless I'm immediately using pwd_db->pw_name (which I am not), I should
        automatically bring this memory from the function/kernel's space to this program's memory space (where I can control).

        Note to self: refer to https://discord.com/channels/476749001912221706/670318091820990475/1220172110698254348 in the future
        for the detailed explanation of what's happening.

        TLDR; By the time char* owner gets the pointer from finishing the call to get_owner, and printf tries to dereference the pointer, 
        too much may have happend (milliseconds is all it takes for something like the scheduler to fuck with Kernel memory).

        Lesson of the day: make a copy of any pointers library functions (or even syscalls) give you.
        
        Now, the *good* thing a library function should do is take in a pointer to an address YOU control, and edit THAT
        ie. stat() takes in &buffer, which exists in your program's memory (and you own that directly)
        There's still no guarentee they won't just store &buffer somewhere and then free(&buffer) randomly, 
        but by convention, they shouldn't since you control that memory.

        System functions on the other hand don't guarentee you anything.
    */
    *owner_buffer = strdup(pwd_db->pw_name);

    // 🚨 Error checking for strdup (basically malloc)
    if (*owner_buffer == NULL)
    {
        fprintf(stderr, "Error: strdup() failed. %s.\n", strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * Performs the $lf command, which lists all files in the current working directory and prints it with a newline
 * @param curr_dir The current directory to $lf
*/
void lf(char* curr_dir)
{
    DIR* dir_object = opendir(curr_dir);

    // 🚨 Error checking: Make sure we can actually open the current_directory
    if (dir_object == NULL)
    {
        fprintf(stderr, "Error: Cannot open the current directory. %s.\n", strerror(errno));
        return;
    }

    // Start reading. curr_file will contain a dirent object for each file in the directory.
    struct dirent* curr_file = readdir(dir_object);

    // Continue to read until our dirent becomes null
    while (curr_file != NULL)
    {
        // Check so we don't print . and ..
        if (strcmp(curr_file->d_name, ".") != 0 && strcmp(curr_file->d_name, "..") != 0)
        {
            puts(curr_file->d_name);
        }

        // Continue reading the new dirent
        curr_file = readdir(dir_object);
    }

    closedir(dir_object);
}

/**
 * Lists all current processes within the system using the <PID> <USER> <COMMAND> format
*/
void lp()
{
    // First, let's obtain all the current process IDs.
    // There are outbound parameters in get_all_pids, so we'll leverage that
    char** pid_arr;
    int pid_arr_size;

    // 🚨 Error checking. If it's -1, just terminate the current command
    if (get_all_pids(&pid_arr, &pid_arr_size) == -1)
    {
        return;
    }

    // Iterate through all process IDs. Now, let's find their user and their command
    for (int i = 0; i < pid_arr_size; i++)
    {
        char* pid = pid_arr[i];

        // ⭐ Fumble with string concatenation to find the absolute path to /proc/<PID>/cmdline that will give us the running command
        char* cmdline_path = malloc(strlen("/proc/") + strlen(pid) + strlen("/cmdline") + 1);   // + 1 for null byte

        // 🚨 Error checking for malloc.
        if (cmdline_path == NULL)
        {
            fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
            continue;
        }

        strcpy(cmdline_path, "/proc/");
        strcat(cmdline_path, pid);
        strcat(cmdline_path, "/cmdline");

        // Open a low level POSIX file descriptor table pointer
        // This file will be read only
        int fp = open(cmdline_path, O_RDONLY);
        
        // 🚨 Error checking: Make sure we can open the cmdline file
        if (fp == -1)
        {  
            fprintf(stderr, "Error: Cannot open file %s. %s.\n", cmdline_path, strerror(errno));
            free(cmdline_path);
            continue;    
        }

        // off_t file_size = get_file_size(fp);
        // printf("%ld\n", file_size);

        // ⭐ Now, read the file content and chuck it into the $process_command buffer
        // Remember in this file, the file content is the process command
        char* process_command = malloc(1024);

        // 🚨 Error checking for malloc.
        if (process_command == NULL)
        {
            fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
            free(cmdline_path);
            continue;
        }

        process_command[0] = '\0';          // Immediately set 1st byte to null terminator. If the process_cmd has something, it'll overwrite this
        read(fp, process_command, 1024);

        // ⭐ Find the person that executed the process. In this case, it's the owner of the pid folder
        // A thing that was slightly omitted in the write-up: the person that initiated the process owns *everything* in the /proc/<pid>
        // folder. That includes cmdline. Hence, we just need to find who owns the pid
        char* process_owner;

        // 🚨 Handle error in case get_owner() failed
        if (get_owner(cmdline_path, &process_owner) == -1)
        {
            free(cmdline_path);
            free(process_command);
            continue;
        }

        // And then we are good.
        printf("%s %s %s\n", pid, process_owner, process_command);

        // Close file
        close(fp);

        // 🗑️ Garbage Collection
        free(process_owner);
        free(cmdline_path);
        free(process_command);
    }
    
    // 🗑️ Garbage Collection

    // DO NOT FREE THESE STRINGS - THESE DIRENT d_names in here are stored in READ/WRITE
    for (int i = 0; i < pid_arr_size; i++)
    {
        //puts(pid_arr[i]);
        free(pid_arr[i]);
    }

    free(pid_arr);

}

/**
 * Get the current executing user's default UID
 * @param default_dir_buffer {Outbound} A buffer to store the default directory char*. You are responsible for free()ing this.
 * @returns -1 if fail. 0 if successful.
*/
int get_default_dir(char** default_dir_buffer)
{
    // First, let's get the executing user's ID
    uid_t uid = getuid();
    
    // Now, we'll actually go to the user's *password* database struct to try to find the user's default directory
    // @see https://linux.die.net/man/3/getpwuid
    struct passwd* pwd_db = getpwuid(uid);

    // 🚨 Error checking whether we can access the password directory
    if (pwd_db == NULL)
    {
        fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
        return -1;
    }

    // We are going to clone this pointer because I do not fucking trust pwd_db to handle memory properly
    *default_dir_buffer = strdup(pwd_db->pw_dir);

    // 🚨 Error checking for strdup (which is basically malloc).
    if (*default_dir_buffer == NULL)
    {
        fprintf(stderr, "Error: strdup() failed. %s.\n", strerror(errno));
        return -1;
    }

    return 0;
}

/**
 * Change directories
 * @param argument_list The list of arguments passed into the command line. This is very important for $cd
 * @param num_arguments The number of arguments in $argument_list
*/
void cd(char** argument_list, int num_arguments)
{
    // 🚨 Error checking: Multiple arguments in $cd (maximum we should only have 1)
    if (num_arguments > 2)
    {
        fprintf(stderr, "Error: Too many arguments to cd.\n");
        return;        
    }

    if (num_arguments < 1)
    {
        fprintf(stderr, "Error: Too few arguments to cd.\n");
        return;        
    }

    // Has no arguments to $cd or the first argument is ~, return default dir            (throw err for can't change into curr dir)
    // this means argument_list should be [cd] and $num_arguments == 1
    if (num_arguments == 1 || strcmp(argument_list[1], "~") == 0)
    {
        char* default_dir;;

        // If get_default_dir had an error, stop running $cd
        // An error has already been thrown there
        if (get_default_dir(&default_dir) == -1)
        {
            return;
        }

        // 🚨 Error checking: Changing directories failed
        // While we do that, change into the default dir
        if (chdir(default_dir) == -1)
        {
            free(default_dir);
            fprintf(stderr, "Error: Cannot change directory to %s. %s.\n", default_dir, strerror(errno));
            return;
        }

        free(default_dir);
    }
    else if (num_arguments == 2)
    {
        // If there is an argument that's not ~, that's the directory we're changing into
        char* dest_dir = argument_list[1];
        char* original_dir = strdup(dest_dir);      // This is here because we will modify $dest_dir later

        // If the first thing in dest_dir is ~/, that means we need to navigate from the default directory.
        // ie. ~/../
        if (strlen(dest_dir) > 1 && dest_dir[0] == '~' && dest_dir[1] == '/')
        {
            // Let's change into that.
            char* default_dir;;
            if (get_default_dir(&default_dir) == -1)
            {
                return;
            }

            // 🚨 Error checking: Changing directories failed
            // If we can't go into ~/, we can't go into subsequent local directories either since permissions are recursive
            if (chdir(default_dir) == -1)
            {
                free(default_dir);
                fprintf(stderr, "Error: Cannot change directory to %s. %s.\n", default_dir, strerror(errno));
                return;
            }

            free(default_dir);

            // Now, change dest_dir to remove the '~/'
            // Remove the first two '~/'
            char* temp_dir = malloc(strlen(dest_dir) - 2 + 1);
            strncpy(temp_dir, dest_dir + 2, strlen(dest_dir) - 2 + 1);
            
            strcpy(dest_dir, temp_dir);
            free(temp_dir);
        }

        // 🚨 Error checking: Changing directories failed
        // While we do that, change into the destination dir
        if (chdir(dest_dir) == -1)
        {
            fprintf(stderr, "Error: Cannot change directory to %s. %s.\n", original_dir, strerror(errno));
            free(original_dir);
            return;
        }

        // Garbage Collection
        free(original_dir);
    }
}

/**
 * Handles external commands (not explicitly defined in the minishell) by forking it and calling exec to execute it.
 * We will use the execvp system call since we get to choose any flavor of exec() to use.
 * This will clone/assume/use the user environment when executing mimishell
 * @param command The command to execute (ie. $ls)
 * @param arguments_list The arguments list for said command (ie. $echo ["hi", "bozo"])
*/
void handle_external_cmd(char* command, char** arguments_list)
{
    // execvp will overwrite your process image, which is very bad for us.
    // Hence, we'll need to fork it and run the code inside a child process
    pid_t pid = fork();

    // 🚨 Error checking: Fork failed
    if (pid == -1)
    {
        fprintf(stderr, "Error: fork() failed. %s.\n", strerror(errno));
        return;
    }

    // If you are the child process, execute the external cmd
    if (pid == 0)
    {
        execvp(command, arguments_list);

        // If we reach here (and the exe didn't just immediately terminate) something went wrong
        fprintf(stderr, "Error: exec() failed. %s.\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    else    // If you are the parent process, wait for the child process to finish
    {
        int child_status_code;              // This status code is useless to us

        // Call waitpid. Meanwhile, error check
        if (waitpid(pid, &child_status_code, 0) == -1)
        {
            fprintf(stderr, "Error: wait() failed. %s.\n", strerror(errno));
        }
    }
}

int main(int argc, char** argv)
{
    // ⭐⭐ First things first - handle SIGINT.
    struct sigaction action = {0};
    action.sa_handler = sig_handler;
    //action.sa_flags = SA_RESTART;

    // Install the handler
    // Meanwhile, 🚨 Error checking: Signal Handler not registering
    // If the signal handler isn't registering, we can still use minishell so don't just exit it
    if (sigaction(SIGINT, &action, NULL) == -1)
    {
        fprintf(stderr, "Error: Cannot register signal handler. %s.\n", strerror(errno));
    }

    // ⭐ We do need to have our current working directory.
    // Let's take care of that
    char* curr_dir = getcwd(NULL, 0);

    // 🚨 Error checking: We can't get the current working directory
    if (curr_dir == NULL)
    {
        fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
        // If we can't read the current working directory, our shell just fails
        exit(EXIT_FAILURE);
    }

    // ⭐ Let's parse the command line input by each stdin shell line
    
    // The following is modified from ShuDong's textbook
    char* line = malloc(1024);

    // Adding the interrupted at the end of this getline while loop since we don't want termination upon SIGINT
    while (1)
    {
        // Print the current working directory in blue
        printf("%s[%s]%s> ", BLUE, curr_dir, DEFAULT);

        if ((fgets(line, 1024, stdin)) != NULL || interrupted == 1)
        {   
            if (interrupted == 1)
            {
                interrupted = 0;            // Change interrupted back to 0
                printf("\n");

                // printf("%s[%s]%s> ", BLUE, curr_dir, DEFAULT);
                continue;
            }
            // First, there is a pesky little newline at the end of getline that ruins a lot of stuff
            // Let's get rid of that
            size_t newline_idx =  strlen(line) - 1; // \n should be at the last index

            // 🚨 Error checking: If the user inputted nothing inside the command, just go to the next line
            if (newline_idx == 0)
            {
                printf("%s[%s]%s> ", BLUE, curr_dir, DEFAULT);
                continue;
            }

            line[newline_idx] = '\0';           // I think overwriting this with \0 shouldn't give us valgrind issues

            // ⭐ Great! Now that we can read from STDIN forever, let's parse each minecraft-like command.
            // These are space delinated. We will use strok from string.h to split

            /*
                First, we will count how many spaces there are in the command so we know how many indices we need to allocate
                in the command array. For instance:

                echo Horrified looks from everyone in the room
                > There are 7 spaces. Hence, there are 7 + 1 = 8 words.
                We are including $echo as a parameter
            */
            int num_arguments = num_chars_in_str(line, ' ') + 1;

            // strtok() ignores delimiters from start and end. Hence, subtract them from the number of ' '
            num_arguments -= num_chars_start_end(line, ' ');

            // Remember we're mallocing pointers (8 bytes)
            // We are adding 1 to the number of arguments we're mallocing because the last one will be the Sentinel (NULL)
            char** arguments_list = malloc((num_arguments + 1) * 8);

            // 🚨 Error checking for malloc.
            if (arguments_list == NULL)
            {
                fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
                continue;       // I don't think we exit here, so we just terminate things right here and now
                //exit(EXIT_FAILURE);
            }

            // Use strtok to read $num_arguments number of ... well, arguments
            // strok uses a token to iterate through each "instance" of the .split() command
            char* token = strtok(line, " ");
            for (int i = 0; i < num_arguments; i++)
            {
                arguments_list[i] = token;

                // Update the token
                token = strtok(NULL, " ");
            }

            // Add the Sentinel
            arguments_list[num_arguments] = NULL;
            
            // for (int i = 0; i < num_arguments; i++)
            // {
            //     puts(arguments_list[i]);
            // }

            // ⭐ Handle command line arguments. Switch statements don't really support strings in C, so we'll use if statement
            char* command = arguments_list[0];

            // When comparing, remember that our commands are terminating (contains) a newline
            if (strcmp(command, "cd") == 0)
            {
                cd(arguments_list, num_arguments);
            }
            else if (strcmp(command, "exit") == 0)
            {
                free(arguments_list);
                break;          // When you get exit, break out of this loop and go straight to the exit instructions
            }
            else if (strcmp(command, "pwd") == 0)
            {
                // This is the same thing we print before every command line
                printf("%s\n", curr_dir);
            }
            else if (strcmp(command, "lf") == 0) 
            {
                lf(curr_dir);
            }
            else if (strcmp(command, "lp") == 0)
            {
                lp();
            }
            else
            {
                // If the commands don't exist on the minishell, spawn a child to take care of this
                handle_external_cmd(command, arguments_list);

                // Stop fgets from running twice in the if statement above
                if (interrupted)
                {
                    interrupted = 0;
                }
            }

            // 🗑️ Garbage Collection now that we're done with arguments_list
            // We don't need to free the pointers inside arguments_list, because strtok actually directly manipulates
            // $line and just replaces delimiters with \0. And on top of that, $line is in the stack
            free(arguments_list);
            free(curr_dir);

            // Updates and re-prints the current working directory in blue
            curr_dir = getcwd(NULL, 0);

            // 🚨 Error checking: We can't get the current working directory
            if (curr_dir == NULL)
            {
                fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
                free(line);
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            // If we're exiting the loop, check if it's because we can no longer getline().
            // This means we have an issue with stdin
            fprintf(stderr, "Error: Failed to read from stdin. %s.\n", strerror(errno));
            free(line);
            free(curr_dir);

            // We're going to make this exit_failure because it completely shuts down our minishell
            exit(EXIT_FAILURE);
        }
    }

    free(line);         // 🗑️ Garbage Collection
    free(curr_dir);

    return EXIT_SUCCESS;
}