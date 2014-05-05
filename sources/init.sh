#!/bin/bash

export TPARTYDIR=`pwd`

export TPARTYINSTALL_DIR=$TPARTYDIR/../build
poco_dir=POCO

if [ ! -e ${TPARTYINSTALL_DIR} ] ; then
  echo  " creating $TPARTYINSTALL_DIR "
  mkdir -p ${TPARTYINSTALL_DIR}
fi

for i in jsoncpp-src-0.5.0  \
        xml2json \
        psqlodbc-09.03.0100 \
        unixODBC-2.3.2.tar
do
  if test -d $i; then
      cd $i
      if test -e "init.sh"; then
        echo --------$i
        ./init.sh
        if [ $? -eq 2 ] ;then
        echo "********************************"
        echo "**  ERROR in installing $i"
        echo "********************************"
        exit 0;
        fi

      fi
      cd ..
    fi    
done
