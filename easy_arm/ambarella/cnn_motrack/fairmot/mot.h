/*******************************************************************************
 * mot.h
 *
 * History:
 *  2020/12/16 - [Du You] create file
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

#ifndef _MOT_H_
#define _MOT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define MOT_MAX_TRACK_NUM	100
#define MOT_REID_DIM		64

class multi_tracker;

struct mot_s {
	int measure_time;
	int image_width;
	int image_height;
	multi_tracker *tracker;
};
typedef struct mot_s mot_t;

typedef struct mot_params_s {
	int log_level;
	int measure_time;

	int image_width;
	int image_height;
	bool embedding_reliable_use_iou;
	float embedding_reliable_distance;
	float embedding_cosine_distance;
	bool overlap_reliable_use_iou;
	float overlap_reliable_distance;
	float overlap_iou_distance;
	bool confirming_use_iou;
	float confirming_distance;
	float duplicate_iou_distance;
	int max_lost_age;
	float fuse_lambda;
	int use_match_priority;
	int merge_match_priority;
	int match_priority_thresh;
	float dt;
	int use_smooth_feature;
	int feature_buffer_size;
	float smooth_alpha;
	int tentative_thresh;
	int verbose;
} mot_params_t;

int mot_init(mot_t *mot, const mot_params_t *params);
void mot_deinit(mot_t *mot);

typedef struct mot_det_s {
	float score;
	int class_id;
	float x_start; // normalized value
	float y_start;
	float x_end;
	float y_end;
	float reid[MOT_REID_DIM];
} mot_det_t;

typedef struct  mot_input_s {
	mot_det_t detections[MOT_MAX_TRACK_NUM];
	int valid_det_count;
} mot_input_t;

typedef struct mot_track_s {
	int class_id;
	int track_id;
	float x_start; // normalized value
	float y_start;
	float x_end;
	float y_end;
} mot_track_t;

typedef struct  mot_result_s {
	mot_track_t tracks[MOT_MAX_TRACK_NUM];
	int valid_track_count;
	int frame_id;
} mot_result_t;

int mot_process(mot_t *mot, mot_input_t *input, mot_result_t *result);

#ifdef __cplusplus
}
#endif

#endif
