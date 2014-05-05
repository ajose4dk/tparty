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

./configure $TPARTYINSTALL_DIR
if [ $? -ne 0 ] ;then
  echo ERROR in configure unixOdbc
  exit 2;
fi
make 
if [ $? -ne 0 ] ;then
  echo ERROR in make unixOdbc
  exit 2;
fi
make install
if [ $? -ne 0 ] ;then
  echo ERROR in make install unixOdbc
  exit 2;
fi
