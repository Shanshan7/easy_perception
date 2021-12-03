#include "image_acquisition.h"
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <sched.h>

#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <time.h>
#include <assert.h>

#include <basetypes.h>
#include <iav_ioctl.h>
#include "datatx_lib.h"

#include <iostream>

#ifndef VERIFY_BUFFERID
#define VERIFY_BUFFERID(x)	do {		\
			if ((x) >= IAV_MAX_CANVAS_BUF_NUM) {	\
				printf("Invalid canvas buffer id %d.\n", (x));	\
				return -1;	\
			}	\
		} while (0)
#endif

typedef enum {
	CAPTURE_NONE = 255,
	CAPTURE_PREVIEW_BUFFER = 0,
	CAPTURE_ME1_BUFFER,
	CAPTURE_ME0_BUFFER,
	CAPTURE_RAW_BUFFER,
	CAPTURE_PYRAMID_BUFFER,
	CAPTURE_TYPE_NUM,
} CAPTURE_TYPE;

#define MB_UNIT (16)
#define MAX_YUV_BUFFER_SIZE		(4096*3000)		// 4096x3000
#define MAX_ME_BUFFER_SIZE		(MAX_YUV_BUFFER_SIZE / 16)	// 1/16 of 4096x3000
#define MAX_FRAME_NUM				(120)
#define MARGIN	(30)
#define HWTIMER_OUTPUT_FREQ	(90000)
#define NAME_SIZE	320

#define YUVCAP_PORT					(2024)

typedef enum {
	AUTO_FORMAT = -1,
	YUV420_IYUV = 0,	// Pattern: YYYYYYYYUUVV
	YUV420_YV12 = 1,	// Pattern: YYYYYYYYVVUU
	YUV420_NV12 = 2,	// Pattern: YYYYYYYYUVUV
	YUV422_YU16 = 3,	// Pattern: YYYYYYYYUUUUVVVV
	YUV422_YV16 = 4,	// Pattern: YYYYYYYYVVVVUUUU
	YUV422_NV16 = 5,	// Pattern: YYYYYYYYUVUVUVUV
	YUV444 = 6,
	YUV_FORMAT_TOTAL_NUM,
	YUV_FORMAT_FIRST = YUV420_IYUV,
	YUV_FORMAT_LAST = YUV_FORMAT_TOTAL_NUM,
} YUV_FORMAT;

typedef struct {
	u8 *in;
	u8 *u;
	u8 *v;
	u64 row;
	u64 col;
	u64 pitch;
} yuv_neon_arg;

int fd_iav;

static int transfer_method = TRANS_METHOD_NFS;
static int port = YUVCAP_PORT;

static int vinc_id = 0;
static u32 current_channel = 0;
static int current_buffer = -1;
static int capture_select = 0;
static int capture_raw_ce = 0;
static int non_block_read = 0;
static int pts_prev = 0;
static int pts = 0;

static u32 yuv_buffer_map = 0;
static int yuv_format = AUTO_FORMAT;

static int pyramid_buffer_map = 0;

static int dump_canvas_map = 0;
static u32 me1_buffer_map = 0;
static u32 me0_buffer_map = 0;
static int frame_count = 1;
static int quit_capture = 0;
static int verbose = 0;
static int info_only = 0;
static int delay_frame_cap_data = 0;
static int G_multi_vin_num = 1;
static int G_canvas_num = 1;
static int dump_canvas_flag = 0;

const char *default_filename_nfs = "/mnt/media/test.yuv";
const char *default_filename_tcp = "media/test";
const char *default_host_ip_addr = "10.0.0.1";
const char *default_filename;

static char filename[256] = "imx327";
static int fd_yuv[IAV_MAX_CANVAS_BUF_NUM];
static int fd_me[IAV_MAX_CANVAS_BUF_NUM];
static int fd_raw = -1;
static int fd_raw_ce = -1;
static int fd_pyramid[IAV_MAX_PYRAMID_LAYERS];

static u8 *dsp_mem = NULL;
static u32 dsp_size = 0;

static u8* dsp_canvas_yuv_buf_mem[IAV_MAX_CANVAS_BUF_NUM];
static u32 dsp_canvas_yuv_buf_size[IAV_MAX_CANVAS_BUF_NUM];
static u32 dsp_canvas_yuv_offset[IAV_MAX_CANVAS_BUF_NUM];
static u8* dsp_canvas_me_buf_mem[IAV_MAX_CANVAS_BUF_NUM];
static u32 dsp_canvas_me_buf_size[IAV_MAX_CANVAS_BUF_NUM];
static u32 dsp_canvas_me_offset[IAV_MAX_CANVAS_BUF_NUM];

static u8* extra_raw_mem = NULL;
static u32 extra_raw_size = 0;
static u8 query_extra_raw_info_flag = 0;

static u8* dsp_pyramid_buf = NULL;
static u32 dsp_pyramid_buf_size = 0;
static u8* gdma_dst_buf = NULL;
static u32 gdma_dst_buf_size = 0;
static int gdma_dst_buf_fd = -1;
static u32 dsp_buf_mapped = 0;
static u8* pyramid_pool_buf = NULL;
static u32 pyramid_pool_buf_size = 0;
static u32 pyramid_pool_buf_mapped = 0;
static u32 pyramid_manual_feed = 0;
static u32 pts_intval[IAV_MAX_CANVAS_BUF_NUM];
static u8 canvas_mf_enable[IAV_MAX_CANVAS_BUF_NUM];
static u8 canvas_yuv_buffer_disable[IAV_MAX_CANVAS_BUF_NUM];
static u8 query_canvasgrp_flag = 0;
static u8 canvas_map_thru_dmabuf = 0;

static int decode_mode = 0;

static struct timeval pre = {0, 0}, curr = {0, 0};

static struct ImageBuffer image_buffer;
volatile int run_camera = 0; 

extern "C"{
extern void chrome_convert(yuv_neon_arg *);
extern void chrome_UV22_convert_to_UV44(yuv_neon_arg *);
extern void chrome_UV20_convert_to_UV44(yuv_neon_arg *);
}

//first second value must in format "x~y" if delimiter is '~'
static int get_two_unsigned_int(const char *name, u32 *first, u32 *second,
	char delimiter)
{
	char tmp_string[16];
	char * separator;

	separator = (char*)strchr(name, delimiter);
	if (!separator) {
		printf("range should be like a%cb \n", delimiter);
		return -1;
	}

	strncpy(tmp_string, name, separator - name);
	tmp_string[separator - name] = '\0';
	*first = atoi(tmp_string);
	strncpy(tmp_string, separator + 1,  name + strlen(name) -separator);
	*second = atoi(tmp_string);

	return 0;
}

static int map_canvas_buffers(void)
{
	struct iav_querymem query_mem;
	struct iav_mem_canvas_info *canvas_info;
	int i;

	query_mem.mid = IAV_MEM_CANVAS;
	query_mem.arg.canvas.id_map = dump_canvas_map;
	if (ioctl(fd_iav, IAV_IOC_QUERY_MEMBLOCK, &query_mem) < 0) {
		perror("IAV_IOC_QUERY_MEMBLOCK");
		return -1;
	}
	canvas_info = &query_mem.arg.canvas;

	memset(dsp_canvas_yuv_buf_mem, 0, sizeof(dsp_canvas_yuv_buf_mem));
	memset(dsp_canvas_me_buf_mem, 0, sizeof(dsp_canvas_me_buf_mem));
	for (i = 0; i < IAV_MAX_CANVAS_BUF_NUM; ++i) {
		if (dump_canvas_map & (1 << i)) {
			dsp_canvas_yuv_buf_size[i] = canvas_info->yuv[i].length;
			dsp_canvas_yuv_offset[i] = canvas_info->yuv[i].offset;
			if (canvas_info->yuv[i].addr) {
				dsp_canvas_yuv_buf_mem[i] = (u8*)mmap(NULL, dsp_canvas_yuv_buf_size[i],
					PROT_READ, MAP_SHARED, fd_iav, canvas_info->yuv[i].addr);
				if (dsp_canvas_yuv_buf_mem[i] == MAP_FAILED) {
					perror("mmap failed\n");
					return -1;
				}
			}
			dsp_canvas_me_buf_size[i] = canvas_info->me[i].length;
			dsp_canvas_me_offset[i] = canvas_info->me[i].offset;
			if (canvas_info->me[i].addr) {
				dsp_canvas_me_buf_mem[i] = (u8*)mmap(NULL, dsp_canvas_me_buf_size[i],
					PROT_READ, MAP_SHARED, fd_iav, canvas_info->me[i].addr);
				if (dsp_canvas_me_buf_mem[i] == MAP_FAILED) {
					perror("mmap failed\n");
					return -1;
				}
			}
		}
	}

	return 0;
}

static int map_extra_raw_buffer(void)
{
	struct iav_querymem query_mem;
	struct iav_mem_part_info *part_info;

	memset(&query_mem, 0, sizeof(query_mem));
	query_mem.mid = IAV_MEM_PARTITION;
	part_info = &query_mem.arg.partition;
	part_info->pid = IAV_PART_EXTRA_RAW;
	if (ioctl(fd_iav, IAV_IOC_QUERY_MEMBLOCK, &query_mem) < 0) {
		perror("IAV_IOC_QUERY_MEMBLOCK");
		return -1;
	}

	extra_raw_size = part_info->mem.length;
	if (extra_raw_size > 0) {
		extra_raw_mem = (u8*)mmap(NULL, extra_raw_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_iav,
			part_info->mem.addr);
		if (extra_raw_mem == MAP_FAILED) {
			perror("mmap IAV_PART_EXTRA_RAW failed\n");
			return -1;
		}
	} else {
		printf("Error: extra raw buffer size is 0.\n");
		return -1;
	}

	return 0;
}

static int map_dsp_buffer(void)
{
	struct iav_querymem query_mem;
	struct iav_mem_part_info *part_info;

	memset(&query_mem, 0, sizeof(query_mem));
	query_mem.mid = IAV_MEM_PARTITION;
	part_info = &query_mem.arg.partition;
	part_info->pid = IAV_PART_DSP;
	if (ioctl(fd_iav, IAV_IOC_QUERY_MEMBLOCK, &query_mem) < 0) {
		perror("IAV_IOC_QUERY_MEMBLOCK");
		return -1;
	}

	dsp_size = part_info->mem.length;
	dsp_mem = (u8*)mmap(NULL, dsp_size, PROT_READ, MAP_SHARED, fd_iav,
		part_info->mem.addr);
	if (dsp_mem == MAP_FAILED) {
		perror("mmap IAV_PART_DSP failed\n");
		return -1;
	}
	dsp_buf_mapped = 1;

	memset(&query_mem, 0, sizeof(query_mem));
	query_mem.mid = IAV_MEM_PARTITION;
	part_info = &query_mem.arg.partition;
	part_info->pid = IAV_PART_PYRAMID_POOL;
	if (ioctl(fd_iav, IAV_IOC_QUERY_MEMBLOCK, &query_mem) < 0) {
		perror("IAV_IOC_QUERY_MEMBLOCK");
		return -1;
	}
	dsp_pyramid_buf_size = part_info->mem.length;
	if (dsp_pyramid_buf_size) {
		printf("user buffer size(0x%x) > 0, GDMA will be used\n", dsp_pyramid_buf_size);
		dsp_pyramid_buf = (u8*)mmap(NULL, dsp_pyramid_buf_size, PROT_READ | PROT_WRITE, MAP_SHARED,
			fd_iav, part_info->mem.addr);
		if (dsp_pyramid_buf == MAP_FAILED) {
			perror("mmap IAV_PART_PYRAMID_POOL failed\n");
			return -1;
		}
	}

	if ((capture_select == CAPTURE_PYRAMID_BUFFER) && pyramid_manual_feed) {
		memset(&query_mem, 0, sizeof(query_mem));
		query_mem.mid = IAV_MEM_PARTITION;
		part_info = &query_mem.arg.partition;
		part_info->pid = IAV_PART_PYRAMID_POOL;
		if (ioctl(fd_iav, IAV_IOC_QUERY_MEMBLOCK, &query_mem) < 0) {
			perror("IAV_IOC_QUERY_MEMBLOCK");
			return -1;
		}

		pyramid_pool_buf_size = part_info->mem.length;
		if (pyramid_pool_buf_size) {
			pyramid_pool_buf = (u8*)mmap(NULL, pyramid_pool_buf_size, PROT_READ, MAP_SHARED, fd_iav,
				part_info->mem.addr);
			if (pyramid_pool_buf == MAP_FAILED) {
				perror("mmap IAV_PART_PYRAMID_POOL failed\n");
				return -1;
			}
			pyramid_pool_buf_mapped = 1;
		}
	}

	return 0;
}

static int alloc_gdma_dst_buf(u32 size)
{
	struct iav_alloc_mem_part_fd alloc_mem_part;

	alloc_mem_part.length = size;
	alloc_mem_part.enable_cache = 1;
	if (ioctl(fd_iav, IAV_IOC_ALLOC_ANON_MEM_PART_FD, &alloc_mem_part) < 0) {
		perror("IAV_IOC_ALLOC_ANON_MEM_PART_FD");
		return -1;
	}
	gdma_dst_buf_fd = alloc_mem_part.dma_buf_fd;
	gdma_dst_buf_size = lseek(gdma_dst_buf_fd, 0, SEEK_END);
	if (gdma_dst_buf_size) {
		gdma_dst_buf = (u8*)mmap(NULL, gdma_dst_buf_size, PROT_READ | PROT_WRITE, MAP_SHARED,
			gdma_dst_buf_fd, 0);
		if (gdma_dst_buf == MAP_FAILED) {
			perror("mmap gdma dst buffer failed\n");
			return -1;
		}
	}

	return 0;
}

static int free_gdma_dst_buf(void)
{
	if (gdma_dst_buf && gdma_dst_buf_size) {
		munmap(gdma_dst_buf, gdma_dst_buf_size);
		gdma_dst_buf = NULL;
		gdma_dst_buf_size = 0;
	}

	if (gdma_dst_buf_fd >= 0) {
		close(gdma_dst_buf_fd);
		gdma_dst_buf_fd = -1;
	}

	return 0;
}

static int map_buffer(void)
{
	int ret = 0;

	if (dump_canvas_flag) {
		ret = map_canvas_buffers();
		if (ret < 0) {
			goto MAP_BUFFER_EXIT;
		}
	} else {
		ret = map_dsp_buffer();
		if (ret < 0) {
			goto MAP_BUFFER_EXIT;
		}
	}

	if (query_extra_raw_info_flag) {
		ret = map_extra_raw_buffer();
		if (ret < 0) {
			goto MAP_BUFFER_EXIT;
		}
	}

MAP_BUFFER_EXIT:
	return ret;
}

static u8* get_buffer_base(int buf_id, int me_flag)
{
	u8* addr = NULL;

	if (buf_id < 0 || buf_id >= IAV_MAX_CANVAS_BUF_NUM) {
		printf("Invaild canvas buf ID %d!\n", buf_id);
		return NULL;
	}
	if (dump_canvas_map & (1 << buf_id)) {
		if (me_flag) {
			addr = dsp_canvas_me_buf_mem[buf_id];
		} else {
			addr = dsp_canvas_yuv_buf_mem[buf_id];
		}
	} else {
		if ((capture_select == CAPTURE_PYRAMID_BUFFER) && pyramid_manual_feed) {
			addr = pyramid_pool_buf;
		} else {
			addr = dsp_mem;
		}
	}
	return addr;
}

static int get_yuv_format(int format, struct iav_yuv_cap *yuv_cap)
{
	int data_format = format;

	if (data_format == AUTO_FORMAT) {
		if (yuv_cap->format == IAV_YUV_FORMAT_YUV420) {
			data_format = YUV420_NV12;
		} else if (yuv_cap->format == IAV_YUV_FORMAT_YUV422) {
			data_format = YUV422_NV16;
		} else {
			printf("Unknown YUV format: %d\n", yuv_cap->format);
		}
	} else {
		/* Auto change the format between 420 and 422. */
		switch (data_format) {
		case YUV420_IYUV:
		case YUV420_YV12:
		case YUV420_NV12:
			/* YUV422 to YUV420 is not supported, change save format to YV16 */
			if (yuv_cap->format == IAV_YUV_FORMAT_YUV422) {
				data_format = YUV422_YV16;
			}
			break;
		case YUV422_YU16:
		case YUV422_YV16:
		case YUV422_NV16:
			/* YUV420 to YUV422 is not supported, change save format to NV12 */
			if (yuv_cap->format == IAV_YUV_FORMAT_YUV420) {
				data_format = YUV420_NV12;
			}
			break;
		default:
			break;
		}
	}

	return data_format;
}

static void get_yuv_format_name(int format, struct iav_yuv_cap *yuv_cap,
	char *format_name)
{
	int data_format;

	if (!format_name) {
		printf("Invalid format name buffer.\n");
		goto GET_YUV_FORMAT_EXIT;
	}

	data_format = get_yuv_format(format, yuv_cap);

	switch (data_format) {
		case YUV420_IYUV:
			sprintf(format_name, "IYUV");
			break;
		case YUV420_YV12:
			sprintf(format_name, "YV12");
			break;
		case YUV420_NV12:
			sprintf(format_name, "NV12");
			break;
		case YUV422_YU16:
			sprintf(format_name, "YU16");
			break;
		case YUV422_YV16:
			sprintf(format_name, "YV16");
			break;
		case YUV422_NV16:
			sprintf(format_name, "NV16");
			break;
		case YUV444:
			sprintf(format_name, "YUV444");
			break;
		default:
			sprintf(format_name, "Unknown [%d]", data_format);
			break;
	}

GET_YUV_FORMAT_EXIT:
	return;
}


static int convert_chroma_format(int buf_id, u8* input, u8* output,
	struct iav_yuv_cap *yuv_cap)
{
	int width, height, pitch;
	u8 *uv_addr = NULL;
	int format;
	yuv_neon_arg yuv;
	int ret = 0;

	if (input == NULL || output == NULL) {
		printf("Invalid pointer NULL!");
		return -1;
	}

	uv_addr = input;
	format = get_yuv_format(yuv_format, yuv_cap);
	width = yuv_cap->width;
	height = yuv_cap->height;
	pitch = yuv_cap->width;

	// input yuv is uv interleaved with padding (uvuvuvuv.....)
	if (yuv_cap->format == IAV_YUV_FORMAT_YUV420) {
		yuv.in = uv_addr;
		yuv.row = height / 2 ;
		yuv.col = width;
		yuv.pitch = pitch;
		if (format == YUV420_YV12) {
			// YV12 format (YYYYYYYYVVUU)
			yuv.u = output + width * height / 4;
			yuv.v = output;
			chrome_convert(&yuv);
		} else if (format == YUV420_IYUV) {
			// IYUV (I420) format (YYYYYYYYUUVV)
			yuv.u = output;
			yuv.v = output + width * height / 4;
			chrome_convert(&yuv);
		} else if (format == YUV444) {
			yuv.u = output;
			yuv.v = output + width * height;
			chrome_UV20_convert_to_UV44(&yuv);
		} else {
			printf("Unexpected! No need chroma convert!\n");
			ret = -1;
		}
	} else if (yuv_cap->format == IAV_YUV_FORMAT_YUV422) {
		yuv.in = uv_addr;
		yuv.row = height;
		yuv.col = width;
		yuv.pitch = pitch;
		if (format == YUV422_YU16) {
			// YU16 format (YYYYYYYYUUUUVVVV)
			yuv.u = output;
			yuv.v = output + width * height / 2;
			chrome_convert(&yuv);
		} else if (format == YUV422_YV16) {
			// YV16 format (YYYYYYYYVVVVUUUU)
			yuv.u = output + width * height / 2;
			yuv.v = output;
			chrome_convert(&yuv);
		} else if (format == YUV444) {
			yuv.u = output;
			yuv.v = output + width * height;
			chrome_UV22_convert_to_UV44(&yuv);
		} else {
			printf("Unexpected! No need chroma convert!\n");
			ret = -1;
		}
	} else {
		printf("Error: Unsupported YUV input format!\n");
		ret = -1;
	}

	return ret;
}

static int save_yuv_data(int fd, int buf_id, struct iav_yuv_cap *yuv_cap)
{
	static int pts_prev = 0, pts = 0;
	int luma_size, chroma_size;
	u8 * luma_addr = NULL, *chroma_addr = NULL, *chroma_convert_addr = NULL;
	int format = get_yuv_format(yuv_format, yuv_cap);
	int ret = 0;

	/* for luma */
	luma_size = yuv_cap->width * yuv_cap->height;
	luma_addr = gdma_dst_buf;

	/* for chroma */
	chroma_addr = luma_addr + luma_size;
	if (yuv_cap->format == IAV_YUV_FORMAT_YUV420) {
		switch (format) {
			case YUV420_NV12:
				chroma_size = luma_size >> 1;
				break;
			case YUV420_IYUV:
			case YUV420_YV12:
				chroma_size = luma_size >> 1;
				chroma_convert_addr = chroma_addr + chroma_size;
				break;
			case YUV444:
				chroma_size = luma_size << 1;
				chroma_convert_addr = chroma_addr + (luma_size >> 1);
				break;
			default:
				printf("Invalid convertion.\n");
				ret = -1;
				break;
		}
	} else if (yuv_cap->format == IAV_YUV_FORMAT_YUV422) {
		switch (format) {
			case YUV422_NV16:
				chroma_size = luma_size;
				break;
			case YUV422_YU16:
			case YUV422_YV16:
				chroma_size = luma_size;
				chroma_convert_addr = chroma_addr + chroma_size;
				break;
			case YUV444:
				chroma_size = luma_size << 1;
				chroma_convert_addr = chroma_addr + luma_size;
				break;
			default:
				printf("Invalid convertion.\n");
				ret = -1;
				break;
		}
	} else {
		printf("Error: Unrecognized yuv data format from DSP!\n");
		ret = -1;
	}

	if (ret != 0) {
		goto SAVE_YUV_DATA_EXIT;
	}

	/* Save YUV data into memory */
	if (verbose) {
		gettimeofday(&curr, NULL);
		pre = curr;
	}

	/* Save UV data into another memory if it needs convert. */
	if (format == YUV420_YV12 || format == YUV420_IYUV ||
		format == YUV444 || format == YUV422_YU16 ||
		format == YUV422_YV16) {
		if (convert_chroma_format(buf_id, chroma_addr, chroma_convert_addr, yuv_cap) < 0) {
			perror("Failed to save chroma data into buffer !\n");
			ret = -1;
			goto SAVE_YUV_DATA_EXIT;
		}
		chroma_addr = chroma_convert_addr;
	}

	if (verbose) {
		gettimeofday(&curr, NULL);
		printf("33. Save UV [%06ld us].\n", (curr.tv_sec - pre.tv_sec) *
			1000000 + (curr.tv_usec - pre.tv_usec));
	}

	/* Write YUV data from memory to file */
	if (amba_transfer_write(fd, luma_addr, luma_size, transfer_method) < 0) {
		perror("Failed to save luma data into file !\n");
		ret = -1;
		goto SAVE_YUV_DATA_EXIT;
	}
	if (amba_transfer_write(fd, chroma_addr, chroma_size, transfer_method) < 0) {
		perror("Failed to save chroma data into file !\n");
		ret = -1;
		goto SAVE_YUV_DATA_EXIT;
	}

	if (verbose) {
		pts = yuv_cap->mono_pts;
		printf("BUF [%d] Y 0x%08x, UV 0x%08x, pitch %u, %ux%u = Seqnum[%u], "
			"PTS [%u-%u].\n", buf_id, (u32)yuv_cap->y_addr_offset,
			(u32)yuv_cap->uv_addr_offset, yuv_cap->pitch, yuv_cap->width,
			yuv_cap->height, yuv_cap->seq_num, pts, (pts - pts_prev));
		pts_prev = pts;
	}

SAVE_YUV_DATA_EXIT:
	return ret;
}

static int copy_yuv_data(int dmabuf_fd, struct iav_yuv_cap *yuv_cap, u8 is_canvas, u8 is_manual_feed)
{
	struct iav_gdma_copy gdma_copy = {0};
	int rval = 0;

	gdma_copy.src_skip_cache_sync = 1;
	gdma_copy.dst_skip_cache_sync = 1;
	gdma_copy.src_offset = yuv_cap->y_addr_offset;
	gdma_copy.dst_offset = 0;
	gdma_copy.src_pitch = yuv_cap->pitch;
	gdma_copy.dst_pitch = yuv_cap->width;
	gdma_copy.width = yuv_cap->width;
	gdma_copy.height = yuv_cap->height;

	if (dmabuf_fd > 0) {
		gdma_copy.src_dma_buf_fd = dmabuf_fd;
		gdma_copy.src_use_dma_buf_fd = 1;
	} else {
		if (is_canvas) {
			gdma_copy.src_mmap_type = is_manual_feed ? IAV_PART_CANVAS_POOL : IAV_PART_DSP;
		} else {
			gdma_copy.src_mmap_type = is_manual_feed ? IAV_PART_PYRAMID_POOL : IAV_PART_DSP;
		}
	}
	gdma_copy.dst_dma_buf_fd = gdma_dst_buf_fd;
	gdma_copy.dst_use_dma_buf_fd = 1;
	if (ioctl(fd_iav, IAV_IOC_GDMA_COPY, &gdma_copy) < 0) {
		perror("IAV_IOC_GDMA_COPY");
		rval = -1;
		goto COPY_YUV_DATA_EXIT;
	}

	gdma_copy.src_offset = yuv_cap->uv_addr_offset;
	gdma_copy.dst_offset = yuv_cap->width * yuv_cap->height;
	if (yuv_cap->format == IAV_YUV_FORMAT_YUV420) {
		gdma_copy.height = yuv_cap->height / 2;
	} else {
		gdma_copy.height = yuv_cap->height;
	}
	if (ioctl(fd_iav, IAV_IOC_GDMA_COPY, &gdma_copy) < 0) {
		perror("IAV_IOC_GDMA_COPY");
		rval = -1;
		goto COPY_YUV_DATA_EXIT;
	}

COPY_YUV_DATA_EXIT:
	return rval;
}
static int get_yuv_buffer_size(struct iav_yuv_cap *yuv_cap, int format)
{
	/* layout: luma + chroma + convert_chroma(if yuv type needs to convert) */
	int luma_size;
	int total_size;

	luma_size = yuv_cap->pitch * ROUND_UP(yuv_cap->height, 16);

	if (yuv_cap->format == IAV_YUV_FORMAT_YUV420) {
		switch (format) {
			case YUV420_IYUV:
			case YUV420_YV12:
			case YUV420_NV12:
				total_size = luma_size * 2;
				break;
			case YUV444:
				total_size = (luma_size * 7) >> 1;
				break;
			default:
				total_size = 0;
				break;
		}
	} else if (yuv_cap->format == IAV_YUV_FORMAT_YUV422) {
		switch (format) {
			case YUV422_NV16:
				total_size = luma_size * 2;
				break;
			case YUV422_YU16:
			case YUV422_YV16:
				total_size = luma_size * 3;
				break;
			case YUV444:
				total_size = luma_size * 4;
				break;
			default:
				total_size = 0;
				break;
		}
	} else {
		total_size = 0;
	}

	total_size = ROUND_UP(total_size, 16);

	return total_size;
}

static int get_max_buffer_size_in_canvasgrp(struct iav_canvasgrpdesc *canvasgrp)
{
	struct iav_yuv_cap *yuv_cap;
	int buf_map = canvasgrp->canvas_id_map;
	int max_size = 0, cur_size = 0;
	int i;

	for (i = 0; i < G_canvas_num; i++) {
		if (buf_map & (1 << i)) {
			yuv_cap = &canvasgrp->yuv[i];
			cur_size = get_yuv_buffer_size(yuv_cap,
				get_yuv_format(yuv_format, yuv_cap));
			if (max_size == 0) {
				max_size = cur_size;
			} else {
				max_size = cur_size > max_size ? cur_size : max_size;
			}
		}
	}

	return max_size;
}

static int dump_yuv()
{
	/* TO DO */
	printf("Canvas mem dump hasn't been supported as source buffer auto stop hasn't been implemented yet.\n");
	return -1;
#if 0
	int i, buf;
	char yuv_file[320];
	u8 *yuv = NULL;
	u8 *p_in = NULL;
	u8 *p_out = NULL;
	yuv_neon_arg yuv_neon;

	int width = 0;
	int height = 0;
	int max_height = 0;
	int pitch = 0;
	int one_line = 0;
	int r_size = 0;
	int w_size = 0;
	int remain_size = 0;

	enum iav_yuv_format canvas_format;
	struct iav_querydesc query_desc;
	struct iav_yuv_cap *yuv_cap;

	for (buf = 0; buf < IAV_MAX_CANVAS_BUF_NUM; ++buf) {
		if (dump_canvas_map & (1 << buf)) {
			/* It's also supported to use 'IAV_DESC_YUV' here. */
			query_desc.qid = IAV_DESC_CANVAS;
			query_desc.arg.canvas.canvas_id = buf;
			query_desc.arg.canvas.non_block_flag &= ~IAV_BUFCAP_NONBLOCK;
			if (ioctl(fd_iav, IAV_IOC_QUERY_DESC, &query_desc) < 0) {
				if (errno == EINTR) {
					continue;		/* back to for() */
				} else {
					perror("IAV_IOC_QUERY_DESC");
					goto dump_yuv_error_exit;
				}
			}

			yuv_cap = &query_desc.arg.canvas.yuv;
			width = yuv_cap->width;
			height = yuv_cap->height;
			/* FIXME: use max height the same as height */
			max_height = height;
			pitch = yuv_cap->pitch;
			canvas_format = yuv_cap->format;

			if (fd_yuv[buf] < 0) {
				memset(yuv_file, 0, sizeof(yuv_file));
				sprintf(yuv_file, "%s_canvas%d_%dx%d.yuv", filename, buf,
					width, height);
				fd_yuv[buf] = amba_transfer_open(yuv_file, transfer_method,
					port++);
				if (fd_yuv[buf] < 0) {
					printf("Cannot open file [%s].\n", yuv_file);
					continue;
				}
			}

			if (canvas_format ==  IAV_YUV_FORMAT_YUV420) {
				w_size = (width * height) * 3 / 2;
				r_size = (pitch * max_height) * 3 / 2;
			} else if (canvas_format ==  IAV_YUV_FORMAT_YUV422) {
				w_size = (width * height) * 2;
				r_size = (pitch * max_height) * 2;
			} else {
				printf("Invalid canvas format [%u]!\n", canvas_format);
				return -1;
			}

			p_in = dsp_canvas_yuv_buf_mem[buf] + dsp_canvas_yuv_offset[buf];
			remain_size = dsp_canvas_yuv_buf_size[buf] - dsp_canvas_yuv_offset[buf];

			if (yuv == NULL) {
				yuv = malloc(w_size);
				if (yuv == NULL) {
					printf("Not enough memory for preview dump !\n");
					goto dump_yuv_error_exit;
				}
			}

			for (i = 0; remain_size >= r_size; i++) { // memcpy pre frame in NV12 format
				p_out = yuv;
				/* copy Y */
				for (one_line = 0; one_line < height; one_line++) {
					memcpy(p_out, p_in, width);
					p_in += pitch;
					p_out += width;
				}
				/* consider max_height here */
				p_in += (max_height - height) * pitch;
				if (canvas_format == IAV_YUV_FORMAT_YUV420) {
					for (one_line = 0; one_line < height / 2; one_line++) {
						memcpy(p_out, p_in, width);
						p_in += pitch;
						p_out += width;
					}
					p_in += ((max_height - height) * pitch) / 2;
				} else if (canvas_format == IAV_YUV_FORMAT_YUV422) {
					yuv_neon.in = p_in;
					yuv_neon.row = height;
					yuv_neon.col = width;
					yuv_neon.pitch = pitch;
					yuv_neon.u = p_out;
					yuv_neon.v = p_out + width * height / 2;
					chrome_convert(&yuv_neon);

					p_in += max_height * pitch;
				}
				remain_size -= r_size;
				if (amba_transfer_write(fd_yuv[buf], yuv, w_size, transfer_method) < 0) {
					perror("Failed to save yuv all memory data into file !\n");
					goto dump_yuv_error_exit;
				}
			}
			printf("Dump YUV: resolution %dx%d in %s format, total frame num "
				"[%d], ""file: %s\n", width, height,
				(canvas_format == IAV_YUV_FORMAT_YUV420) ? "NV12" : "YU16",
				i, yuv_file);
		}
	}

	if (yuv) {
		free(yuv);
	}
	return 0;

dump_yuv_error_exit:
	if (yuv) {
		free(yuv);
	}
	return -1;
#endif
}

void process_extra_raw_info(struct iav_canvasdesc *canvas, int num)
{
	struct iav_extra_raw_cap *extra_raw;
	int i = 0;
	u32 pitch = 0, height = 0;
	unsigned long data_addr_offset = 0;
	u8 canvas_id = canvas->canvas_id;

	extra_raw = &canvas->extra_raw;
	pitch = extra_raw->pitch;
	height = extra_raw->height;
	data_addr_offset = extra_raw->data_addr_offset;

	for (i = 0; i < height; i++){
		printf("Canvas[%d] : Frame[%d], the first value of line[%d] = 0x%x\n", canvas_id,
			num, i, *(unsigned int *)(extra_raw_mem + data_addr_offset + pitch * i));
	}
}

static int capture_yuv(u32 canvas_map, u32 canvas_map_thru_dmabuf, int count, int save_flag)
{
	int i = 0;
	int buf = 0;
	int save[IAV_MAX_CANVAS_BUF_NUM];
	int write_flag[IAV_MAX_CANVAS_BUF_NUM];
	int non_stop = 0;
	int yuv_buffer_size = 0;
	char yuv_file[320];
	char format_str[32];
	struct iav_querydesc query_desc;
	struct iav_canvasdesc *canvas;
	struct iav_yuv_cap *yuv_cap;
	struct iav_yuv_cap yuv_cap_cache;
	u64 pts_prev[IAV_MAX_CANVAS_BUF_NUM] = {0};
	u64 pts[IAV_MAX_CANVAS_BUF_NUM] = {0};
	u16 intval = 0;
	int rval = 0;

	memset(save, 0, sizeof(save));
	memset(write_flag, 0, sizeof(write_flag));

	if (count == 0) {
		non_stop = 1;
	}

	i = 0;
	while ((i < count || non_stop) && !quit_capture) {
		/* query */
		memset(&query_desc, 0, sizeof(query_desc));

		for (buf = 0; buf < G_canvas_num; buf++) {
			if (!(canvas_map & (1 << buf))) {
				continue;
			}

			if (canvas_yuv_buffer_disable[buf]) {
				printf("Canvas[%d] yuv buffer is disabled, cannot get yuv data!\n", buf);
				rval = -1;
				break;
			}

			query_desc.qid = IAV_DESC_CANVAS;
			canvas = &query_desc.arg.canvas;
			canvas->canvas_id = buf;
			canvas->query_extra_raw = query_extra_raw_info_flag;
			canvas->yuv_use_dma_buf_fd = !!(canvas_map_thru_dmabuf & (1 << buf));
			canvas->me_use_dma_buf_fd = 0;
			canvas->skip_cache_sync = 1;
			if (!non_block_read) {
				canvas->non_block_flag &= ~IAV_BUFCAP_NONBLOCK;
			} else {
				canvas->non_block_flag |= IAV_BUFCAP_NONBLOCK;
			}
			if (verbose) {
				gettimeofday(&curr, NULL);
				pre = curr;
			}
			if (ioctl(fd_iav, IAV_IOC_QUERY_DESC, &query_desc) < 0) {
				perror("IAV_IOC_QUERY_DESC");
				i--;
				continue;
			}
			if (query_extra_raw_info_flag) {
				process_extra_raw_info(canvas, i);
			}

			if (verbose) {
				gettimeofday(&curr, NULL);
				printf("11. Query DESC [%06ld us].\n", 1000000 *
					(curr.tv_sec - pre.tv_sec)	+ (curr.tv_usec - pre.tv_usec));
			}

			/* gdma copy allocate */
			if (gdma_dst_buf_fd < 0 && save_flag) {
				yuv_cap = &canvas->yuv;
				yuv_buffer_size = get_yuv_buffer_size(yuv_cap,
					get_yuv_format(yuv_format, yuv_cap));

				if (yuv_buffer_size == 0) {
					printf("buffer size need allocate is 0!.\n");
					rval = -1;
					break;
				}

				if (alloc_gdma_dst_buf(yuv_buffer_size)) {
					rval = -1;
					break;
				}
			}

			yuv_cap = &canvas->yuv;

			if (verbose) {
				gettimeofday(&curr, NULL);
				pre = curr;
			}

			if ((yuv_cap->y_addr_offset == 0) ||
				(yuv_cap->uv_addr_offset == 0)) {
				printf("YUV buffer [%d] address is NULL! Skip to next!\n", buf);
				continue;
			}

			get_yuv_format_name(yuv_format, yuv_cap, format_str);

			if (save_flag) {
				if (fd_yuv[buf] < 0) {
					memset(yuv_file, 0, sizeof(yuv_file));
					sprintf(yuv_file, "%s_canvas%d_%dx%d_%s.yuv", filename,
						buf, yuv_cap->width, yuv_cap->height, format_str);
					fd_yuv[buf] = amba_transfer_open(yuv_file, transfer_method, port++);
					if (fd_yuv[buf] < 0) {
						printf("Cannot open file [%s].\n", yuv_file);
						rval = -1;
						break;
					}
				}

				rval = copy_yuv_data(canvas->yuv_dma_buf_fd,
					&canvas->yuv, 1, canvas_mf_enable[buf]);
				if (canvas->yuv_dma_buf_fd) {
					close(canvas->yuv_dma_buf_fd);
					canvas->yuv_dma_buf_fd = 0;
				}
				if (rval < 0) {
					printf("Failed to copy yuv data of buf [%d].\n", buf);
					break;
				}

				if (verbose) {
					gettimeofday(&curr, NULL);
					printf("12. GDMA copy [%06ld us].\n", 1000000 *
						(curr.tv_sec - pre.tv_sec) + (curr.tv_usec - pre.tv_usec));
				}

				if (delay_frame_cap_data) {
					if (write_flag[buf] == 0) {
						write_flag[buf] = 1;
						yuv_cap_cache = *yuv_cap;
					} else {
						write_flag[buf] = 0;
						if (save_yuv_data(fd_yuv[buf], buf, &yuv_cap_cache) < 0) {
							printf("Failed to save YUV data of buf [%d].\n", buf);
							rval = -1;
							break;
						}
					}
				} else {
					if (save_yuv_data(fd_yuv[buf], buf, yuv_cap) < 0) {
						printf("Failed to save YUV data of buf [%d].\n", buf);
						rval = -1;
						break;
					}
				}

				if (save[buf] == 0) {
					save[buf] = 1;

					printf("Delay %d frame capture YUV data.\n", delay_frame_cap_data);
					printf("Capture YUV cavas(buffer)%d: size[%dx%d] in %s format\n",
						buf, yuv_cap->width, yuv_cap->height, format_str);
				}
			} else {
				pts[buf] = yuv_cap->mono_pts;
				if (verbose) {
					printf("Capture YUV cavas(buffer)%d: size[%dx%d] in %s format "
						"Seqnum[%u] PTS [%llu-%llu], led_enable %d. \n", buf, yuv_cap->width,
						yuv_cap->height, format_str, yuv_cap->seq_num, pts[buf],
						(pts[buf] - pts_prev[buf]), canvas->led_enable);
				}

				if (pts_prev[buf]) {
					intval = pts[buf] - pts_prev[buf];
					if ((intval > pts_intval[buf] + MARGIN) ||
						(intval < pts_intval[buf] - MARGIN)) {
						printf("Error! Discontinuous mono PTS, frame lost for "
							"canvas[%d]!\n", buf);
					}
				}
				pts_prev[buf] = pts[buf];
			}

			if(canvas_mf_enable[buf]) {
				if (ioctl(fd_iav, IAV_IOC_RELEASE_CANVAS_BUF, canvas) < 0) {
					perror("IAV_IOC_RELEASE_CANVAS_BUF");
					rval = -1;
					break;
				}
			}
		}

		if (rval < 0) {
			break;
		}

		i++;
	}

	if (gdma_dst_buf_fd >= 0) {
		free_gdma_dst_buf();
	}

	return rval;
}

static int capture_multi_yuv(u32 canvas_map, u32 canvas_map_thru_dmabuf, int count, int save_flag)
{
	int i, j, k, non_stop = 0;
	int save[IAV_MAX_CANVAS_BUF_NUM];
	int yuv_buffer_size = 0;
	char yuv_file[320], format_str[32];
	struct iav_querydesc query_desc;
	struct iav_canvasgrpdesc *canvasgrp = NULL;
	struct iav_yuv_cap *yuv_cap;
	u64 pts_prev[IAV_MAX_CANVAS_BUF_NUM] = {0};
	u64 pts[IAV_MAX_CANVAS_BUF_NUM] = {0};
	u16 intval = 0;
	int rval = 0;

	memset(save, 0, sizeof(save));

	if (count == 0) {
		non_stop = 1;
	}

	i = 0;
	while ((i < count || non_stop) && !quit_capture) {
		/* query */
		memset(&query_desc, 0, sizeof(query_desc));
		query_desc.qid = IAV_DESC_CANVASGRP;
		canvasgrp = &query_desc.arg.canvasgrp;
		canvasgrp->canvas_id_map = canvas_map;
		canvasgrp->skip_cache_sync = 1;
		canvasgrp->yuv_use_dma_buf_fd = !!canvas_map_thru_dmabuf;
		canvasgrp->me_use_dma_buf_fd = 0;

		if (verbose) {
			gettimeofday(&curr, NULL);
			pre = curr;
		}
		if (ioctl(fd_iav, IAV_IOC_QUERY_DESC, &query_desc) < 0) {
			perror("IAV_IOC_QUERY_DESC");
			rval = -1;
			break;
		}
		if (verbose) {
			gettimeofday(&curr, NULL);
			printf("11. Query DESC [%06ld us].\n", 1000000 *
				(curr.tv_sec - pre.tv_sec)  + (curr.tv_usec - pre.tv_usec));
		}

		/* gdma copy allocate */
		if (gdma_dst_buf_fd < 0 && save_flag) {
			yuv_buffer_size = get_max_buffer_size_in_canvasgrp(canvasgrp);

			if (yuv_buffer_size == 0) {
				printf("buffer size need allocate is 0!.\n");
				rval = -1;
				break;
			}

			if (alloc_gdma_dst_buf(yuv_buffer_size)) {
				rval = -1;
				break;
			}
		}

		if (canvas_map != canvasgrp->canvas_id_map) {
			printf("Target canvas map %d, return canvas map %d.\n",
				canvas_map, canvasgrp->canvas_id_map);
			canvas_map = canvasgrp->canvas_id_map;
		}

		for (j = 0; j < G_canvas_num; j++) {
			if (canvas_map & (1 << j)) {
				if (canvas_yuv_buffer_disable[j]) {
					printf("Canvas[%d] yuv buffer is disabled, cannot get yuv data!\n", j);
					rval = -1;
					break;
				}

				yuv_cap = &canvasgrp->yuv[j];

				if (verbose) {
					gettimeofday(&curr, NULL);
					pre = curr;
				}

				if ((yuv_cap->y_addr_offset == 0) ||
					(yuv_cap->uv_addr_offset == 0)) {
					printf("YUV buffer [%d] address is NULL! Skip to next!\n", j);
					continue;
				}

				get_yuv_format_name(yuv_format, yuv_cap, format_str);

				if (save_flag) {
					if (fd_yuv[j] < 0) {
						memset(yuv_file, 0, sizeof(yuv_file));
						sprintf(yuv_file, "%s_canvas%d_%dx%d_%s.yuv", filename,
							j, yuv_cap->width, yuv_cap->height, format_str);
						fd_yuv[j] = amba_transfer_open(yuv_file, transfer_method, port++);
						if (fd_yuv[j] < 0) {
							printf("Cannot open file [%s].\n", yuv_file);
							rval = -1;
							break;
						}
					}

					rval = copy_yuv_data(canvasgrp->yuv_dma_buf_fd[j],
						&canvasgrp->yuv[j], 1, canvas_mf_enable[j]);
					if (canvasgrp->yuv_dma_buf_fd[j]) {
						close(canvasgrp->yuv_dma_buf_fd[j]);
						canvasgrp->yuv_dma_buf_fd[j] = 0;
					}
					if (rval < 0) {
						/* close all dma-buf:fd */
						for (k = 0; k < G_canvas_num; k++) {
							if (canvasgrp->yuv_dma_buf_fd[k]) {
								close(canvasgrp->yuv_dma_buf_fd[k]);
								canvasgrp->yuv_dma_buf_fd[k] = 0;
							}
						}
						printf("Failed to copy yuv data of buf [%d].\n", j);
						break;
					}

					if (verbose) {
						gettimeofday(&curr, NULL);
						printf("12. GDMA copy [%06ld us].\n", 1000000 *
							(curr.tv_sec - pre.tv_sec) + (curr.tv_usec - pre.tv_usec));
					}

					if (save_yuv_data(fd_yuv[j], j, yuv_cap) < 0) {
						printf("Failed to save YUV data of buf [%d].\n", j);
						rval = -1;
						break;
					}

					if (save[j] == 0) {
						save[j] = 1;
						printf("Capture YUV cavas(buffer)%d: size[%dx%d] in %s format\n",
							j, yuv_cap->width, yuv_cap->height, format_str);
					}
				} else {
					pts[j] = yuv_cap->mono_pts;
					if (verbose) {
						printf("Capture YUV cavas(buffer)%d: size[%dx%d] in %s "
							"format Seqnum[%u] PTS [%llu-%llu]. \n", j,
							yuv_cap->width, yuv_cap->height, format_str,
							yuv_cap->seq_num, pts[j], (pts[j] - pts_prev[j]));
					}

					if (pts_prev[j]) {
						intval = pts[j] - pts_prev[j];
						if ((intval > pts_intval[j] + MARGIN) ||
							(intval < pts_intval[j] - MARGIN)) {
							printf("Error! Discontinuous mono PTS, frame lost "
								"for canvas[%d]!\n", j);
						}
					}
					pts_prev[j] = pts[j];
				}
			}
		}

		if (rval < 0) {
			break;
		}

		i++;
	}


	if (gdma_dst_buf_fd >= 0) {
		free_gdma_dst_buf();
	}
	return rval;

}

static int capture_pyramid(int pyramid_map, int count, int save_flag)
{
	int i, buf;
	char pyramid_file[320];
	int non_stop = 0;
	char format_str[32];
	struct iav_querydesc query_desc;
	struct iav_yuv_cap *pyramid_cap;
	struct iav_feed_pyramid feed_pyramid;
	struct iav_pyramiddesc *pyramid = NULL;
	int yuv_buffer_size = 0;
	u64 pts_prev[IAV_MAX_PYRAMID_LAYERS] = {0};
	u64 pts[IAV_MAX_PYRAMID_LAYERS] = {0};
	int rval = 0;

	if (count == 0) {
		non_stop = 1;
	}

	i = 0;
	while ((i < count || non_stop) && !quit_capture) {
		memset(&query_desc, 0, sizeof(query_desc));
		query_desc.qid = IAV_DESC_PYRAMID;
		pyramid = &query_desc.arg.pyramid;
		pyramid->chan_id = current_channel;
		pyramid->skip_cache_sync = 1;
		if (!non_block_read) {
			pyramid->non_block_flag &= ~IAV_BUFCAP_NONBLOCK;
		} else {
			pyramid->non_block_flag |= IAV_BUFCAP_NONBLOCK;
		}

		if (verbose) {
			gettimeofday(&curr, NULL);
			pre = curr;
		}

		if (pyramid_manual_feed) {
			feed_pyramid.chan_id = current_channel;
			if (ioctl(fd_iav, IAV_IOC_FEED_PYRAMID_BUF, &feed_pyramid) < 0) {
				perror("IAV_IOC_FEED_PYRAMID_BUF");
				rval = -1;
				break;
			}
		}

		if (ioctl(fd_iav, IAV_IOC_QUERY_DESC, &query_desc) < 0) {
			if ((errno == EINTR) || (errno == EAGAIN)) {
				continue;		/* back to for() */
			} else {
				perror("IAV_IOC_QUERY_DESC");
				rval = -1;
				break;
			}
		}
		if (verbose) {
			gettimeofday(&curr, NULL);
			printf("11. Query DESC [%06ld us].\n", 1000000 *
				(curr.tv_sec - pre.tv_sec)  + (curr.tv_usec - pre.tv_usec));
		}

		for (buf = 0; buf < IAV_MAX_PYRAMID_LAYERS; ++buf) {
			if ((pyramid_map & (1 << buf)) == 0) {
				continue;
			}

			if ((pyramid->layers_map & (1 << buf)) == 0) {
				printf("Pyramid channel %d: layer %d is not switched on\n",
					current_channel, buf);
				continue;
			}

			pyramid_cap = &pyramid->layers[buf];
			get_yuv_format_name(yuv_format, pyramid_cap, format_str);

			if (save_flag) {
				if (fd_pyramid[buf] < 0) {
					memset(pyramid_file, 0, sizeof(pyramid_file));
					sprintf(pyramid_file, "%s_chan_%d_%d_%dx%d.yuv", filename, current_channel,
						buf, pyramid_cap->width, pyramid_cap->height);
					if (fd_pyramid[buf] < 0) {
						fd_pyramid[buf] = amba_transfer_open(pyramid_file,
							transfer_method, port++);
					}
					if (fd_pyramid[buf] < 0) {
						printf("Cannot open file [%s].\n", pyramid_file);
						rval = -1;
						break;
					}
				}

				yuv_buffer_size = get_yuv_buffer_size(pyramid_cap,
					get_yuv_format(yuv_format, pyramid_cap));
				if (alloc_gdma_dst_buf(yuv_buffer_size)) {
					rval = -1;
					break;
				}

				if (verbose) {
					gettimeofday(&curr, NULL);
					pre = curr;
				}

				if (copy_yuv_data(-1, pyramid_cap, 0, decode_mode || pyramid_manual_feed) < 0) {
					printf("Failed to copy yuv data of buf [%d].\n", buf);
					rval = -1;
					break;
				}
				if (verbose) {
					gettimeofday(&curr, NULL);
					printf("12. GDMA copy [%06ld us].\n", 1000000 *
						(curr.tv_sec - pre.tv_sec) + (curr.tv_usec - pre.tv_usec));
				}

				if (save_yuv_data(fd_pyramid[buf], buf, pyramid_cap) < 0) {
					printf("Failed to save Pyramid data of buf [%d].\n", buf);
					rval = -1;
					break;
				}

				printf("Capture_Pyramid_buffer: Pyramid layer %d for chan %d "
					"resolution %dx%d in %s format\n", buf, current_channel,
					pyramid_cap->width, pyramid_cap->height, format_str);

				if (gdma_dst_buf_fd >= 0) {
					free_gdma_dst_buf();
				}
			} else {
				pts[buf] = pyramid_cap->mono_pts;
				printf("Capture_Pyramid_buffer: Pyramid layer %d for chan %d "
					"resolution %dx%d in %s format Seqnum[%u] PTS [%llu-%llu].\n", buf, current_channel,
					pyramid_cap->width, pyramid_cap->height, format_str,
					pyramid_cap->seq_num, pts[buf], (pts[buf] - pts_prev[buf]));
				pts_prev[buf] = pts[buf];
			}
		}
		if (rval < 0) {
			break;
		}

		if (pyramid_manual_feed) {
			if (ioctl(fd_iav, IAV_IOC_RELEASE_PYRAMID_BUF, pyramid) < 0) {
				perror("IAV_IOC_RELEASE_PYRAMID_BUF");
				rval = -1;
				break;
			}
		}

		++i;
	}

	if (gdma_dst_buf_fd >= 0) {
		free_gdma_dst_buf();
	}
	return rval;
}

static int save_me_luma_buffer(int buf_id, int dma_buf_fd, u8* output, struct iav_me_cap *me_cap)
{
	u8 *in = NULL, *out = NULL, *base = NULL;
	u32 buf_size = 0;
	int i;

	if (me_cap->pitch < me_cap->width) {
		printf("pitch size smaller than width!\n");
		return -1;
	}

	if (dma_buf_fd >= 0) {
		buf_size = lseek(dma_buf_fd, 0, SEEK_END);
		base = (u8*)mmap(NULL, buf_size, PROT_READ | PROT_WRITE, MAP_SHARED,
			dma_buf_fd, 0);
	} else {
		base = get_buffer_base(buf_id, 1);
	}

	if (base == NULL) {
		printf("Invalid buffer address for buffer %d ME!"
			" Map ME buffer from DSP first.\n", buf_id);
		return -1;
	}

	if (me_cap->pitch == me_cap->width) {
		memcpy(output, base + me_cap->data_addr_offset,
			me_cap->width * me_cap->height);
	} else {
		in = base + me_cap->data_addr_offset;
		out = output;
		for (i = 0; i < me_cap->height; i++) {	//row
			memcpy(out, in, me_cap->width);
			in += me_cap->pitch;
			out += me_cap->width;
		}
	}

	if (dma_buf_fd >= 0) {
		munmap(base, buf_size);
		close(dma_buf_fd);
	}

	return 0;
}

static int save_me_data(int fd, int dma_buf_fd, int buf_id, struct iav_me_cap *me_cap,
	u8 *y_buf, u8 *uv_buf)
{
	static u32 pts_prev = 0, pts = 0;

	save_me_luma_buffer(buf_id, dma_buf_fd, y_buf, me_cap);

	if (amba_transfer_write(fd, y_buf, me_cap->width * me_cap->height,
		transfer_method) < 0) {
		perror("Failed to save ME data into file !\n");
		return -1;
	}

	if (amba_transfer_write(fd, uv_buf, me_cap->width * me_cap->height / 2,
		transfer_method) < 0) {
		perror("Failed to save ME data into file !\n");
		return -1;
	}

	if (verbose) {
		pts = me_cap->mono_pts;
		printf("BUF [%d] 0x%08x, pitch %d, %dx%d, seqnum [%d], PTS [%d-%d].\n",
			buf_id, (u32)me_cap->data_addr_offset, me_cap->pitch,
			me_cap->width, me_cap->height, me_cap->seq_num,
			pts, (pts - pts_prev));
		pts_prev = pts;
	}

	return 0;
}

static int capture_me(u32 canvas_map, u32 canvas_map_thru_dmabuf, int count, int is_me1, int save_flag)
{
	int i, buf, save[IAV_MAX_CANVAS_BUF_NUM], non_stop = 0;
	int write_flag[IAV_MAX_CANVAS_BUF_NUM];
	char me_file[320];
	u8 *luma = NULL, *chroma = NULL;
	struct iav_querydesc query_desc;
	struct iav_canvasdesc *canvas;
	struct iav_me_cap *me_cap, me_cap_cache;
	u64 pts_prev[IAV_MAX_CANVAS_BUF_NUM] = {0};
	u64 pts[IAV_MAX_CANVAS_BUF_NUM] = {0};
	int rval = 0;

	if (save_flag) {
		luma = (u8*)malloc(MAX_ME_BUFFER_SIZE);
		if (luma == NULL) {
			printf("Not enough memory for ME buffer capture !\n");
			goto CAPTURE_ME_EXIT;
		}

		//clear UV to be B/W mode, UV data is not useful for motion detection,
		//just fill UV data to make YUV to be YV12 format, so that it can play in YUV player
		chroma = (u8*)malloc(MAX_ME_BUFFER_SIZE);
		if (chroma == NULL) {
			printf("Not enough memory for ME buffer capture !\n");
			goto CAPTURE_ME_EXIT;
		}
		memset(chroma, 0x80, MAX_ME_BUFFER_SIZE);
		memset(save, 0, sizeof(save));
		memset(write_flag, 0, sizeof(write_flag));
		memset(luma, 1, MAX_ME_BUFFER_SIZE);
	}

	if (count == 0) {
		non_stop = 1;
	}

	i = 0;
	while ((i < count || non_stop) && !quit_capture) {
		/* query */
		memset(&query_desc, 0, sizeof(query_desc));
		for (buf = 0; buf < G_canvas_num; buf++) {
			if (!(canvas_map & (1 << buf))) {
				continue;
			}

			query_desc.qid = IAV_DESC_CANVAS;
			canvas = &query_desc.arg.canvas;
			canvas->canvas_id = buf;
			canvas->me_use_dma_buf_fd = !!(canvas_map_thru_dmabuf & (1 << buf));
			if (!non_block_read) {
				canvas->non_block_flag &= ~IAV_BUFCAP_NONBLOCK;
			} else {
				canvas->non_block_flag |= IAV_BUFCAP_NONBLOCK;
			}

			if (verbose) {
				gettimeofday(&curr, NULL);
				pre = curr;
			}
			if (ioctl(fd_iav, IAV_IOC_QUERY_DESC, &query_desc) < 0) {
				if (errno == EINTR) {
					continue;		/* back to for() */
				} else {
					perror("IAV_IOC_QUERY_DESC");
					rval = -1;
					break;
				}
			}
			if (verbose) {
				gettimeofday(&curr, NULL);
				printf("Query DESC [%06ld us].\n", 1000000 *
					(curr.tv_sec - pre.tv_sec)	+ (curr.tv_usec - pre.tv_usec));
			}

			me_cap = is_me1 ? &canvas->me1 : &canvas->me0;

			if (me_cap->width == 0) {
				printf("Canvas[%d] doesn't have ME buffer!\n", buf);
				rval = -1;
				break;
			}

			if (save_flag) {
				if (fd_me[buf] < 0) {
					memset(me_file, 0, sizeof(me_file));
					if (!me_cap->width) {
						rval = -1;
						break;
					}
					sprintf(me_file, "%s_canvas%d_me_%dx%d.yuv", filename, buf,
						me_cap->width, me_cap->height);
					fd_me[buf] = amba_transfer_open(me_file, transfer_method, port++);
					if (fd_me[buf] < 0) {
						printf("Cannot open file [%s].\n", me_file);
						rval = -1;
						break;
					}
				}

				if (delay_frame_cap_data) {
					if (write_flag[buf] == 0) {
						write_flag[buf] = 1;
						me_cap_cache = *me_cap;
					} else {
						write_flag[buf] = 0;
						if (save_me_data(fd_me[buf], canvas->me_dma_buf_fd, buf, &me_cap_cache, luma, chroma) < 0) {
							printf("Failed to save ME data of buf [%d].\n", buf);
							rval = -1;
							break;
						}
					}
				} else {
					if (save_me_data(fd_me[buf], canvas->me_dma_buf_fd, buf, me_cap, luma, chroma) < 0) {
						printf("Failed to save ME data of buf [%d].\n", buf);
						rval = -1;
						break;
					}
				}

				if (save[buf] == 0) {
					save[buf] = 1;
					printf("Delay %d frame capture me data.\n", delay_frame_cap_data);
					printf("Save_me_buffer: resolution %dx%d with Luma only.\n",
						me_cap->width, me_cap->height);
				}
			} else {
				pts[buf] = me_cap->mono_pts;
				printf("Me_buffer: resolution %dx%d with Luma only,"
					"seq num[%u] PTS [%llu-%llu] idsp pts[%u].\n",
					me_cap->width, me_cap->height, me_cap->seq_num,
					pts[buf], (pts[buf] - pts_prev[buf]), me_cap->dsp_pts);
				pts_prev[buf] = pts[buf];
			}
		}

		if (rval < 0) {
			break;
		}

		++i;
	}


CAPTURE_ME_EXIT:
	if (luma)
		free(luma);
	if (chroma)
		free(chroma);
	return rval;
}

static int capture_multi_me(int canvas_map, int count, int is_me1, int save_flag)
{
	int i, j, save[IAV_MAX_CANVAS_BUF_NUM], non_stop = 0;
	char me_file[320];
	u8 *luma = NULL, *chroma = NULL;
	struct iav_querydesc query_desc;
	struct iav_canvasgrpdesc *canvasgrp;
	struct iav_me_cap *me_cap;
	u64 pts_prev[IAV_MAX_CANVAS_BUF_NUM] = {0};
	u64 pts[IAV_MAX_CANVAS_BUF_NUM] = {0};
	int rval = 0;

	if (save_flag) {
		luma = (u8*)malloc(MAX_ME_BUFFER_SIZE);
		if (luma == NULL) {
			printf("Not enough memory for ME buffer capture !\n");
			goto CAPTURE_MULTIPLE_ME_EXIT;
		}

		//clear UV to be B/W mode, UV data is not useful for motion detection,
		//just fill UV data to make YUV to be YV12 format, so that it can play in YUV player
		chroma = (u8*)malloc(MAX_ME_BUFFER_SIZE);
		if (chroma == NULL) {
			printf("Not enough memory for ME buffer capture !\n");
			goto CAPTURE_MULTIPLE_ME_EXIT;
		}
		memset(chroma, 0x80, MAX_ME_BUFFER_SIZE);
		memset(save, 0, sizeof(save));
		memset(luma, 1, MAX_ME_BUFFER_SIZE);
	}

	if (count == 0) {
		non_stop = 1;
	}

	i = 0;
	while ((i < count || non_stop) && !quit_capture) {
		/* query */
		memset(&query_desc, 0, sizeof(query_desc));
		query_desc.qid = IAV_DESC_CANVASGRP;
		canvasgrp = &query_desc.arg.canvasgrp;
		canvasgrp->canvas_id_map = canvas_map;

		if (verbose) {
			gettimeofday(&curr, NULL);
			pre = curr;
		}
		if (ioctl(fd_iav, IAV_IOC_QUERY_DESC, &query_desc) < 0) {
			if (errno == EINTR) {
				continue;		/* back to for() */
			} else {
				perror("IAV_IOC_QUERY_DESC");
				rval = -1;
				break;
			}
		}
		if (verbose) {
			gettimeofday(&curr, NULL);
			printf("Query DESC [%06ld us].\n", 1000000 *
				(curr.tv_sec - pre.tv_sec)	+ (curr.tv_usec - pre.tv_usec));
		}

		if (canvas_map != canvasgrp->canvas_id_map) {
			printf("Target query canvas map 0x%x, return canvas map 0x%x.\n",
				canvas_map, canvasgrp->canvas_id_map);
			canvas_map = canvasgrp->canvas_id_map;
		}

		for (j = 0; j < G_canvas_num; j++) {
			if (canvas_map & (1 << j)) {
				me_cap = is_me1 ? &canvasgrp->me1[j] : &canvasgrp->me0[j];

				if (me_cap->data_addr_offset == 0) {
					printf("Canvas[%d] doesn't have ME buffer! Skip to next!\n", j);
					continue;
				}

				if (save_flag) {
					if (fd_me[j] < 0) {
						memset(me_file, 0, sizeof(me_file));
						if (!me_cap->width) {
							continue;
						}
						sprintf(me_file, "%s_canvas%d_me_%dx%d.yuv", filename, j,
							me_cap->width, me_cap->height);
						fd_me[j] = amba_transfer_open(me_file, transfer_method,
							port++);
						if (fd_me[j] < 0) {
							printf("Cannot open file [%s].\n", me_file);
							continue;
						}
					}

					if (save_me_data(fd_me[j], canvasgrp->me_dma_buf_fd[i], j, me_cap, luma, chroma) < 0) {
						printf("Failed to save ME data of buf [%d].\n", j);
						continue;
					}

					if (save[j] == 0) {
						save[j] = 1;
						printf("Save_me_buffer: resolution %dx%d with Luma only.\n",
							me_cap->width, me_cap->height);
					}
				} else {
					pts[j] = me_cap->mono_pts;
					printf("Me_buffer: resolution %dx%d with Luma only,"
						"seq num[%u] PTS [%llu-%llu] idsp pts[%u].\n",
						me_cap->width, me_cap->height, me_cap->seq_num,
						pts[j], (pts[j] - pts_prev[j]), me_cap->dsp_pts);
					pts_prev[j] = pts[j];
				}
			}
		}
		++i;
	}

CAPTURE_MULTIPLE_ME_EXIT:
	if (luma)
		free(luma);
	if (chroma)
		free(chroma);
	return rval;
}

static int save_raw_data(int *fd, int is_raw_ce, struct iav_rawbufdesc *raw_desc)
{
	char raw_file[NAME_SIZE];
	u32 buffer_size;
	int write_size;
	int rval = 0;
	struct iav_gdma_copy gdma_copy = {0};

	if (*fd < 0) {
		/* create fd */
		memset(raw_file, 0, sizeof(raw_file));
		if (is_raw_ce) {
			sprintf(raw_file, "%s_raw_ce_%dx%d_%d", filename, raw_desc->ce_width,
				raw_desc->height, raw_desc->ce_pitch);
		} else {
			sprintf(raw_file, "%s_raw_%dx%d_%d", filename, raw_desc->width,
				raw_desc->height, raw_desc->pitch);
		}

		if (raw_desc->packed) {
			sprintf(raw_file + strlen(raw_file), "_pck_%dbits",
				raw_desc->bits_per_pixel);
		}

		if (raw_desc->raw_compressed) {
			sprintf(raw_file + strlen(raw_file), "_cpr");
		}

		sprintf(raw_file + strlen(raw_file), ".raw");

		*fd = amba_transfer_open(raw_file, transfer_method, port++);
		if (*fd < 0) {
			printf("Cannot open file [%s].\n", raw_file);
			rval = -1;
			goto SAVE_RAW_DATA_EXIT;
		}
	}

	if (is_raw_ce) {
		buffer_size = (raw_desc->ce_width * raw_desc->height) << 1;
	} else {
		buffer_size = (raw_desc->width * raw_desc->height) << 1;
	}

	if (gdma_dst_buf_fd < 0) {
		if (alloc_gdma_dst_buf(buffer_size) < 0) {
			rval = -1;
			goto SAVE_RAW_DATA_EXIT;
		}
	}
	gdma_copy.src_skip_cache_sync = 1;
	gdma_copy.dst_skip_cache_sync = 1;

	if (is_raw_ce) {
		gdma_copy.src_offset = raw_desc->ce_addr_offset;
		gdma_copy.dst_offset = 0;
		gdma_copy.src_pitch = raw_desc->ce_pitch;
		if (raw_desc->format == IAV_RAW_FORMAT_YUV422) {
			gdma_copy.height = raw_desc->height << 1;
			gdma_copy.dst_pitch = raw_desc->ce_width;
			gdma_copy.width = raw_desc->ce_width;
		} else {
			gdma_copy.height = raw_desc->height;
			gdma_copy.dst_pitch = raw_desc->ce_width << 1;
			gdma_copy.width = raw_desc->ce_width << 1;
		}
	} else {
		gdma_copy.src_offset = raw_desc->raw_addr_offset;
		gdma_copy.dst_offset = 0;
		gdma_copy.src_pitch = raw_desc->pitch;
		if (raw_desc->format == IAV_RAW_FORMAT_YUV422) {
			gdma_copy.height = raw_desc->height << 1;
			gdma_copy.dst_pitch = raw_desc->width;
			gdma_copy.width = raw_desc->width;
		} else {
			gdma_copy.height = raw_desc->height;
			gdma_copy.dst_pitch = raw_desc->width << 1;
			gdma_copy.width = raw_desc->width << 1;
		}
	}

	gdma_copy.src_mmap_type = IAV_PART_DSP;
	gdma_copy.dst_dma_buf_fd = gdma_dst_buf_fd;
	gdma_copy.dst_use_dma_buf_fd = 1;
	if (ioctl(fd_iav, IAV_IOC_GDMA_COPY, &gdma_copy) < 0) {
		perror("IAV_IOC_GDMA_COPY");
		rval = -1;
		goto SAVE_RAW_DATA_EXIT;
	}

	write_size = amba_transfer_write(*fd, gdma_dst_buf, buffer_size, transfer_method);
	if (write_size < 0) {
		perror("Failed to save RAW data into file !\n");
		rval = -1;
		goto SAVE_RAW_DATA_EXIT;
	}

	if (write_size != buffer_size) {
		printf("File size is %d. But write size is %d. Maybe no enough space!\n", buffer_size, write_size);
	}

SAVE_RAW_DATA_EXIT:
	if (gdma_dst_buf_fd >= 0) {
		free_gdma_dst_buf();
	}

	return rval;
}

static int process_raw_data(int save_flag, struct iav_rawbufdesc *raw_desc)
{
	int rval = 0;

	if (!raw_desc->pitch || !raw_desc->height || !raw_desc->width) {
		printf("Raw data resolution %ux%u with pitch %u is incorrect!\n",
			raw_desc->width, raw_desc->height, raw_desc->pitch);
		rval = -1;
		goto PROCESS_RAW_EXIT;
	}

	if (capture_raw_ce) {
		if (!raw_desc->ce_pitch || !raw_desc->height || !raw_desc->ce_width) {
			printf("Contrast enhance raw data resolution %ux%u with pitch %u is incorrect!\n",
				raw_desc->ce_width, raw_desc->height, raw_desc->ce_pitch);
			rval = -1;
			goto PROCESS_RAW_EXIT;
		}
	}

	if (save_flag) {
		if (save_raw_data(&fd_raw, 0, raw_desc) < 0) {
			rval = -1;
			goto PROCESS_RAW_EXIT;
		}
		if (capture_raw_ce) {
			if (save_raw_data(&fd_raw_ce, 1, raw_desc) < 0) {
				rval = -1;
				goto PROCESS_RAW_EXIT;
			}
		}
	} else {
		pts = raw_desc->mono_pts;
		printf("Raw data resolution %u x %u with pitch %u, PTS [%u-%u]..\n",
			raw_desc->width, raw_desc->height, raw_desc->pitch, pts, (pts - pts_prev));
		if (capture_raw_ce) {
			printf("Contrast enhance raw data resolution %u x %u with pitch %u, PTS [%u-%u]..\n",
				raw_desc->ce_width, raw_desc->height, raw_desc->ce_pitch, pts, (pts - pts_prev));
		}
		pts_prev = pts;
	}

PROCESS_RAW_EXIT:
	return rval;
}

static int capture_raw(int count, int save_flag)
{
	struct iav_rawbufdesc *raw_desc;
	struct iav_querydesc query_desc;
	int rval = 0;

	if (count <= 0) {
		count = 1;
	}

	while (count > 0) {
		memset(&query_desc, 0, sizeof(query_desc));
		query_desc.qid = IAV_DESC_RAW;
		raw_desc = &query_desc.arg.raw;
		raw_desc->vin_id = vinc_id;
		raw_desc->skip_cache_sync = 1;
		if (ioctl(fd_iav, IAV_IOC_QUERY_DESC, &query_desc) < 0) {
			if (errno == EINTR) {
				// skip to do nothing
			} else {
				perror("IAV_IOC_QUERY_DESC");
				rval = -1;
				break;
			}
		}

		rval = process_raw_data(save_flag, raw_desc);
		if (rval < 0) {
			break;
		}
		count--;
	}

	if (rval >= 0) {
		printf("save raw buffer done!\n");
	}

	if (fd_raw_ce >= 0) {
		amba_transfer_close(fd_raw_ce, transfer_method);
		fd_raw_ce = -1;
	}
	if (fd_raw >= 0) {
		amba_transfer_close(fd_raw, transfer_method);
		fd_raw = -1;
	}
	return rval;
}

static int check_state(void)
{
	int state;
	if (ioctl(fd_iav, IAV_IOC_GET_IAV_STATE, &state) < 0) {
		perror("IAV_IOC_GET_IAV_STATE");
		exit(2);
	}

	if ((state != IAV_STATE_PREVIEW) && (state != IAV_STATE_ENCODING) &&
		(state != IAV_STATE_DECODING)) {
		printf("IAV is not in preview / encoding /decoding state, cannot get yuv buf!\n");
		return -1;
	}

	if (state == IAV_STATE_DECODING) {
		decode_mode = 1;
	}

	return 0;
}

static int get_resource_info(void)
{
	struct iav_system_resource resource;
	struct iav_pyramid_cfg pyramid_cfg;
	u8 i, frame_rate;

	// system resource
	memset(&resource, 0, sizeof(struct iav_system_resource));
	resource.encode_mode = DSP_CURRENT_MODE;
	if (ioctl(fd_iav, IAV_IOC_GET_SYSTEM_RESOURCE, &resource) < 0) {
		perror("IAV_IOC_GET_SYSTEM_RESOURCE\n");
		return -1;
	}

	G_multi_vin_num = resource.chan_num;
	G_canvas_num = resource.canvas_num;

	for (i = 0; i < IAV_MAX_CANVAS_BUF_NUM; i++) {
		frame_rate = resource.canvas_cfg[i].frame_rate;
		if (frame_rate != 0) {
			pts_intval[i] = HWTIMER_OUTPUT_FREQ / frame_rate;
		}
		canvas_mf_enable[i] = resource.canvas_cfg[i].manual_feed;
		canvas_yuv_buffer_disable[i] = resource.canvas_cfg[i].disable_yuv_dram;
	}

	if (current_channel < G_multi_vin_num) {
		memset(&pyramid_cfg, 0, sizeof(struct iav_pyramid_cfg));
		pyramid_cfg.chan_id = current_channel;
		if (ioctl(fd_iav, IAV_IOC_GET_PYRAMID_CFG, &pyramid_cfg) < 0) {
			perror("IAV_IOC_GET_PYRAMID_CFG\n");
			return -1;
		}
		pyramid_manual_feed = pyramid_cfg.manual_feed;
	} else {
		printf("The channel_id[%d] cannot excess channel_num[%d]\n",
			current_channel, G_multi_vin_num);
		return -1;
	}

	return 0;
}

static int get_yuv_data(struct iav_yuv_cap *yuv_cap, u8* buffer)
{
	static int pts_prev = 0, pts = 0;
	int luma_size, chroma_size;
	u8 * luma_addr = NULL, *chroma_addr = NULL, *chroma_convert_addr = NULL;
	int ret = 0;

	/* for luma */
	luma_size = yuv_cap->width * yuv_cap->height;
	luma_addr = gdma_dst_buf;

	/* for chroma */
	chroma_addr = luma_addr + luma_size;
	chroma_size = luma_size >> 1;
	chroma_convert_addr = chroma_addr + chroma_size;

	/* Save YUV data into memory */
	if (verbose) {
		gettimeofday(&curr, NULL);
		pre = curr;
	}

	/* Save UV data into another memory if it needs convert. */
	if (convert_chroma_format(0, chroma_addr, chroma_convert_addr, yuv_cap) < 0) {
		LOG(ERROR) << "Failed to save chroma data into buffer!";
		ret = -1;
		goto SAVE_YUV_DATA_EXIT;
	}
	chroma_addr = chroma_convert_addr;

	if (verbose) {
		gettimeofday(&curr, NULL);
		printf("33. Save UV [%06ld us].\n", (curr.tv_sec - pre.tv_sec) *
			1000000 + (curr.tv_usec - pre.tv_usec));
	}

//	printf("luma_addr:0x%x, luma_size:%d, chroma_addr:0x%x, chroma_size:%d\n", luma_addr, luma_size, chroma_addr, chroma_size);
	if (verbose) {
        gettimeofday(&curr, NULL);
        pre = curr;
    }
	memcpy(buffer, luma_addr, luma_size);
	memcpy(buffer + luma_size, chroma_addr, chroma_size);
	if (verbose) {
        gettimeofday(&curr, NULL);
        printf("44. Copy yuv [%06ld us].\n", (curr.tv_sec - pre.tv_sec) *
            1000000 + (curr.tv_usec - pre.tv_usec));
    }

	if (verbose) {
		pts = yuv_cap->mono_pts;
		printf("BUF Y 0x%08x, UV 0x%08x, pitch %u, %ux%u = Seqnum[%u], "
			"PTS [%u-%u].\n", (u32)yuv_cap->y_addr_offset,
			(u32)yuv_cap->uv_addr_offset, yuv_cap->pitch, yuv_cap->width,
			yuv_cap->height, yuv_cap->seq_num, pts, (pts - pts_prev));
		pts_prev = pts;
	}

SAVE_YUV_DATA_EXIT:
	return ret;
}

static int capture_yuv_data(int buffer_id, u8* buffer)
{
	int i = 0;
	int yuv_buffer_size = 0;
	struct iav_querydesc query_desc;
	struct iav_canvasdesc *canvas;
	struct iav_yuv_cap *yuv_cap;
	int rval = 0;

		/* query */
	memset(&query_desc, 0, sizeof(query_desc));

	query_desc.qid = IAV_DESC_CANVAS;
	canvas = &query_desc.arg.canvas;
	canvas->canvas_id = buffer_id;
	canvas->query_extra_raw = 0;
	canvas->yuv_use_dma_buf_fd = 0;
	canvas->me_use_dma_buf_fd = 0;
	canvas->skip_cache_sync = 1;
	canvas->non_block_flag &= ~IAV_BUFCAP_NONBLOCK;

	if (verbose) {
		gettimeofday(&curr, NULL);
		pre = curr;
	}
	if (ioctl(fd_iav, IAV_IOC_QUERY_DESC, &query_desc) < 0) {
		LOG(ERROR) << "IAV_IOC_QUERY_DESC error";
		return -1;
	}

	if (verbose) {
		gettimeofday(&curr, NULL);
		printf("11. Query DESC [%06ld us].\n", 1000000 *
			(curr.tv_sec - pre.tv_sec)	+ (curr.tv_usec - pre.tv_usec));
	}

	/* gdma copy allocate */
	if (gdma_dst_buf_fd < 0) {
		yuv_cap = &canvas->yuv;
		yuv_buffer_size = get_yuv_buffer_size(yuv_cap,
					get_yuv_format(yuv_format, yuv_cap));

		if (yuv_buffer_size == 0) {
			LOG(ERROR) << "buffer size need allocate is 0!";
			return -1;
		}
		if (verbose) {
			gettimeofday(&curr, NULL);
			pre = curr;
	    }

		if (alloc_gdma_dst_buf(yuv_buffer_size)) {
			return -1;
		}
		if (verbose) {
			gettimeofday(&curr, NULL);
			printf("alloc gdma [%06ld us].\n", 1000000 *
					(curr.tv_sec - pre.tv_sec)  + (curr.tv_usec - pre.tv_usec));
		}
	}

	yuv_cap = &canvas->yuv;

	if (verbose) {
		gettimeofday(&curr, NULL);
		pre = curr;
	}

	if ((yuv_cap->y_addr_offset == 0) ||
		(yuv_cap->uv_addr_offset == 0)) {
		LOG(ERROR) << "YUV buffer address is NULL! Skip to next!";
		return -1;
	}

	rval = copy_yuv_data(canvas->yuv_dma_buf_fd, &canvas->yuv, 0, 0);
	if (canvas->yuv_dma_buf_fd) {
		close(canvas->yuv_dma_buf_fd);
		canvas->yuv_dma_buf_fd = 0;
	}
	if (rval < 0) {
		LOG(ERROR) << "Failed to copy yuv data of buf " << buffer;
		return -1;
	}

	if (verbose) {
		gettimeofday(&curr, NULL);
		printf("12. GDMA copy [%06ld us].\n", 1000000 *
			(curr.tv_sec - pre.tv_sec) + (curr.tv_usec - pre.tv_usec));
	}
	if (get_yuv_data(yuv_cap, buffer) < 0) {
		LOG(ERROR) << "Failed to save YUV data of buf " << buffer;
		return -1;
	}

	if (gdma_dst_buf_fd >= 0) {
		free_gdma_dst_buf();
	}

	return rval;
}

// static int put_image_buffer(cv::Mat &image_mat)
// {
// 	int rval = 0;
// 	pthread_mutex_lock(&image_buffer.lock);  
//     if ((image_buffer.writepos + 1) % IMAGE_BUFFER_SIZE == image_buffer.readpos)  
//     {  
//         pthread_cond_wait(&image_buffer.notfull, &image_buffer.lock);  
//     }
// 	image_buffer.buffer[image_buffer.writepos] = image_mat;
//     image_buffer.writepos++;  
//     if (image_buffer.writepos >= IMAGE_BUFFER_SIZE)  
//         image_buffer.writepos = 0;  
//     pthread_cond_signal(&image_buffer.notempty);  
//     pthread_mutex_unlock(&image_buffer.lock); 
// 	// LOG(INFO) << "put src image";
// 	return rval;
// }

static void *run_camera_pthread(void* data)
{
	// unsigned long time_start, time_end;
	int buffer_id = 3;
	prctl(PR_SET_NAME, "camera_pthread");
	while(run_camera) {
		pthread_mutex_lock(&image_buffer.lock);  
		if ((image_buffer.writepos + 1) % IMAGE_BUFFER_SIZE == image_buffer.readpos)  
		{  
			pthread_cond_wait(&image_buffer.notfull, &image_buffer.lock);  
		}
		memset(image_buffer.buffer[image_buffer.writepos], 0, IMAGE_YUV_SIZE * sizeof(u8));
		if(capture_yuv_data(buffer_id, image_buffer.buffer[image_buffer.writepos]) < 0)
		{
			LOG(ERROR) << "capture yuv data fail!";
		}
		else
		{
			image_buffer.writepos++;
		}
		if (image_buffer.writepos >= IMAGE_BUFFER_SIZE)  
			image_buffer.writepos = 0;  
		pthread_cond_signal(&image_buffer.notempty);  
		pthread_mutex_unlock(&image_buffer.lock); 
	}
	run_camera = 0;
	LOG(WARNING) << "Camera thread quit.";
	return NULL;
}

ImageAcquisition::ImageAcquisition()
{
	run_camera = 0;
	pthread_id = 0;
	LOG(WARNING) << IMAGE_BUFFER_SIZE;
}

ImageAcquisition::~ImageAcquisition()
{
	if(run_camera > 0)
	{
		stop();
	}
	pthread_mutex_destroy(&image_buffer.lock);
    pthread_cond_destroy(&image_buffer.notempty);
    pthread_cond_destroy(&image_buffer.notfull);
    quit_capture = 1;
	if(fd_iav >= 0)
	{
		close(fd_iav);
	}
	LOG(WARNING) << "~ImageAcquisition()";
}

int ImageAcquisition::open_camera()
{
	u32 buffer_map = 0;
    // open the device
	if ((fd_iav = open("/dev/iav", O_RDWR, 0)) < 0) 
    {
        LOG(ERROR) << "open /dev/iav fail!";
		return -1;
	}
    //check iav state
	if (check_state() < 0)
    {
        LOG(ERROR) << "check_state fail!";
        return -1;
    }
    if (map_buffer() < 0)
    {
        LOG(ERROR) << "map_buffer fail!";
        return -1;
    }
	for (int i = 0; i < IAV_MAX_CANVAS_BUF_NUM; ++i) {
		fd_yuv[i] = -1;
		fd_me[i] = -1;
	}
	for (int i = 0; i < IAV_MAX_PYRAMID_LAYERS; ++i) {
		fd_pyramid[i] = -1;
	}

	current_buffer = 2;
	VERIFY_BUFFERID(current_buffer);
	capture_select = CAPTURE_PREVIEW_BUFFER;
	yuv_buffer_map |= (1 << current_buffer);
	strcpy(filename, "imx327");
	yuv_format = YUV420_IYUV;

	if (capture_select != CAPTURE_RAW_BUFFER && capture_select != CAPTURE_PYRAMID_BUFFER) {
		if (buffer_map) {
			if (yuv_buffer_map || me0_buffer_map || me1_buffer_map) {
				LOG(ERROR) << "cannot query canvas group and single YUV/ME at the same time.";
				return -1;
			}

			if (current_buffer >= 0) {
				LOG(ERROR) << "select canvas cannot use with select group canvas.";
				return -1;
			}

			if (delay_frame_cap_data) {
				LOG(ERROR) << "In querying canvas group mode, option '-d' is not supported.";
				return -1;
			}

			switch (capture_select) {
			case CAPTURE_PREVIEW_BUFFER:
				yuv_buffer_map = buffer_map;
				break;
			case CAPTURE_ME0_BUFFER:
				me0_buffer_map = buffer_map;
				break;
			case CAPTURE_ME1_BUFFER:
				me1_buffer_map = buffer_map;
				break;
			default:
				break;
			}
			canvas_map_thru_dmabuf &= buffer_map;
		} else if (current_buffer < 0) {
			LOG(ERROR) << "Invalid buffer id.";
			return -1;
		} else {
			/* do nothing */
		}
	}

	pthread_mutex_init(&image_buffer.lock, NULL);  
    pthread_cond_init(&image_buffer.notempty, NULL);  
    pthread_cond_init(&image_buffer.notfull, NULL);  
    image_buffer.readpos = 0;  
    image_buffer.writepos = 0;
	LOG(INFO) << "Camera open success!";
    return 0;
}

int ImageAcquisition::start()
{
    int ret = 0;
    run_camera = 1;
	image_buffer.readpos = 0;  
    image_buffer.writepos = 0;
	pthread_attr_t attr; 
    pthread_attr_init( &attr ); 
    pthread_attr_setdetachstate(&attr,1); 
    ret = pthread_create(&pthread_id, &attr, run_camera_pthread, NULL);
	if(ret < 0)
    {
        run_camera = 0;
        LOG(ERROR) << "satrt camera pthread fail!";
    }
	LOG(INFO) << "satrt camera pthread success!";
	return ret;
}

int ImageAcquisition::stop()
{
	int ret = 0;
	run_camera = 0;
	if (pthread_id > 0) {
		pthread_cond_signal(&image_buffer.notfull);
		pthread_cond_signal(&image_buffer.notempty);  
		pthread_mutex_unlock(&image_buffer.lock);
		pthread_join(pthread_id, NULL);
		pthread_id = 0;
	}
	LOG(WARNING) << "stop camera success";
	return ret;
}

void ImageAcquisition::get_image(cv::Mat &src_image)
{
	if(pthread_id > 0)
	{
		pthread_mutex_lock(&image_buffer.lock);  
		if (image_buffer.writepos == image_buffer.readpos)  
		{  
			pthread_cond_wait(&image_buffer.notempty, &image_buffer.lock);  
		}
		cv::Mat yuvImg(IMAGE_HEIGHT + IMAGE_HEIGHT / 2, IMAGE_WIDTH, CV_8UC1, image_buffer.buffer[image_buffer.readpos]);
		cv::cvtColor(yuvImg, src_image, cv::COLOR_YUV2BGR_IYUV);
		image_buffer.readpos++;  
		if (image_buffer.readpos >= IMAGE_BUFFER_SIZE)  
			image_buffer.readpos = 0; 
		pthread_cond_signal(&image_buffer.notfull);  
		pthread_mutex_unlock(&image_buffer.lock);
	}
}

void ImageAcquisition::get_yuv(unsigned char* addr)
{
	if(pthread_id > 0)
	{
		pthread_mutex_lock(&image_buffer.lock);  
		if (image_buffer.writepos == image_buffer.readpos)  
		{  
			pthread_cond_wait(&image_buffer.notempty, &image_buffer.lock);  
		}
		memcpy(addr, image_buffer.buffer[image_buffer.readpos], IMAGE_YUV_SIZE * sizeof(unsigned char));
		image_buffer.readpos++;  
		if (image_buffer.readpos >= IMAGE_BUFFER_SIZE)  
			image_buffer.readpos = 0; 
		pthread_cond_signal(&image_buffer.notfull);  
		pthread_mutex_unlock(&image_buffer.lock);
	}
}