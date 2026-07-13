#!/bin/sh
var=foo
var=issou

case $var in
(foo|bar|baz)
    echo "fail: $var != foo|bar|baz";;
$var)
    echo 'good';;
(horse) echo "fail: horse ???";; (a|b|issou) echo "ok (correct)"

esac

