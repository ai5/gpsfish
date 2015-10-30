#!/bin/sh
mkdir -p tmp
rm -f tmp/gps1out tmp/gps2out
touch -f tmp/gps1out tmp/gps2out
sleep 1
tail -f tmp/gps2out | ./sente -issente 0 -opname GPS2 -myname GPS1  >> tmp/gps1out  2> logs/`date +%Y%m%d%H%MGPS1`.log &
sleep 1
tail -f tmp/gps1out | ./sente -issente 1 -opname GPS1 -myname GPS2  >> tmp/gps2out  2> logs/`date +%Y%m%d%H%MGPS2`.log &
echo started
