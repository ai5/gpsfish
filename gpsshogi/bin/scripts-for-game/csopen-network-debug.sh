#!/bin/bash -
# policy: comment out/in 
echo $0

#server='192.168.0.1' # for gpw
#server='wdoor.c.u-tokyo.ac.jp' # for test
server="gserver.computer-shogi.org"

table_size=4800000 # for 8GB?

export TABLE_SIZE=$table_size
export TABLE_RECORD_LIMIT=200

export SHOGIUSER=testgps
export SHOGIPASS=hogetaro

#export CHALLENGE=t
export SERVER=$server;
export LIMIT=1400
export GPSOPTS="-m35 -N1 -e test"

exec ./network.pl
