#!/bin/sh
export PATH="$PATH:/bin:/sbin"

init.sh --na
modprobe imx250
test_aaa_service -a &
rmmod ambarella_fb
modprobe ambarella_fb resolution=1280x720 mode=clut8bpp

modprobe cavalry
cavalry_load -f /lib/firmware/cavalry.bin -r

test_encode -i 2432x2048 --hdmi 720p --ors 1280x720 --mctf-cmpr 0 -X --bsize 2432x2048 --bmaxsize 2432x2048 -K --bsize 1920x1080 --bmaxsize 1920x1080 -J --btype prev

test_encode -A -H 1080p -b 3 -e
