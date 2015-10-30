#!/bin/sh -

while true
do
  logger -s "gps_wdoor: Restarting..."

  env MULTIGPS=1 GPSOPTS="-Pa -x" SHOGIUSER=gps SHOGIPASS=gps GAMENAME=gps-1500-0 LOOP=-1 ./network_x1.pl

  logger -s "gps_wdoor: Sleeping..."
  sleep 600
done

