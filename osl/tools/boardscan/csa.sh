#! /bin/sh

for file in *s.pnm; do
  number=${file/s.pnm/}
  echo $number
  ./board.rb $number > $number.csa
done
