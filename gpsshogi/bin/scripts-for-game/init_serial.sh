#!/bin/sh
line=/dev/ttyS0
speed=1200
/bin/stty -brkint -icrnl ixoff -imaxbel -opost \
-onlcr -isig -icanon -iexten -echo -echoe -echok \
-echoctl -echoke  $speed <$line

