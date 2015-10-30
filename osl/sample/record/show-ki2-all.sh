#!/usr/bin/zsh

dir="${1:-../../../2chkifu}"

for i in $dir/*.(ki2|KI2)
do
  basename="$(basename $i)"
  echo "$i"
  "$(dirname $0)/show-ki2" -q $i
  if [ $? -ne 0 ];then
    echo "ERROR: $i"
    move "$i" "$dir/ERROR"
  fi
done
