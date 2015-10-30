#!/bin/sh -
env GPSOPTS="-Pa -x" SHOGIUSER=nagai_gps SHOGIPASS=nagai_gps LIMIT=3600 NODELIMIT=6200000 LOOP=-1 \
  GAMENAME=nagai_gps-36000-0 ./network_x1.pl
