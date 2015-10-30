#!/bin/sh

export LANG=ja_JP.EUC-JP
export LD_LIBRARY_PATH=/home/daigo/cprojects/gpsshogi/201204-dashboardtest/osl/release-so

if [ -f dump.log ] ; then
 mv dump.log dump.log.`/bin/date +%Y%m%d_%H%M%S`
fi
lein run --expiration 30 --dump-interval 3
