#!/bin/sh

if [ $# -ne 3 ]; then
    echo Usage: ./seq.sh FIRST INCREMENT LAST 1>&2
    exit 1
fi

i1=$1

if [ $1 -lt $3 ]; then
    if [ $2 -lt 1 ]; then
        exit 1
    fi
    while [ $i1 -lt $(($3 + 1)) ]; do
        echo $i1
        i1=$(($i1 + $2))
    done
elif [ $1 -gt $3 ]; then
    if [ $2 -gt -1 ]; then
        exit 1
    fi
    while [ $i1 -ge $3 ]; do
        echo $i1
        i1=$(($i1 + $2))
    done
else
    if [ $2 -eq 0 ]; then
        exit 1
    fi
    echo $1
fi
exit 0
