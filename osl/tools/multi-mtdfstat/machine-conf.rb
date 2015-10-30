MACHINES = ["gopteron8", "gopteron8", "gopteron7", "gopteron7", "gopteron6", "gopteron6"]
#COMMAND = '/opt/gps/yoshiki/gpsshogi/bin/mtdfstat'
COMMAND = 'env OSL_HOME=/home/users/yoshiki /home/users/yoshiki/mtdfstat'
ARG = '-l 1600 -e progress -O - -v 0'
#ARG = '-l 1600 -S 30 -e progress -O - -v 0'
