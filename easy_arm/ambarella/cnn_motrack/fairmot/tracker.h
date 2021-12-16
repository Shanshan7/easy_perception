#ifndef _TRACKER_H_
#define _TRACKER_H_

#include "../common/data_struct.h"
#include "../common/utils.h"

#include "fairmot.h"
#include "mot.h"

#define FAIRMOT_INPUT_NAME			"image"
#define FAIRMOT_HEATMAP_NAME		"hm"
#define FAIRMOT_HEATMAP_MAX_NAME	"hmax"
#define FAIRMOT_WH_NAME				"wh"
#define FAIRMOT_REG_NAME			"reg"
#define FAIRMOT_ID_NAME				"id"

#define IMG_DATA_QUEUE_SIZE			3
#define MAX_PATH_STRLEN			    256
#define MAX_LABEL_LEN				128
#define HIGH_RESOLUTION_BEGIN		800

// #define FAIRMOT_MODEL_FILE_NAME		"onnx_fairmot_cavalry.bin"
#define FAIRMOT_MODEL_PATH          "/tmp/onnx_fairmot_cavalry.bin"
EA_LOG_DECLARE_LOCAL(EA_LOG_LEVEL_NOTICE);

typedef struct track_params_s {
	int run_mode;
	int record;
	// char input_dir[MAX_PATH_STRLEN + 1];
	char output_dir[MAX_PATH_STRLEN + 1];

	const char *model_path; // absolute path
	const char *input_node;
	const char *output_heatmap;
	const char *output_heatmap_max;
	const char *output_wh;
	const char *output_reg;
	const char *output_id;
    
	int log_level;
	int draw_mode;
	int canvas_id;
	int stream_id;
	int use_pyramid;
	int pyramid[2];
	int top_k;
	float det_threshold;
	float det_min_area;
	float nms_threshold;
	int measure_time;

	int mot_image_width;
	int mot_image_height;
	int embedding_reliable_use_iou;
	float embedding_reliable_distance;
	float embedding_cosine_distance;
	int overlap_reliable_use_iou;
	float overlap_reliable_distance;
	float overlap_iou_distance;
	int confirming_use_iou;
	float confirming_distance;
	float duplicate_iou_distance;
	int max_lost_age;
	float fuse_lambda;
	int use_match_priority;
	int merge_match_priority;
	int match_priority_thresh;
	float delta_time;
	int use_smooth_feature;
	int feature_buffer_size;
	float smooth_alpha;
	int tentative_thresh;
	int mot_verbose;
} track_params_t;

typedef struct track_ctx_s {
	track_params_t params;

	int sig_flag;
	int loop_count;

	ea_img_resource_t *img_resource;
	fairmot_t fairmot;
	fairmot_result_t net_result;
	mot_t mot;
	mot_result_t mot_result;
	ea_display_t *display;

	FILE *result_file;
	FILE *record_file;

	ea_img_resource_data_t img_data[IMG_DATA_QUEUE_SIZE];
	int head;
	int tail;
	int det_stop_flag;
	pthread_t det_tidp;
} track_ctx_t;

int amba_cv_env_init(track_ctx_t *track_ctx);
int amba_track_init(track_ctx_t *track_ctx, track_params_t *params);
int amba_track_run_loop(track_ctx_t *track_ctx, std::map<int, TrajectoryParams> &track_idx_map);  // track_params_t *params
int amba_draw_detection(std::map<int, TrajectoryParams> track_idx_map, track_ctx_t *track_ctx, uint32_t dsp_pts);
void *det_thread_func(void *arg);
void amba_cv_env_deinit(track_ctx_t *track_ctx);
void amba_track_deinit(track_ctx_t *track_ctx);

#endif // _TRACKER_H_