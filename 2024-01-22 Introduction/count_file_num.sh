#!/bin/sh
LS_OUTPUT=$(ls)

# array_tricky.sh check_executable.sh count_file_num.sh hello_world.sh nine_plus_ten.sh variables.sh
echo $LS_OUTPUT

# Let's make this an array
LS_ARR=( $LS_OUTPUT )

echo ${#LS_ARR[@]}      # This should be 6
echo ${LS_ARR[@]}