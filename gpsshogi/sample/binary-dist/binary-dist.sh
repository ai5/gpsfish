#!/bin/sh
#
# Script for building binaries to distribute them.
#
# 0. Install Debian Lenny
#   - mingw is required to build Windows binaries
#   % sudo aptitude install mingw32 mingw32-binutils mingw32-runtime
#
# 1. Set up the following directory layout for osl and gpsshogi
#   binary-dist.sh (copied from gpsshogi/sample/binary-dist/binary-dist.sh)
#   osl/
#   gpsshogi/
#
#   * Edit binary-dist.sh (this file)
#
# 2. Prepare Boost source and bjam build environment.
#   $HOME/local/boost_1_40_0
#
#   * Do _NOT_ export BOOST_ROOT (i.e. unset BOOST_ROOT).
#
#   * export BOOST_BUILD_PATH (=$HOME/local/boost_1_40_0/tools/build/v2)
#
#   * Build bjam command:
#     % cd tools/jam/src
#     % ./build.sh 
#
#   * Configure bjam conf files
#     - osl/user-config-{windows,linux,mac}.jam # see osl/user-config.jam.sample
#
# 3. Run this script
#    binary-dist.sh [-d date] <os> <arch>
#      os:   linux, windows, mac
#      arch: pentium4, amd64, core2
#
#    For example, 
#    $ ./binary-dist.sh linux pentium4 2>&1 | tee binary-dist.log
#    $ ./binary-dist.sh windows pentium4 2>&1 | tee binary-dist.log
#    $ ./binary-dist.sh mac core2 2>&1 | tee binary-dist.log
#
#    * Binaries will be put in $HOME/temp/current/
#

while getopts d:h option
do
  case "$option" in
    d)
      DATE="$OPTARG";;
    h)
      echo "USAGE: $0 [-d DATE] os arch" 
      exit 0;;
  esac
done
shift `expr "$OPTIND" - 1`

##
# Variables. Configure them as you like.
#

# linux or windows or mac
OS=${1:-linux}
# pentium4 or amd64 for linux, pentirum4 for windows, core2 for mac
ARCH=${2:-pentium4}
BJAM_OPTIONS="-d2 -q"
#  * -j2 concurrency
#  * debug-synbols=on or off
#  * -a  forcely rebuild all files
BJAM_OPTIONS="$BJAM_OPTIONS -j2"
BJAM_COMMANDS=""


BOOSTDIR=${BOOSTDIR:-$HOME/local/boost_1_40_0}
BOOST_COPYRIGHT=$BOOSTDIR/LICENSE_1_0.txt
if [ "$OS" = mac ] ; then
  BJAM=$BOOSTDIR/tools/jam/src/bin.macosxx86_64/bjam
else
  BJAM=$BOOSTDIR/tools/jam/src/bin.linuxx86_64/bjam
fi
QT_COPYRIGHT=/usr/share/doc/libqt4-dev/copyright
# only for mac
QT_SRC=$HOME/local/qt-everywhere-opensource-src-4.6.2
INSTALL=/usr/bin/install
NKF=/usr/bin/nkf
TBB_HOME=${TBB_HOME:-$HOME/local/tbb}

CURRENT_DIR=$(cd $(dirname $0) && pwd)
GPS_DIR=$CURRENT_DIR/gpsshogi
BINARY_DIST_DIR=$GPS_DIR/sample/binary-dist
OSL_DIR=$CURRENT_DIR/osl
if [ "$OS" = mac ] ; then
  TMP_DIR=/tmp/gpsshogi
else
  TMP_DIR=/dev/shm/gpsshogi
fi

if [ "$OS" = windows ] ; then
  BJAM_OPTIONS="$BJAM_OPTIONS -sJAMUNAME=CYGWIN"
  BJAM_COMMANDS="$BJAM_COMMANDS address-model=32 instruction-set=$ARCH cxxflags=-fno-strict-aliasing toolset=gcc-4.2 target-os=windows"
else if [ "$ARCH" = pentium4 ]; then
  BJAM_COMMANDS="$BJAM_COMMANDS address-model=32 instruction-set=$ARCH architecture=x86"
fi
fi
if [ "$OS" = mac ] ; then
  BJAM_COMMANDS="$BJAM_COMMANDS toolset=darwin instruction-set=conroe"
fi
BJAM_OPTIONS="$BJAM_OPTIONS --user-config=$OSL_DIR/user-config-$OS.jam"

##
# Main
#

# Copy FROM file to TO file. Then correct line breaks for Windows.
# @param from
# @param to
#
function install_txt()
{
  $INSTALL -m 644 $1 $2
  if [ "$OS" = windows ] ; then
    $NKF -Es -Lw -c --overwrite $2
  fi

}

echo OS="$OS"
echo ARCH="$ARCH"
echo BJAM_OPTIONS="$BJAM_OPTIONS $BJAM_COMMANDS"

OUTPUT_DIR=$TMP_DIR/output
SINGLE_EXE=$TMP_DIR/dist
SMP_EXE=$TMP_DIR/dist-smp
DATA_DIR=$TMP_DIR/data

DATE=${DATE:-`date +%Y%m%d`}

### CLEAN ###
echo "Cleaning..."
rm -fr $TMP_DIR
mkdir $TMP_DIR
mkdir $OUTPUT_DIR
mkdir $SINGLE_EXE
mkdir $SMP_EXE
mkdir $DATA_DIR

### DATA ###
echo "Preparing data files..."
cd $GPS_DIR
for dir in rating
do
  mkdir $DATA_DIR/$dir
  $INSTALL -m 644 data/$dir/*.txt $DATA_DIR/$dir
done

for file in sibling-attack.pair eval.txt eval-info.txt progress.txt progress-info.txt
do
  $INSTALL -m 644 data/$file $DATA_DIR
done

$INSTALL -m 644 data/joseki.dat $DATA_DIR

for file in AUTHORS COPYING LICENSE ; do
  install_txt $file $DATA_DIR/$file
done

# GPL
if [ "$OS" != mac ] ; then
  install_txt /usr/share/common-licenses/GPL-2 $DATA_DIR/GPL-2
else
  install_txt $BINARY_DIST_DIR/GPL-2 $DATA_DIR/GPL-2
fi

cd $DATA_DIR/..
zip -r $OUTPUT_DIR/gpsshogi-data_${DATE}.zip data

### OSL executables ###
#cd $OSL_DIR
#"$BJAM" $BJAM_OPTIONS install $BJAM_COMMANDS release smp=off link=static
#"$BJAM" $BJAM_OPTIONS install $BJAM_COMMANDS release smp=on  link=static
#"$BJAM" binary-dist binary-dist-smp $BJAM_OPTIONS $BJAM_COMMANDS 
#rm dist/release/usr/bin/*
#rm dist-smp/release/usr/bin/*
#cp dist/release/usr/bin/*     $SINGLE_EXE
#cp dist-smp/release/usr/bin/* $SMP_EXE

### GPS executables ###
echo "Compiling..."
cd $GPS_DIR
rm dist/release/usr/bin/*
rm dist-smp/release/usr/bin/*
"$BJAM" $BJAM_OPTIONS binary-dist binary-dist-smp $BJAM_COMMANDS 
if [ $OS = linux ] ; then
  "$BJAM" $BJAM_OPTIONS binary-dist-viewer binary-dist-smp-viewer $BJAM_COMMANDS 
fi
if [ $OS = mac ] ; then
  "$BJAM" $BJAM_OPTIONS binary-dist-viewer binary-dist-smp-viewer $BJAM_COMMANDS 
fi
cp dist/release/usr/bin/*     $SINGLE_EXE
cp dist-smp/release/usr/bin/* $SMP_EXE
if [ $OS = windows ] ; then
  cp /usr/share/doc/mingw32-runtime/mingwm10.dll.gz $TMP_DIR
  gunzip $TMP_DIR/mingwm10.dll.gz
  chmod a+x $TMP_DIR/mingwm10.dll
  for dir in $SINGLE_EXE $SMP_EXE ; do
    cp -p $TMP_DIR/mingwm10.dll $dir
  done
fi
if [ $OS = mac ] ; then
  for dir in $SINGLE_EXE $SMP_EXE ; do
    mkdir $dir/gpsshogi_viewer.app
    mkdir $dir/gpsshogi_viewer.app/Contents
    mkdir $dir/gpsshogi_viewer.app/Contents/MacOS
    mkdir $dir/gpsshogi_viewer.app/Contents/Resources
    mv $dir/gpsshogi_viewer $dir/gpsshogi_viewer.app/Contents/MacOS
    cp $BINARY_DIST_DIR/Info.plist $dir/gpsshogi_viewer.app
    cp -a $QT_SRC/src/gui/mac/qt_menu.nib $dir/gpsshogi_viewer.app/Contents/Resources
  done
fi

## LICENSE FILES ##
echo "Preparing license files..."
for dir in $SINGLE_EXE $SMP_EXE ; do
  # OSL
  for file in AUTHORS LICENSE ; do
    install_txt $OSL_DIR/$file $dir/$file.osl
  done

  # GPS
  for file in AUTHORS COPYING LICENSE ; do
    install_txt $GPS_DIR/$file $dir/$file
  done

  # GPL
  if [ "$OS" != mac ] ; then
    install_txt /usr/share/common-licenses/GPL-2 $dir/GPL-2
  else
    install_txt $BINARY_DIST_DIR/GPL-2 $dir/GPL-2
  fi

  # BOOST
  install_txt $BOOST_COPYRIGHT $dir/copyright.boost

  if [ "$OS" = linux ] ; then
    # Qt
    install_txt $QT_COPYRIGHT $dir/copyright.qt
  fi

  # TCMALLOC
  if [ "$OS" = linux ] ; then
    for file in AUTHORS COPYING ; do
      install_txt $OSL_DIR/lib/third_party/tcmalloc-0.93/$file $dir/$file.google_perftools
    done
  fi

  # TBB
  if [ "$OS" = mac ] ; then
    install_txt $TBB_HOME/COPYING $dir/copyright.tbb
  fi

  # MinGW
  if [ "$OS" = windows ] ; then
    install_txt /usr/share/doc/mingw32-runtime/copyright $dir/copyright.mingw
  fi
done


### zip ###
echo "Archiving..."
cd $SINGLE_EXE
zip -r $OUTPUT_DIR/gpsshogi-bin-single-${OS}-${ARCH}_${DATE}.zip *
cd $SMP_EXE
zip -r $OUTPUT_DIR/gpsshogi-bin-smp-${OS}-${ARCH}_${DATE}.zip *

### OUTPUT ###
echo "Copying results..."
if [ "$OS" != mac ] ; then
  cp -ar $OUTPUT_DIR $HOME/temp/current
else
  cp -a $OUTPUT_DIR $HOME/temp/current
fi

rm -fr $TMP_DIR

echo "Finished"

exit 0
