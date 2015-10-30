#!/bin/sh

export LANG=ja_JP.EUC-JP
export LD_LIBRARY_PATH=../../../osl/release-so

exec java -d64 -server -classpath target/gpsdashboard-1.2.1-standalone.jar gpsdashboard.monitor_swing --host localhost
