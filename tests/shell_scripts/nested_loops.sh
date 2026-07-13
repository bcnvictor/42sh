foo() {
    for i in 1 2 3 4 5; do
        echo $i
        if echo $i; then
            echo "c'est le troisieme"
        fi
        for j in 1 2 3 4 5; do
            echo $j
        done
    done
}

bar() {
    while true; do
        echo a
        if echo 1 && true && echo "bon bah d'accord"; then
            until true; do
                echo $1 >>bar.txt
                echo "c chaud la"
                if echo 1 && echo 2 && echo 3; then
                    echo 1 >>bar.txt
                    break 2
                else
                    echo txt >>bar.txt
                fi
            done
        else
            for i in 1 2 3 4 5; do
                if true; then
                    echo $1
                    echo "pas de math d'accord"
                else
                    echo "bah ouais mais c'est pas toi qui decide"
                fi
            done
            break
        fi
    done
}

foo

echo "..."
