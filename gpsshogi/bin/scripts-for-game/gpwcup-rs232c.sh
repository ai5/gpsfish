#!/bin/bash -
echo $0

time_limit=1500
byoyomi=0

#table_size=150000 # for 1GB
table_size=300000 # for 2GB

limit=2000

export TABLE_RECORD_LIMIT=-4 

./gpsshogi.pl -T $time_limit -B $byoyomi -t $table_size -l $limit $*
