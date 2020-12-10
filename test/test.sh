#!/bin/bash
# 3 = p
# 4 = n
# 5 = i
test () {
    rm $1
    printf "Vector size, seed, number of threads, iterations, time\n" >> $1
    for ((i = 0; i < $2; i++)) do
        while IFS=" " read p seed threads iters time
        do
            printf "%s,%s,%s,%s,%s\n" \
                $p $seed $threads $iters ${time/. /,}
        done < <(
# test not sec   ../saxpy-sec/saxpy-sec.out -p $3 -n $4 -i $5 \
                 ../saxpy-sec/saxpy-sec.out -p $3 -i $4 \
                 | tail -4 | head -2   \
                 | sed 's/[^0-9.]/ /g' \
                 | sed 's/ \+/ /g'     \
                 | sed 'N;s/\n/ /'     \
                ) >> $1
    done
}

main () {
    # test with threads
    # test "data1.csv" 10 10000000 1 1
    # test "data2.csv" 10 10000000 2 1
    # test "data3.csv" 10 10000000 4 1
    # test "data4.csv" 10 10000000 8 1
    # test "data5.csv" 10 10000000 1 10
    # test "data6.csv" 10 10000000 2 10
    # test "data7.csv" 10 10000000 4 10
    # test "data8.csv" 10 10000000 8 10
    # test "data9.csv" 10 10000000 1 100
    # test "data10.csv" 10 10000000 2 100
    # test "data11.csv" 10 10000000 4 100
    # test "data12.csv" 10 10000000 8 100
    # test "data13.csv" 10 10000000 1 1000
    # test "data14.csv" 10 10000000 2 1000
    # test "data15.csv" 10 10000000 4 1000
    # test "data16.csv" 10 10000000 8 1000

    test "data1.csv" 10 10000 1
    test "data2.csv" 10 10000 10
    test "data3.csv" 10 10000 100
    test "data4.csv" 10 10000 1000
    test "data5.csv" 10 100000 1
    test "data6.csv" 10 100000 10
    test "data7.csv" 10 100000 100
    test "data8.csv" 10 100000 1000
    test "data9.csv" 10 1000000 1
    test "data10.csv" 10 1000000 10
    test "data11.csv" 10 1000000 100
    test "data12.csv" 10 1000000 1000
    test "data13.csv" 10 10000000 1
    test "data14.csv" 10 10000000 10
    test "data15.csv" 10 10000000 100
    test "data16.csv" 10 10000000 1000
    exit 0
}

main "$@"