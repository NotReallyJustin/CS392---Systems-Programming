#!/bin/sh

# Lab Assignment: Write a bash file that prints out which flags out of hlp are being used right now.
# If there is an illegal flag, put it in STDERR and exit 1

# Use a tracker and a list to track the flags. This takes care of duplicating flags
hlp_tracker=(false false false);
hlp_list=("h" "l" "p")
illegal=false

while getopts ":hlp" flag; do
    case $flag in
        h)
            hlp_tracker[0]=true;
            ;;
        
        l)
            hlp_tracker[1]=true;
            ;;
        p)
            hlp_tracker[2]=true;
            ;;        
        *)
            illegal=true;
            ;;
    esac
done

# If we have an illegal option, chuck it in STDERR
if [[ $illegal -eq 1 ]]; then
    echo "illegal option!" >&2 ;
    exit 1;
else
    # We're checking hlp_tracker. If true, add the hlp_list item to output flags
    output_flags=""

    for i in ${!hlp_tracker[@]}; do
        if [[ hlp_tracker[i] ]]; then
            output_flags="$output_flags${hlp_list[i]}"
        fi
    done;

    echo "Flags $output_flags are used";
    exit 0;
fi
