#!/bin/sh
[ $# -ne 1 ] && echo "Sorry, expected 1 argument but $# were passed" && exit 1
[ ! -f $1 ] && echo "$1:" && echo "	is not a valid file"  && exit 2
cat $1

