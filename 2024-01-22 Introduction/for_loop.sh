#!/bin/sh
# This iterates through all files on the current directory

LS_ARR=( $(ls) );

# Iterate using for in normally
for i in ${LS_ARR[@]}; do
    echo $i
done;

# Iterate using for in (range)
for i in ${!LS_ARR[@]}; do
    echo ${LS_ARR[$i]}
done;

for i in {1..10}; do
    echo $i
done;