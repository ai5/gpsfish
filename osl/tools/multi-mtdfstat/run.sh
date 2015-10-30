#! /bin/sh 

BASE=/opt/gps/$USER/gpsshogi-nodist/data/problems
name=test1
for problem in tesuji/1 tesuji/2 rakuraku/1 rakuraku/2 amateur/1 100/1 100/3 100/2 100/4 kakoi seme-uke; do
  dir=$BASE/$problem
  problem_name=`echo $problem | sed -e 's!/!-!g'`
  $HOME/run.rb $dir > $HOME/$name-$problem_name
done
