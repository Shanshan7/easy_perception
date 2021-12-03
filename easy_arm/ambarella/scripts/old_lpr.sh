init.sh --yuv480p
modprobe imx316_mipi bus_id=0x2 vinc_id=0x1 trigger_mode=1
modprobe ar0234_mipi bus_id=0x0 vinc_id=0x0

test_tuning -a &

rmmod ambarella_fb
modprobe ambarella_fb resolution=240x180 mode=clut8bpp

modprobe cavalry
cavalry_load -f /lib/firmware/cavalry.bin -r

test_encode --resource-cfg /usr/local/bin/scripts/cv25_vin0_wuxga_vin1_240x180_tof_linear.lua --hdmi 720p --raw-capture 1 --enc-raw-yuv 1 --enc-mode 0 --extra-raw-dram-buf-num 16 --vsync-detect-disable 1

test_encode -C -h 1920x1200 -b 2 -e
rtsp_server &
test_mempart -m 4 -f
test_mempart -m 4 -s 0x04000000
test_ssd_lpr -b ./lpr/mobilenetv1_ssd_cavalry.bin --in data --out mbox_loc --out mbox_conf_flatten --class 2 --background_id 0 --top_k 50 --nms_top_k 100 --nms_threshold 0.45 --pri_threshold 0.3 --priorbox ./lpr/lpr_priorbox_fp32.bin -s 2 -i 2 -t 1 -b ./lpr/segfree_inception_cavalry.bin --in data --out prob -b ./lpr/LPHM_cavalry.bin --in data --out dense

