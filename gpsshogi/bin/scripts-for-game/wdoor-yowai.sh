#!/bin/sh -

while true
do
  logger -s "gps_wdoor-yowai: Restarting..."

  env MULTIGPS=1 GPSOPTS="-xa" SHOGIUSER=yowai_gps SHOGIPASS=yowai_gps GAMENAME=yowai_gps-1500-0 LIMIT=500 LOOP=-1 ./network_x1.pl

  logger -s "gps_wdoor-yowai: Sleeping..."
  sleep 600
done

