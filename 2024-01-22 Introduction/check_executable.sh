#!/bin/sh

# This program checks if a file is executable.
# If not, we give it permissions to execute

if [[ ! (-x ./array_tricky.sh) ]]; then
    chmod +x ./array_tricky.sh
fi