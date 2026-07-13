varcase=hello

case $varcase in 
hi) echo "fail: hello != hi";;
hello) echo "$varcase correct !";;
bonjour) echo "fail: bonjour != hello";;
esac

