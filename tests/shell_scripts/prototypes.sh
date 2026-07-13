#!/bin/sh
if [ $# -ne 1 ]; then
    exit 1
fi
sed -nr -E '/^((int)|(void)|(char)|(long)|(double)|(float)|(struct)).*\)$/s/$/;/p' "$1"
