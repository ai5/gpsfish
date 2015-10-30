# Prerequires

## redis client API

    aptitude install hiredis

or download from https://github.com/antirez/hiredis

    git clone git://github.com/antirez/hiredis.git

## Google logging

    aptitude install libgoogle-glog-dev

or download from http://code.google.com/p/google-glog/  


# Master

    $ ./master -v -f ../../../gpsshogi/data/joseki.dat \
      --redis-host <host> --redis-port <port> --redis-password <password> \
      -p black

# Client

    $ ./client --redis-host <host> --redis-port <port> --redis-password <password> \
               -v 0 \
               --depth 1600

# Histogram

    $ ./histogram --redis-host <host> --redis-port <port> --redis-password <password> \
                  --depth 1600 \
                  -p black

# License

Same as gpsshogi

Copyright (C) 2011-2012 Team GPS
