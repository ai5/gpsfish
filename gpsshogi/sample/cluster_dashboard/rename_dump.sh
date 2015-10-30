#!/bin/sh

nth=${1:?NTH}
newfile="dump.log.`/bin/date +%Y%m%d`_$nth"

if [ -f "$HOME/temp/current/$newfile" ] ; then
 echo "ERROR: duplicated"
 exit 1
fi

nice cp dump.log "$HOME/temp/current/$newfile"
cat /dev/null > dump.log

nice gzip "$HOME/temp/current/$newfile"

