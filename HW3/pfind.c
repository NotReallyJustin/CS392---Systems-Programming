/*
Justin Chen
CS392 HW 3
I pledge my honor that I have abided by the Stevens Honor System.
*/
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

/**
 * Checks the validity of the permission string given by the user input.
 * @param perm_str The permission string the user has inputted
 * @returns 1 if the string is valid, 0 otherwise
*/
int is_valid_perm(char* perm_str)
{
    char perm_bits[] = {'r','w','x'};
    int curr_bit_pos = 0;                   // Index of perm_bits we are currently on so we can try to validate that char string

    // First, check the length of perm_str.
    // Permission strings must always be 9 for rwxrwxrwx
    if (strlen(perm_str) != 9)
    {
        return 0;
    }

    // Next, we create a loop to check the positions of perm_str
    for (int i = 0; i < 9; i++)
    {
        char curr_char = perm_str[i];

        // If the current character is a - or a valid perm_bit, then we goog. Else, it's bad and we exit.
        if (curr_char != perm_bits[curr_bit_pos] && curr_char != '-')
        {
            return 0;
        }

        // Updates the curr_bit_pos. We are looping back using %3 since this perm_bits array is size 3
        curr_bit_pos = (curr_bit_pos + 1) % 3;
    }

    // If all goes well, return 1 (true)
    return 1;
}

/**
 * Traverses through a given directory from $dir_path, and prints all files which permissions match $permission_nums
 * @param dir_path The directory to traverse
 * @param permission_nums 3 digit number, similar to those in $chmod, that represent file permissions for user, group, and other
*/
void print_matching_perms(char* dir_path, int* permission_nums)
{
    // Before we start, we need to work in terms of absolute path. Let's $cd there and $pwd ourselves.

    // This is a syscall, so let's error check for -1
    if (chdir(dir_path) == -1)
    {
        perror("");
        exit(EXIT_FAILURE);
    }
    
    char* abs_path;
    abs_path = getcwd(NULL, 0);
    
    // ⭐ First, let's open the specified directory using the absolute path
    DIR* curr_dir = opendir(abs_path);

    // ⭐ Now, we continuously read from curr_dir until we run into a null pointer
    // In which case we know that we have no more files left
    struct dirent* curr_file;
    curr_file = readdir(curr_dir);

    while (curr_file != NULL)
    {
        // Now, we must obtain the file path. We have the name and curr directory path.
        // We malloc an extra byte for the / character between dir_path and file name
        char* file_path = malloc(strlen(abs_path) + 1 + strlen(curr_file->d_name) + 1);
        strcpy(file_path, abs_path);
        strcat(file_path, "/");
        strcat(file_path, curr_file->d_name);

        // ⭐ Now, we run the stat function to find the metadata about our current file
        struct stat curr_file_stats;
        stat(file_path, &curr_file_stats);

        // ⭐ 1. First, we check that it's a normal file.
        // If it's not a normal file, we won't do anything with it
        // Technically S_ISREG doesn't return 1 or 0, but anything that's not 0 or 0000... is true, so we good
        if (S_ISREG(curr_file_stats.st_mode))
        {
            // ⭐ 2. Check the file permissions.
            // We have the $chmod numbers - so all we need to do is compare that with the RWX mask for each group

            // First, let's derive the permission group numbers.
            // It's not the most efficient, but it is the most readable way of doing this which is gonna be important
            // when I study with this later

            // We need to ASR by 6 bits since that's gets us the value of the 3rd digit of the $chmod octal number
            // ie. st_mode & S_IRWXU returns *0*700. To get that 7, we need *0*700 >> 6. This is now *0*7, which we can use
            int user_perm = (curr_file_stats.st_mode & S_IRWXU) >> 6;
            int group_perm = (curr_file_stats.st_mode & S_IRWXG) >> 3;
            int other_perm = (curr_file_stats.st_mode & S_IRWXO);       // No shift required - this is the last $chmod

            // Now, check all 3 group perms. If they match, print the absolute file path
            if (
                user_perm == permission_nums[0] &&
                group_perm == permission_nums[1] &&
                other_perm == permission_nums[2]
            )
            {
                puts(file_path);
            }
        }
        else if (S_ISDIR(curr_file_stats.st_mode) && strcmp(curr_file->d_name, ".") != 0 && strcmp(curr_file->d_name, "..") != 0)
        {
            // If it's a directory we're looking at, recursively print matching perms for that directory
            // Remember to not handle directories . and .., since those will give you a self loop forever
            print_matching_perms(file_path, permission_nums);
        }

        // Garbage collection
        free(file_path);

        // Updates curr_file
        curr_file = readdir(curr_dir);
    }

    // Garbage collection
    free(abs_path);
}

/**
 * Main entry point to our pfind command.
 * @param argc The number of arguments (should be 1, this is guarenteed)
 * @param argv The actual arguments
 * @returns An exit code (when main terminates, the exit handler will return this)
*/
int main(int argc, char** argv)
{
    char* permission_str = argv[2];

    // Checks if our permission string is invalid.
    if (is_valid_perm(permission_str) == 0)
    {
        fprintf(stderr, "Error: Permissions string '%s' is invalid.\n", permission_str);
        return EXIT_FAILURE;
    }

    // Let's convert our permission_str into 3 digit numbers that represent $chmod permissions
    int permission_nums[3];
    int pow_of_2[3] = {4, 2, 1};        // 2^2, 2^1, 2^0. We'll need them to create $chmod numbers
    int curr_idx = 0;           // Current index of permission_nums we are editing

    int curr_perm_num = 0;      // The current permission number (such as 7, 4, 1... etc) we have
    for (int i = 0; i < strlen(permission_str); i++)
    {
        // Create perm_position to track which bit in each group's permission we are working with.
        int perm_position = i % 3;

        // Calculate permission number of current item - if it's not empty
        if (permission_str[i] != '-')
        {
            curr_perm_num += pow_of_2[perm_position];
        }

        // At the end bits 2, 5, and 8 (end of each group's permissions)
        // Incrememnt curr_idx, reset our running total, and add this group's permission number into permission_nums
        if (perm_position == 2)
        {
            permission_nums[curr_idx] = curr_perm_num;
            curr_perm_num = 0;
            curr_idx = curr_idx + 1;
        }
    }

    print_matching_perms(argv[1], permission_nums);

    return EXIT_SUCCESS;
}