#!/bin/sh -
umask 002
renice 20 $$ >/dev/null 2>&1
HOME=/home/shogi-server
year=`date +%Y`
mkdir -p $HOME/www/x/html/$year > /dev/null 2>&1
cd $HOME/www/x/html/$year
nice perl -w ~/bin/summary.pl $year -1
