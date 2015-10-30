#!/bin/sh

PW=`cat ./.gps_l.password`

while true
do

  logger -s "gps_l: Restarting..."

  env GPSNAME="./gpsone -vc " GPSOPTS="-Pa -x" SHOGIUSER=gps_l \
      SHOGIPASS="$PW" GAMENAME=floodgate-900-0 LOOP=-1 MULTIGPS=1 ./network_x1.pl

  logger -s "gps_l: Sleeping..."
  sleep 1800
done
