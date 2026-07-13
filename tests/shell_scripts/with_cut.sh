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
    if [ $val -lt $((1 + $2)) ]; then
        exit 1
    fi
    val1=$(head "$1" -n $2 | tail -n 1 | cut -d ";" -f "2")
    if [ "$val1" = "" ]; then
        exit 1
    fi
    val2=$(head "$1" -n $2 | tail -n 1 | cut -d ";" -f "3")
    printf "%s is %s\n" "$val1" "$val2"
else
    exit 1
fi
