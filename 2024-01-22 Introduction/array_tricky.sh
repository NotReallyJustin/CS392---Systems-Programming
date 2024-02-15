#!/bin/sh
arr=();
arr[3]=1;

# Let's calculate array size
# This size is 1, because Linux array size is the # of elements that's non-empty
# I REPEAT: LINUX LIST SIZE ONLY COUNTS NON-EMPTY
echo ${#arr[@]}

# Let's print the array
# All the other elements are null, so nothing is printed
echo ${arr[@]}