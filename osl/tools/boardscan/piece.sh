#! /bin/sh

for file in *s.pnm; do
  ./detect.rb $file
  dir=${file/s.pnm/}
  if [ ! -d $dir ]; then
    mkdir $dir
  fi
  echo $dir
  mv test??.pnm $dir
done
