#!/bin/zsh

rm out/traces/*
for trace in traces/*.pcap(|ng); do
    ./packetstats-test.py $trace | less
    # local out=$(./packetstats-test.py "$trace" | tail -n1)
    # if [[ out != 'All the list of unique addresses or port #s seem to match. Good Job!' ]]; then
    #     echo "oops $trace"
    # fi

    # local out=$(diff <(./packetstats -m -a -u -t -f "$trace") <(./packetstats-example -m -a -u -t -f "$trace"))
    # if [[ -n "$out" ]]; then
    #     echo "$out" >> "out/$trace"
    # fi
done
