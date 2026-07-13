#!/bin/sh
touch ' '
touch '\'
touch -- --
touch '|'
touch \"
touch \'
touch -- --\$i\*\'\"\\
touch "# Exams are fun!"
touch -- ";\`kill -9 0\`"
var=0
file=""
while [ $var -lt 50 ];do
    file=${file}$((${var}+1))\/
    mkdir $file
    var=$(($var+1))
done
touch ${file}farfaraway
chmod 0644 *
