#!/bin/sh -
umask 002
cd /var/www/shogi/tools/view/cache
find *.svg \( -not -cmin -15 \) -exec rm {} \;
