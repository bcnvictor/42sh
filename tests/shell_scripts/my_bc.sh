#!/bin/sh
if [ $# -gt 0 ]; then
    echo $(($1))
else
    while read line; do
        echo $(($line))
    done
fi
