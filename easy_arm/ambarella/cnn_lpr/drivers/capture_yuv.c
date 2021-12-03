#include "capture_yuv.h"
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

#include <iav_ioctl.h>
#include <signal.h>

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

static int fd_iav;

static int yuv_format = YUV420_IYUV;

static int verbose = 0;

static u8* dsp_mem = NULL;
static u32 dsp_size = 0;

static u8* gdma_dst_buf = NULL;
static u32 gdma_dst_buf_size = 0;
static int gdma_dst_buf_fd = -1;

static struct timeval pre = {0, 0}, curr = {0, 0};

extern void chrome_convert(yuv_neon_arg *);

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

static int convert_chroma_format(u8* input, u8* output,
	struct iav_yuv_cap *yuv_cap)
{
	int width, height, pitch;
	u8 *uv_addr = NULL;
	yuv_neon_arg yuv;
	int ret = 0;

	if (input == NULL || output == NULL) {
		printf("Invalid pointer NULL!");
		return -1;
	}

	uv_addr = input;
	width = yuv_cap->width;
	height = yuv_cap->height;
	pitch = yuv_cap->width;

	yuv.in = uv_addr;
	yuv.row = height / 2 ;
	yuv.col = width;
	yuv.pitch = pitch;

	// IYUV (I420) format (YYYYYYYYUUVV)
	yuv.u = output;
	yuv.v = output + width * height / 4;
	chrome_convert(&yuv);

	return ret;
}

static int save_yuv_data(struct iav_yuv_cap *yuv_cap, u8* buffer)
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
	if (convert_chroma_format(chroma_addr, chroma_convert_addr, yuv_cap) < 0) {
		perror("Failed to save chroma data into buffer !\n");
		ret = -1;
		goto SAVE_YUV_DATA_EXIT;
	}
	chroma_addr = chroma_convert_addr;

	if (verbose) {
		gettimeofday(&curr, NULL);
		printf("33. Save UV [%06ld us].\n", (curr.tv_sec - pre.tv_sec) *
			1000000 + (curr.tv_usec - pre.tv_usec));
	}

	/* Write YUV data from memory to file */
#if 0
	if (write(fd, luma_addr, luma_size) < 0) {
		perror("Failed to save luma data into file !\n");
		ret = -1;
		goto SAVE_YUV_DATA_EXIT;
	}
	if (write(fd, chroma_addr, chroma_size) < 0) {
		perror("Failed to save chroma data into file !\n");
		ret = -1;
		goto SAVE_YUV_DATA_EXIT;
	}
#else 
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

#endif
	if (1) {
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

static int copy_yuv_data(int dmabuf_fd, struct iav_yuv_cap *yuv_cap)
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
		gdma_copy.src_mmap_type = IAV_PART_DSP;
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
static int get_yuv_buffer_size(struct iav_yuv_cap *yuv_cap)
{
	/* layout: luma + chroma + convert_chroma(if yuv type needs to convert) */
	int luma_size;
	int total_size;

	luma_size = yuv_cap->pitch * ROUND_UP(yuv_cap->height, 16);
	total_size = luma_size * 2;

	total_size = ROUND_UP(total_size, 16);

	return total_size;
}

int capture_yuv(int buffer_id, unsigned char* buffer)
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
		perror("IAV_IOC_QUERY_DESC");
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
		yuv_buffer_size = get_yuv_buffer_size(yuv_cap);

		if (yuv_buffer_size == 0) {
			printf("buffer size need allocate is 0!.\n");
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
		printf("YUV buffer [%d] address is NULL! Skip to next!\n", buffer);
		return -1;
	}

	rval = copy_yuv_data(canvas->yuv_dma_buf_fd, &canvas->yuv);
	if (canvas->yuv_dma_buf_fd) {
		close(canvas->yuv_dma_buf_fd);
		canvas->yuv_dma_buf_fd = 0;
	}
	if (rval < 0) {
		printf("Failed to copy yuv data of buf [%d].\n", buffer);
		return -1;
	}

	if (verbose) {
		gettimeofday(&curr, NULL);
		printf("12. GDMA copy [%06ld us].\n", 1000000 *
			(curr.tv_sec - pre.tv_sec) + (curr.tv_usec - pre.tv_usec));
	}
	if (save_yuv_data(yuv_cap, buffer) < 0) {
		printf("Failed to save YUV data of buf [%d].\n", buffer);
		return -1;
	}

	if (gdma_dst_buf_fd >= 0) {
		free_gdma_dst_buf();
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

	return 0;
}

int capture_yuv_init(void)
{
	//open the device
	if ((fd_iav = open("/dev/iav", O_RDWR, 0)) < 0) {
		perror("/dev/iav");
		return -1;
	}

	if (check_state() < 0)
		return -1;

	if (map_dsp_buffer() < 0)
		return -1;

	return 0;
}

int capture_yuv_close(void)
{
	if(fd_iav >= 0)
	{
		close(fd_iav);
	}
	return 0;
}
