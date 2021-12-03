#include "tof_316_acquisition.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <assert.h>
#include <time.h>
#include <iav_ioctl.h>
#include <getopt.h>
#include <sched.h>
#include <sys/stat.h>
#include <basetypes.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <iostream>
#include <fstream>
#include <arm_neon.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <string.h>
#include <dlfcn.h>
#include <arpa/inet.h>
#include "tof_if.h"
#include "amba_cvwarp.h"
#include "lib_smartfb.h"
#include "fmt_conv.h"
#include "p2dio.hpp"
#include "PhaseData.hpp"
#include "p2dcore.hpp"
#include "rc4.h"
#include <linux/spi/spidev.h>

#define PHASE_OFFSET_456	(96)
#define PHASE_OFFSET_316	(66)
#define FREQ_OFFSET_316 	(82)
#define TMP_OFFSET			(78)
#define DUMMY_CHAN			(0)
#define PHASE_NUM			(8)
#define SINGLE_PHASE_NUM 	(4)
#define LIGHT_V 			(300000000)
#define PAI					(3.14159265)
#define MAX_COLOR_NUM		(360)
#define NEON_WIDTH			(8)
#define MAX_FB_INDEX		(256)
#define CV_SPACE_NUM		(2)
#define MAX_BUFFER_NUM		(10)
#define SHUTTER_1BY100000	(5120)		/* (512000000 / 100000) */
#define SHUTTER_1BY1000		(512000)	/* (1000us for experiments) */
#define IMX316_INT_A		(0x2110)	/* integration_time A [31:0] */
#define IMX316_INT_B		(0x2114)	/* integration_time B [31:0] */
#define WARP_FIX16_BIN			"/usr/local/bin/warp_cavalry_2_2_in240x180_fx16.bin"
#define WARP_FIX8_BIN			"/usr/local/bin/warp_cavalry_2_2_in240x180_fx8.bin"
#define WARP_640x480_FIX16_BIN 	"/usr/local/bin/warp_cavalry_2_2_in640x480_fx16.bin"
#define WARP_640x480_FIX8_BIN 	"/usr/local/bin/warp_cavalry_2_2_in640x480_fx8.bin"
#define CV_WARP_PLUGIN			(char*)"/usr/lib/amba_cvwarp.so"
#define K_DATA					(char*)"/usr/local/bin/imx316_calib.bin"
#define EXTRA_LINES			(1)
#define MAX_DIST_316		(7.5f)
#define MAX_DIST_456		(7.5f)
#define HEADER_SECTOR_SIZE 	(4096)
#define CALIB_FILE_SIZE 	(0x280000)	//2.5MB
#define SPI_OFFSET 			(86)
#define SPI_LEN 			(90)

#ifndef CHECK_LIMIT
	#define CHECK_LIMIT(x, low, high) MAX((low), MIN((x), (high)))
#endif

#ifndef AM_IOCTL
#define AM_IOCTL(_filp, _cmd, _arg)	\
	do { 						\
		if (ioctl(_filp, _cmd, _arg) < 0) {	\
			perror(#_cmd);		\
			return -1;			\
		}						\
	} while (0)
#endif

typedef struct osd_clut_s {
	u8 v;
	u8 u;
	u8 y;
	u8 alpha;
} osd_clut_t;

typedef struct osd_info_s {
	int enable;
	u32 buf_data[MAX_BUFFER_NUM];
} osd_info_t;

typedef struct resolution {
	int width;
	int height;
	int pitch;
	int raw_pitch;
	int size;
	int depth_pitch;
	int amp_pitch;
	int depth_size;
	int amp_size;
} reso;

typedef struct fb_clut_t {
	u16 r_table[256];
	u16 g_table[256];
	u16 b_table[256];
	u16 trans_table[256];
} fb_clut;

struct sensor_data {
	int temperature;
	int phase_index;
	int freq_id;
};

typedef struct tof_mem_ptr_t {
	u16 *p_depth;
	u8 *calib_data;
	u8 *fb_mem;
	u8 *gdma_dst_buf;
	float *p_depth_float;
	s16 *pAminusB1;
	s16 *pAminusB2;
	s16 *pAminusB3;
	s16 *pAminusB4;
	s16 *pAminusB1f2;
	s16 *pAminusB2f2;
	s16 *pAminusB3f2;
	s16 *pAminusB4f2;
	float *pXmap;
	float *pYmap;
	float *pZmap;
	float *pPhasemap;
	myPhaseData *phaData;
	myPhaseData *phaData2;
	myPhaseData *targetData;
	void *ctx_float2fixed;
	void *ctx_fixed2float;
	void *fixedPhase;
	u8 *pConfidence;
	void *warp_handle;
	amba_cvwarp_api *g_cvwarp;
} tof_mem_ptr;

typedef struct arg_t {
	int start;
	int end;
	int pitch;
	int width;
	int height;
	u16 *addr;
	sem_t notice_sem;
	sem_t finish_sem;
} ARG;

typedef struct{
	int target_confidence;
	int target_tolerant_upper;
	int target_tolerant_lower;
	u32 adj_ratio_inc;//unit =16
	u32 adj_ratio_dec;//unit =16
	u32 max_expo_time;//Q9
	u32 min_expo_time;//Q9
	u32 init_expo_time;
	u32 skip_frame;
	u32 reversed[7];
} tof_ae_config_t;

enum numeric_debug_options {
	DEBUG_TIME = 0,
	DEBUG_RAW_DATA = 1,
	MIN_DEBUG_NUM = DEBUG_TIME,
	MAX_DEBUG_NUM = DEBUG_RAW_DATA,
};

enum cv_extra_lines {
	EXTRA_LINES_1 = 1,
	EXTRA_LINES_3 = 3,
	EXTRA_LINES_7 = 7,
};

enum sensor_type {
	SENSOR_IMX456 = 0,
	SENSOR_IMX316 = 1,
	SENSOR_MAX_NUM = SENSOR_IMX316,
};

enum sensor_wid_hei {
	WIDTH_456 = 640,
	WIDTH_316 = 240,
	HEIGHT_456 = 480,
	HEIGHT_316 = 180,
};

static char file_prefix[MAX_STRING_LENGTH] = "/root";
static u32 gdma_dst_buf_pid = 0;
static u32 gdma_dst_buf_size = 0;
static int fd_iav = -1;
static u8 *dsp_mem = NULL;
static int debug = -1;
static int speed_flag = 1;
static int clut_start = 0;
static int clut_end = MAX_FB_INDEX;
static int clut_ratio = 1;
static int color_num = MAX_COLOR_NUM;
static u8 *content = NULL;
static int verbose = 0;
static int spi_read = 0;
static int sensor_type = SENSOR_IMX316;
static tof_mem_ptr tof_mem;
static struct warp_ctrl g_LDCinfo;
static int nConfidenceOffset = 0;
static tof_ae_config_t tof_ae_cfg;
static int do_acceleration = 0;
static int single_mode = 0;
static int phase_number = SINGLE_PHASE_NUM;
static int thConfidence = 1; 
static int auto_exposure_flag = 0;
static int init_exposure_time = SHUTTER_1BY1000;
static int trigger_flag = 1;

#define SPI_COMMAND_LENGTH 	(4)
#define SPI_DATA_LENGTH 	(8)
#define SPI_MAX_LENGTH 		(1024)
#define SPI_VALID_LENGTH 	(SPI_MAX_LENGTH - SPI_COMMAND_LENGTH)

static int fd_spi = -1;
static const char *device = "/dev/spidev1.0";

static struct TOFBuffer tof_buffer;  

volatile int run_tof = 0;
volatile int has_sleep = 1;

static int read_calibration_data(u8 *calib_addr, struct calibration_info *align_info)
{
	int ret = 0;
	int i;
	int bits = 8, speed = 1 * 1000 * 1000;
	int batch_num = 0, remain_num = 0;
	u8 command[SPI_COMMAND_LENGTH] = {0x03, 0x00, 0x00, 0x00};
	u32 addr = 0x03080000;
	u32 t_adr = 0;
	u8 spi_data[SPI_MAX_LENGTH] = {0};
	struct spi_ioc_transfer tr;

	do {
		if (fd_spi < 0) {
			fd_spi = open(device, O_RDWR);
			if (fd_spi < 0) {
				perror("open SPI fail\n");
				ret = -1;
				break;
			}
		}
	
		if (calib_addr == nullptr) {
			fprintf(stderr, "Invalid argument: calib_addr is NULL\n");
			ret = -1;
			break;
		}
		if (align_info == nullptr) {
			fprintf(stderr, "Invalid argument: align_info is NULL\n");
			ret = -1;
			break;
		}
		addr = addr + align_info->offset;
		memset(&tr, 0, sizeof(tr));
		tr.delay_usecs = 0;
		tr.speed_hz = speed;
		tr.bits_per_word = bits;
		batch_num = align_info->length / SPI_VALID_LENGTH;   //batch_num:4096/1024=4
		remain_num = align_info->length - batch_num * SPI_VALID_LENGTH;
		for (i = 0; i < batch_num; ++i) {
			t_adr = ntohl(addr) ;
			memcpy(command, &t_adr, SPI_COMMAND_LENGTH);
			tr.tx_buf = (unsigned long)command;
			tr.rx_buf = (unsigned long)(spi_data);
			tr.len = SPI_MAX_LENGTH;
			ret = ioctl(fd_spi, SPI_IOC_MESSAGE(1), &tr);
			if (ret < 1) {
				perror("can't send spi message");
				ret = -1;
				break;
			}
			addr += SPI_VALID_LENGTH;
			memcpy(calib_addr + i * SPI_VALID_LENGTH,
				spi_data + SPI_COMMAND_LENGTH, SPI_VALID_LENGTH);
		}

		t_adr = ntohl(addr);
		memcpy(command, &t_adr, SPI_COMMAND_LENGTH);
		printf("Spi batch-remain %d-%d\n", batch_num, remain_num);
		printf("Reading remaining addr %x-%x-%x-%x\n",
			command[0], command[1], command[2], command[3]);
		tr.tx_buf = (unsigned long)command;
		tr.rx_buf = (unsigned long)(spi_data);
		tr.len = remain_num + SPI_COMMAND_LENGTH;
		ret = ioctl(fd_spi, SPI_IOC_MESSAGE(1), &tr);
		if (ret < 1) {
			perror("can't send spi message");
			ret = -1;
			break;
		}
		memcpy(calib_addr + batch_num * SPI_VALID_LENGTH,
			spi_data + SPI_COMMAND_LENGTH, remain_num);
	} while(0);
	
	if (fd_spi >= 0) {
		close(fd_spi);
		fd_spi = -1;
	}

	return ret;
}

static int read_spi_calib(amba_cvwarp_api *cvwarp, p2dIO *p2dio)
{
	struct calibration_info cali_info;
	const int header_sector_size = HEADER_SECTOR_SIZE;
	const int calib_file_max_sector_size = CALIB_FILE_SIZE;
	unsigned short version = 0;
	unsigned char header_buffer[header_sector_size];
	unsigned char flash_read_buf[calib_file_max_sector_size];
	header_struct HeaderInfo;
	int ret = 0;
	int kDataVer = p2dio->bin_buffer[0];
#if 0
	int fd;
#endif
	do {
		cali_info.offset = 0;
		cali_info.length = 0;

		memset(&HeaderInfo, 0, sizeof(header_struct));
		memset(header_buffer, 0, sizeof(header_buffer));
		memset(flash_read_buf, 0, sizeof(flash_read_buf));
		//READ HEADER
		cali_info.length = sizeof(header_buffer)/sizeof(header_buffer[0]);
//		(cvwarp->read_cali_data)(header_buffer, &cali_info);
		read_calibration_data(header_buffer, &cali_info);
#if 0
		fd = open("calib.txt", O_CREAT | O_TRUNC | O_RDWR, 0600);
		int size = write(fd, header_buffer, 4096);
		close(fd);
#endif
		version = *(unsigned int*)(header_buffer+0);
		printf("Header version = 0x%02x\n", version);
		cali_info.offset = *(unsigned int*)(header_buffer + SPI_OFFSET);
		cali_info.length = *(unsigned int*)(header_buffer + SPI_LEN);

		printf("cali_info.offset %d\n", cali_info.offset);
		printf("cali_info.length %d\n", cali_info.length);

		if(version == 0x01 || version == 0x02) {
			//retrieve sensor_type
			if(HeaderInfo.sensor_type == NULL) {
				HeaderInfo.sensor_type = new char[SIZEOF_SENSOR_TYPE + 1];
				memset(HeaderInfo.sensor_type, 0, SIZEOF_SENSOR_TYPE + 1);
			}
			memcpy(HeaderInfo.sensor_type, header_buffer + OFFSETOF_SENSOR_TYPE, SIZEOF_SENSOR_TYPE);
			HeaderInfo.encrypt_key = new char[SIZEOF_ENCRYPTION_KEY + 1];
			memset(HeaderInfo.encrypt_key, 0, SIZEOF_SENSOR_TYPE + 1);
			memcpy(HeaderInfo.encrypt_key, header_buffer + OFFSETOF_ENCRYPTION_KEY, SIZEOF_ENCRYPTION_KEY);
			memcpy(&(HeaderInfo.data_size), header_buffer + OFFSETOF_DATA_SIZE, SIZEOF_DATA_SIZE);
			memcpy(&(HeaderInfo.data_offset), header_buffer + OFFSETOF_DATA_OFFSET, SIZEOF_DATA_OFFSET);
		} else {
			printf("Unknown Flash Header Version\n");
			ret = -1;
			break;
		}

#if 1
		RC4 rc4;
		rc4.rc4_input((unsigned char *)HeaderInfo.encrypt_key, SIZEOF_ENCRYPTION_KEY);
#endif
		BYTE *enblock = (BYTE *)p2dio->bin_buffer;
//		(cvwarp->read_cali_data)(enblock, &cali_info);
		read_calibration_data(enblock, &cali_info);
#if 0
		fd = open("calib-data.txt", O_CREAT | O_TRUNC | O_RDWR, 0600);
		size = write(fd, enblock, cali_info.length);
                close(fd);
#endif
#if 0	
		ret = Liteon_UploadCalibration((BYTE *)enblock, HeaderInfo.data_size, FLASH_HEADER_START_ADDR + HeaderInfo.data_offset, Callback);
		//printf("%s:%d retCode = %d\n", __FILE__, __LINE__, ret);
		if (ret != 0){
			return false;
		}
#endif
		//decrypt
#if 1
		if(version == 0x01) {
			for(int x = 0; x < HeaderInfo.data_size; x++) {
				enblock[x] = (BYTE)(((char)(enblock[x])) ^ rc4.rc4_output_v01());
			}
		} else {
			for(int x = 0; x< HeaderInfo.data_size; x++) {
				enblock[x] = (BYTE)(((char)(enblock[x])) ^ rc4.rc4_output());
			}
		}
#endif
		if (HeaderInfo.sensor_type != NULL) {
			delete [] HeaderInfo.sensor_type;
		}
		if (HeaderInfo.encrypt_key != NULL) {
			delete [] HeaderInfo.encrypt_key;
		}


		printf("[KData version = %02d] \n",kDataVer);
		if(kDataVer == KDATA_VER1) {
			printf("kData version is VERSION1...\n");
		} else if(kDataVer == KDATA_VER2) {
			printf("kData version is VERSION2...\n");
		} else if(kDataVer == KDATA_VER3) {
			printf("kData version is VERSION3...\n");
		} else {
			printf("kData version is NoVERSION...\n");
		}

		p2dio->ParsingCalibDatabuffer(p2dio->bin_buffer);
	} while (0);

	return ret;
}

static int prepare_init_dewarp_depth(int width, int height,
	amba_cvwarp_api *cvwarp, struct warp_ctrl *info,
	struct cvwarp_intrinsic_t *intrinsic,
	myCameraParameters *campars, float *xy_map, p2dIO *p2dio)
{
	cvwarp_init_t warp_init;
	char spi_dev[MAX_STRING_LENGTH] = "/dev/spidev1.0";

	memset(&warp_init, 0, sizeof(warp_init));
	warp_init.width = width;
	warp_init.height = height;
	warp_init.space_num = CV_SPACE_NUM;
	warp_init.extra_lines = EXTRA_LINES;
	warp_init.optimize = 0;
	warp_init.verbose = 0;
	warp_init.dev_node = spi_dev;

	// use external LDC map
	warp_init.use_cust_map = 1;
	intrinsic->mapx = xy_map;
	intrinsic->mapy = xy_map + (warp_init.width + warp_init.extra_lines) *
		(warp_init.height + warp_init.extra_lines);
	cvwarp->init(&warp_init);
#if 0
	if (spi_read) {
		read_spi_calib(cvwarp, p2dio);
	}
#endif
	cvwarp->create_vector(intrinsic, NULL);
	cvwarp->configure(info);

	return 0;
}

static int write_file(int fd, u8 *buf, u32 size)
{
	int ret = 0;

	if (buf == NULL) {
		printf("%s(): buffer is NULL\n", __func__);
		ret = -1;
		return ret;
	}
	if (size != (u32)write(fd, buf, size)) {
		printf("%s(): error calling write()\n", __func__);
		ret = -1;
		return ret;
	}

	return ret;
}

static int open_file(const char *path, const int flag, const int mode)
{
	int fd = -1;
	int ret = 0;

	if (path == NULL) {
		printf("%s(): file path %s is NULL\n", __func__, path);
		ret  = -1;
		return ret;
	}
	fd = open(path, flag, mode);
	if (fd < 0) {
		printf("%s(): cannot open file %s\n", __func__, path);
		ret  = -1;
		return ret;
	}
	ret = fd;

	return ret;
}

static int map_dsp_buffer(void)
{
	struct iav_querymem querybuf;
	struct iav_mem_part_info *part_info = NULL;
	int ret = 0;
	u32 dsp_size = 0;

	memset(&querybuf, 0, sizeof(querybuf));
	part_info = &querybuf.arg.partition;
	querybuf.mid = IAV_MEM_PARTITION;
	part_info->pid = IAV_PART_DSP;

	AM_IOCTL(fd_iav, IAV_IOC_QUERY_MEMBLOCK, &querybuf);
	dsp_size = part_info->mem.length;
	dsp_mem = (u8 *)mmap(NULL, dsp_size, PROT_READ | PROT_WRITE, MAP_SHARED,
		fd_iav, part_info->mem.addr);
	if (dsp_mem == MAP_FAILED) {
		printf("mmap dsp_mem (%d) failed\n", errno);
		ret = -1;
	}
	return ret;
}

static int init_sem(sem_t *g_sem)
{
	int ret = 0;
	if (sem_init(g_sem, 0, 0)) {
		perror("sem_init");
		ret = -1;
	}

	return ret;
}

static int deinit_sem(sem_t *g_sem)
{
	int ret = 0;
	if (sem_destroy(g_sem)) {
		perror("sem_close");
		ret = -1;
	}

	return ret;
}

static int alloc_gdma_dst_buf(u32 size, tof_mem_ptr *ptr)
{
	struct iav_alloc_mem_part alloc_mem_part;
	struct iav_querymem query_mem;
	struct iav_mem_part_info *part_info;

	alloc_mem_part.length = size;
	alloc_mem_part.enable_cache = 1;
	AM_IOCTL(fd_iav, IAV_IOC_ALLOC_ANON_MEM_PART, &alloc_mem_part);
	gdma_dst_buf_pid = alloc_mem_part.pid;

	memset(&query_mem, 0, sizeof(query_mem));
	query_mem.mid = IAV_MEM_PARTITION;
	part_info = &query_mem.arg.partition;
	part_info->pid = gdma_dst_buf_pid;

	AM_IOCTL(fd_iav, IAV_IOC_QUERY_MEMBLOCK, &query_mem);
	gdma_dst_buf_size = part_info->mem.length;
	if (gdma_dst_buf_size) {
		ptr->gdma_dst_buf = (u8 *)mmap(NULL, gdma_dst_buf_size,
			PROT_READ | PROT_WRITE, MAP_SHARED,
			fd_iav, part_info->mem.addr);
		if (ptr->gdma_dst_buf == MAP_FAILED) {
			perror("mmap gdma dst buffer failed\n");
			return -1;
		}
	}

	return 0;
}

static int free_gdma_dst_buf(void)
{
	struct iav_free_mem_part free_mem_part;

	if (tof_mem.gdma_dst_buf && gdma_dst_buf_size) {
		munmap(tof_mem.gdma_dst_buf, gdma_dst_buf_size);
	}
	tof_mem.gdma_dst_buf = NULL;
	gdma_dst_buf_size = 0;

	free_mem_part.pid = gdma_dst_buf_pid;
	AM_IOCTL(fd_iav, IAV_IOC_FREE_MEM_PART, &free_mem_part);
	gdma_dst_buf_pid = 0;

	return 0;
}

static int get_phase_id(u8 *embed_addr, int width, struct sensor_data *sensor)
{
	u8 *line0 = embed_addr;
	u8 *line1 = embed_addr + width * 4;

	sensor->temperature = *(line0 + (TMP_OFFSET << 1) + 1) - 40;
	if (sensor_type == SENSOR_IMX316) {
		sensor->phase_index = *(line1 + (PHASE_OFFSET_316 << 1) + 1);
		sensor->freq_id = *(line1 + (FREQ_OFFSET_316 << 1) + 1);
	} else {
		sensor->phase_index = *(line1 + (PHASE_OFFSET_456 << 1) + 1);
	}

	return 0;
}

static int save_raw_data(reso *win)
{
	u32 frame_size = 0, buffer_size = 0;
	char save_path[MAX_STRING_LENGTH] = "/root/raw_data.bin";
	int fd = 0;

	frame_size = win->raw_pitch * win->height;
	buffer_size = PHASE_NUM * frame_size;

	fd = open_file(save_path, O_CREAT | O_WRONLY, S_IREAD | S_IWRITE);
	write_file(fd, tof_mem.gdma_dst_buf, buffer_size);

	exit(1);
	return 0;
}

static int set_vin_reg(int fd_iav, u32 addr, u32 data);
static int get_4_raw_data(struct sensor_data *sensor, reso *win,
	unsigned long *prev_pts0, unsigned long *prev_pts4, int *repeat, int dual_flag)
{
	struct iav_rawseqdesc *rawseq_desc = NULL;
	struct iav_querydesc query_desc;
	struct iav_gdma_copy gdma_copy = {0};
	u32 src_size = 0;
	u32 dst_size = 0;
	int end_shift = 0;
	int phase_id = 0;
	int freq_id = 0;
	int id_offset = 0, freq_offset = 0;
	int start_index = 0;
	int ret = 0;
	int i;

	if (trigger_flag) {
		set_vin_reg(fd_iav, 0x2100, 0x41);
		usleep(70 * 1000);
	}

	do {
		src_size = win->raw_pitch * win->height;
		dst_size = win->raw_pitch * win->height;
		memset(&query_desc, 0, sizeof(query_desc));
		query_desc.qid = IAV_DESC_RAWSEQ;
		rawseq_desc = &query_desc.arg.rawseq;
		rawseq_desc->vin_id = 1;

		if (sensor_type == SENSOR_IMX316) {
			id_offset = src_size + win->raw_pitch + (PHASE_OFFSET_316 << 1) + 1;
			freq_offset = src_size + win->raw_pitch + (FREQ_OFFSET_316 << 1) + 1;
		} else {
			id_offset = src_size + win->raw_pitch + (PHASE_OFFSET_456 << 1) + 1;
		}

		if (ioctl(fd_iav, IAV_IOC_QUERY_DESC, &query_desc) < 0) {
			if (errno == EINTR) {
			} else {
				perror("IAV_IOC_QUERY_DESC");
				ret = -1;
                return ret;
			}
		}

		phase_id = *(dsp_mem + rawseq_desc->seq_addr_offset[0] + id_offset);
		end_shift = (phase_id == 3) ? 0 : (phase_id + 1);
		//get_phase_id((u8 *)(dsp_mem + rawseq_desc->seq_addr_offset[0] + frame_size), width, sensor);
		start_index = end_shift + PHASE_NUM - 1;
		freq_id = *(dsp_mem + rawseq_desc->seq_addr_offset[start_index] + freq_offset);
		//printf("start %ld, prev %ld\n", //rawseq_desc->seq_pts[start_index], *prev_pts);
		if (freq_id == 1 ||
			rawseq_desc->seq_pts[start_index] == *prev_pts0 ||
			rawseq_desc->seq_pts[start_index] == 0 ||
			rawseq_desc->seq_pts[start_index] == *prev_pts4
			) {
			*repeat = 1;
			break;
		}
		*prev_pts4 = rawseq_desc->seq_pts[start_index - 4];
		*prev_pts0 = rawseq_desc->seq_pts[start_index];
		for (i = 0; i < PHASE_NUM; i++) {
			gdma_copy.src_pitch = rawseq_desc->pitch;
			gdma_copy.dst_pitch = rawseq_desc->pitch;
			gdma_copy.width = rawseq_desc->pitch;
			gdma_copy.height = rawseq_desc->height - 2;
			gdma_copy.src_mmap_type = IAV_PART_DSP;
			gdma_copy.dst_mmap_type = gdma_dst_buf_pid;
			gdma_copy.src_offset = rawseq_desc->seq_addr_offset[end_shift + PHASE_NUM - 1 - i];
			gdma_copy.dst_offset = i * dst_size;

			AM_IOCTL(fd_iav, IAV_IOC_GDMA_COPY, &gdma_copy);
		}

		if (debug == DEBUG_RAW_DATA) {
			save_raw_data(win);
		}

		if (debug == DEBUG_TIME) {
			for (i = 0; i < PHASE_NUM; ++i) {
				get_phase_id((u8 *)(dsp_mem +
					rawseq_desc->seq_addr_offset[start_index - i] + src_size),
					win->width, sensor);
				printf("index %d, phase %d, freq%d, addr %ld, pts %ld, pitch %d\n", i,
					sensor->phase_index,
					sensor->freq_id,
					rawseq_desc->seq_addr_offset[start_index - i],
					rawseq_desc->seq_pts[start_index - i], rawseq_desc->pitch);
			}
		}
	} while (0);
	return ret;
}

static int malloc_gdma_buffer(reso *win, tof_mem_ptr *ptr)
{
	int frame_size = 0, buffer_size = 0;
	int ret = 0;

	frame_size = win->raw_pitch * win->height;
	buffer_size = PHASE_NUM * frame_size;
	if (alloc_gdma_dst_buf(buffer_size, ptr) < 0) {
		printf("alloc_gdma_dst_buf error\n");
		ret = -1;
	}

	return ret;
}

static int config_tof(reso *win, tof_option_t *opt, tof_mem_ptr *ptr)
{
	int ret = 0;

	win->pitch = win->width;
	win->raw_pitch = 4 * win->width;
	win->amp_pitch = ROUND_UP(win->width, 32);
	win->depth_pitch = ROUND_UP(win->width * 2, 32);
	win->size = win->pitch * win->height;
	win->depth_size = win->depth_pitch * win->height;
	win->amp_size = win->amp_pitch * win->height;

	malloc_gdma_buffer(win, ptr);
	ptr->p_depth = (u16 *)malloc(sizeof(u16) * win->size);
	if (ptr->p_depth == NULL) {
		perror("malloc fail\n");
		ret = -1;
	}
	return ret;
}

static int debug_time(struct timespec *time0, struct timespec *time1,
	struct timespec *time2,struct timespec *time3)
{
	int raw = 0, a_b = 0, cal_dep = 0;

	raw = (time1->tv_sec - time0->tv_sec) * 1000 + (time1->tv_nsec - time0->tv_nsec) / 1000000;
	a_b = (time2->tv_sec - time1->tv_sec) * 1000 + (time2->tv_nsec - time1->tv_nsec) / 1000000;
	cal_dep = (time3->tv_sec - time2->tv_sec) * 1000 + (time3->tv_nsec - time2->tv_nsec) / 1000000;
	printf("raw%d, a_b %d, dep %d\n", raw, a_b, cal_dep);

	return 0;
}

static int set_vin_reg(int fd_iav, u32 addr, u32 data)
{
	struct vindev_reg reg;
	int ret = 0;

	reg.vsrc_id = 1;
	reg.addr = addr;
	reg.data = data;

	if (ioctl(fd_iav, IAV_IOC_VIN_SET_REG, &reg) < 0) {
		perror("IAV_IOC_VIN_SET_REG\n");
		ret = -1;
	}

	return ret;
}

#if 0
static int get_vin_reg(int fd_iav, u32 addr, u32 *data)
{
	struct vindev_reg reg;
	int ret = 0;

	reg.vsrc_id = 1;
	reg.addr = addr;
	reg.data = 0;

	do {
		if (ioctl(fd_iav, IAV_IOC_VIN_GET_REG, &reg) < 0) {
			perror("IAV_IOC_VIN_GET_REG\n");
			ret = -1;
		}
		*data = reg.data;
	} while (0);

	return ret;
}
#endif

static int imx316_set_expo_time(int fd_iav,u32 expo_time,u8 dual_freq)//dual_freq =0 or 1
{
	u32 num_line = ((u64)expo_time * 120) >> 9;

	if (dual_freq) {
		set_vin_reg(fd_iav, IMX316_INT_A + 0, (num_line >> 24) & 0xFF);
		set_vin_reg(fd_iav, IMX316_INT_A + 1, (num_line >> 16) & 0xFF);
		set_vin_reg(fd_iav, IMX316_INT_A + 2, (num_line >>  8) & 0xFF);
		set_vin_reg(fd_iav, IMX316_INT_A + 3, (num_line >>  0) & 0xFF);
		set_vin_reg(fd_iav, IMX316_INT_B + 0, (num_line >> 24) & 0xFF);
		set_vin_reg(fd_iav, IMX316_INT_B + 1, (num_line >> 16) & 0xFF);
		set_vin_reg(fd_iav, IMX316_INT_B + 2, (num_line >>  8) & 0xFF);
		set_vin_reg(fd_iav, IMX316_INT_B + 3, (num_line >>  0) & 0xFF);
	} else {
		set_vin_reg(fd_iav, IMX316_INT_A + 0, (num_line >> 24) & 0xFF);
		set_vin_reg(fd_iav, IMX316_INT_A + 1, (num_line >> 16) & 0xFF);
		set_vin_reg(fd_iav, IMX316_INT_A + 2, (num_line >>  8) & 0xFF);
		set_vin_reg(fd_iav, IMX316_INT_A + 3, (num_line >>  0) & 0xFF);
	}

	return 0;
}

#if 0
static int imx316_get_expo_time(int fd_iav, u8 dual_freq)//dual_freq =0 or 1
{
	u32 expo_time;
	u32 tmp[4];

	if (dual_freq) {
		get_vin_reg(fd_iav, IMX316_INT_A + 0, &tmp[0]);
		get_vin_reg(fd_iav, IMX316_INT_A + 1, &tmp[1]);
		get_vin_reg(fd_iav, IMX316_INT_A + 2, &tmp[2]);
		get_vin_reg(fd_iav, IMX316_INT_A + 3, &tmp[3]);
		get_vin_reg(fd_iav, IMX316_INT_B + 0, &tmp[4]);
		get_vin_reg(fd_iav, IMX316_INT_B + 1, &tmp[5]);
		get_vin_reg(fd_iav, IMX316_INT_B + 2, &tmp[6]);
		get_vin_reg(fd_iav, IMX316_INT_B + 3, &tmp[7]);
	} else {
		get_vin_reg(fd_iav, IMX316_INT_A + 0, &tmp[0]);
		get_vin_reg(fd_iav, IMX316_INT_A + 1, &tmp[1]);
		get_vin_reg(fd_iav, IMX316_INT_A + 2, &tmp[2]);
		get_vin_reg(fd_iav, IMX316_INT_A + 3, &tmp[3]);
	}

	expo_time = (tmp[0] & 0xFF) << 24 |
		(tmp[1] & 0xFF) << 16 |
		(tmp[2] & 0xFF) << 8 |
		(tmp[3] & 0xFF) ;
	printf("expo_time = %d ms\n", expo_time / 120);

	return 0;
}
#endif

static int img_set_tof_expo_time(int fd_iav,u32 expo_time, int dual_flag)
{
	imx316_set_expo_time(fd_iav,expo_time, dual_flag);

	return 0;
}

static int tof_decode_deinit()
{
	amba_cvwarp_api *g_cvwarp = tof_mem.g_cvwarp;
	if (g_cvwarp) {
		g_cvwarp->exit();
	}
	if (tof_mem.warp_handle) {
		dlclose(tof_mem.warp_handle);
		tof_mem.warp_handle = NULL;
	}

	destroy_fixed2float_context(tof_mem.ctx_float2fixed);
	destroy_fixed2float_context(tof_mem.ctx_fixed2float);
	if (tof_mem.warp_handle) {
		dlclose(tof_mem.warp_handle);
		tof_mem.warp_handle = NULL;
	}
	if (tof_mem.fixedPhase) {
		free(tof_mem.fixedPhase);
		tof_mem.fixedPhase = NULL;
	}
	if (tof_mem.pZmap) {
		free(tof_mem.pZmap);
		tof_mem.pZmap = NULL;
	}
	if (tof_mem.phaData) {
		free(tof_mem.phaData);
		tof_mem.phaData = NULL;
	}
	if (tof_mem.phaData2) {
		free(tof_mem.phaData2);
		tof_mem.phaData2 = NULL;
	}
	if (tof_mem.targetData) {
		free(tof_mem.targetData);
		tof_mem.targetData = NULL;
	}

	return 0;
}

static void sigstop()
{
	if (gdma_dst_buf_pid) {
		free_gdma_dst_buf();
	}
	if (tof_mem.p_depth) {
		free(tof_mem.p_depth);
		tof_mem.p_depth = NULL;
	}
	if (tof_mem.p_depth_float) {
		free(tof_mem.p_depth_float);
		tof_mem.p_depth_float = NULL;
	}
	if (tof_mem.calib_data) {
		free(tof_mem.calib_data);
		tof_mem.calib_data = NULL;
	}
	tof_decode_deinit();
}

typedef enum {
	EFM_TYPE_YUV_FILE = 0,
	EFM_TYPE_YUV_CAPTURE = 1,
	EFM_TYPE_TOTAL_NUM,
	EFM_TYPE_FIRST = EFM_TYPE_YUV_FILE,
	EFM_TYPE_LAST = EFM_TYPE_TOTAL_NUM,
} EFM_TYPE;

static int map_buffers(void)
{
	int ret = 0;
    clut_start = 20;
    clut_end = 200;
	ret = map_dsp_buffer();
	return ret;
}

static int init_accerleration(tof_mem_ptr *ptr,
	float *xy_map, myCameraParameters *campars, reso *win, p2dIO *p2dio)
{
	struct warp_ctrl *warp_info = &g_LDCinfo;
	struct cvwarp_intrinsic_t intrinsic;
	fixed2float_option_t option;
	int ret = 0;

	do {
		strcpy(warp_info->input[0].bin, WARP_FIX16_BIN);
		strcpy(warp_info->input[1].bin, WARP_FIX8_BIN);
		warp_info->net_num = 2;

		ptr->warp_handle = dlopen(CV_WARP_PLUGIN, RTLD_LAZY);
		if (ptr->warp_handle == NULL) {
			printf("open warp handl error %s!\n", dlerror());
			ret = -1;
			break;
		}
		ptr->g_cvwarp = (amba_cvwarp_api*)dlsym(ptr->warp_handle, "amba_cvwarp");
		if (ptr->g_cvwarp == NULL) {
			printf("g_cvwarp error!\n");
			ret = -1;
			break;
		}
		prepare_init_dewarp_depth(win->width, win->height,
		ptr->g_cvwarp, warp_info, &intrinsic, campars, xy_map, p2dio);

		option.fixed_is_signed = 0;
		option.fixed_bits = 16;
		option.mantissa_bits = 10;
		ptr->fixedPhase =(u16 *) malloc(win->width * win->height * sizeof(s16));
		ptr->ctx_float2fixed = setup_float2fixed_context(&option);
		ptr->ctx_fixed2float = setup_fixed2float_context(&option);
	} while (0);

	return ret;
}

static int tof_decode_init(tof_mem_ptr *ptr, reso *win, p2dIO *p2dio)
{
	myPhaseData *data = NULL;
	int ret = 0;
	float *xy_map;
	myCameraParameters *campars;

	do {
		if (!spi_read) {
			if (!p2dio->Load_setting(K_DATA)) {
				printf("Loading fail!\n");
				ret = -1;
				break;
			}
		} else {
			read_spi_calib(ptr->g_cvwarp, p2dio);
		}
		if (p2dio->usingDualFreq()) {
			ptr->phaData = new myPhaseData();
			data = ptr->phaData;
			data->initLDC(p2dio->bin_bufferf1);
			ptr->pAminusB1 = data->getAmusB1_ptr();
			ptr->pAminusB2 = data->getAmusB2_ptr();
			ptr->pAminusB3 = data->getAmusB3_ptr();
			ptr->pAminusB4 = data->getAmusB4_ptr();

			ptr->phaData2 = new myPhaseData();
			data = ptr->phaData2;
			data->initLDC(p2dio->bin_bufferf2);
			ptr->pAminusB1f2 = data->getAmusB1_ptr();
			ptr->pAminusB2f2 = data->getAmusB2_ptr();
			ptr->pAminusB3f2 = data->getAmusB3_ptr();
			ptr->pAminusB4f2 = data->getAmusB4_ptr();

			ptr->targetData = new myPhaseData();
			data = ptr->targetData;
			data->initLDC(p2dio->bin_bufferf1);
			data->setModulationFreq(20000000);
			ptr->pXmap = data->getXvalue_ptr();
			ptr->pYmap = data->getYvalue_ptr();
			ptr->pZmap = data->getZvalue_ptr();
			ptr->pPhasemap = data->phase_map_LDC;
			phase_number = PHASE_NUM;
			printf("%s:%d usingDualFreq\n", __func__, __LINE__);
		} else {
			ptr->phaData = new myPhaseData();
			data = ptr->phaData;
			data->initLDC(p2dio->bin_buffer);
			ptr->pAminusB1 = data->getAmusB1_ptr();
			ptr->pAminusB2 = data->getAmusB2_ptr();
			ptr->pAminusB3 = data->getAmusB3_ptr();
			ptr->pAminusB4 = data->getAmusB4_ptr();
			ptr->pZmap = data->getZvalue_ptr();
			ptr->pPhasemap = data->phase_map_LDC;
			printf("%s:%d usingSingleFreq\n", __func__, __LINE__);
		}

		data = ptr->phaData;
		win->width = data->getSensorWidth();
		win->height = data->getSensorHeight();
		ptr->pConfidence = data->getConfidenceMap_ptrLDC();
		nConfidenceOffset = (win->width + 16) * win->height / 2 + 128;	//Confidence @ image center

		if (!data->getReady()) {
			printf("not ready\n");
			ret = -1;
			break;
		}

		xy_map = new float [(win->width + EXTRA_LINES) * (win->height + EXTRA_LINES) * 2];
		data->build_LDC_xymap_amba(xy_map);
		campars = data->DEB_getCampars();
		printf("win->width:%d, win->height:%d\n", win->width, win->height);
		printf("camera intrinsic: \n");
		printf("%.1f, %.1f, %.3f, %.3f\n", campars->cx, campars->cy, campars->fx, campars->fy);
		printf("%.3f, %.3f, %.3f, %.3f, %.3f\n", campars->k1, campars->k2, campars->p1, campars->p2, campars->k3);

		if (do_acceleration) {
			if (init_accerleration(ptr, xy_map, campars, win, p2dio) < 0) {
				printf("init_accerleration fail\n");
				ret= -1;
				break;
			}
		}
	} while (0);

	return ret;
}

static int init_amba_tof_ae_config(tof_ae_config_t* p_tof_ae_cfg, p2dIO *p2dio)
{
	p_tof_ae_cfg->target_confidence = 600;
	p_tof_ae_cfg->target_tolerant_upper = 40;
	p_tof_ae_cfg->target_tolerant_lower = 40;
	p_tof_ae_cfg->adj_ratio_inc = 18;//unit =16
	p_tof_ae_cfg->adj_ratio_dec = 14;//unit =16
	p_tof_ae_cfg->max_expo_time = init_exposure_time;//Q9
	p_tof_ae_cfg->min_expo_time = SHUTTER_1BY100000;//Q9
	p_tof_ae_cfg->init_expo_time = init_exposure_time;
	p_tof_ae_cfg->skip_frame = 1;

	img_set_tof_expo_time(fd_iav, p_tof_ae_cfg->init_expo_time,
		p2dio->usingDualFreq() ? 1 : 0);

	return 0;
}

static int amba_tof_ae_control(int fd_iav,int in_confidence,
	tof_ae_config_t* p_tof_ae_cfg,u32* out_expo_time, p2dIO *p2dio)
{

	static u8 is_first_flag= 1;
	//static u8 skip_frame=0;
	static u32 curr_expo_time= 0;
	u32 adj_ratio= 16;
	u32 next_expo_time = 0;
	int ret = 0;

	do {
		if(is_first_flag) {
			//skip_frame =p_tof_ae_cfg->skip_frame;
			curr_expo_time= p_tof_ae_cfg->init_expo_time;
			is_first_flag =0;
		}
#if 0
		if(skip_frame --) {
			return 0;
		}
#endif
		if (in_confidence < p_tof_ae_cfg->target_confidence - 20) {
			adj_ratio = p_tof_ae_cfg->adj_ratio_inc;
		} else if (in_confidence > p_tof_ae_cfg->target_confidence + 20) {
			adj_ratio = p_tof_ae_cfg->adj_ratio_dec;
		} else {
			adj_ratio = 16;
			break;
		}

		next_expo_time= curr_expo_time * adj_ratio / 16;
		curr_expo_time = CHECK_LIMIT(next_expo_time, p_tof_ae_cfg->min_expo_time,
			p_tof_ae_cfg->max_expo_time);
		*out_expo_time= curr_expo_time;
		img_set_tof_expo_time(fd_iav, curr_expo_time, p2dio->usingDualFreq() ? 1 : 0);
	} while (0);

	return ret;
}

static int liteon_post_processing(p2dIO *p2dio, myPhaseData *data1,
	myPhaseData *data2, myPhaseData *target_data)
{
	u8 *pCon1ptr = NULL, *pConTptr = NULL;
	float thFlyingPixel = (float)0.05;
	float thMoving = (float)0.05;

	if (p2dio->usingDualFreq()) {
		// copy confidence map of frequency 1 to targetData
		// due to targetData only contain synthesised phase map
		pCon1ptr = data1->getConfidenceMap_ptrLDC();
		pConTptr = target_data->getConfidenceMap_ptrLDC();
		memcpy(pConTptr, pCon1ptr, sizeof(u8) * data1->getSensorHeight() * data1->getSensorWidth());
		target_data->TemporalFilterV01(target_data->phase_map_LDC, pConTptr, 0, thMoving);
		target_data->filterLowConfidencePixel(thConfidence);
		target_data->phase2Zmap();
		target_data->flyingPixelRemove(thFlyingPixel);
		target_data->phase2PointCloudLDC();
	} else {
		data1->TemporalFilterV01(data1->phase_map_LDC, data1->getConfidenceMap_ptrLDC(), 0, thMoving);
		data1->filterLowConfidencePixel(thConfidence);
		data1->flyingPixelRemove(thFlyingPixel);
		data1->phase2PointCloudLDC();
	}

	return 0;
}

static int liteon_set_up_raw_mem(p2dIO *p2dio,
	tof_mem_ptr *tof_ptr, reso *win,
	myPhaseData *data1, myPhaseData *data2,
	myPhaseData *target_data)
{
	s16 *phase1 = NULL, *phase2 = NULL, *phase3 = NULL, *phase4 = NULL;
	s16  *phase1f2 = NULL, *phase2f2 = NULL, *phase3f2 = NULL, *phase4f2 = NULL;
	int frame_size = win->width * win->height * 4;
	int i;
	float currentTemperature = 30;

	phase1 = (s16 *)(tof_ptr->gdma_dst_buf);
	phase2 = (s16 *)(tof_ptr->gdma_dst_buf + frame_size);
	phase3 = (s16 *)(tof_ptr->gdma_dst_buf + 2 * frame_size);
	phase4 = (s16 *)(tof_ptr->gdma_dst_buf + 3 * frame_size);
	if (p2dio->usingDualFreq()) {
		phase1f2 = (s16 *)(tof_ptr->gdma_dst_buf + 4 * frame_size);
		phase2f2 = (s16 *)(tof_ptr->gdma_dst_buf + 5 * frame_size);
		phase3f2 = (s16 *)(tof_ptr->gdma_dst_buf + 6 * frame_size);
		phase4f2 = (s16 *)(tof_ptr->gdma_dst_buf + 7 * frame_size);
	}

	for (i = 0; i < win->width * win->height; i++) {
		tof_ptr->pAminusB1[i] = short((phase1[2 * i] >> 4) - (phase1[2 * i + 1] >> 4));
		tof_ptr->pAminusB2[i] = short((phase2[2 * i] >> 4) - (phase2[2 * i + 1] >> 4));
		tof_ptr->pAminusB3[i] = short((phase3[2 * i] >> 4) - (phase3[2 * i + 1] >> 4));
		tof_ptr->pAminusB4[i] = short((phase4[2 * i] >> 4) - (phase4[2 * i + 1] >> 4));
	}

	data1->UpdateCurrentTemperature(currentTemperature);
	data1->PhaseDecodeLDC();
	if (p2dio->usingDualFreq()) {
		for (i = 0; i < win->width * win->height; i++) {
			tof_ptr->pAminusB1f2[i] = short((phase1f2[2 * i] >> 4) - (phase1f2[2 * i + 1] >> 4));
			tof_ptr->pAminusB2f2[i] = short((phase2f2[2 * i] >> 4) - (phase2f2[2 * i + 1] >> 4));
			tof_ptr->pAminusB3f2[i] = short((phase3f2[2 * i] >> 4) - (phase3f2[2 * i + 1] >> 4));
			tof_ptr->pAminusB4f2[i] = short((phase4f2[2 * i] >> 4) - (phase4f2[2 * i + 1] >> 4));
		}
		data2->UpdateCurrentTemperature(currentTemperature);
		data2->PhaseDecodeLDC();
		target_data->dualFrequencySynthesis(data1->phase_map_LDC,
			data2->phase_map_LDC,
			target_data->phase_map_LDC,
			data1->getModulationFreq(),
			data2->getModulationFreq(),
			0.5);
	}

	return 0;
}

static int run_warp(struct warp_ctrl *warp_info, tof_mem_ptr *tof_ptr)
{
	amba_cvwarp_api *warp_api = NULL;

	warp_info->input[0].data = (u8 *)tof_ptr->fixedPhase;
	warp_info->input[1].data = (u8 *)tof_ptr->pConfidence;
	warp_api = tof_ptr->g_cvwarp;
	warp_api->run(warp_info);

	return 0;
}

static int do_tof_process(tof_mem_ptr *tof_ptr, reso *win, p2dIO *p2dio)
{
	myPhaseData *data1 = NULL, *data2 = NULL, *target_data = NULL;
	u32 out_expo_time = 0;
	int currentConfidence = *(tof_ptr->pConfidence + nConfidenceOffset) << 4;
	struct warp_ctrl *warp_info = &g_LDCinfo;

	data1 = tof_ptr->phaData;
	data2 = tof_ptr->phaData2;
	target_data = tof_ptr->targetData;

	liteon_set_up_raw_mem(p2dio, tof_ptr, win, data1, data2, target_data);
	if (do_acceleration) {
		float2fixed(tof_ptr->ctx_float2fixed, win->width * win->height,
		tof_ptr->pPhasemap, tof_ptr->fixedPhase);
		run_warp(warp_info, tof_ptr);
		fixed2float(tof_ptr->ctx_fixed2float, win->width * win->height,
			tof_ptr->fixedPhase, tof_ptr->pPhasemap);
	}
	liteon_post_processing(p2dio, data1, data2, target_data);
	if (auto_exposure_flag) {
		amba_tof_ae_control(fd_iav, currentConfidence, &tof_ae_cfg, &out_expo_time, p2dio);
	}

	return 0;
}

// static int put_buffer(reso *win, tof_mem_ptr *tof_ptr)
// {
// 	float *Xvalue = NULL, *Yvalue = NULL, *Zvalue = NULL;
// 	int nImgsize = win->width * win->height;
	
// 	Xvalue = tof_ptr->targetData->getXvalue_ptr();
// 	Yvalue = tof_ptr->targetData->getYvalue_ptr();
// 	Zvalue = tof_ptr->targetData->getZvalue_ptr();

// 	pthread_mutex_lock(&tof_buffer.lock);  
//     if ((tof_buffer.writepos + 1) % TOF_BUFFER_SIZE == tof_buffer.readpos)  
//     {  
//         pthread_cond_wait(&tof_buffer.notfull, &tof_buffer.lock);  
//     }

// 	for (int i = 0; i < nImgsize; i++)
// 	{
// 		tof_buffer.buffer_x[tof_buffer.writepos][i] = Xvalue[i];
// 		tof_buffer.buffer_y[tof_buffer.writepos][i] = Yvalue[i];
// 		tof_buffer.buffer_z[tof_buffer.writepos][i] = Zvalue[i];
// 	}
    
//     tof_buffer.writepos++;  
//     if (tof_buffer.writepos >= TOF_BUFFER_SIZE)  
//         tof_buffer.writepos = 0;  
//     pthread_cond_signal(&tof_buffer.notempty);  
//     pthread_mutex_unlock(&tof_buffer.lock);  

// 	return 0;
// }

static void *run_tof_pthread(void* data)
{
	struct sensor_data sensor;
	struct timespec time0, time1, time2, time3;
	reso win_t;
	reso *win = &win_t;
	tof_option_t option;
	tof_mem_ptr *tof_ptr = &tof_mem;
	int repeat = 0;
	unsigned long prev_pts0 = 0;
	unsigned long prev_pts4 = 0;
	p2dIO p2dio;
	float *Xvalue = NULL, *Yvalue = NULL, *Zvalue = NULL;
	int nImgsize = 0;

	prctl(PR_SET_NAME, "tof_pthread");

	if (tof_decode_init(tof_ptr, win, &p2dio) < 0) {
		LOG(ERROR) << "tof_decode_init fail!";
        run_tof = 0;
	}

	init_amba_tof_ae_config(&tof_ae_cfg, &p2dio);
	if (config_tof(win, &option, tof_ptr) < 0){
		LOG(ERROR) << "config_tof fail!";
		run_tof = 0;
	}

	while(run_tof) {
		if (!repeat) {
			if (debug == DEBUG_TIME) {
				clock_gettime(CLOCK_REALTIME, &time0);
			}
		}
		repeat = 0;
		memset(&sensor, 0, sizeof(sensor));
		if (get_4_raw_data(&sensor, win, &prev_pts0, &prev_pts4,
			&repeat, p2dio.usingDualFreq() ? 1 : 0) < 0) {
			LOG(ERROR) << "get_4_raw_data fail";
            run_tof = 0;
		}
		if (repeat) {
			continue;
		}

		if (debug == DEBUG_TIME) {
			clock_gettime(CLOCK_REALTIME, &time1);
		}

		do_tof_process(tof_ptr, win, &p2dio);

		nImgsize = win->width * win->height;
		Xvalue = tof_ptr->targetData->getXvalue_ptr();
		Yvalue = tof_ptr->targetData->getYvalue_ptr();
		Zvalue = tof_ptr->targetData->getZvalue_ptr();

		pthread_mutex_lock(&tof_buffer.lock);  
		if ((tof_buffer.writepos + 1) % TOF_BUFFER_SIZE == tof_buffer.readpos)  
		{  
			pthread_cond_wait(&tof_buffer.notfull, &tof_buffer.lock);  
		}

		for (int i = 0; i < nImgsize; i++)
		{
			// tof_buffer.buffer_x[tof_buffer.writepos][i] = Xvalue[i];
			// tof_buffer.buffer_y[tof_buffer.writepos][i] = Yvalue[i];
			tof_buffer.buffer_z[tof_buffer.writepos][i] = Zvalue[i];
		}
		
		tof_buffer.writepos++;  
		if (tof_buffer.writepos >= TOF_BUFFER_SIZE)  
			tof_buffer.writepos = 0;  
		pthread_cond_signal(&tof_buffer.notempty);  
		pthread_mutex_unlock(&tof_buffer.lock);

		if (debug == DEBUG_TIME) {
			clock_gettime(CLOCK_REALTIME, &time3);
			debug_time(&time0, &time1, &time2, &time3);
		}
		if(has_sleep > 0)
		{
			sleep(1);
		}
	}
	run_tof = 0;
	sigstop();
	LOG(WARNING) << "TOF thread quit.";
	return NULL;
}

TOF316Acquisition::TOF316Acquisition()
{
    run_tof = 0;
	pthread_id = 0;
}

TOF316Acquisition::~TOF316Acquisition()
{	
	if(run_tof > 0)
	{
		stop();
	}
	pthread_mutex_destroy(&tof_buffer.lock);
    pthread_cond_destroy(&tof_buffer.notempty);
    pthread_cond_destroy(&tof_buffer.notfull);
	if(fd_iav >= 0)
	{
		close(fd_iav);
		fd_iav = -1;
	}
	LOG(WARNING) << "~TOF316Acquisition()";
}

int TOF316Acquisition::open_tof()
{
    if(fd_iav >= 0)
    {
        close(fd_iav);
		fd_iav = -1;
		LOG(WARNING) << "close /dev/iav";
    }
    fd_iav = open("/dev/iav", O_RDWR, 0);
	if (fd_iav < 0) {
		LOG(ERROR) << "open /dev/iav fail";
        return -1;
	}
    if (map_buffers() < 0) {
		LOG(ERROR) << "map_buffers error";
        return -1;
	}
	pthread_mutex_init(&tof_buffer.lock, NULL);  
    pthread_cond_init(&tof_buffer.notempty, NULL);  
    pthread_cond_init(&tof_buffer.notfull, NULL);  
    tof_buffer.readpos = 0;  
    tof_buffer.writepos = 0;
	LOG(INFO) << "TOF init success";
	return 0;
}

int TOF316Acquisition::start()
{
	int ret = 0;
    run_tof = 1;
	tof_buffer.readpos = 0;  
    tof_buffer.writepos = 0;
    ret = pthread_create(&pthread_id, NULL, run_tof_pthread, NULL);
	if(ret < 0)
    {
        run_tof = 0;
        LOG(ERROR) << "start tof pthread fail!";
    }
	LOG(INFO) << "start tof pthread success!";
	return ret;
}

int TOF316Acquisition::stop()
{
	int ret = 0;
	run_tof = 0;
	if (pthread_id > 0) {
		pthread_cond_signal(&tof_buffer.notfull);
		pthread_cond_signal(&tof_buffer.notempty);  
		pthread_mutex_unlock(&tof_buffer.lock);
		pthread_join(pthread_id, NULL);
		pthread_id = 0;
	}
	LOG(WARNING) << "stop TOF success";
	return ret;
}

void TOF316Acquisition::set_up()
{
	has_sleep = 0;
}

void TOF316Acquisition::set_sleep()
{
	has_sleep = 1;
}

void TOF316Acquisition::get_tof_depth_map(cv::Mat &depth_map)
{
	int i, j, index;
	float inv_dst = 0;
	float max_dst = 0;
	uchar* depth_ptr = depth_map.ptr<uchar>(0);
	if (sensor_type == SENSOR_IMX316) {
		max_dst = MAX_DIST_316;
	} else {
		max_dst = MAX_DIST_456;
	}
	max_dst = 3.0f;
	if(pthread_id > 0) 
	{
		pthread_mutex_lock(&tof_buffer.lock);  
		if (tof_buffer.writepos == tof_buffer.readpos)  
		{  
			pthread_cond_wait(&tof_buffer.notempty, &tof_buffer.lock);  
		}
		for(int i = 0; i < MAX_POINT_CLOUD && run_tof > 0; i++)
		{
			if (tof_buffer.buffer_z[tof_buffer.readpos][i] > max_dst || \
					tof_buffer.buffer_z[tof_buffer.readpos][i] < 0.8f || \
					(tof_buffer.buffer_z[tof_buffer.readpos][i] == 0))
					{
						*depth_ptr = 0;
					} 
					else 
					{
						inv_dst = max_dst - tof_buffer.buffer_z[tof_buffer.readpos][i];
						*depth_ptr = static_cast<uchar>(inv_dst * 255 / max_dst);
					}
			depth_ptr++;
		} 
		tof_buffer.readpos++;  
		if (tof_buffer.readpos >= TOF_BUFFER_SIZE)  
			tof_buffer.readpos = 0; 
		pthread_cond_signal(&tof_buffer.notfull);  
		pthread_mutex_unlock(&tof_buffer.lock);
	}
}

void TOF316Acquisition::get_tof_Z(unsigned char* addr)
{
	if(pthread_id > 0) 
	{
		pthread_mutex_lock(&tof_buffer.lock);  
		if (tof_buffer.writepos == tof_buffer.readpos)  
		{  
			pthread_cond_wait(&tof_buffer.notempty, &tof_buffer.lock);  
		}
		memcpy(addr, (unsigned char*)tof_buffer.buffer_z[tof_buffer.readpos], DEPTH_WIDTH * DEPTH_HEIGTH * sizeof(float));
		tof_buffer.readpos++;  
		if (tof_buffer.readpos >= TOF_BUFFER_SIZE)  
			tof_buffer.readpos = 0; 
		pthread_cond_signal(&tof_buffer.notfull);  
		pthread_mutex_unlock(&tof_buffer.lock);
	}
}

// void TOF316Acquisition::get_tof_pc(PointCloud &point_cloud)
// {
// 	int i, j, index;
// 	float inv_dst = 0;
// 	float max_dst = 0;
// 	if (sensor_type == SENSOR_IMX316) {
// 		max_dst = MAX_DIST_316;
// 	} else {
// 		max_dst = MAX_DIST_456;
// 	}
// 	max_dst = 3.0f;
// 	point_cloud.clear(); 
// 	if(pthread_id > 0) 
// 	{
// 		pthread_mutex_lock(&tof_buffer.lock);  
// 		if (tof_buffer.writepos == tof_buffer.readpos)  
// 		{  
// 			pthread_cond_wait(&tof_buffer.notempty, &tof_buffer.lock);  
// 		}
// 		for(int i = 0; i < MAX_POINT_CLOUD; i++)
// 		{
// 			if(tof_buffer.buffer_z[tof_buffer.readpos][i] >= 0.8f && \
// 				tof_buffer.buffer_z[tof_buffer.readpos][i] <= max_dst)
// 				{
// 					struct Point temp_point;
// 					temp_point.x = tof_buffer.buffer_x[tof_buffer.readpos][i];
// 					temp_point.y = tof_buffer.buffer_y[tof_buffer.readpos][i];
// 					temp_point.z = tof_buffer.buffer_z[tof_buffer.readpos][i];
// 					temp_point.index = i;
// 					point_cloud.push_back(temp_point);
// 				}
// 		} 
// 		tof_buffer.readpos++;  
// 		if (tof_buffer.readpos >= TOF_BUFFER_SIZE)  
// 			tof_buffer.readpos = 0; 
// 		pthread_cond_signal(&tof_buffer.notfull);  
// 		pthread_mutex_unlock(&tof_buffer.lock);
// 	}
// }

// void TOF316Acquisition::get_tof_data(PointCloud &point_cloud, cv::Mat &depth_map)
// {
// 	int i, j, index;
// 	float inv_dst = 0;
// 	float max_dst = 0;
// 	uchar* depth_ptr = depth_map.ptr<uchar>(0);
// 	if (sensor_type == SENSOR_IMX316) {
// 		max_dst = MAX_DIST_316;
// 	} else {
// 		max_dst = MAX_DIST_456;
// 	}
// 	max_dst = 3.0f;
// 	point_cloud.clear(); 
// 	if(pthread_id > 0) 
// 	{
// 		pthread_mutex_lock(&tof_buffer.lock);  
// 		if (tof_buffer.writepos == tof_buffer.readpos)  
// 		{  
// 			pthread_cond_wait(&tof_buffer.notempty, &tof_buffer.lock);  
// 		}
// 		for(int i = 0; i < MAX_POINT_CLOUD; i++)
// 		{
// 			if(tof_buffer.buffer_z[tof_buffer.readpos][i] >= 0.8f && \
// 				tof_buffer.buffer_z[tof_buffer.readpos][i] <= max_dst)
// 				{
// 					struct Point temp_point;
// 					temp_point.x = tof_buffer.buffer_x[tof_buffer.readpos][i];
// 					temp_point.y = tof_buffer.buffer_y[tof_buffer.readpos][i];
// 					temp_point.z = tof_buffer.buffer_z[tof_buffer.readpos][i];
// 					temp_point.index = i;
// 					point_cloud.push_back(temp_point);
// 				}
// 			if (tof_buffer.buffer_z[tof_buffer.readpos][i] > max_dst || \
// 					tof_buffer.buffer_z[tof_buffer.readpos][i] < 0.8f || \
// 					(tof_buffer.buffer_z[tof_buffer.readpos][i] == 0))
// 					{
// 						*depth_ptr = 0;
// 					} 
// 					else 
// 					{
// 						inv_dst = max_dst - tof_buffer.buffer_z[tof_buffer.readpos][i];
// 						*depth_ptr = static_cast<uchar>(inv_dst * 255 / max_dst);
// 					}
// 			depth_ptr++;
// 		} 
// 		tof_buffer.readpos++;  
// 		if (tof_buffer.readpos >= TOF_BUFFER_SIZE)  
// 			tof_buffer.readpos = 0; 
// 		pthread_cond_signal(&tof_buffer.notfull);  
// 		pthread_mutex_unlock(&tof_buffer.lock);
// 	}
// }

int TOF316Acquisition::dump_ply(const char* save_path, const PointCloud &src_cloud)
{
	char ply_header[100];
	sprintf(ply_header, "element vertex %ld\n", src_cloud.size());
	FILE *fptr;
	fptr = fopen(save_path, "w");

	fprintf(fptr, "ply\n");
	fprintf(fptr, "format ascii 1.0\n");
	fprintf(fptr, "%s", ply_header);
	fprintf(fptr, "property float x\nproperty float y\nproperty float z\n");
	fprintf(fptr, "property uchar red\nproperty uchar green\nproperty uchar blue\n");
	fprintf(fptr, "end_header\n");
	for (size_t i = 0; i < src_cloud.size(); i++)
	{
		fprintf(fptr, "%f %f %f\n", src_cloud[i].x, src_cloud[i].y, src_cloud[i].z);
		fprintf(fptr, "%d %d %d\n", 255, 0, 0);
	}
	fclose(fptr);
	LOG(INFO) << "save ply OK...";
	return 0;
}

int TOF316Acquisition::dump_bin(const std::string &save_path, const PointCloud &src_cloud)
{
	std::ofstream out_file(save_path, std::ios::binary);
	for (size_t i = 0; i < src_cloud.size(); i++)
	{
		float data[3];
		data[0] = src_cloud[i].x;
        data[1] = src_cloud[i].y;
        data[2] = src_cloud[i].z;
		out_file.write(reinterpret_cast<char*>(data), sizeof(data));
	}
	out_file.close();
	LOG(INFO) << "save bin OK...";
	return 0;
}

int TOF316Acquisition::read_bin(const std::string &file_path, PointCloud &result_cloud)
{
	float data[3] = {0};
	std::ifstream in_file(file_path, std::ios::in|std::ios::binary);
	if(!in_file)
    {
		LOG(ERROR) << file_path << " open error!";
        return -1;
    }
	result_cloud.clear();
	while(in_file.read(reinterpret_cast<char*>(data), sizeof(data)))
	{
		TOF316Acquisition::Point point;
		point.x = data[0];
		point.y = data[1];
		point.z = data[2];
		result_cloud.push_back(point);
	}
	in_file.close();
	LOG(INFO) << "read bin OK...";
	return 0;
}
