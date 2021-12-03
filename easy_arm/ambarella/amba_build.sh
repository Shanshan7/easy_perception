#!/bin/bash
BUILD_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
build_path="${BUILD_PATH}/build"
if [ -n "$2" ]; then
    install_prefix=$2
else
    install_prefix=${build_path}/amba
fi

function setBulidPath()
{
    if [ ! -d $build_path ];then
        mkdir $build_path
    else
        echo $build_path
    fi
}

function helpDoc()
{
    echo ""
    echo "compile project : ./build.sh [aarch64/clean/install] [install dir]"
    echo ""
}

if [ "$1" == 'aarch64' ]; then
    source ./env.sh
    echo "platform is $1"
    setBulidPath
    cd build
    cmake -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DCMAKE_INSTALL_PREFIX=${install_prefix} ..
    make -j8
elif [ "$1" == 'install' ]; then
    echo "install"
    cd build
    make install
elif [ "$1" == 'clean' ]; then
    echo "clean"
    rm -rf $build_path
else
    echo "This Platform not support:$1"
    helpDoc
    exit 1
fi
