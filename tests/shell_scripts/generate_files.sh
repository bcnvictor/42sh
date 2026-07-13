#!/bin/sh
FILENAME=default
NUMBER=1
EXTENSION=txt
while [ $# -gt 0 ]; do
    case "$1" in
    -f | --filename)
        shift
        FILENAME=$1
        ;;
    -n | --number)
        shift
        NUMBER=$1
        ;;
    -e | --extension)
        shift
        EXTENSION=$1
        ;;
    *)
        exit 1
        ;;
    esac
    shift
done
for i in $(seq $NUMBER);do
    touch -- "$FILENAME-$i.$EXTENSION"
done
