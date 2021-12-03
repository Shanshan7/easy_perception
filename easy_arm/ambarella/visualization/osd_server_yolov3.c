/*******************************************************************************
 * osd_server_yolo3.cpp
 *
 * History:
 *    2018/09/19  - [Zhenwu Xue] created
 *
 * Copyright (c) 2018 Ambarella, Inc.
 *
 * This file and its contents ( "Software" ) are protected by intellectual
 * property rights including, without limitation, U.S. and/or foreign
 * copyrights. This Software is also the confidential and proprietary
 * information of Ambarella, Inc. and its licensors. You may not use, reproduce,
 * disclose, distribute, modify, or otherwise prepare derivative works of this
 * Software or any portion thereof except pursuant to a signed license agreement
 * or nondisclosure agreement with Ambarella, Inc. or its authorized affiliates.
 * In the absence of such an agreement, you agree to promptly notify and return
 * this Software to Ambarella, Inc.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 * MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL AMBARELLA, INC. OR ITS AFFILIATES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; COMPUTER FAILURE OR MALFUNCTION; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <getopt.h>
#include <math.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/fb.h>
#include "lib_smartfb.h"

char const *names[] = {"bike", "bus", "car", "motor", "person", "rider", "truck"};

#define LOGACT(x) (1.0f / (1.0f + exp(-x)))

#define ROUND_UP_32(x) ((x)&0x1f ? (((x)&0xffffffe0) + 32) : (x))

#define IW 416
#define IH 416

#define CLASS 7
#define EACHACHOR 12
#define OUTPUTNUM 36

#define W0 (IW / 32)
#define H0 (IH / 32)
#define P0 (ROUND_UP_32(4 * W0))/4

#define W1 (IW / 16)
#define H1 (IH / 16)
#define P1 (ROUND_UP_32(4 * W1))/4

#define W2 (IW / 8)
#define H2 (IH / 8)
#define P2 (ROUND_UP_32(4 * W2))/4

#define IDX0(o) ((z * EACHACHOR + (o)) * H0 * P0 + y * P0 + x)
#define IDX1(o) ((z * EACHACHOR + (o)) * H1 * P1 + y * P1 + x)
#define IDX2(o) ((z * EACHACHOR + (o)) * H2 * P2 + y * P2 + x)
#define MAX_OBJECTS (3 * (H0 * W0 + H1 * W1 + H2 * W2))


const float anchors[] = {8.95,8.57, 12.43,26.71, 19.71,14.43, 26.36,58.52, 36.09,25.55, 64.42,42.90, 96.44,79.10, 158.37,115.59, 218.65,192.90};

typedef struct
{
	int left;
	int right;
	int low;
	int high;
	int id;
	float prob;
} object_t;

static int g_length = (H0 * P0 + H1 * P1 + H2 * P2) * OUTPUTNUM * 4;
static int g_port = 27182;
static int g_verbose = 0;
static int g_parent = -1;
static int g_child = -1;
static int live_flag = 1;
static float g_threshold = 0.2;

static object_t objects[MAX_OBJECTS];
static int valid[MAX_OBJECTS];
static int num = 0;
smartfb_box_t fb_box;
smartfb_textbox_t fb_textbox;

static struct option long_options[] = {
	{"server_port", 1, 0, 'p'},
	{"data_length", 1, 0, 'l'},
	{"threshold", 1, 0, 't'},
	{"verbose", 0, 0, 'v'},

	{0, 0, 0, 0},
};

static const char *short_options = "p:l:t:v";

int init_parameter(int argc, char **argv)
{
	int c;

	while (1) {
		c = getopt_long(argc, argv, short_options, long_options, NULL);
		if (c < 0) {
			break;
		}

		switch (c) {
		case 'l':
			g_length = atoi(optarg);
			break;
		case 'p':
			g_port = atoi(optarg);
			break;
		case 't':
			g_threshold = atof(optarg);
			break;
		case 'v':
			g_verbose = 1;
			break;

		default:
			printf("osd_server: unknown switch -%c!\n\n", c);
			return -1;
		}
	}

	return 0;
}

static void sigstop(int a)
{
	live_flag = 0;
}

void print_usage()
{
	printf("osd_server_yolov3: Show CNN inference result on OSD\n");
	printf("Usage: osd_server_yolov3\n");
	printf("                   -l <data length of the final output, default=2227680>\n");
	printf("                   -p <server port, default=27182>\n");
	printf("                   -t <threshold, default=0.12>\n\n");

	printf("                   -v /* verbose */\n");
}

static int recv_frame(int fd, char *dest, int size)
{
	int ret;
	int recv_size = 0;

	do {
		ret = recv(fd, dest + recv_size, size - recv_size, 0);
		if (ret > 0) {
			recv_size += ret;
		} else {
			printf("ERROR: Unable to receive data from client!\n");
			ret = -1;
			break;
		}
	} while (recv_size < size);

	return ret;
}

static int frame_buffer_init(uint8_t vout_id, int *width, int *height)
{
	smartfb_init_t fb_type;
	struct fb_var_screeninfo var;
	fb_type.vout = vout_id;
	memset(&fb_box, 0, sizeof(smartfb_box_t));
	memset(&fb_textbox, 0, sizeof(smartfb_textbox_t));
	if (smartfb_init(&fb_type) < 0) {
		return -1;
	}
	smartfb_get_var(&var);
	*width = var.xres;
	*height = var.yres;
	return 0;
}

static int smartfb_draw_rectangle(int x1, int y1, int x2, int y2, int thickness, int color)
{
	smartfb_obj_t *fb_obj;
	fb_obj = &fb_box.obj;
	if (x2 > x1) {
		fb_box.width = x2 - x1;
		fb_obj->offset_x = x1;
	} else {
		fb_box.width = x1 - x2;
		fb_obj->offset_x = x2;
	}
	if (y2 > y1) {
		fb_box.height = y2 - y1;
		fb_obj->offset_y = y1;
	} else {
		fb_box.height = y1 - y2;
		fb_obj->offset_y = y2;
	}
	fb_box.line_thickness = thickness;
	fb_obj->color = color;
	smartfb_draw_box(&fb_box);
	return 0;
}

static int smartfb_draw_text(const char *text_data, int x, int y,
			int fontsize, int thickness, int color, int bg_color, int bg_alpha)
{
	smartfb_box_t *textbox;
	smartfb_obj_t *fb_obj;
	textbox = &fb_textbox.box;
	fb_obj = &textbox->obj;
	fb_textbox.font_size = fontsize;
	textbox->line_thickness = thickness;
	fb_textbox.wrap_line = 0;
	fb_textbox.bold = 0;
	fb_textbox.italic = 0;
	fb_obj->color = color;
	fb_obj->bg_color = bg_color;
	fb_obj->bg_alpha = bg_alpha;
	fb_obj->offset_x = x;
	fb_obj->offset_y = y;
	textbox->width = fb_textbox.font_size * strlen(text_data);
	textbox->height = fb_textbox.font_size;
	strcpy(&fb_textbox.text[0], text_data);
	smartfb_draw_textbox(&fb_textbox);
	return 0;
}

int main(int argc, char **argv)
{
	int ret, i, cnt;
	int k;
	int z, y, x, c, C;
	char str_data[64];
	float objectness, max;
	float rx, ry, rw, rh;
	int ax, ay, aw, ah;
	float *df;
	int merge;
	struct sockaddr_in addr;
	object_t *ob1, *ob2;
	int n;
	int left, right, low, high, a1, a2, sa;
	int id[7];
	int area[7];
	static int width, height;
	float *float_src;

	signal(SIGINT, sigstop);
	signal(SIGQUIT, sigstop);
	signal(SIGTERM, sigstop);

	if (argc < 2) {
		print_usage();
		printf("osd_server_yolov3 running ...\n server_port default=27182 \n\n");
	}

	ret = init_parameter(argc, argv);
	if (ret < 0) {
		print_usage();
		return -1;
	}

	if (g_length < 1) {
		printf("ERROR: data length must be greater than 0!\n");
		print_usage();
		return -1;
	}

	printf("osd_server_yolov3 input_data_length=%d\n\n", g_length);
	float_src = (float *)malloc(g_length);
	if (!float_src) {
		printf("ERROR: Out of memory!\n");
		return -1;
	}
	g_parent = socket(AF_INET, SOCK_STREAM, 0);
	if (g_parent < 0) {
		printf("ERROR: Unable to create parent socket!\n");
		return -1;
	}

	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(g_port);
	printf("osd_server open port %d\n", g_port);

	ret = bind(g_parent, (struct sockaddr *)&addr, sizeof(addr));

	if (ret < 0) {
		printf("ERROR: Unable to bind parent socket!\n");
		sigstop(0);
	}

	ret = listen(g_parent, 1);
	if (ret < 0) {
		printf("ERROR: Unable to listen at parent socket!\n");
		sigstop(0);
	}

	g_child = accept(g_parent, (struct sockaddr *)0, (socklen_t *)0);
	if (g_child < 0) {
		printf("ERROR: Unable to accept client!\n");
		sigstop(0);
	}
	printf("Client is accepted.\n");
	if(frame_buffer_init(SMARTFB_ANALOG_VOUT, &width, &height) < 0){
		printf("init smartfb failed\n");
		sigstop(0);
	}
	printf("OSD Resolution w=%d, h=%d\n", width, height);

	cnt = 0;
	while (live_flag) {
		ret = recv_frame(g_child, (char *)float_src, g_length);
		if (ret < 0) {
			printf("ERROR: Unable to receive data!\n");
			break;
		}
		cnt++;
		memset(valid, 0, sizeof(valid));
		memset(area, 0, sizeof(area));
		num = 0;

		df = float_src;
		for (y = 0; y < H0; y++) {
			for (x = 0; x < W0; x++) {
				for (z = 0; z < 3; z++) {
					objectness = LOGACT((float)df[IDX0(4)]);
					if (objectness <= g_threshold) {
						continue;
					}

					max = -1e10;
					C = 0;
					for (c = 0; c < CLASS; c++) {
						if (df[IDX0(5 + c)] >= max) {
							max = df[IDX0(5 + c)];
							C = c;
						}
					}

					max = LOGACT(max);
					if (max <= g_threshold) {
						continue;
					}

					objectness *= max;
					if (objectness <= g_threshold) {
						continue;
					}

					rx = (x + LOGACT(df[IDX0(0)])) / W0;
					ry = (y + LOGACT(df[IDX0(1)])) / H0;
					rw = (exp(df[IDX0(2)]) * anchors[2 * (z + 6) + 0]) / IW;
					rh = (exp(df[IDX0(3)]) * anchors[2 * (z + 6) + 1]) / IH;

					rx *= width;
					ry *= height;
					rw *= width;
					rh *= height;

					rx -= rw / 2;
					ry -= rh / 2;

					ax = (int)(rx + 0.5);
					ay = (int)(ry + 0.5);
					aw = (int)(rw + 0.5);
					ah = (int)(rh + 0.5);

					if (ax < 0) {
						ax = 0;
					}
					if (ax >= width) {
						ax = width - 1;
					}
					if (aw < 0) {
						aw = 0;
					}
					if (aw >= width) {
						aw = width - 1;
					}

					if (ay < 0) {
						ay = 0;
					}
					if (ay >= height) {
						ay = height - 1;
					}
					if (ah < 0) {
						ah = 0;
					}
					if (ah >= height) {
						ah = height - 1;
					}

					objects[num].left = ax;
					objects[num].low = ay;
					objects[num].right = ax + aw - 1;
					objects[num].high = ay + ah - 1;
					objects[num].id = C;
					objects[num].prob = objectness;
					valid[num] = 1;
					num++;
				}
			}
		}

		df = df + OUTPUTNUM * H0 * P0;
		for (y = 0; y < H1; y++) {
			for (x = 0; x < W1; x++) {
				for (z = 0; z < 3; z++) {
					objectness = LOGACT((float)df[IDX1(4)]);
					if (objectness <= g_threshold) {
						continue;
					}

					max = -1e10;
					C = 0;
					for (c = 0; c < CLASS; c++) {
						if (df[IDX1(5 + c)] >= max) {
							max = df[IDX1(5 + c)];
							C = c;
						}
					}

					max = LOGACT(max);
					if (max <= g_threshold) {
						continue;
					}

					objectness *= max;
					if (objectness <= g_threshold) {
						continue;
					}

					rx = (x + LOGACT(df[IDX1(0)])) / W1;
					ry = (y + LOGACT(df[IDX1(1)])) / H1;
					rw = (exp(df[IDX1(2)]) * anchors[2 * (z + 3) + 0]) / IW;
					rh = (exp(df[IDX1(3)]) * anchors[2 * (z + 3) + 1]) / IH;

					rx *= width;
					ry *= height;
					rw *= width;
					rh *= height;

					rx -= rw / 2;
					ry -= rh / 2;

					ax = (int)(rx + 0.5);
					ay = (int)(ry + 0.5);
					aw = (int)(rw + 0.5);
					ah = (int)(rh + 0.5);

					if (ax < 0) {
						ax = 0;
					}
					if (ax >= width) {
						ax = width - 1;
					}
					if (aw < 0) {
						aw = 0;
					}
					if (aw >= width) {
						aw = width - 1;
					}

					if (ay < 0) {
						ay = 0;
					}
					if (ay >= height) {
						ay = height - 1;
					}
					if (ah < 0) {
						ah = 0;
					}
					if (ah >= height) {
						ah = height - 1;
					}

					objects[num].left = ax;
					objects[num].low = ay;
					objects[num].right = ax + aw - 1;
					objects[num].high = ay + ah - 1;
					objects[num].id = C;
					objects[num].prob = objectness;
					valid[num] = 1;
					num++;
				}
			}
		}

		df = df + OUTPUTNUM * H1 * P1;
		for (y = 0; y < H2; y++) {
			for (x = 0; x < W2; x++) {
				for (z = 0; z < 3; z++) {
					objectness = LOGACT((float)df[IDX2(4)]);
					if (objectness <= g_threshold) {
						continue;
					}

					max = -1e10;
					C = 0;
					for (c = 0; c < CLASS; c++) {
						if (df[IDX2(5 + c)] >= max) {
							max = df[IDX2(5 + c)];
							C = c;
						}
					}

					max = LOGACT(max);
					if (max <= g_threshold) {
						continue;
					}

					objectness *= max;
					if (objectness <= g_threshold) {
						continue;
					}

					rx = (x + LOGACT(df[IDX2(0)])) / W2;
					ry = (y + LOGACT(df[IDX2(1)])) / H2;
					rw = (exp(df[IDX2(2)]) * anchors[2 * (z + 0) + 0]) / IW;
					rh = (exp(df[IDX2(3)]) * anchors[2 * (z + 0) + 1]) / IH;

					rx *= width;
					ry *= height;
					rw *= width;
					rh *= height;

					rx -= rw / 2;
					ry -= rh / 2;

					ax = (int)(rx + 0.5);
					ay = (int)(ry + 0.5);
					aw = (int)(rw + 0.5);
					ah = (int)(rh + 0.5);

					if (ax < 0) {
						ax = 0;
					}
					if (ax >= width) {
						ax = width - 1;
					}
					if (aw < 0) {
						aw = 0;
					}
					if (aw >= width) {
						aw = width - 1;
					}

					if (ay < 0) {
						ay = 0;
					}
					if (ay >= height) {
						ay = height - 1;
					}
					if (ah < 0) {
						ah = 0;
					}
					if (ah >= height) {
						ah = height - 1;
					}

					objects[num].left = ax;
					objects[num].low = ay;
					objects[num].right = ax + aw - 1;
					objects[num].high = ay + ah - 1;
					objects[num].id = C;
					objects[num].prob = objectness;
					valid[num] = 1;
					num++;
				}
			}
		}

		do {
			merge = 0;

			for (i = 0, ob1 = objects; i < num; i++, ob1++) {
				if (!valid[i]) {
					continue;
				}

				a1 = (ob1->right - ob1->left + 1) * (ob1->high - ob1->low + 1);
				n = 0;

				for (k = 0, ob2 = objects; k < num; k++, ob2++) {
					if (k == i || !valid[k] || ob2->id != ob1->id) {
						continue;
					}

					left = ob1->left;
					low = ob1->low;
					right = ob1->right;
					high = ob1->high;

					if (ob2->left > left) {
						left = ob2->left;
					}
					if (ob2->right < right) {
						right = ob2->right;
					}
					if (ob2->low > low) {
						low = ob2->low;
					}
					if (ob2->high < high) {
						high = ob2->high;
					}

					if (left >= right || low >= high) {
						continue;
					}

					a2 = (ob2->right - ob2->left + 1) * (ob2->high - ob2->low + 1);
					sa = (right - left + 1) * (high - low + 1);

					if (sa >= 30 * a1 / 100 || sa >= 30 * a2 / 100) {
						if (ob2->prob <= ob1->prob) {
							valid[k] = 0;
						} else {
							n++;
						}

						merge++;
					}
				}

				if (n) {
					valid[i] = 0;
					continue;
				}
			}
		} while (merge);

		for (i = 0; i < num; i++) {
			if (!valid[i]) {
				continue;
			}

			a1 = (objects[i].right - objects[i].left + 1) * (objects[i].high - objects[i].low + 1);
			if (a1 > area[objects[i].id]) {
				area[objects[i].id] = a1;
				id[objects[i].id] = i;
			}
		}
		smartfb_clear_buffer();

		for (i = 0; i < num; i++) {
			if (!valid[i]) {
				continue;
			}
			smartfb_draw_rectangle(objects[i].left, objects[i].low, objects[i].right,
				objects[i].high, width / 600 + 1, objects[i].id % SMARTFB_COLOR_NUM);
			if (i == id[objects[i].id] && area[objects[i].id] > 0) {
				smartfb_draw_text(names[objects[i].id], objects[i].left + 5, objects[i].low,
					width / 90 + 10, 1, objects[i].id % SMARTFB_COLOR_NUM,
					SMARTFB_COLOR_WHITE, SMARTFB_ALPHA_LEVEL0);
			}
		}
		sprintf(str_data, "%d", cnt);
		smartfb_draw_text(str_data, width / 20, height / 20, width / 60 + 20, 1, SMARTFB_COLOR_MAGENTA,
			SMARTFB_COLOR_WHITE, SMARTFB_ALPHA_LEVEL0);
		smartfb_display();

	}
	if (g_parent >= 0) {
		close(g_parent);
		g_parent = -1;
	}

	if (g_child >= 0) {
		close(g_child);
		g_child = -1;
	}

	if (float_src) {
		free(float_src);
		float_src = NULL;
	}
	smartfb_deinit();

	printf("\n\n*************osd_server_yolov3 exit!*************\n");
	return 0;
}
