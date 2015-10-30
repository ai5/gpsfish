#!/bin/sh -
umask 002
today=`date "+%Y%m%d"`
HOME=/home/shogi-server
renice 20 $$ >/dev/null 2>&1

go () {
  event=$1
  mkdir -p $HOME/www/$event/html/current > /dev/null 2>&1
  cd $HOME/www/$event/html/current
  nice perl -w $HOME/bin/summary.pl
  # rating
  cd $HOME/www/
  rm tmp.yaml 2>/dev/null
  if sort x/floodgate-results.txt | uniq | $HOME/bin/mk_rate --fixed-rate-player gpsfish_normal_1c --fixed-rate 2800 > tmp.yaml; then
      cp -p tmp.yaml $event/current-floodgate.yaml
  fi
  if sort x/floodgate-results.txt | uniq | $HOME/bin/mk_rate --ignore 14 --half-life-ignore 14 --fixed-rate-player gpsfish_normal_1c --fixed-rate 2800 > tmp.yaml; then
      cp -p tmp.yaml $event/current-floodgate14.yaml
      mv tmp.yaml $event/rating/players-floodgate14-$today.yaml
  fi
  cd $HOME/www/$event
  $HOME/bin/mk_html --footer $HOME/bin/floodgate-footer.html --wdoor < current-floodgate.yaml > players-floodgate.html
  $HOME/bin/mk_html --footer $HOME/bin/floodgate-footer.html --wdoor < current-floodgate14.yaml > players-floodgate14.html
  $HOME/bin/floodgate-summary.pl floodgate-players.txt floodgate-history.txt
}
go LATEST
# go 2008-02-03

# clean up cache
cd $HOME/www/view/cache
find . -name "*.svg" -ctime +0 -exec rm -f {} \;

#
#$HOME/bin/floodgate-history.pl > /var/www/shogi/floodgate-history.html
