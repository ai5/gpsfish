#!/bin/sh -
env GPSNAME="/usr/local/bin/ruby -I /home/ktanaka/work/gpsshogi-nodist/slugshogi /home/ktanaka/work/gpsshogi-nodist/slugshogi/gpsshogi_par3.rb" GPSOPTS="-P -x" SHOGIUSER=gps_parX3 SHOGIPASS=gps_parX3 LOOP=-1 \
  GAMENAME=floodgate-900-0 ./network_x1.pl
