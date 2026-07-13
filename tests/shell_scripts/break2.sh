for i in 1 2 3 4 5;
do
    echo $i
    for j in 6 7 8 9;
    do
        echo $i
        break 2;
    done;
done;
