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

cp -rf include $TPARTYINSTALL_DIR/

if [ $? -ne 0 ] ;then
  echo ERROR in installing jsoncpp
  exit 2;
fi
