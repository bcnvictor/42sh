#!/bin/sh
var="txt"
if [ $# -gt 0 ];then
    var=$1
fi
var1=*.$var
[ "$var1" = \*\.$var ] || exit 1
rm *.$var
