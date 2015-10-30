#!/bin/sh

for p in black white
do
  nice ./master -v -f ../../data/joseki.dat \
                --redis-host ${GPS_REDIS_HOST:?GPS_REDIS_HOST not found} \
                --redis-port ${GPS_REDIS_PORT:?GPS_REDIS_PORT not found} \
                --redis-password ${GPS_REDIS_PASSWORD:?GPS_REDIS_PASSWORD not found} \
                -p $p 2> debug_$p.txt
done

exit 0
