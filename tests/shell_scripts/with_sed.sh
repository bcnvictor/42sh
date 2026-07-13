#!/bin/sh

if [ $# -ne 2 ]; then
    exit 1
fi
if [ "$2" = "nan" ]; then
   exit 1
fi
if [ $2 -lt 1 ]; then
    exit 1
fi
if [ -f "$1" ]; then
    val=$(wc -l < "$1")
    if [ "$val" -lt $2 ]; then
        exit 1
    fi
    line=$(sed -n "${2}p" "$1")
    if [ "$line" = "" ]; then
        exit 1
    fi
    echo "$line" | sed -E "s/^[^;]*;([^;]*);([^;]*);[^;]*;$/\1 is \2/"
else
    exit 1
fi
