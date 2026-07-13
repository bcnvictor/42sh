#!/bin/sh

if [ $# -ne 1 ]; then
    exit 1
fi

VAL=1
INT=$1

while [ $INT -gt 1 ]; do
    VAL=$(($VAL * INT))
    INT=$(($INT - 1))
done

echo $VAL
