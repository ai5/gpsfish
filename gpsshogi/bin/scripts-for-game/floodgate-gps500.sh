#!/bin/sh

while true
do
  logger -s "gps500: Restarting..."

  env SERVER=localhost GPSOPTS="-xa" SHOGIUSER=gps500 MULTIGPS=1 \
  SHOGIPASS=floodgate GAMENAME=floodgate-900-0 LIMIT=500 LOOP=-1 ./network_x1.pl

  logger -s "gps500: Sleeping..."
  sleep 1800
done

