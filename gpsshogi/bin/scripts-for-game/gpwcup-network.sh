#!/bin/bash -
# policy: comment out/in 
echo $0

server='192.168.20.1' # for gpw
#server='wdoor.c.u-tokyo.ac.jp' # for test

#table_size=150000 # for 1GB
table_size=600000 # for 2GB?

export TABLE_SIZE=$table_size
export TABLE_RECORD_LIMIT=299

export SHOGIUSER=teamgps
export SHOGIPASS=os4QRTvls

#export CHALLENGE=t
export SERVER=$server;
export LIMIT=1600
export GPSOPTS="-a -P -m35"
export GPSNAME="./gpsone -vc"

exec ./network.pl
