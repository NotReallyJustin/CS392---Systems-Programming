#!/bin/sh

# This program checks if 9 + 10 == 21
NINE_PLUS_TEN=$((9 + 10));

if [[ $NINE_PLUS_TEN -eq 21 ]]; then 
    echo "Yay! It's 21!";
else
    echo "This is a scam. The conspiracy theorists say 9 + 10 = $NINE_PLUS_TEN";
fi

