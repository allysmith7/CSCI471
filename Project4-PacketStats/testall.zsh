#!/bin/zsh

for trace in traces/*.pcap(|ng); do
  ./packetstats -f "$trace" >! "out/$trace"
done