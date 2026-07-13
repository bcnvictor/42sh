varcase=woof

case $varcase in 
cat) echo "fail: case 1";;
horse) echo "fail: case 2";;
*) echo "other animal (correct)"
esac

