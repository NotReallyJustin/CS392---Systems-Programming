#!/bin/bash

# *******************************************************************************
#  Author  : Justin Chen
#  Date    : 1/26/2024
#  Description: CS392 - Homework 1
#  Pledge  : I pledge my honor that I have abided by the Stevens Honor System.
# ******************************************************************************

# Before we start, let's declare a function that will print out the annoying usage statement
#echo "$@";
print_usage () {

cat << EOF
Usage: rbin.sh [-hlp] [list of files]
   -h: Display this help;
   -l: List files in the recycle bin;
   -p: Empty all files in the recycle bin;
   [list of files] with no other flags,
        these files will be moved to the
        recycle bin.
EOF

}

# ⭐ Very important - whenever rbin.sh is invoked, if a recycle bin does not exist in the users' home directory, create it
readonly RBIN_PATH=~/.recycle/;

# If the recycle bin doesn't exist:
if [[ ! -d $RBIN_PATH ]]; then
    # Create the recycle bin
    mkdir $RBIN_PATH;

    # Give the user perms to read, write, and execute perms in the recycle bin
    # Other users can just get read perms
    chmod 744 $RBIN_PATH;
fi

# ⭐ First, let's parse the rbin inputs. We will use an array to store the flags.
num_flags_true=0
flags_set=(0 0 0)   # Corresponds to h, l, and p

# Use getopts flag in a while loop to loop through all flags that exist
while getopts ":hlp" flag; do
    case $flag in
        h)
            # If a flag exists, AND if it's not already set set it in $flags_set
            # Add 1 to the number of flags set
            if [[ ! ${flags_set[0]} -eq 1 ]]; then
                flags_set[0]=1;
                num_flags_true=$(( $num_flags_true + 1 ))
            fi
            ;;
        l)
            if [[ ! ${flags_set[1]} -eq 1 ]]; then
                flags_set[1]=1;
                num_flags_true=$(( $num_flags_true + 1 ))
            fi
            ;;
        p)
            if [[ ! ${flags_set[1]} -eq 2 ]]; then
                flags_set[2]=1;
                num_flags_true=$(( $num_flags_true + 1 ))
            fi
            ;;
        *)
            # If we get here, we have an invalid argument
            # Print out error message and usage statement
            echo "Error: Unknown option '-$OPTARG'". >&2;           # Use of $OPTARG taken from tldp
            print_usage;

            # Exit with status 1
            exit 1;
            ;;
    esac
done

# ⭐ Second, let's parse file inputs.
# First, create an array for the file inputs
files_to_process=()

# Let's loop through all arguments. If they don't fall under a flag (has -), then they're assumed to be file elements
for argument in "$@"; do
    
    # Wildcards in if statements are weird. I just went without the quotes for it to match
    if [[ $argument != -* ]]; then
        # Append to files_to_process
        files_to_process+=("$argument")
    fi

done

# ⭐ Now, we will print errors in the # of parameters and flags

# 🚨 Throw an error if there's more than 1 flag
if [[ $num_flags_true -gt 1 ]]; then
    echo "Error: Too many options enabled." >&2;
    print_usage;
    exit 1;
fi

# 🚨 Throw an error if there's any flags and has file inputs
if [[ $num_flags_true -gt 0 && ${#files_to_process[@]} -gt 0 ]]; then
    echo "Error: Too many options enabled." >&2;
    print_usage;
    exit 1;
fi

# ⭐ Now that we're done error handling, let's handle the flags and command line inputs.
if [[ ${flags_set[0]} -eq 1 ]]; then

    # Help flag - print the usage.
    print_usage;

elif [[ ${flags_set[1]} -eq 1 ]]; then

    # Lists all files in the recycle bin
    ls -lAF $RBIN_PATH;

elif [[ ${flags_set[2]} -eq 1 ]]; then

    # Empty all files in the recycle bin
    # Because we are creating the recycle bin again every single time we run this script, we can just remove .recycle
    rm -r $RBIN_PATH;

elif [[ ${#files_to_process[@]} -gt 0 ]]; then

    # Loop through all the file paths specified in the arguments and chuck it inside the recycle bin.
    for file_path in ${files_to_process[@]}; do

        # If the file doesn't exist, print out a warning
        if [[ ! -e $file_path ]]; then
            echo "Warning: '$file_path' not found." >&2 ;
        else   
            mv $file_path $RBIN_PATH;
        fi

    done

else
    # If there are no flags, print usage
    print_usage;
fi

# Exit normally
exit 0;