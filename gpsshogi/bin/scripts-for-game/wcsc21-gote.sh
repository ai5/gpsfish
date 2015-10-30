#!/bin/sh -
WCSC_HOME=/home/wcsc21
cd $WCSC_HOME/bin
./util/gpsshogi-fileio.pl -d $WCSC_HOME/remote-csa \
  -c $WCSC_HOME/bin/wcsc21-search-gote.sh
