#!/bin/bash
BUILD_PATH=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
third_path="${BUILD_PATH}/third_party"
build_path="${BUILD_PATH}/build"
install_prefix=${build_path}/cyberRT
setup_path=${install_prefix}/bin/setup.bash

function setInstallPath()
{
    if [ -n "$1" ]; then
        install_prefix=$1
    fi
    echo "install path: ${install_prefix}"
}

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
    echo "compile project : ./cyber_build.sh [docker/aarch64/x86_64/clean/install] [install dir]"
    echo ""
}

function dockerBuild()
{
    echo "platform is $1"
    setInstallPath /usr/local/cyberRT
    setBulidPath
    setup_path=${install_prefix}/bin/docker_setup.bash
    cd build
    cmake -DBUILD_DOCKER=ON -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=x86_64 -DCMAKE_INSTALL_PREFIX=${install_prefix} ..
    make -j8
    make install
    echo "source ${setup_path}" >> /etc/bash.bashrc
    if [ ! -d "/easy_data/cyber_data/log" ]; then
      echo "cyber log not exist, create dir /easy_data/cyber_data/log"
      mkdir -p /easy_data/cyber_data/log
    fi
}

function aarch64Build()
{
    echo "platform is $1"
    setInstallPath "$2"
    setBulidPath
    cd build
    export LD_LIBRARY_PATH=${third_path}/$1/protobuf/lib:$LD_LIBRARY_PATH
    cmake -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=aarch64 -DCMAKE_INSTALL_PREFIX=${install_prefix} ..
    make -j8
}

function x86_64Build()
{
    echo "platform is $1"
    setInstallPath "$2"
    setBulidPath
    cd build
    export LD_LIBRARY_PATH=${third_path}/$1/protobuf/lib:$LD_LIBRARY_PATH
    cmake -DCMAKE_SYSTEM_NAME=Linux -DCMAKE_SYSTEM_PROCESSOR=x86_64 -DCMAKE_INSTALL_PREFIX=${install_prefix} ..
    make -j8
}

function installCyber()
{
    echo "install"
    setInstallPath "$1"
    cd build
    make install
    echo "source ${setup_path}" >> ~/.bashrc
    if [ ! -d "/home/$USER/cyber_data/log" ]; then
      echo "cyber log not exist, create dir /home/$USER/cyber_data/log"
      mkdir -p /home/$USER/cyber_data/log
    fi
}

if [ "$1" == 'docker' ]; then
    dockerBuild
elif [ "$1" == 'aarch64' ]; then
    aarch64Build "$1" "$2"
elif [ "$1" == 'x86_64' ]; then
    x86_64Build "$1" "$2"
elif [ "$1" == 'install' ]; then
    installCyber "$2"
elif [ "$1" == 'clean' ]; then
    echo "clean"
    rm -rf $build_path
else
    echo "This Platform not support:$1"
    helpDoc
    exit 1
fi
