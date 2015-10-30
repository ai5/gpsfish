#!/bin/sh

while true
do
  logger -s "gps_normal: Restarting..."

  env SERVER=localhost GPSOPTS="-Pa -x -e progress" MULTIGPS=1 \
  SHOGIUSER=gps_normal SHOGIPASS=floodgate GAMENAME=floodgate-900-0 \
  LOOP=-1 ./network_x1.pl

  logger -s "gps_normal: Sleeping..."
  sleep 1800
done
