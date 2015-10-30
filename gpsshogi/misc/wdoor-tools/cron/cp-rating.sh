#!/bin/sh -
umask 002
event=LATEST
HOME=/home/shogi-server
cd $HOME/www/$event
mkdir rating > /dev/null 2>&1;
today=`date "+%Y%m%d"`
cp current-floodgate.yaml players-floodgate.yaml
cp current-floodgate14.yaml players-floodgate14.yaml
cp players-floodgate.yaml rating/players-floodgate-$today.yaml
cp players-floodgate14.yaml rating/players-floodgate14-$today.yaml
cp players-floodgate.html rating/players-floodgate-$today.html
cp players-floodgate14.html rating/players-floodgate14-$today.html
cp players-others.yaml rating/players-others-$today.yaml
cp players-others.html rating/players-others-$today.html
#2012-08 replaced by symlink: players.yaml => players-floodgate.yaml
#if cat floodgate-results.txt others-results.txt | $HOME/bin/mk_rate-from-grep > tmp-daily.yaml; then
#  mv tmp-daily.yaml players.yaml
#fi
cd rating
$HOME/bin/players_graph.rb --output-dir g *floodgate14-*.yaml
#
