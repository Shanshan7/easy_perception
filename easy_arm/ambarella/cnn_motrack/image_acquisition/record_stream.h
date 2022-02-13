#ifndef __RECORD_STREAM_H__
#define __RECORD_STREAM_H__

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
#include <sys/select.h>
#include <time.h>
#include <assert.h>
#include <arpa/inet.h>
#include <time.h>

#include <signal.h>
#include <basetypes.h>
#include <iav_ioctl.h>

#include <sys/statfs.h>

#include <pthread.h>

#ifndef AM_IOCTL
#define AM_IOCTL(_filp, _cmd, _arg)       \
	do                                    \
	{                                     \
		if (ioctl(_filp, _cmd, _arg) < 0) \
		{                                 \
			perror(#_cmd);                \
			return -1;                    \
		}                                 \
	} while (0)
#endif

#define SVCT_PORT_OFFSET (16)
#define FAST_SEEK_PORT_OFFSET (20)
#define MAX_SVCT_LAYERS (4)

#define DISK_DIRECTORY ("/sdcard/")
#define TEN_MINUTE_TO_MISECOND (600000)
#define ENCODEN_VIDEO_STREAM_ID (1)

// total frames we need to capture
static int md5_idr_number = -1;

// debug options
#undef DEBUG_PRINT_FRAME_INFO
#define VERSION 0x00000005
#define PTS_IN_ONE_SECOND (90000)

static int nofile_flag = 0;
static int frame_info_flag = 0;
static int show_bsb_stats_flag = 0;
static int file_size_flag = 0;
static int file_size_mega_byte = 100;
static int remove_time_string_flag = 0;
static int split_svct_layer_flag = 0;
static int split_fast_seek_flag = 0;

// verbose
static int verbose_mode = 0;

// check stream cfg for dram test
static u32 stream_mask = 0;	  // indicate stream status
static u8 enable_hp_fps = 0;

typedef struct
{
	int session_id;	  // stream encoding session
	u64 total_frames; // count how many frames encoded, help to divide for session before session id is implemented
	u64 total_bytes;  // count how many bytes encoded
	int pic_type;	  // picture type,  non changed during encoding, help to validate encoding state.
	u64 monotonic_pts;
	u64 enc_done_ts;
	u64 enc_done_ts_diff_sum;
	u64 i_num; // record i frame's number
	u64 total_frames_i;
	u64 total_frames_p;
	u64 total_frames_b;
	u64 total_bytes_i;
	u64 total_bytes_p;
	u64 total_bytes_b;
	u64 hw_pts;

	u64 total_frames2;	  // for fps measured every fps_statistics_interval frames
	struct timeval time2; // for fps measured every fps_statistics_interval frames

	u64 total_bytes_gop;	 // for bitrate measured within the scope of one gop
	struct timeval time_gop; // for bitrate measured within the scope of one gop

} stream_encoding_state_t;

typedef struct stream_files_s
{
	int session_id;				  // stream encoding session
	u64 total_bytes;			  // count how many bytes encoded
	int fd;						  // stream write file handle
	int fd_info;				  // info write file handle
	int fd_svct[MAX_SVCT_LAYERS]; // file descriptor for svct streams
	int fd_fast_seek;			  // file descriptor for fast seek streams
	int gop_structure;			  // store gop structure
	int fast_seek_intvl;		  // store fast seek intvl
} stream_files_t;

typedef struct frame_info_s
{
	u32 pic_type : 3;
	u32 reserved : 29;
	u32 data_addr_offset;
	u32 frame_cnt;
	u32 size;
	u64 dsp_pts;
	u64 arm_pts;
} frame_info_t;

struct stream_info
{
	enum iav_stream_type type;	 // encode type
	enum iav_stream_state state; // encode state
	int canvas;					 // source canvas
	int abs_fps;				 // abs fps

	int hflip;
	int vflip;
	int rotate_cw;
};

static frame_info_t frame_info;							 ///[MAX_ENCODE_STREAM_NUM];
static stream_files_t stream_files;						 ///[MAX_ENCODE_STREAM_NUM];

// static int write_video_file_run = 1;

class RecordStream {
   public:
   int fd_iav;
   u8 *bsb_mem;
   u32 bsb_size;
   struct timeval pre;
  // bitstream filename base
   char filename[512] = "/sdcard/encoded_stream_record_10min_0.h264";
   const char *default_filename = "/sdcard/encoded_stream_record_10min";
   int record_file_index = 0;

  long long get_disk_space_free(void);
  int init_stream_files(void);
  int close_stream_files();
  int deinit_stream_files();
  int is_new_session(struct iav_framedesc *framedesc);
  u8 is_new_frame(struct iav_framedesc *framedesc);
  u8 is_last_framedesc(struct iav_framedesc *framedesc);
  int get_time_string(char *time_str, int len);
  int write_frame_info_header();
  int write_frame_info(struct iav_framedesc *framedesc);
  int check_h26x_info(enum iav_stream_type stream_type, int stream_id);
// check session and update file handle for write when needed
  int check_session_file_handle(struct iav_framedesc *framedesc, int new_session);
  int update_files_data(struct iav_framedesc *framedesc, int new_session);
  int write_svct_file(unsigned char *in, int len, int fd);
  int write_fast_seek_file(unsigned char *in, int len, int fd);

  int identify_nal_ref_idc(unsigned char *in, int in_len);

  int identify_nuh_temporal_id_plus1(unsigned char *in, int in_len);
  int get_svct_layer(enum iav_stream_type stream_type, int stream_id, unsigned char *in,
						  int in_len, int *ret_layer);

  int write_svct_files(enum iav_stream_type stream_type, /*int transfer_method,*/
							int stream_id, unsigned char *in, int in_len);
	
  int write_fast_seek_files(enum iav_stream_type stream_type,
								 int stream_id, unsigned char *in, int in_len);
  int write_video_file(struct iav_framedesc *framedesc, int new_frame);
  int flush_frame_desc(void);
  int release_frame_desc(struct iav_framedesc *frame_desc);
  int show_bsb_stats(void);
  int write_stream(u64 *total_frames, u64 *total_bytes);
  int show_waiting(void);
  int map_bsb(void);

  int init_data(void);
  int capture_encoded_video(int run_write_video_file);
  int deinit_data();
};

#endif
