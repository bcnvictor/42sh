#!/bin/sh

if [ $# -ne 1 ]; then
    exit 1
fi
if [ -f $1 ]; then
    while IFS= read -r LINE; do
        word=$(printf "%s" "$LINE" | wc -c)
        if [ $word -ge 80 ]; then
            printf "%s\n" "$LINE"
        fi
    done < "$1"
else
    exit 1
fi
