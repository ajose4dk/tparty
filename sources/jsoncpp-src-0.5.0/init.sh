#/bin/bash
TPDIR=`pwd`/..

if [ -z "$TPARTYDIR" ]; then
  export TPARTYDIR=`pwd`/..
fi

if [ -z "$TPARTYINSTALL_DIR" ]; then
  export TPARTYINSTALL_DIR=$TPARTYDIR/../build
fi

if [ "$MUDBUILD" == "Y" ] ; then
export CXXFLAGS="-fmudflapth";
export LDFLAGS="-lmudflapth -lpthread";
fi

mkdir -p $TPARTYINSTALL_DIR/lib
scons platform=linux-gcc
GCC_VER=`gcc -v 2>&1 | tail -1 | awk '{print $3}'`
#cp libs/linux-gcc-$GCC_VER/*.a $TPARTYINSTALL_DIR/lib/
cp libs/linux-gcc-*/*.a $TPARTYINSTALL_DIR/lib/
cp libs/linux-gcc-*/*.so $TPARTYINSTALL_DIR/lib/
cp libs/linux-gcc-*/*.so $TPARTYINSTALL_DIR/lib/libjson.so
cp -rf include $TPARTYINSTALL_DIR/

if [ $? -ne 0 ] ;then
  echo ERROR in installing jsoncpp
  exit 2;
fi
