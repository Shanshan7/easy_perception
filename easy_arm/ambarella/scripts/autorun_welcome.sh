#!/bin/sh
export PATH="$PATH:/bin:/sbin"

init.sh --yuv480p
modprobe imx316_mipi bus_id=0x2 vinc_id=0x1
modprobe imx327_mipi bus_id=0 vinc_id=0
test_tuning -a &

rmmod ambarella_fb
modprobe ambarella_fb resolution=240x180 mode=clut8bpp
modprobe cavalry
cavalry_load -f /lib/firmware/cavalry.bin -r

test_encode --resource-cfg /usr/local/bin/scripts/cv25_vin0_1080p_vin1_240x180_tof_linear.lua --hdmi 720p --raw-capture 1 --enc-raw-yuv 1 --enc-mode 0 --extra-raw-dram-buf-num 16

test_encode -C -h 1920x1080 -b 2 -e
rtsp_server &
test_mempart -m 4 -f
test_mempart -m 4 -s 0x04000000

# init.sh --yuv480p
# modprobe imx316_mipi bus_id=0x2 vinc_id=0x1 trigger_mode=1
# modprobe ar0234_mipi bus_id=0x0 vinc_id=0x0

# test_tuning -a &

# rmmod ambarella_fb
# modprobe ambarella_fb resolution=240x180 mode=clut8bpp

# modprobe cavalry
# cavalry_load -f /lib/firmware/cavalry.bin -r

# test_encode --resource-cfg /usr/local/bin/scripts/cv25_vin0_wuxga_vin1_240x180_tof_linear.lua --hdmi 720p --raw-capture 1 --enc-raw-yuv 1 --enc-mode 0 --extra-raw-dram-buf-num 16 --vsync-detect-disable 1

# test_encode -C -h 1920x1200 -b 2 -e
# rtsp_server &
# test_mempart -m 4 -f
# test_mempart -m 4 -s 0x04000000

ntpdate 10.0.0.102 &

sleep 3

export LD_LIBRARY_PATH=/data:$LD_LIBRARY_PATH

if [ ! -d "/data/glog_file" ]; then
    mkdir /data/glog_file
fi

if [ ! -d "/data/save_data" ]; then
    mkdir /data/save_data
fi

/data/test_lpr &
