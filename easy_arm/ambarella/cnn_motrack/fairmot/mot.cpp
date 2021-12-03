/*******************************************************************************
 * mot.cpp
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
#include <string.h>

#include <eazyai.h>

#include "multi_tracker.h"
#include "mot.h"

EA_LOG_DECLARE_LOCAL(EA_LOG_LEVEL_NOTICE);

int mot_init(mot_t *mot, const mot_params_t *params)
{
	int rval = 0;

	do {
		RVAL_OK(mot != NULL);
		RVAL_ASSERT(params != NULL);
		EA_LOG_SET_LOCAL(params->log_level);

		memset(mot, 0, sizeof(mot_t));
		mot->measure_time = params->measure_time;
		mot->image_width = params->image_width;
		mot->image_height = params->image_height;

		mot->tracker = new multi_tracker(
			params->image_width,
			params->image_height,
			params->embedding_reliable_use_iou,
			params->embedding_reliable_distance,
			params->embedding_cosine_distance,
			params->overlap_reliable_use_iou,
			params->overlap_reliable_distance,
			params->overlap_iou_distance,
			params->confirming_use_iou,
			params->confirming_distance,
			params->duplicate_iou_distance,
			params->max_lost_age,
			params->fuse_lambda,
			params->use_match_priority,
			params->merge_match_priority,
			params->match_priority_thresh,
			params->dt,
			params->use_smooth_feature,
			params->feature_buffer_size,
			params->smooth_alpha,
			params->tentative_thresh,
			params->verbose);
		RVAL_ASSERT(mot->tracker != NULL);
	} while (0);

	if (rval < 0) {
		if (mot->tracker) {
			delete mot->tracker;
		}
	}

	return rval;
}

void mot_deinit(mot_t *mot)
{
	if (mot->tracker) {
		delete mot->tracker;
	}

	EA_LOG_NOTICE("mot_deinit\n");
}

int mot_process(mot_t *mot, mot_input_t *input, mot_result_t *result)
{
	int rval = 0;
	int i, k;
	std::vector<s_track_ptr> new_tracks;
	detection_box tlwh;
	reid_feature feature;
	EA_MEASURE_TIME_DECLARE();

	do {
		RVAL_ASSERT(mot != NULL);
		RVAL_ASSERT(input != NULL);
		RVAL_ASSERT(result != NULL);
		RVAL_ASSERT(MOT_REID_DIM == feature.cols());

		for (i = 0; i < input->valid_det_count; i++) {
			tlwh(0) = input->detections[i].x_start * mot->image_width;
			tlwh(1) = input->detections[i].y_start * mot->image_height;
			tlwh(2) = (input->detections[i].x_end - input->detections[i].x_start) * mot->image_width;
			tlwh(3) = (input->detections[i].y_end - input->detections[i].y_start) * mot->image_height;
			for (k = 0; k < feature.cols(); k++) {
				feature(k) = input->detections[i].reid[k];
			}

			new_tracks.push_back(std::shared_ptr<s_track>(
				new s_track(tlwh, input->detections[i].score, feature,
				mot->image_width, mot->image_height)));
		}

		if (mot->measure_time) {
			EA_MEASURE_TIME_START();
		}

		mot->tracker->update(new_tracks);

		if (mot->measure_time) {
			EA_MEASURE_TIME_END("mot");
		}

		result->valid_track_count = 0;
		result->frame_id = mot->tracker->frame_id();
		for(const s_track_ptr &track : mot->tracker->available_tracks())
		{
			if (result->valid_track_count >= MOT_MAX_TRACK_NUM) {
				EA_LOG_NOTICE("no more buffer to receive more track result\n");
				break;
			}

			result->tracks[result->valid_track_count].class_id = 1;
			result->tracks[result->valid_track_count].track_id = track->track_id();
			result->tracks[result->valid_track_count].x_start = std::min((float)mot->image_width - 1, std::max(0.0f, track->tlwh()(0)));
			result->tracks[result->valid_track_count].y_start = std::min((float)mot->image_height - 1, std::max(0.0f, track->tlwh()(1)));
			result->tracks[result->valid_track_count].x_end = std::min((float)mot->image_width, std::max(0.0f, track->tlwh()(0) + track->tlwh()(2)));
			result->tracks[result->valid_track_count].y_end = std::min((float)mot->image_height, std::max(0.0f, track->tlwh()(1) + track->tlwh()(3)));
			result->tracks[result->valid_track_count].x_start /= mot->image_width;
			result->tracks[result->valid_track_count].y_start /= mot->image_height;
			result->tracks[result->valid_track_count].x_end /= mot->image_width;
			result->tracks[result->valid_track_count].y_end /= mot->image_height;

			result->valid_track_count++;
		}

		EA_LOG_NOTICE("det: %d track: %d\n", input->valid_det_count, result->valid_track_count);
	} while (0);

	return rval;
}
