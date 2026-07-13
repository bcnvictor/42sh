#!/bin/sh

if true;
then
    echo 'ab'
else
    echo 'ba'
fi
if false;
then
    echo 'ab'
else
    echo 'ba'
fi
