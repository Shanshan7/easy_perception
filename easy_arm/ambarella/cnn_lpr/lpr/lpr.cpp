/*******************************************************************************
 * lpr.cpp
 *
 * History:
 *  2020/08/24  - [Junshuai ZHU] created
 *
 *
 * Copyright (C) 2020 Ambarella International LP
 *
 * This file and its contents ("Software") are protected by intellectual
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

#include <time.h>
#include <sstream>
#include <basetypes.h>
#include <eazyai.h>
#include "cnn_lpr/lpr_third_party/Pipeline.h"
#include "ssd_lpr_common.h"
#include "lpr.hpp"
#include "utils.hpp"

#define LICENSE_PREPROC_RESIZE_W	(140)
#define LICENSE_PREPROC_RESIZE_H	(60)
#define LICENSE_CHAR_CLASS			(84) // 83 valid Chinese char with a null.
#define LPR_LICNESE_UPSCALE_W		(0.3f)
#define LPR_LICNESE_UPSCALE_H		(2.0f)
#define HORIZONTALPADDING_FRONT		(4)
#define HORIZONTALPADDING_BACK		(7)
#define CHINESE_PROVINCE_CHAR_IDX	(31)
#define CHINESE_SPECIAL_CHAR_IDX	(63)

EA_LOG_DECLARE_LOCAL(EA_LOG_LEVEL_NOTICE);

static void upscale_normalized_rectangle(float x_min, float y_min,
	float x_max, float y_max, float w_ratio, float h_ratio,
	bbox_param_t *bbox_scaled)
{
	float obj_h = y_max - y_min;
	float obj_w = x_max - x_min;

	bbox_scaled->norm_min_x = max(0.0f, (x_min - obj_w * w_ratio / 2));
	bbox_scaled->norm_min_y = max(0.0f, (y_min - obj_h * h_ratio / 2));
	bbox_scaled->norm_max_x = min(1.0f, x_min + obj_w * (1.0f + w_ratio / 2));
	bbox_scaled->norm_max_y = min(1.0f, y_min + obj_h * (1.0f + h_ratio / 2));

	return;
}

static inline int judge_char_type(int index)
{
	return ((index < CHINESE_PROVINCE_CHAR_IDX) || (index > CHINESE_SPECIAL_CHAR_IDX));
}

static void lpr_result_analyse(ea_tensor_t *tensor,
	std::vector<std::string> mapping_table, license_list_t *license_result,
	uint16_t license_num)
{
	uint32_t i = 0, j = 0;
	int index = 0, index_save = 0;
	int compare = 0;
	int class_num = LICENSE_CHAR_CLASS;
	float *period_start_pos = NULL;
	float conf_sum = 0;
	float confidence = 0;
	int length_count = 0;
	std::string license_text = "";
	std::pair<int,float> index_conf(0, 0.0f);
	std::vector<std::pair<int, float>> index_conf_list;
	cv::Mat network_output(ea_tensor_shape(tensor)[1] * ea_tensor_shape(tensor)[2],
		ea_tensor_shape(tensor)[3], 0x05,
		ea_tensor_data(tensor), ea_tensor_pitch(tensor));
	license_result->license_num = 0;

	for (j = 0; j < license_num; ++j) {
		network_output.data = (uint8_t*)(
			(uint64_t)ea_tensor_data(tensor) +
			j * ea_tensor_shape(tensor)[1] * ea_tensor_shape(tensor)[2] * ea_tensor_pitch(tensor));
		network_output = network_output.reshape(1, class_num);
		cv::transpose(network_output, network_output);
		index_conf_list.clear();
		for (i = 0; i < MAX_LICENSE_LENGTH; ++i) {
			period_start_pos = ((float *) (network_output.data) + i * class_num);
			index = std::max_element(period_start_pos, period_start_pos + class_num) - period_start_pos;
			if (index != class_num - 1 && (i == 0 || index != index_save)) {
				confidence = *(period_start_pos + index);
				index_conf.first = index;
				index_conf.second = confidence;
				index_conf_list.push_back(index_conf);
			}
			index_save = index;
		}
		license_text = "";
		length_count = 0;
		conf_sum = 0.0f;
		// if the first two chars are Chinese word.
		if (index_conf_list.size() > 1 &&
			judge_char_type(index_conf_list[0].first) &&
			judge_char_type(index_conf_list[1].first)) {
			compare = (index_conf_list[0].second < index_conf_list[1].second) ? 1 : 0;
			license_text += mapping_table[index_conf_list[compare].first];
			conf_sum += index_conf_list[compare].second;
			++length_count;
			i = 2;
		} else {
			i = 0;
		}
		while (i < index_conf_list.size()) {
			license_text += mapping_table[index_conf_list[i].first];
			conf_sum += index_conf_list[i].second;
			++length_count;
			++i;
		}
		memset(license_result->license_info[j].text, 0,
			sizeof(license_result->license_info[j].text));
		strncpy(license_result->license_info[j].text,
			license_text.c_str(), sizeof(license_result->license_info[j].text) - 1);
		license_result->license_info[j].text[sizeof(license_result->license_info[j].text) - 1] = '\0';
		license_result->license_info[j].conf = conf_sum / length_count;
		++license_result->license_num;
	}

	return;
}

static int LPR_init_network(LPR_net_ctx_t *LPR_net_ctx)
{
	int rval = 0;

	do {
		LPR_net_ctx->net_param.print_time = !!LPR_net_ctx->net_verbose;
		LPR_net_ctx->net_param.verbose_print= 0;
		LPR_net_ctx->net_param.abort_if_preempted = 0;
		LPR_net_ctx->net_param.split_num= 0;
		LPR_net_ctx->net = ea_net_new(&LPR_net_ctx->net_param);
		RVAL_ASSERT(LPR_net_ctx->net != NULL);

		ea_net_config_input(LPR_net_ctx->net, LPR_net_ctx->input_name);
		ea_net_config_output(LPR_net_ctx->net, LPR_net_ctx->output_name);
		RVAL_OK(ea_net_load(LPR_net_ctx->net, EA_NET_LOAD_FILE,
			(void *)LPR_net_ctx->net_name, MAX_DETECTED_LICENSE_NUM));
		LPR_net_ctx->input_tensor = ea_net_input(LPR_net_ctx->net, LPR_net_ctx->input_name);
		LPR_net_ctx->output_tensor = ea_net_output(LPR_net_ctx->net, LPR_net_ctx->output_name);

	} while (0);

	return rval;
}

static void LPR_deinit_network(ea_net_t *net)
{
	ea_net_free(net);

	return;
}

int LPR_init(LPR_ctx_t *LPR_ctx)
{
	int i, rval = 0;
	size_t rgb_shape[4] = {1, 3, LPR_ctx->img_h, LPR_ctx->img_w};
	size_t cropped_rgb_shape[4] = {1, 3, LICENSE_PREPROC_RESIZE_H, LICENSE_PREPROC_RESIZE_W};
	size_t cropped_mat_shape[4] = {1, 1, LICENSE_PREPROC_RESIZE_H, LICENSE_PREPROC_RESIZE_W * 3};;

	do {
		RVAL_ASSERT(LPR_ctx != NULL);
		LPR_ctx->prc = (void*)new pr::PipelinePR();
		RVAL_ASSERT(LPR_ctx->prc != NULL);
		RVAL_OK(LPR_init_network(&LPR_ctx->LPHM_net_ctx));
		RVAL_OK(LPR_init_network(&LPR_ctx->LPR_net_ctx));

		LPR_ctx->rgb_img = ea_tensor_new(EA_U8, rgb_shape, 0);
		RVAL_ASSERT(LPR_ctx->rgb_img != NULL);

		for (i = 0; i < MAX_DETECTED_LICENSE_NUM; ++i) {
			LPR_ctx->cropped_rgb_img[i] = ea_tensor_new(EA_U8, cropped_rgb_shape, 0);
			LPR_ctx->cropped_mat_img[i] = ea_tensor_new(EA_U8, cropped_mat_shape, 0);
			RVAL_ASSERT(LPR_ctx->cropped_rgb_img[i] != NULL);
			RVAL_ASSERT(LPR_ctx->cropped_mat_img[i] != NULL);
		}

	} while (0);

	if (rval < 0) {
		LPR_deinit(LPR_ctx);
	}

	return rval;
}

void LPR_deinit(LPR_ctx_t *LPR_ctx)
{
	int i;

	if (LPR_ctx->prc != NULL) {
		delete (pr::PipelinePR *)LPR_ctx->prc;
	}
	LPR_deinit_network(LPR_ctx->LPR_net_ctx.net);
	LPR_deinit_network(LPR_ctx->LPHM_net_ctx.net);
	LPR_ctx->LPR_net_ctx.net = NULL;
	LPR_ctx->LPHM_net_ctx.net = NULL;
	ea_tensor_free(LPR_ctx->rgb_img);
	LPR_ctx->rgb_img = NULL;
	for (i = 0; i < MAX_DETECTED_LICENSE_NUM; ++i) {
		ea_tensor_free(LPR_ctx->cropped_rgb_img[i]);
		LPR_ctx->cropped_rgb_img[i] = NULL;
		ea_tensor_free(LPR_ctx->cropped_mat_img[i]);
		LPR_ctx->cropped_mat_img[i] = NULL;
	}

	return;
}

int LPR_run(LPR_ctx_t *LPR_ctx, ea_tensor_t *input_tensor, uint16_t license_num,
	void *bbox_param_p, license_list_t *license_result)
{
	int rval = 0;
	uint16_t i = 0;
	bbox_param_t scaled_bbox_recg[MAX_DETECTED_LICENSE_NUM];
	ea_roi_t roi_group[MAX_DETECTED_LICENSE_NUM];
	cv::Mat deskrewed_img;
	std::vector<cv::Mat> deskrewed_img_list;
	uint32_t start_pos = 0, end_pos = 0;
	float *mapping_output = NULL;
	uint64_t debug_time = 0;
	bbox_param_t *bbox_param = NULL;
	pr::PipelinePR *prc = NULL;
	ea_tensor_t *net_input_tensor = NULL;
	ea_tensor_t *net_output_tensor = NULL;
	uint64_t step_size = 0;

	do {
		if (license_num == 0) {
			break;
		}
		RVAL_ASSERT(LPR_ctx != NULL);
		RVAL_ASSERT(input_tensor != NULL);
		RVAL_ASSERT(bbox_param_p != NULL);
		RVAL_ASSERT(license_result != NULL);
		RVAL_ASSERT(LPR_ctx->prc != NULL);
		bbox_param = (bbox_param_t *)bbox_param_p;
		prc = (pr::PipelinePR *)LPR_ctx->prc;
		SAVE_TENSOR_IN_DEBUG_MODE("LPR_pyd.jpg", input_tensor, LPR_ctx->debug_en);

		// convert input YUV image to BRG format
		TIME_MEASURE_START(LPR_ctx->debug_en);
		RVAL_OK(ea_cvt_color_resize(input_tensor,
			LPR_ctx->rgb_img, EA_COLOR_YUV2BGR_NV12, EA_VP));
		TIME_MEASURE_END("[LPR] pyd to rgb convert time", LPR_ctx->debug_en);
		SAVE_TENSOR_IN_DEBUG_MODE("LPR_rgb.jpg", input_tensor, LPR_ctx->debug_en);

		// upscale the detected bbox so that the LPR could get complete image
		license_num = min(MAX_DETECTED_LICENSE_NUM, license_num);
		for (i = 0; i < license_num; ++i) {
			upscale_normalized_rectangle(bbox_param[i].norm_min_x,
				bbox_param[i].norm_min_y, bbox_param[i].norm_max_x,
				bbox_param[i].norm_max_y, LPR_LICNESE_UPSCALE_W,
				LPR_LICNESE_UPSCALE_H, &scaled_bbox_recg[i]);
			roi_group[i].x = scaled_bbox_recg[i].norm_min_x * LPR_ctx->img_w;
			roi_group[i].y = scaled_bbox_recg[i].norm_min_y * LPR_ctx->img_h;
			roi_group[i].w = (scaled_bbox_recg[i].norm_max_x -
				scaled_bbox_recg[i].norm_min_x) * LPR_ctx->img_w;
			roi_group[i].h = (scaled_bbox_recg[i].norm_max_y -
				scaled_bbox_recg[i].norm_min_y) * LPR_ctx->img_h;
		}

		// crop the ROI regions from the complete BGR image
		TIME_MEASURE_START(LPR_ctx->debug_en);
		RVAL_OK(ea_crop_resize(&LPR_ctx->rgb_img, 1, LPR_ctx->cropped_rgb_img,
			license_num, roi_group, EA_TENSOR_COLOR_MODE_BGR, EA_VP));
		TIME_MEASURE_END("[LPR] rgb crop roi time", LPR_ctx->debug_en);

		deskrewed_img_list.clear();
		net_input_tensor = LPR_ctx->LPHM_net_ctx.input_tensor;
		step_size = ea_tensor_shape(net_input_tensor)[1] *
			ea_tensor_shape(net_input_tensor)[2] *
			ea_tensor_pitch(net_input_tensor);

		// convert to BGR interleave for cv::Mat mem format
		for (i = 0; i < license_num; ++i) {
			TIME_MEASURE_START(LPR_ctx->debug_en);
			RVAL_OK(ea_cvt_color_resize(LPR_ctx->cropped_rgb_img[i],
				LPR_ctx->cropped_mat_img[i], EA_COLOR_TRANSPOSE, EA_VP));
			RVAL_OK(ea_tensor_sync_cache(LPR_ctx->cropped_mat_img[i], EA_VP, EA_CPU));
			TIME_MEASURE_END("[LPR] rgb to mat time", LPR_ctx->debug_en);
			SAVE_TENSOR_GROUP_IN_DEBUG_MODE("LPR_cropped_mat", i,
				LPR_ctx->cropped_mat_img[i], LPR_ctx->debug_en);
		}

		// descrew the license plate image
		for (i = 0; i < license_num; ++i) {
			TIME_MEASURE_START(LPR_ctx->debug_en);
			cv::Mat cropped_img(LICENSE_PREPROC_RESIZE_H, LICENSE_PREPROC_RESIZE_W, 0x10,
				(uint8_t*)ea_tensor_data(LPR_ctx->cropped_mat_img[i]),
				ea_tensor_pitch(LPR_ctx->cropped_mat_img[i]));
			deskrewed_img = prc->fineMapping->FineMappingVertical(cropped_img);
			deskrewed_img_list.push_back(deskrewed_img);
			TIME_MEASURE_END("[LPR] deskrew time", LPR_ctx->debug_en);
			SAVE_MAT_GROUP_IN_DEBUG_MODE("LPR_deskrewed", i, deskrewed_img, LPR_ctx->debug_en);

			TIME_MEASURE_START(LPR_ctx->debug_en);
			cv::resize(deskrewed_img, deskrewed_img,
				cv::Size(ea_tensor_shape(net_input_tensor)[3],
				ea_tensor_shape(net_input_tensor)[2]));
			convert_cvMat_to_rgb(deskrewed_img,
				(uint8_t*)((uint64_t)ea_tensor_data(net_input_tensor) +
				i * step_size), 1, 1);
			TIME_MEASURE_END("[LPR] mat resize & to rgb time", LPR_ctx->debug_en);
		}
		if (rval < 0) {
			break;
		}
		// detect license plate horizontal edge
		TIME_MEASURE_START(LPR_ctx->debug_en);
		RVAL_OK(ea_tensor_sync_cache(LPR_ctx->LPHM_net_ctx.input_tensor, EA_CPU, EA_VP));
		RVAL_OK(ea_net_forward(LPR_ctx->LPHM_net_ctx.net, license_num));
		RVAL_OK(ea_tensor_sync_cache(LPR_ctx->LPHM_net_ctx.output_tensor, EA_VP, EA_CPU));
		TIME_MEASURE_END("[LPR] LPHM net forward time", LPR_ctx->debug_en);

		// remove the redundant zone
		net_input_tensor = LPR_ctx->LPR_net_ctx.input_tensor;
		step_size = ea_tensor_shape(net_input_tensor)[1] *
			ea_tensor_shape(net_input_tensor)[2] *
			ea_tensor_pitch(net_input_tensor);
		net_output_tensor = LPR_ctx->LPHM_net_ctx.output_tensor;
		for (i = 0; i < license_num; ++i) {
			deskrewed_img = deskrewed_img_list[i];
			mapping_output = (float*)((uint64_t)
				ea_tensor_data(net_output_tensor) +
				i * ea_tensor_shape(net_output_tensor)[1] *
				ea_tensor_shape(net_output_tensor)[2] *
				ea_tensor_pitch(net_output_tensor));
			start_pos = (uint32_t)(mapping_output[0] * deskrewed_img.cols);
			end_pos = (uint32_t)(mapping_output[1] * deskrewed_img.cols);
			start_pos = max((int)(start_pos - HORIZONTALPADDING_FRONT), 0);
			end_pos = min((int)(end_pos + HORIZONTALPADDING_BACK), (deskrewed_img.cols - 1));
			deskrewed_img  = deskrewed_img.colRange(start_pos, end_pos).clone();
			cv::transpose(deskrewed_img, deskrewed_img);
			cv::resize(deskrewed_img, deskrewed_img,
				cv::Size(ea_tensor_shape(net_input_tensor)[3],
				ea_tensor_shape(net_input_tensor)[2]));
			SAVE_MAT_GROUP_IN_DEBUG_MODE("LPR_cut_license", i, deskrewed_img, LPR_ctx->debug_en);
			convert_cvMat_to_rgb(deskrewed_img, (uint8_t*)((uint64_t)
				ea_tensor_data(net_input_tensor) +
				i * step_size), 1, 1);
		}

		// license plate char recognition
		TIME_MEASURE_START(LPR_ctx->debug_en);
		RVAL_OK(ea_tensor_sync_cache(LPR_ctx->LPR_net_ctx.input_tensor, EA_CPU, EA_VP));
		RVAL_OK(ea_net_forward(LPR_ctx->LPR_net_ctx.net, license_num));
		RVAL_OK(ea_tensor_sync_cache(LPR_ctx->LPR_net_ctx.output_tensor, EA_VP, EA_CPU));
		TIME_MEASURE_END("[LPR] LPR net forward time", LPR_ctx->debug_en);

		TIME_MEASURE_START(LPR_ctx->debug_en);
		lpr_result_analyse(LPR_ctx->LPR_net_ctx.output_tensor,
			pr::CH_PLATE_CODE, license_result, license_num);
		TIME_MEASURE_END("[LPR] LPR post process time", LPR_ctx->debug_en);
	} while (0);

	return rval;
}

