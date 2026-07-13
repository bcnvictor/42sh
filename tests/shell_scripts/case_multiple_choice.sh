#!/bin/sh
var=foo
var=issou

case $var in 
(foo|bar|baz) 
    echo "fail: $var != foo|bar|baz";; 
(horse) echo "fail: horse ???";; (a|b|issou) echo "ok (correct)";;

esac

