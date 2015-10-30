#!/bin/sh -
server="gserver.computer-shogi.org"
#server='192.168.0.1' #for GPW2003 cup
#server='localhost' #for GPW2003 cup
#table_size=10000000 # for 6GB
table_size=150000 # for 1GB
#table_size=100000
env SERVER=$server TABLE_RECORD_LIMIT=-4 CHALLENGE=t SHOGIUSER=teamgps SHOGIPASS=os4QRTvls TABLE_SIZE=$table_size LIMIT=2000 ./network.pl
#env SERVER=$server CHALLENGE=t SHOGIUSER=testgps SHOGIPASS=hogetaro TABLE_SIZE=$table_size LIMIT=2000 ./network.pl
