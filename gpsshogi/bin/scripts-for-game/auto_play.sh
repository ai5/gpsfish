#!/bin/zsh

if [ -z $1 ]; then
 echo "usage $0 gamename"
 exit 1
fi

repeat 50
do
echo next sente
./gpsshogi.pl -s -g $1
echo sleep 3 sec
sleep 3
echo next gote
echo waiting for opponent hand
./gpsshogi.pl -g $1
echo sleep 30 sec
sleep 30
done
