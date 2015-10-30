#!/bin/bash -
# policy: comment out/in 
echo $0

server=192.168.0.1
#server="gserver.computer-shogi.org"
#server='wdoor.c.u-tokyo.ac.jp' # for test

table_size=8000000 # for 16GB?

export TABLE_SIZE=$table_size
export TABLE_RECORD_LIMIT=200  

export SHOGIUSER=TeamGPS
export SHOGIPASS=os4QRTvls
#export SHOGIUSER=TestGPS
#export SHOGIPASS=hogetaro
#export SHOGIPASS=gps-1500-0 # for wdoor test

#export CHALLENGE=t
export SERVER=$server;
export LIMIT=1600
export GPSOPTS="-aP -m35 -N16 -e test --weight-coef 3 --weight-initial-coef 3"
export GPSNAME="./gpsone -vc"

exec ./network.pl
