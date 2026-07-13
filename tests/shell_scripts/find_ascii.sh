#!/bin/sh

ls $1 | while read filename; do
    file "$1/$filename" | grep -i "ASCII text"
done
