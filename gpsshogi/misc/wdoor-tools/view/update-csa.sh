#!/bin/sh
umask 002

(
cd /home/shogi-server/www/x
nice /home/shogi-server/bin/update-csa.pl 20*
)
