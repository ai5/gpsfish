#!/bin/sh -
WCSC_HOME=/home/wcsc21
DATE=`date +%Y%m%d-%H%M%S`
$WCSC_HOME/bin/csa2usi.pl --server_stdio=+ --no-send_pv --pawn_value=1000 \
  --usi_engine "$WCSC_HOME/bin/usi.pl -l$WCSC_HOME/log -c $WCSC_HOME/config-wcsc21.txt" \
  --logdir=$WCSC_HOME/log 2>$WCSC_HOME/search-sente-$DATE.log
