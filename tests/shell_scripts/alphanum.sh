#!/bin/sh

while read input; do
    if [ $(echo "$input" | grep -E -c "^[ 	]*$") -ge 1 ]; then
        echo it is empty
    elif $(echo "$input" | grep -E -q "^[0-9]$"); then
        echo it is a digit
    elif $(echo "$input" | grep -E -q -i "^[a-z]+$"); then
        echo it is a word
    elif $(echo "$input" | grep -E -q "^[0-9]+$"); then
        echo it is a number
    elif $(echo "$input" | grep -E -q -i "^[a-z0-9]+$"); then
        echo it is an alphanum
    else
        echo it is too complicated
        exit 0
    fi
done
