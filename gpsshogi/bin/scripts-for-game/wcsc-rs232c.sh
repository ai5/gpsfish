#!/bin/bash -
echo $0

time_limit=1500
byoyomi=0

table_size=10000000 # for 6GB

limit=1400

export TABLE_RECORD_LIMIT=-6

./gpsshogi.pl -T $time_limit -B $byoyomi -t $table_size -l $limit $*
