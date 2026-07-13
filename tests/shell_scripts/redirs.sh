echo toto > test.txt; echo foo >> test.txt; cat < test.txt; echo $?
echo foo > ezo > cat | cat -e cat ; echo $?
echo Hello > test.txt | cat -e test.txt; echo $?
echo biktor > BIKTOR; cat BIKTOR; echo $?
echo toto > test.txt; echo foo >> test.txt; cat < test.txt; echo $?
echo thomas > ship.txt >; echo $?
echo 1 2 3 > mul_redir.txt 1 2 3 > mul_redir2.txt > cat < mul_redir.txt; echo $?
echo Error 1>&2; echo $?
echo no good >/dev/null; echo $?
> BIKTOR echo biktor; cat BIKTOR; echo $?
> test.txt echo toto; >> test.txt echo foo; < test.txt cat; echo $?
1>&2 echo Error; echo $?
1>&2 echo Error 1>&2; echo $?
echo no good >/dev/null; echo $?
rm test.txt; echo Hello > test.txt; cat test.txt
