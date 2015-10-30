#! /usr/bin/env bash

ulimit -v hard
ulimit -s hard

# for wcsc
bash ./scripts-for-game/wcsc-rs232c.sh $*

# for gpwcup
# bash ./scripts-for-game/gpwcup-rs232c.sh $*

# for xxxcup
# bash ./scripts-for-game/xxxcup-rs232c.sh $*
