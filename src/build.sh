#!/bin/bash

if [ "$1" == "clean" -o "$1" == "c" ]; then
    if [ ! -d "_tmp" ]; then
        echo -e "\nno need clean."
        exit 0
    fi
    cd _tmp
    make clean
    cd ..
    rm -rf rpc/idl/.[a-zA-Z0-9]*
    echo -e "\nclean done."
    exit 0
elif [ "$1" == "distclean" -o "$1" == "dc" ]; then
    rm -rf _tmp depends
    rm -rf rpc/idl/.[a-zA-Z0-9]*
    echo -e "\ndistclean done."
    exit 0
fi


if [ $# != 0 ]; then
    echo -e "\narguments error."
    exit 0
fi


mkdir -p _tmp
cd _tmp
cmake -DCMAKE_INSTALL_PREFIX=`pwd`/../depends ..
make 
make install
echo -e "\nbuild done."
