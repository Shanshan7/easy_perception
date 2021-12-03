#!/bin/sh

ntpdate 10.0.0.102 &

sleep 6

export LD_LIBRARY_PATH=/data:$LD_LIBRARY_PATH

if [ ! -d "/data/glog_file" ]; then
    mkdir /data/glog_file
fi

if [ ! -d "/data/save_data" ]; then
    mkdir /data/save_data
fi

./test_lpr
