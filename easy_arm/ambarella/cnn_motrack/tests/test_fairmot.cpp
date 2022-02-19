/*******************************************************************************
 * test_fairmot.c
 *
 * History:
 *  2020/12/14  - [Du You] created
 *
 *
 * Copyright (c) 2020 Ambarella International LP
 *
 * This file and its contents ( "Software" ) are protected by intellectual
 * property rights including, without limitation, U.S. and/or foreign
 * copyrights. This Software is also the confidential and proprietary
 * information of Ambarella International LP and its licensors. You may not use, reproduce,
 * disclose, distribute, modify, or otherwise prepare derivative works of this
 * Software or any portion thereof except pursuant to a signed license agreement
 * or nondisclosure agreement with Ambarella International LP or its authorized affiliates.
 * In the absence of such an agreement, you agree to promptly notify and return
 * this Software to Ambarella International LP.
 *
 * This file includes sample code and is only for internal testing and evaluation.  If you
 * distribute this sample code (whether in source, object, or binary code form), it will be
 * without any warranty or indemnity protection from Ambarella International LP or its affiliates.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF NON-INFRINGEMENT,
 * MERCHANTABILITY, AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL AMBARELLA INTERNATIONAL LP OR ITS AFFILIATES BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; COMPUTER FAILURE OR MALFUNCTION; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************/

#include <config.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <wchar.h>

#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <linux/fb.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <map>
#include <vector>
#include <opencv2/core.hpp>

#include <eazyai.h>

#include "fairmot.h"
#include "mot.h"

#define MAX_PATH_STRLEN			    256
#define MAX_LABEL_LEN				128
#define HIGH_RESOLUTION_BEGIN		800

#define FAIRMOT_INPUT_NAME			"image"
#define FAIRMOT_HEATMAP_NAME		"hm"
#define FAIRMOT_HEATMAP_MAX_NAME	"hmax"
#define FAIRMOT_WH_NAME				"wh"
#define FAIRMOT_REG_NAME			"reg"
#define FAIRMOT_ID_NAME				"id"

#define FAIRMOT_MODEL_FILE_NAME			"onnx_fairmot_cavalry.bin"
#define FAIRMOT_DET_RESULT_FILE_NAME	"result.txt"
#define FAIRMOT_DET_RECORD_FILE_NAME	"record.mot"

#define IMG_DATA_QUEUE_SIZE			3

typedef struct track_idx_params {
	int latest_frame_id=-1;
	int draw_flag=0;
	float mean_velocity;
	std::vector<float> velocity_vector;
	std::vector<float> pedestrian_x_start;
	std::vector<float> pedestrian_y_start;
	std::vector<cv::Point2f> trajectory_position;
} track_idx_params_t;
std::map<int, track_idx_params_t> track_idx_map;

enum {
	DRAW_MODE_VOUT,
	DRAW_MODE_STREAM,
};

enum {
	RUN_MODE_LIVE,
	RUN_MODE_FILE,
	RUN_MODE_REPLAY,
};

typedef struct live_params_s {
	int run_mode;
	int record;
	char input_dir[MAX_PATH_STRLEN + 1];
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
} live_params_t;

typedef struct live_ctx_s {
	live_params_t params;

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
} live_ctx_t;

EA_LOG_DECLARE_LOCAL(EA_LOG_LEVEL_NOTICE);
static live_ctx_t live_ctx;

static int cv_env_init(live_ctx_t *live_ctx, live_params_t *params)
{
	int rval = 0;
	int features = 0;

	do {
		features = EA_ENV_ENABLE_IAV
			| EA_ENV_ENABLE_CAVALRY
			| EA_ENV_ENABLE_VPROC
			| EA_ENV_ENABLE_NNCTRL;
		if (params->run_mode == RUN_MODE_LIVE) {
			if (params->draw_mode == DRAW_MODE_VOUT) {
				features |= EA_ENV_ENABLE_OSD_VOUT;
			} else {
				features |= EA_ENV_ENABLE_OSD_STREAM;
			}
		} else {
			features |= EA_ENV_ENABLE_OSD_JPEG;
		}

		RVAL_OK(ea_env_open(features));

		if (params->run_mode == RUN_MODE_LIVE) {
			if (params->use_pyramid) {
				live_ctx->img_resource = ea_img_resource_new(EA_PYRAMID, (void *)(unsigned long)params->pyramid[0]);
			} else {
				live_ctx->img_resource = ea_img_resource_new(EA_CANVAS, (void *)(unsigned long)params->canvas_id);
			}
		} else {
			live_ctx->img_resource = ea_img_resource_new(EA_JPEG_FOLDER, (void *)params->input_dir);
		}

		RVAL_ASSERT(live_ctx->img_resource != NULL);

		if (params->run_mode == RUN_MODE_LIVE) {
			if (params->draw_mode == DRAW_MODE_VOUT) {
				live_ctx->display = ea_display_new(EA_DISPLAY_VOUT, EA_DISPLAY_ANALOG_VOUT, EA_DISPLAY_BBOX_TEXTBOX, NULL);
			} else {
				live_ctx->display = ea_display_new(EA_DISPLAY_STREAM, params->stream_id, EA_DISPLAY_BBOX_TEXTBOX, NULL);
			}
		} else {
			live_ctx->display = ea_display_new(EA_DISPLAY_JPEG, EA_TENSOR_COLOR_MODE_BGR, EA_DISPLAY_BBOX_TEXTBOX, (void *)params->output_dir);
		}

		RVAL_ASSERT(live_ctx->display != NULL);
	} while(0);

	if (rval < 0) {
		if (live_ctx->display) {
			ea_display_free(live_ctx->display);
			live_ctx->display = NULL;
		}

		if (live_ctx->img_resource) {
			ea_img_resource_free(live_ctx->img_resource);
			live_ctx->img_resource = NULL;
		}
	}

	return rval;
}

static void cv_env_deinit(live_ctx_t *live_ctx, live_params_t *params)
{
	ea_display_free(live_ctx->display);
	live_ctx->display = NULL;
	ea_img_resource_free(live_ctx->img_resource);
	live_ctx->img_resource = NULL;
	ea_env_close();
}

static void calculate_tracking_trajectory(live_ctx_t *live_ctx) {
	// read lost_frame_count
    for (std::map<int, track_idx_params_t>::iterator it = track_idx_map.begin(); it != track_idx_map.end();)
    {
		if (it->second.latest_frame_id != -1 && (live_ctx->mot_result.frame_id - it->second.latest_frame_id) > 3) {
			track_idx_map.erase(it->first);
		}
		else {
			++it;
		}
		it->second.draw_flag = 0;
    }

	// write id in
	for (int i = 0; i < live_ctx->mot_result.valid_track_count; i++) {
		int track_id = live_ctx->mot_result.tracks[i].track_id;
		float x_start = live_ctx->mot_result.tracks[i].x_start;
		float y_start = live_ctx->mot_result.tracks[i].y_start;
		// EA_LOG_NOTICE("id: %d, distancex: %f, distancey: %f", track_id, x_start, y_start);
		cv::Point2f trajectory_position_current = cv::Point2f(
					live_ctx->mot_result.tracks[i].x_start + (live_ctx->mot_result.tracks[i].x_end - live_ctx->mot_result.tracks[i].x_start) / 2,
					live_ctx->mot_result.tracks[i].y_end);

		std::map<int, track_idx_params_t>::iterator iter;
		iter = track_idx_map.find(track_id); 
		if (iter != track_idx_map.end()) {
			EA_LOG_NOTICE("Founded!!!");
			track_idx_map[track_id].trajectory_position.push_back(trajectory_position_current);
			// calculate the latest velocity
			float latest_frame_x_start = track_idx_map[track_id].pedestrian_x_start.back();
			float latest_frame_y_start = track_idx_map[track_id].pedestrian_y_start.back();

			float velocity_current = sqrt(pow((latest_frame_x_start - x_start) * ea_display_obj_params(live_ctx->display)->dis_win_w, 2) + \
			pow((y_start - latest_frame_y_start) * ea_display_obj_params(live_ctx->display)->dis_win_h, 2)) / 0.04 * 0.017; // s per meter m/s

			track_idx_map[track_id].pedestrian_x_start.push_back(live_ctx->mot_result.tracks[i].x_start);
			track_idx_map[track_id].pedestrian_y_start.push_back(live_ctx->mot_result.tracks[i].y_start);
			track_idx_map[track_id].velocity_vector.push_back(velocity_current);
			track_idx_map[track_id].mean_velocity = velocity_current;
			// if (track_idx_map[track_id].velocity_vector.size() < 3) {
			// 	track_idx_map[track_id].mean_velocity = 1.38;
			// }
			// else {

			// }
			track_idx_map[track_id].draw_flag = 1;
			// track_idx_map[track_id].latest_frame_id = live_ctx->mot_result.frame_id;
		}
		else {
			EA_LOG_NOTICE("No Founded!!!");
			track_idx_params_t track_idx_param;
			track_idx_param.draw_flag = 1;
			EA_LOG_NOTICE("Frame id: %d", live_ctx->mot_result.frame_id);
			EA_LOG_NOTICE("latest_frame_id: %d", track_idx_param.latest_frame_id);
			// track_idx_param.latest_frame_id = live_ctx->mot_result.frame_id;
			track_idx_param.pedestrian_x_start.push_back(x_start);
			track_idx_param.pedestrian_y_start.push_back(y_start);
			track_idx_param.velocity_vector.push_back(1.38);
			track_idx_param.mean_velocity = 1.38;
			track_idx_param.trajectory_position.push_back(trajectory_position_current);
			track_idx_map.insert(std::pair<int, track_idx_params_t>(track_id, track_idx_param));
		}
	}
}

static int draw_detection(live_ctx_t *live_ctx, live_params_t *params, const ea_tensor_t *tensor, uint32_t dsp_pts)
{
	int rval = 0;
	char text[MAX_LABEL_LEN];
	int border_thickness;
	int i;

	ea_display_obj_params(live_ctx->display)->obj_win_w = 1.0;
	ea_display_obj_params(live_ctx->display)->obj_win_h = 1.0;
	if (ea_display_obj_params(live_ctx->display)->dis_win_w < HIGH_RESOLUTION_BEGIN) {
		ea_display_obj_params(live_ctx->display)->border_thickness = 2;
		ea_display_obj_params(live_ctx->display)->font_size = 18;
	} else {
		ea_display_obj_params(live_ctx->display)->border_thickness = 5;
		ea_display_obj_params(live_ctx->display)->font_size = 32;
	}

	ea_display_obj_params(live_ctx->display)->text_color = EA_16_COLORS_WHITE;

	do {
		EA_LOG_NOTICE("Frame & Counted");
		snprintf(text, MAX_LABEL_LEN, "Frame %d", live_ctx->mot_result.frame_id, live_ctx->mot_result.valid_track_count);
		border_thickness = ea_display_obj_params(live_ctx->display)->border_thickness;
		ea_display_obj_params(live_ctx->display)->border_thickness = 0;
		ea_display_set_textbox(live_ctx->display, text, 20 / ea_display_obj_params(live_ctx->display)->dis_win_w, 
		20 / ea_display_obj_params(live_ctx->display)->dis_win_h,
			ea_display_obj_params(live_ctx->display)->font_size * strlen(text) * 2 / ea_display_obj_params(live_ctx->display)->dis_win_w,
			ea_display_obj_params(live_ctx->display)->font_size * 2 / ea_display_obj_params(live_ctx->display)->dis_win_h);

		snprintf(text, MAX_LABEL_LEN, "Counted %d", live_ctx->mot_result.valid_track_count);
		ea_display_obj_params(live_ctx->display)->border_thickness = 0;
		ea_display_set_textbox(live_ctx->display, text, 
		    20 / ea_display_obj_params(live_ctx->display)->dis_win_w, 60 / ea_display_obj_params(live_ctx->display)->dis_win_h,
			ea_display_obj_params(live_ctx->display)->font_size * strlen(text) * 2 / ea_display_obj_params(live_ctx->display)->dis_win_w,
			ea_display_obj_params(live_ctx->display)->font_size * 2 / ea_display_obj_params(live_ctx->display)->dis_win_h);

		// calculate person attribute
		EA_LOG_NOTICE("calculate person attribute");
		calculate_tracking_trajectory(live_ctx);

		// draw person box
		EA_LOG_NOTICE("draw person box");
		ea_display_obj_params(live_ctx->display)->border_thickness = border_thickness;
		ea_display_obj_params(live_ctx->display)->font_size = 8;
		for (i = 0; i < live_ctx->mot_result.valid_track_count; i++) {
			if (params->record) {
				snprintf(text, MAX_LABEL_LEN, "%d x=%.04f", live_ctx->mot_result.tracks[i].track_id, live_ctx->mot_result.tracks[i].x_start);
			} else {
				snprintf(text, MAX_LABEL_LEN, "ID %d V %.02f", live_ctx->mot_result.tracks[i].track_id, 
				live_ctx->mot_result.tracks[i].x_start); // live_ctx->mot_result.tracks[i].x_start
			}

			ea_display_obj_params(live_ctx->display)->box_color = (ea_16_colors_t)(live_ctx->mot_result.tracks[i].track_id % EA_16_COLORS_MAX_NUM);
			ea_display_set_bbox(live_ctx->display, text,
				live_ctx->mot_result.tracks[i].x_start, live_ctx->mot_result.tracks[i].y_start,
				live_ctx->mot_result.tracks[i].x_end - live_ctx->mot_result.tracks[i].x_start,
				live_ctx->mot_result.tracks[i].y_end - live_ctx->mot_result.tracks[i].y_start);
		}

		// draw ID trajectory
		// for (std::map<int, track_idx_params_t>::iterator it = track_idx_map.begin(); it != track_idx_map.end(); ++it)
		// {
		// 	if(it->second.draw_flag == 1) {
		// 		ea_display_obj_params(live_ctx->display)->border_thickness = 2;
		// 		ea_display_obj_params(live_ctx->display)->box_color = (ea_16_colors_t)(it->first % EA_16_COLORS_MAX_NUM);
		// 		snprintf(text, MAX_LABEL_LEN, "");
		// 		// EA_LOG_NOTICE("ID: %d, position_size: %d", it->first, it->second.trajectory_position.size());
		// 		for (int j = 0; j < it->second.trajectory_position.size(); j++) {
		// 			// EA_LOG_NOTICE("ID: %d, position_size_x: %f, position_size_y: %f", it->first, it->second.trajectory_position[j].x,
		// 			// it->second.trajectory_position[j].y);
		// 			if (it->second.trajectory_position[j].x <= 1.0 - 2 / ea_display_obj_params(live_ctx->display)->dis_win_w&& 
		// 				it->second.trajectory_position[j].y <= 1.0 - 2 / ea_display_obj_params(live_ctx->display)->dis_win_h) {
		// 				ea_display_set_bbox(live_ctx->display, text,
		// 				it->second.trajectory_position[j].x, it->second.trajectory_position[j].y,
		// 				2 / ea_display_obj_params(live_ctx->display)->dis_win_w,
		// 				2 / ea_display_obj_params(live_ctx->display)->dis_win_h);
		// 			}
		// 		}
		// 	}
		// }

		if (params->run_mode == RUN_MODE_LIVE) {
			ea_display_refresh(live_ctx->display, (void *)(unsigned long)dsp_pts);
		} else {
			ea_display_refresh(live_ctx->display, (void *)tensor/*tensor*/);
		}
	} while (0);

	return rval;
}

static int save_result(live_ctx_t *live_ctx)
{
	int rval = 0;
	int i;

	if (live_ctx->mot_result.valid_track_count > 0) {
		for (i = 0; i < live_ctx->mot_result.valid_track_count; i++) {
			fprintf(live_ctx->result_file, "%d %f %f %f %f %d\n",
				live_ctx->mot_result.tracks[i].class_id,
				live_ctx->mot_result.tracks[i].x_start,
				live_ctx->mot_result.tracks[i].y_start,
				live_ctx->mot_result.tracks[i].x_end,
				live_ctx->mot_result.tracks[i].y_end,
				live_ctx->mot_result.tracks[i].track_id);
		}

		fprintf(live_ctx->result_file, "\n");
	}

	return rval;
}

static void *det_thread_func(void *arg)
{
	int rval = 0;
	live_ctx_t *ctx = (live_ctx_t *)arg;
	live_params_t *params = &ctx->params;
	ea_img_resource_data_t data;

	do {
		RVAL_OK(ea_img_resource_hold_data(ctx->img_resource, &data));
		// save resource image
		// live_ctx->display = ea_display_new(EA_DISPLAY_JPEG, EA_COLOR_YUV2RGB_NV12, EA_DISPLAY_BBOX_TEXTBOX, (void *)fairmot_input(&ctx->fairmot));

		if (params->run_mode == RUN_MODE_LIVE) {
			RVAL_ASSERT(data.tensor_group != NULL);
			RVAL_ASSERT(data.tensor_num >= 1);
			if (params->use_pyramid) {
				RVAL_ASSERT(data.tensor_group[params->pyramid[1]] != NULL);
				RVAL_OK(ea_cvt_color_resize(data.tensor_group[params->pyramid[1]], fairmot_input(&ctx->fairmot), EA_COLOR_YUV2RGB_NV12, EA_VP));
			} else {
				RVAL_ASSERT(data.tensor_group[0] != NULL);
				RVAL_OK(ea_cvt_color_resize(data.tensor_group[0], fairmot_input(&ctx->fairmot), EA_COLOR_YUV2RGB_NV12, EA_VP));
			}
		} else {
			if (data.tensor_group != NULL) {
				RVAL_ASSERT(data.tensor_group[0] != NULL);
				RVAL_OK(ea_cvt_color_resize(data.tensor_group[0], fairmot_input(&ctx->fairmot), EA_COLOR_BGR2RGB, EA_VP));
			}
		}

		ctx->img_data[ctx->tail] = data;
		ctx->tail++;
		if (ctx->tail == IMG_DATA_QUEUE_SIZE) {
			ctx->tail = 0;
		}
#if 0	// save image data in tensor to JPEG file for debug use
		if (data.tensor_group) {
			//RVAL_OK(ea_tensor_to_jpeg(data.tensor_group[0], EA_TENSOR_COLOR_MODE_YUV_NV12, "/sdcard/mmcblk1/fairmot/out/src.jpg"));
			//RVAL_OK(ea_tensor_to_jpeg(fairmot_input(&ctx->fairmot), EA_TENSOR_COLOR_MODE_RGB, "/sdcard/mmcblk1/fairmot/out/input.jpg"));
		}
#endif
		RVAL_OK(fairmot_vp_forward_in_parallel(&ctx->fairmot));
	} while (ctx->det_stop_flag == 0);

	return (void *)(unsigned long)rval;
}

#define	NO_ARG	0
#define	HAS_ARG	1

enum {
	OPTION_MODEL_PATH,
	OPTION_INPUT_NODE,
	OPTION_OUTPUT_HEAPMAP,
	OPTION_OUTPUT_HEATMAP_MAX,
	OPTION_OUTPUT_WH,
	OPTION_OUTPUT_REG,
	OPTION_OUTPUT_ID,
	OPTION_MOT_IMAGE_WIDTH,
	OPTION_MOT_IMAGE_HEIGHT,
	OPTION_EMBEDDING_RELIABLE_USE_IOU,
	OPTION_EMBEDDING_RELIABLE_DISTANCE,
	OPTION_EMBEDDING_COSINE_DISTANCE,
	OPTION_OVERLAP_RELIABLE_USE_IOU,
	OPTION_OVERLAP_RELIABLE_DISTANCE,
	OPTION_OVERLAP_IOU_DISTANCE,
	OPTION_CONFIRMING_USE_IOU,
	OPTION_CONFIRMING_DISTANCE,
	OPTION_DUPLICATE_IOU_DISTANCE,
	OPTION_MAX_LOST_AGE,
	OPTION_FUSE_LAMBDA,
	OPTION_USE_MATCH_PRIORITY,
	OPTION_MERGE_MATCH_PRIORITY,
	OPTION_MATCH_PRIORITY_THRESH,
	OPTION_DELTA_TIME,
	OPTION_USE_SMOOTH_FEATURE,
	OPTION_FEATURE_BUFFER_SIZE,
	OPTION_SMOOTH_ALPHA,
	OPTION_TENTATIVE_THRESH,
	OPTION_MOT_VERBOSE,
};

static struct option long_options[] = {
	{"run_mode", HAS_ARG, 0, 'm'},
	{"record", HAS_ARG, 0, 'r'},
	{"input_dir", HAS_ARG, 0, 'i'},
	{"output_dir", HAS_ARG, 0, 'o'},
	{"log_level", HAS_ARG, 0, 'l'},
	{"draw_mode", HAS_ARG, 0, 'a'},
	{"canvas_id", HAS_ARG, 0, 'b'},
	{"stream_id", HAS_ARG, 0, 's'},
	{"use_pyramid", HAS_ARG, 0, 'k'},
	{"det_threshold", HAS_ARG, 0, 'f'},
	{"det_min_area", HAS_ARG, 0, 'e'},
	{"nms_threshold", HAS_ARG, 0, 'n'},
	{"top_k", HAS_ARG, 0, 'p'},
	{"measure_time", HAS_ARG, 0, 't'},
	{"model_path", HAS_ARG, 0, OPTION_MODEL_PATH},
	{"input_node", HAS_ARG, 0, OPTION_INPUT_NODE},
	{"output_heatmap", HAS_ARG, 0, OPTION_OUTPUT_HEAPMAP},
	{"output_heatmap_max", HAS_ARG, 0, OPTION_OUTPUT_HEATMAP_MAX},
	{"output_wh", HAS_ARG, 0, OPTION_OUTPUT_WH},
	{"output_reg", HAS_ARG, 0, OPTION_OUTPUT_REG},
	{"output_id", HAS_ARG, 0, OPTION_OUTPUT_ID},
	{"mot_image_width", HAS_ARG, 0, OPTION_MOT_IMAGE_WIDTH},
	{"mot_image_height", HAS_ARG, 0, OPTION_MOT_IMAGE_HEIGHT},
	{"embedding_reliable_use_iou", HAS_ARG, 0, OPTION_EMBEDDING_RELIABLE_USE_IOU},
	{"embedding_reliable_distance", HAS_ARG, 0, OPTION_EMBEDDING_RELIABLE_DISTANCE},
	{"embedding_cosine_distance", HAS_ARG, 0, OPTION_EMBEDDING_COSINE_DISTANCE},
	{"overlap_reliable_use_iou", HAS_ARG, 0, OPTION_OVERLAP_RELIABLE_USE_IOU},
	{"overlap_reliable_distance", HAS_ARG, 0, OPTION_OVERLAP_RELIABLE_DISTANCE},
	{"overlap_iou_distance", HAS_ARG, 0, OPTION_OVERLAP_IOU_DISTANCE},
	{"confirming_use_iou", HAS_ARG, 0, OPTION_CONFIRMING_USE_IOU},
	{"confirming_distance", HAS_ARG, 0, OPTION_CONFIRMING_DISTANCE},
	{"duplicate_iou_distance", HAS_ARG, 0, OPTION_DUPLICATE_IOU_DISTANCE},
	{"max_lost_age", HAS_ARG, 0, OPTION_MAX_LOST_AGE},
	{"fuse_lambda", HAS_ARG, 0, OPTION_FUSE_LAMBDA},
	{"use_match_priority", HAS_ARG, 0, OPTION_USE_MATCH_PRIORITY},
	{"merge_match_priority", HAS_ARG, 0, OPTION_MERGE_MATCH_PRIORITY},
	{"match_priority_thresh", HAS_ARG, 0, OPTION_MATCH_PRIORITY_THRESH},
	{"delta_time", HAS_ARG, 0, OPTION_DELTA_TIME},
	{"use_smooth_feature", HAS_ARG, 0, OPTION_USE_SMOOTH_FEATURE},
	{"feature_buffer_size", HAS_ARG, 0, OPTION_FEATURE_BUFFER_SIZE},
	{"smooth_alpha", HAS_ARG, 0, OPTION_SMOOTH_ALPHA},
	{"tentative_thresh", HAS_ARG, 0, OPTION_TENTATIVE_THRESH},
	{"mot_verbose", HAS_ARG, 0, OPTION_MOT_VERBOSE},
	{0, 0, 0, 0}
};

static const char *short_options = "m:r:i:o:l:a:b:s:k:f:e:n:p:t:";

struct hint_s {
	const char *arg;
	const char *str;
};

static const struct hint_s hint[] = {
	{"", "\t\t\trun mode 0=live, 1=file, 2=replay."},
	{"", "\t\t\trecord detections for replaying or not."},
	{"", "\t\t\tpath to folder containing model cavalry binary, record file and JEPG images."},
	{"", "\t\t\tpath to folder saving output files."},
	{"", "\t\t\tlog level 0=None, 1=Error, 2=Notice, 3=Debug, 4=Verbose."},
	{"", "\t\t\tdraw mode, 0=draw on VOUT, 1=draw on stream."},
	{"", "\t\t\tYUV canvas ID."},
	{"", "\t\t\tstream ID to draw."},
	{"", "\t\tpyramid parameters, in order of channel_id,pyramid_det_layer."},
	{"", "\t\tdetection threshold."},
	{"", "\t\tdetection min area."},
	{"", "\t\tnms threshold."},
	{"", "\t\t\ttop k on each class."},
	{"", "\t\tmeasure time."},
	{"", "\t\t\tpath of the model."},
	{"", "\t\t\tinput node name."},
	{"", "\t\toutput node name of heatmap."},
	{"", "\t\toutput node name of heatmap max."},
	{"", "\t\t\toutput node name of WH."},
	{"", "\t\t\toutput node name of REG."},
	{"", "\t\t\toutput node name of ID."},
	{"", "\t\tmot image width."},
	{"", "\t\tmot image height."},
	{"", "\tembedding reliable use iou."},
	{"", "embedding reliable distance."},
	{"", "\tembedding cosine distance."},
	{"", "\toverlap reliable use iou."},
	{"", "\toverlap reliable distance."},
	{"", "\toverlap iou distance."},
	{"", "\t\tconfirming use iou."},
	{"", "\tconfirming distance."},
	{"", "\tduplicate iou distance."},
	{"", "\t\tmax lost age."},
	{"", "\t\tfuse lambda."},
	{"", "\t\tuse match priority."},
	{"", "\tmerge match priority."},
	{"", "\tmatch priority thresh."},
	{"", "\t\t\tdelta time."},
	{"", "\t\tuse smooth feature."},
	{"", "\tfeature buffer size."},
	{"", "\t\tsmooth alpha."},
	{"", "\t\ttentative threshold."},
	{"", "\t\tmot verbose."},
};

static void usage(void)
{
	printf("test_fairmot usage:\n");
	for (size_t i = 0; i < sizeof(long_options) / sizeof(long_options[0]) - 1; i++) {
		if (isalpha(long_options[i].val)) {
			printf("-%c ", long_options[i].val);
		} else {
			printf("   ");
		}
		printf("--%s", long_options[i].name);
		if (hint[i].arg[0] != 0)
			printf(" [%s]", hint[i].arg);
		printf("\t%s\n", hint[i].str);
	}
}

static int parse_param(int argc, char **argv, live_params_t *params)
{
	int rval = 0;
	int ch;
	int option_index = 0;
	int value;

	do {
		opterr = 0;
		while ((ch = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
			switch (ch) {
			case 'm':
				params->run_mode = atoi(optarg);
				if (params->run_mode != RUN_MODE_LIVE
					&& params->run_mode != RUN_MODE_FILE
					&& params->run_mode != RUN_MODE_REPLAY) {
					printf("run_mode is wrong, %s\n", optarg);
					return -1;
				}
				break;
			case 'r':
				params->record = atoi(optarg);
				if (params->record != 0 && params->record != 1) {
					printf("record is wrong, %s\n", optarg);
					return -1;
				}
				break;
			case 'i':
				value = strlen(optarg);
				if (value == 0) {
					printf("input_dir is empty\n");
					rval = -1;
					break;
				}

				if (value > MAX_PATH_STRLEN) {
					printf("input_dir should be no more than %d characters, %s\n", MAX_PATH_STRLEN, optarg);
					rval = -1;
					break;
				}

				snprintf(params->input_dir, sizeof(params->input_dir), "%s", optarg);
				if (optarg[value - 1] != '/') {
					strncat(params->input_dir, "/", MAX_PATH_STRLEN - strlen(params->input_dir));
				}
				break;
			case 'o':
				value = strlen(optarg);
				if (value == 0) {
					printf("output_dir is empty\n");
					rval = -1;
					break;
				}

				if (value > MAX_PATH_STRLEN) {
					printf("output_dir should be no more than %d characters, %s\n", MAX_PATH_STRLEN, optarg);
					rval = -1;
					break;
				}

				snprintf(params->output_dir, sizeof(params->output_dir), "%s", optarg);
				if (optarg[value - 1] != '/') {
					strncat(params->output_dir, "/", MAX_PATH_STRLEN - strlen(params->output_dir));
				}
				break;
			case 'l':
				params->log_level = atoi(optarg);
				if (params->log_level < EA_LOG_LEVEL_NONE || params->log_level > EA_LOG_LEVEL_VERBOSE) {
					printf("log level is wrong, %d\n", params->log_level);
					return -1;
				}
				break;
			case 'a':
				params->draw_mode = atoi(optarg);
				if (params->draw_mode < 0) {
					printf("draw mode is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case 'b':
				params->canvas_id = atoi(optarg);
				if (params->canvas_id < 0) {
					printf("YUV canvas id is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case 's':
				params->stream_id = atoi(optarg);
				if (params->stream_id < 0) {
					printf("stream id is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case 'k':
				value = sscanf(optarg, "%d,%d",
					&params->pyramid[0], &params->pyramid[1]);
				if (value != 2) {
					printf("pyramid parameters are wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				params->use_pyramid = 1;
				break;
			case 'f':
				value = sscanf(optarg, "%f",
					&params->det_threshold);
				if (value != 1) {
					printf("det_threshold is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case 'e':
				value = sscanf(optarg, "%f",
					&params->det_min_area);
				if (value != 1) {
					printf("det_min_area is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case 'n':
				value = sscanf(optarg, "%f",
					&params->nms_threshold);
				if (value != 1) {
					printf("nms_threshold is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case 'p':
				params->top_k = atoi(optarg);
				if (params->top_k < 0) {
					printf("top k is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case 't':
				params->measure_time = atoi(optarg);
				if (params->measure_time != 0 && params->measure_time != 1) {
					printf("measure_time is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;

			case OPTION_MODEL_PATH:
				value = strlen(optarg);
				if (value == 0) {
					printf("model_path is empty\n");
					rval = -1;
					break;
				}

				params->model_path = optarg;
				break;
			case OPTION_INPUT_NODE:
				value = strlen(optarg);
				if (value == 0) {
					printf("input node name is empty\n");
					rval = -1;
					break;
				}

				params->input_node = optarg;
				break;
			case OPTION_OUTPUT_HEAPMAP:
				value = strlen(optarg);
				if (value == 0) {
					printf("output name of heatmap is empty\n");
					rval = -1;
					break;
				}

				params->output_heatmap = optarg;
				break;
			case OPTION_OUTPUT_HEATMAP_MAX:
				value = strlen(optarg);
				if (value == 0) {
					printf("output name of heatmap max is empty\n");
					rval = -1;
					break;
				}

				params->output_heatmap_max = optarg;
				break;
			case OPTION_OUTPUT_WH:
				value = strlen(optarg);
				if (value == 0) {
					printf("output name of WH is empty\n");
					rval = -1;
					break;
				}

				params->output_wh = optarg;
				break;
			case OPTION_OUTPUT_REG:
				value = strlen(optarg);
				if (value == 0) {
					printf("output name of REG is empty\n");
					rval = -1;
					break;
				}

				params->output_reg = optarg;
				break;
			case OPTION_OUTPUT_ID:
				value = strlen(optarg);
				if (value == 0) {
					printf("output name of ID is empty\n");
					rval = -1;
					break;
				}

				params->output_id = optarg;
				break;
			case OPTION_MOT_IMAGE_WIDTH:
				params->mot_image_width = atoi(optarg);
				if (params->mot_image_width <= 0) {
					printf("mot_image_width is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_MOT_IMAGE_HEIGHT:
				params->mot_image_height = atoi(optarg);
				if (params->mot_image_height <= 0) {
					printf("mot_image_height is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_EMBEDDING_RELIABLE_USE_IOU:
				params->embedding_reliable_use_iou = atoi(optarg);
				if (params->embedding_reliable_use_iou != 0
					&& params->embedding_reliable_use_iou != 1) {
					printf("embedding_reliable_use_iou is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_EMBEDDING_RELIABLE_DISTANCE:
				value = sscanf(optarg, "%f",
					&params->embedding_reliable_distance);
				if (value != 1) {
					printf("embedding_reliable_distance is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_EMBEDDING_COSINE_DISTANCE:
				value = sscanf(optarg, "%f",
					&params->embedding_cosine_distance);
				if (value != 1) {
					printf("embedding_cosine_distance is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_OVERLAP_RELIABLE_USE_IOU:
				params->overlap_reliable_use_iou = atoi(optarg);
				if (params->overlap_reliable_use_iou != 0
					&& params->overlap_reliable_use_iou != 1) {
					printf("overlap_reliable_use_iou is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_OVERLAP_RELIABLE_DISTANCE:
				value = sscanf(optarg, "%f",
					&params->overlap_reliable_distance);
				if (value != 1) {
					printf("overlap_reliable_distance is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_OVERLAP_IOU_DISTANCE:
				value = sscanf(optarg, "%f",
					&params->overlap_iou_distance);
				if (value != 1) {
					printf("overlap_iou_distance is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_CONFIRMING_USE_IOU:
				params->confirming_use_iou = atoi(optarg);
				if (params->confirming_use_iou != 0
					&& params->confirming_use_iou != 1) {
					printf("confirming_use_iou is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_CONFIRMING_DISTANCE:
				value = sscanf(optarg, "%f",
					&params->confirming_distance);
				if (value != 1) {
					printf("confirming_distance is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_DUPLICATE_IOU_DISTANCE:
				value = sscanf(optarg, "%f",
					&params->duplicate_iou_distance);
				if (value != 1) {
					printf("duplicate_iou_distance is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_MAX_LOST_AGE:
				params->max_lost_age = atoi(optarg);
				if (params->max_lost_age <= 0) {
					printf("max_lost_age is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_FUSE_LAMBDA:
				value = sscanf(optarg, "%f",
					&params->fuse_lambda);
				if (value != 1) {
					printf("fuse_lambda is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_USE_MATCH_PRIORITY:
				params->use_match_priority = atoi(optarg);
				if (params->use_match_priority != 0
					&& params->use_match_priority != 1) {
					printf("use_match_priority is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_MERGE_MATCH_PRIORITY:
				params->merge_match_priority = atoi(optarg);
				if (params->merge_match_priority != 0
					&& params->merge_match_priority != 1) {
					printf("merge_match_priority is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_MATCH_PRIORITY_THRESH:
				params->match_priority_thresh = atoi(optarg);
				if (params->match_priority_thresh <= 0) {
					printf("match_priority_thresh is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_DELTA_TIME:
				value = sscanf(optarg, "%f",
					&params->delta_time);
				if (value != 1) {
					printf("delta_time is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_USE_SMOOTH_FEATURE:
				params->use_smooth_feature = atoi(optarg);
				if (params->use_smooth_feature != 0
					&& params->use_smooth_feature != 1) {
					printf("use_smooth_feature is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_FEATURE_BUFFER_SIZE:
				params->feature_buffer_size = atoi(optarg);
				if (params->feature_buffer_size <= 0) {
					printf("feature_buffer_size is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_SMOOTH_ALPHA:
				value = sscanf(optarg, "%f",
					&params->smooth_alpha);
				if (value != 1) {
					printf("smooth_alpha is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_TENTATIVE_THRESH:
				params->tentative_thresh = atoi(optarg);
				if (params->tentative_thresh <= 0) {
					printf("mot_image_width is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			case OPTION_MOT_VERBOSE:
				params->mot_verbose = atoi(optarg);
				if (params->mot_verbose != 0
					&& params->mot_verbose != 1) {
					printf("mot_verbose is wrong, %s\n", optarg);
					rval = -1;
					break;
				}
				break;
			default:
				printf("unknown option found: %d, %c\n", ch, ch);
				rval = -1;
				break;
			}
		}
	} while (0);

	return rval;
}

static int init_param(int argc, char **argv, live_params_t *params)
{
	int rval = 0;

	memset(params, 0, sizeof(live_params_t));

	params->run_mode = RUN_MODE_LIVE;
	params->record = 0;
	params->log_level = EA_LOG_LEVEL_NOTICE;
	params->draw_mode = 0;
	params->canvas_id = 1;
	params->stream_id = 0;
	params->det_threshold = 0.45f;
	params->det_min_area = 0.0038f;
	params->nms_threshold = 0.7f;
	params->top_k = 50;
	params->measure_time = 0;
	params->mot_image_width = 1920;
	params->mot_image_height = 1080;
	params->embedding_reliable_use_iou = 0;
	params->embedding_reliable_distance = 0.2f;
	params->embedding_cosine_distance = 0.35f;
	params->overlap_reliable_use_iou = 1;
	params->overlap_reliable_distance = 0.3f;
	params->overlap_iou_distance = 0.6f;
	params->confirming_use_iou = 1;
	params->confirming_distance = 0.6f;
	params->duplicate_iou_distance = 0.1f;
	params->max_lost_age = 35;
	params->fuse_lambda = 0.99f;
	params->use_match_priority = 0;
	params->merge_match_priority = 0;
	params->match_priority_thresh = 6;
	params->delta_time = 0.1f;
	params->use_smooth_feature = 0;
	params->feature_buffer_size = 20;
	params->smooth_alpha = 0.5f;
	params->tentative_thresh = 10;
	params->mot_verbose = 0;

	do {
		if (argc < 2) {
			usage();
			exit(0);
		} else {
			rval = parse_param(argc, argv, params);
			if (rval < 0) {
				break;
			}
		}

		EA_LOG_SET_LOCAL(params->log_level);

		EA_LOG_NOTICE("live parameters:\n");
		EA_LOG_NOTICE("\trun_mode: %d (0=live, 1=file, 2=replay)\n", params->run_mode);
		EA_LOG_NOTICE("\trecord: %d (0=not record, 1=record)\n", params->record);
		EA_LOG_NOTICE("\tinput_dir path: %s\n", params->input_dir);
		EA_LOG_NOTICE("\toutput_dir path: %s\n", params->output_dir);
		EA_LOG_NOTICE("\tlog level: %d (0=None, 1=Error, 2=Notice, 3=Debug, 4=Verbose)\n", params->log_level);
		EA_LOG_NOTICE("\tdraw mode: %d (0=VOUT, 1=stream)\n", params->draw_mode);
		EA_LOG_NOTICE("\tYUV canvas ID: %d\n", params->canvas_id);
		EA_LOG_NOTICE("\tstream ID: %d\n", params->stream_id);
		EA_LOG_NOTICE("\tuse pyramid: %d, channel_id: %d, pyramid_det_layer: %d\n",
			params->use_pyramid, params->pyramid[0], params->pyramid[1]);
		EA_LOG_NOTICE("\tdet_threshold: %f\n", params->det_threshold);
		EA_LOG_NOTICE("\tdet_min_area: %f\n", params->det_min_area);
		EA_LOG_NOTICE("\tnms_threshold: %f\n", params->nms_threshold);
		EA_LOG_NOTICE("\ttop k: %d\n", params->top_k);
		EA_LOG_NOTICE("\tmeasure time: %d\n", params->measure_time);
		EA_LOG_NOTICE("\tmot_image_width: %d\n", params->mot_image_width);
		EA_LOG_NOTICE("\tmot_image_height: %d\n", params->mot_image_height);
		EA_LOG_NOTICE("\tembedding_reliable_use_iou: %d\n", params->embedding_reliable_use_iou);
		EA_LOG_NOTICE("\tembedding_reliable_distance: %f\n", params->embedding_reliable_distance);
		EA_LOG_NOTICE("\tembedding_cosine_distance: %f\n", params->embedding_cosine_distance);
		EA_LOG_NOTICE("\toverlap_reliable_use_iou: %d\n", params->overlap_reliable_use_iou);
		EA_LOG_NOTICE("\toverlap_reliable_distance: %f\n", params->overlap_reliable_distance);
		EA_LOG_NOTICE("\toverlap_iou_distance: %f\n", params->overlap_iou_distance);
		EA_LOG_NOTICE("\tconfirming_use_iou: %d\n", params->confirming_use_iou);
		EA_LOG_NOTICE("\tconfirming_distance: %f\n", params->confirming_distance);
		EA_LOG_NOTICE("\tduplicate_iou_distance: %f\n", params->duplicate_iou_distance);
		EA_LOG_NOTICE("\tmax_lost_age: %d\n", params->max_lost_age);
		EA_LOG_NOTICE("\tfuse_lambda: %f\n", params->fuse_lambda);
		EA_LOG_NOTICE("\tuse_match_priority: %d\n", params->use_match_priority);
		EA_LOG_NOTICE("\tmerge_match_priority: %d\n", params->merge_match_priority);
		EA_LOG_NOTICE("\tmatch_priority_thresh: %d\n", params->match_priority_thresh);
		EA_LOG_NOTICE("\tdelta_time: %f\n", params->delta_time);
		EA_LOG_NOTICE("\tuse_smooth_feature: %d\n", params->use_smooth_feature);
		EA_LOG_NOTICE("\tfeature_buffer_size: %d\n", params->feature_buffer_size);
		EA_LOG_NOTICE("\tsmooth_alpha: %f\n", params->smooth_alpha);
		EA_LOG_NOTICE("\ttentative_thresh: %d\n", params->tentative_thresh);
		EA_LOG_NOTICE("\tmot_verbose: %d\n", params->mot_verbose);
		EA_LOG_NOTICE("\tmodel path: %s\n", params->model_path);
		EA_LOG_NOTICE("\tinput_node: %s\n", params->input_node);
		EA_LOG_NOTICE("\toutput_heatmap: %s\n", params->output_heatmap);
		EA_LOG_NOTICE("\toutput_heatmap_max: %s\n", params->output_heatmap_max);
		EA_LOG_NOTICE("\toutput_wh: %s\n", params->output_wh);
		EA_LOG_NOTICE("\toutput_reg: %s\n", params->output_reg);
		EA_LOG_NOTICE("\toutput_id: %s\n", params->output_id);

		if (strlen(params->input_dir) == 0) {
			if (strlen(params->model_path) == 0) {
				EA_LOG_ERROR("input dir should not be empty when model path is not set\n");
				rval = -1;
				break;
			}
		}

		if ((params->run_mode != RUN_MODE_REPLAY && params->record) || params->run_mode == RUN_MODE_FILE) {
			if (strlen(params->output_dir) == 0) {
				EA_LOG_ERROR("output dir should not be empty when in run mode 1 or when record is enabled\n");
				rval = -1;
				break;
			}
		}
	} while (0);

	return rval;
}

static int live_init(live_ctx_t *live_ctx, live_params_t *params)
{
	int rval = 0;
	fairmot_params_t net_params;
	mot_params_t mot_params;
	char model_path[MAX_PATH_STRLEN + 1] = {0};
	char file_path[MAX_PATH_STRLEN + 1] = {0};

	do {
		RVAL_OK(cv_env_init(live_ctx, params));

		memset(&net_params, 0, sizeof(fairmot_params_t));
		net_params.log_level = params->log_level;
		snprintf(model_path, sizeof(model_path), "%s", params->input_dir);
		strncat(model_path, FAIRMOT_MODEL_FILE_NAME, MAX_PATH_STRLEN - strlen(model_path));

		if (params->model_path == NULL) {
			net_params.model_path = model_path;
		} else {
			net_params.model_path = params->model_path;
		}

		if (params->input_node == NULL) {
			net_params.input_name = FAIRMOT_INPUT_NAME;
		} else {
			net_params.input_name = params->input_node;
		}

		if (params->output_heatmap == NULL) {
			net_params.hm_name = FAIRMOT_HEATMAP_NAME;
		} else {
			net_params.hm_name = params->output_heatmap;
		}

		if (params->output_heatmap_max == NULL) {
			net_params.hmax_name = FAIRMOT_HEATMAP_MAX_NAME;
		} else {
			net_params.hmax_name = params->output_heatmap_max;
		}

		if (params->output_wh == NULL) {
			net_params.wh_name = FAIRMOT_WH_NAME;
		} else {
			net_params.wh_name = params->output_wh;
		}

		if (params->output_reg == NULL) {
			net_params.reg_name = FAIRMOT_REG_NAME;
		} else {
			net_params.reg_name = params->output_reg;
		}

		if (params->output_id == NULL) {
			net_params.id_name = FAIRMOT_ID_NAME;
		} else {
			net_params.id_name = params->output_id;
		}

		net_params.det_threshold = params->det_threshold;
		net_params.det_min_area = params->det_min_area;
		net_params.nms_threshold = params->nms_threshold;
		net_params.keep_top_k = params->top_k;
		net_params.measure_time = params->measure_time;
		RVAL_OK(fairmot_init(&live_ctx->fairmot, &net_params));
		memset(&mot_params, 0, sizeof(mot_params_t));
		mot_params.log_level = params->log_level;
		mot_params.measure_time = params->measure_time;
		mot_params.image_width = params->mot_image_width;
		mot_params.image_height = params->mot_image_height;
		mot_params.embedding_reliable_use_iou = params->embedding_reliable_use_iou;
		mot_params.embedding_reliable_distance = params->embedding_reliable_distance;
		mot_params.embedding_cosine_distance = params->embedding_cosine_distance;
		mot_params.overlap_reliable_use_iou = params->overlap_reliable_use_iou;
		mot_params.overlap_reliable_distance = params->overlap_reliable_distance;
		mot_params.overlap_iou_distance = params->overlap_iou_distance;
		mot_params.confirming_use_iou = params->confirming_use_iou;
		mot_params.confirming_distance = params->confirming_distance;
		mot_params.duplicate_iou_distance = params->duplicate_iou_distance;
		mot_params.max_lost_age = params->max_lost_age;
		mot_params.fuse_lambda = params->fuse_lambda;
		mot_params.use_match_priority = params->use_match_priority;
		mot_params.merge_match_priority = params->merge_match_priority;
		mot_params.match_priority_thresh = params->match_priority_thresh;
		mot_params.dt = params->delta_time;
		mot_params.use_smooth_feature = params->use_smooth_feature;
		mot_params.feature_buffer_size = params->feature_buffer_size;
		mot_params.smooth_alpha = params->smooth_alpha;
		mot_params.tentative_thresh = params->tentative_thresh;
		mot_params.verbose = params->mot_verbose;
		RVAL_OK(mot_init(&live_ctx->mot, &mot_params));

		if (params->run_mode == RUN_MODE_FILE) {
			snprintf(file_path, sizeof(file_path), "%s", params->output_dir);
			strncat(file_path, FAIRMOT_DET_RESULT_FILE_NAME, MAX_PATH_STRLEN - strlen(file_path));
			live_ctx->result_file = fopen(file_path, "w");
			if (live_ctx->result_file == NULL) {
				EA_LOG_ERROR("fopen() failed on %s\n", file_path);
				rval = -1;
				break;
			}
		}

		if (params->run_mode == RUN_MODE_REPLAY) {
			snprintf(file_path, sizeof(file_path), "%s", params->input_dir);
			strncat(file_path, FAIRMOT_DET_RECORD_FILE_NAME, MAX_PATH_STRLEN - strlen(file_path));
			live_ctx->record_file = fopen(file_path, "rb");
			if (live_ctx->record_file == NULL) {
				EA_LOG_ERROR("fopen() failed on %s\n", file_path);
				rval = -1;
				break;
			}
		} else if (params->record) {
			snprintf(file_path, sizeof(file_path), "%s", params->output_dir);
			strncat(file_path, FAIRMOT_DET_RECORD_FILE_NAME, MAX_PATH_STRLEN - strlen(file_path));
			live_ctx->record_file = fopen(file_path, "wb");
			if (live_ctx->record_file == NULL) {
				EA_LOG_ERROR("fopen() failed on %s\n", file_path);
				rval = -1;
				break;
			}
		}

		if (params->run_mode != RUN_MODE_REPLAY) {
			RVAL_OK(pthread_create(&live_ctx->det_tidp, NULL,
				det_thread_func, live_ctx));
		}
	} while (0);

	return rval;
}

static void live_deinit(live_ctx_t *live_ctx, live_params_t *params)
{
	if (live_ctx->result_file) {
		fclose(live_ctx->result_file);
	}

	if (live_ctx->record_file) {
		fclose(live_ctx->record_file);
	}

	mot_deinit(&live_ctx->mot);
	fairmot_deinit(&live_ctx->fairmot);
	cv_env_deinit(live_ctx, params);
	EA_LOG_NOTICE("live_deinit\n");
}

static int live_run_loop(live_ctx_t *live_ctx, live_params_t *params)
{
	int rval = 0;
	ea_calc_fps_ctx_t calc_fps_ctx;
	float fps;
	mot_input_t mot_input;
	size_t read_num;
	int i;
	EA_MEASURE_TIME_DECLARE();

	memset(&calc_fps_ctx, 0, sizeof(ea_calc_fps_ctx_t));
	calc_fps_ctx.count_period = 100;

	do {
		if (params->run_mode != RUN_MODE_REPLAY) {
			if (params->measure_time) {
				EA_MEASURE_TIME_START();
			}

			RVAL_OK(fairmot_arm_post_process_in_parallel(&live_ctx->fairmot, &live_ctx->net_result));
			if (params->run_mode == RUN_MODE_FILE
				&& live_ctx->img_data[live_ctx->head].tensor_group == NULL) {
				break;
			}

			live_ctx->loop_count++;
			memset(&mot_input, 0, sizeof(mot_input_t));
			RVAL_ASSERT(live_ctx->net_result.reid_dim == MOT_REID_DIM);
			mot_input.valid_det_count = live_ctx->net_result.valid_det_count;
			for (i = 0; i < live_ctx->net_result.valid_det_count; i++) {
				mot_input.detections[i].class_id = live_ctx->net_result.detections[i].id;
				mot_input.detections[i].score = live_ctx->net_result.detections[i].score;
				mot_input.detections[i].x_start = live_ctx->net_result.detections[i].x_start;
				mot_input.detections[i].x_end = live_ctx->net_result.detections[i].x_end;
				mot_input.detections[i].y_start = live_ctx->net_result.detections[i].y_start;
				mot_input.detections[i].y_end = live_ctx->net_result.detections[i].y_end;
				memcpy(mot_input.detections[i].reid, live_ctx->net_result.detections[i].reid, sizeof(mot_input.detections[i]));
#if 0 // for debug use
				FILE *f = fopen("/sdcard/mmcblk1/fairmot/out/reid.bin", "w");
				fwrite(mot_input.detections[i].reid, sizeof(float), MOT_REID_DIM, f);
				fclose(f);
#endif
			}

			if (live_ctx->record_file) {
				fwrite(&mot_input, sizeof(mot_input_t), 1, live_ctx->record_file);
			}

			RVAL_OK(mot_process(&live_ctx->mot, &mot_input, &live_ctx->mot_result))

			fps = ea_calc_fps(&calc_fps_ctx);
			if (fps > 0) {
				EA_LOG_NOTICE("fps %.1f\n", fps);
			}

			if (params->run_mode == RUN_MODE_LIVE && params->use_pyramid) {
				draw_detection(live_ctx, params,
					live_ctx->img_data[live_ctx->head].tensor_group[params->pyramid[1]],
					live_ctx->img_data[live_ctx->head].dsp_pts);
			} else {
				draw_detection(live_ctx, params,
					live_ctx->img_data[live_ctx->head].tensor_group[0],
					live_ctx->img_data[live_ctx->head].dsp_pts);
			}

			if (params->run_mode == RUN_MODE_FILE) {
				save_result(live_ctx);
			}

			ea_img_resource_drop_data(live_ctx->img_resource, &live_ctx->img_data[live_ctx->head]);
			live_ctx->head++;
			if (live_ctx->head == IMG_DATA_QUEUE_SIZE) {
				live_ctx->head = 0;
			}

			if (params->measure_time) {
				EA_MEASURE_TIME_END("loop:");
			}

			if (live_ctx->sig_flag) {
				break;
			}
		} else {
			read_num = fread(&mot_input, sizeof(mot_input_t), 1, live_ctx->record_file);
			RVAL_ASSERT(read_num == 0 || read_num == 1);
			if (read_num == 0) {
				break;
			}

			live_ctx->loop_count++;
			RVAL_OK(mot_process(&live_ctx->mot, &mot_input, &live_ctx->mot_result));
		}
	} while (1);

	if (params->run_mode != RUN_MODE_REPLAY) {
		live_ctx->det_stop_flag = 1;
		fairmot_vp_forward_signal(&live_ctx->fairmot);
		pthread_join(live_ctx->det_tidp, NULL);
	}

	EA_LOG_NOTICE("FairMOT stops after %d loops\n", live_ctx->loop_count);

	return rval;
}

static void sig_stop(int a)
{
	(void)a;
	live_ctx.sig_flag = 1;
}

int main(int argc, char **argv)
{
	int rval = 0;

	signal(SIGINT, sig_stop);
	signal(SIGQUIT, sig_stop);
	signal(SIGTERM, sig_stop);

	memset(&live_ctx, 0, sizeof(live_ctx_t));

	do {
		rval = init_param(argc, argv, &live_ctx.params);
		if (rval < 0) {
			break;
		}

		RVAL_OK(live_init(&live_ctx, &live_ctx.params));
		RVAL_OK(live_run_loop(&live_ctx, &live_ctx.params));
	} while (0);

	live_deinit(&live_ctx, &live_ctx.params);

	return rval;
}
