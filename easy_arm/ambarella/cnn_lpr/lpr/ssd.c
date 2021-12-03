/*******************************************************************************
 * ssd.c
 *
 * History:
 *  2020/07/28  - [Qiangqiang Zhang] created
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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <eazyai.h>
#include "lib_data_process.h"
#include "ssd.h"

#define RVAL_ASSERT_STRING_NULL(statement) \
	do { \
		if (((statement) == NULL) || (strlen(statement) == 0)) { \
			rval = -1; \
			printf("[Error]: %s is NULL !\n", #statement); \
			printf("         [Trace] File: %s, Function: %s, Line: %d\n", __FILE__, __func__, __LINE__); \
			break; \
		} \
	} while (0)

#define DEAFULT_CLASS_NUM	(21)
static const char default_class_names[DEAFULT_CLASS_NUM][SSD_NET_MAX_LABEL_LEN] = {"background",
							"aeroplane", "bicycle", "bird", "boat",
							"bottle", "bus", "car", "cat", "chair",
							"cow", "diningtable", "dog", "horse",
							"motorbike", "person", "pottedplant",
							"sheep", "sofa", "train", "tvmonitor"};

EA_LOG_DECLARE_LOCAL(EA_LOG_LEVEL_NOTICE);

static void print_dproc_ssd_config(dproc_ssd_detection_output_config_t *ssd_config)
{
	printf("******************************\n");
	printf("Dproc SSD config\n");
	printf("******************************\n");
	printf("Image width:\t\t%d\n", ssd_config->img_width);
	printf("Image height:\t\t%d\n", ssd_config->img_height);
	printf("All bboxes num:\t\t%d\n", ssd_config->num_all_bboxes);
	printf("Number of classes:\t%d\n", ssd_config->num_classes);
	printf("Background label id:\t%d\n", ssd_config->background_label_id);
	printf("Share Location:\t\t%d\n", ssd_config->share_location);
	printf("BBox code type:\t\t%d\n", ssd_config->code_type);
	printf("Confidence threshold:\t%.2f\n", ssd_config->conf_threshold);
	printf("Keep top K objects:\t%d\n", ssd_config->keep_top_k);
	printf("NMS threshold:\t\t%.2f\n", ssd_config->nms_threshold);
	printf("NMS top K:\t\t%d\n", ssd_config->nms_top_k);
	printf("******************************\n");

	return;
}

static void print_dproc_class_dbg_info(dproc_ssd_class_dbg_info_t *dbg_info, uint32_t all_bbox_num,
	dproc_ssd_detection_output_config_t* config)
{
	uint32_t class_id = 0;
	printf("*******************************\n");
	printf("Dproc class debug info\n");
	printf("*******************************\n");
	printf("Confidence threshold: %f\n", config->conf_threshold);
	printf("Number of PriorBox: %d\n", all_bbox_num);
	printf("NMS Top K: %d\n", config->nms_top_k);
	printf("-------------------------------------------\n");
	printf("Class Index  Filtered Boxes  Post NMS Boxes\n");
	printf("-------------------------------------------\n");
	for (class_id = 0; class_id < config->num_classes; class_id++) {
		if (class_id != config->background_label_id &&
			dbg_info[class_id].num_bboxes_above_threshold > 0) {
			printf("%d\t\t%d\t\t%d\n",
			class_id,
			dbg_info[class_id].num_bboxes_above_threshold,
			dbg_info[class_id].num_bboxes_post_nms);
		}
	}

	return;
}

static void print_dproc_ssd_results(dproc_ssd_detection_output_result_t *ssd_result,
	int ssd_result_num, int unnormalized)
{
	int i = 0;
	printf("*******************************\n");
	printf("Dproc SSD detection results\n");
	printf("*******************************\n");
	printf("Detected [%d] objects\n", ssd_result_num);

	if (ssd_result_num > 0) {
		printf("-----------------------------------------------------------------------\n");
		printf("#\tlabel\tmin(x, y)\t\tmax(x, y)\t\tscore\n");
		printf("-----------------------------------------------------------------------\n");
		for (i = 0; i < ssd_result_num; i++) {
			if (unnormalized) {
				printf("%d\t%d\t(%4.0f, %4.0f)\t(%4.0f, %4.0f)\t%.5f\n",
				i+1, ssd_result[i].label,
				ssd_result[i].bbox.x_min, ssd_result[i].bbox.y_min,
				ssd_result[i].bbox.x_max, ssd_result[i].bbox.y_max,
				ssd_result[i].score);
			} else {
				printf("%d\t%d\t(%1.5f, %1.5f)\t(%1.5f, %1.5f)\t%.5f\n",
				i+1, ssd_result[i].label,
				ssd_result[i].bbox.x_min, ssd_result[i].bbox.y_min,
				ssd_result[i].bbox.x_max, ssd_result[i].bbox.y_max,
				ssd_result[i].score);
			}
		}
	}
	printf("-----------------------------------------------------------------------\n\n");

	return;
}

static int get_label_from_file(const char *label_file, char (*labels)[SSD_NET_MAX_LABEL_LEN], uint32_t *label_num)
{
	int rval = 0;
	uint32_t label_count = 0;
	FILE *fp_label = NULL;
	char *label_line_start = NULL;
	char *label_line_end = NULL;
	char label_line[SSD_NET_MAX_LABEL_LEN];

	do {
		// load label from file
		fp_label = fopen(label_file, "r");
		if (fp_label == NULL) {
			printf("can't open label_file[%s] !\n", label_file);
			rval = -1;
			break;
		}

		while (!feof(fp_label) && (label_count < SSD_NET_MAX_LABEL_NUM)) {
			memset(label_line, 0, SSD_NET_MAX_LABEL_LEN);
			if (fgets(label_line, SSD_NET_MAX_LABEL_LEN, fp_label) == NULL) { //Read a line
				printf("fgets error !\n");
				rval = -1;
				break;
			}

			// SSD_NET_MAX_LABEL_LEN is too small, fp_label may be truncated.
			if (strlen(label_line) >= SSD_NET_MAX_LABEL_LEN - 1) {
				printf("SSD_NET_MAX_LABEL_LEN[%d] is too small, fp_label may be truncated!\n", SSD_NET_MAX_LABEL_LEN);
				rval = -1;
				break;
			}

			//remove '\''
			label_line_start = strchr(label_line, '\'');
			label_line_end = strrchr(label_line, '\'');
			if (label_line_start && label_line_end && (label_line_end > label_line_start + 1)) {
				*label_line_end = '\0';
				strcpy(labels[label_count], label_line_start + 1);
			} else {
				printf("label file[%s] format error !\n", label_file);
				rval = -1;
				break;
			}

			label_count++;
		}

		if (rval < 0) {
			break;
		}

		*label_num = label_count;
	} while (0);

	if (fp_label) {
		fclose(fp_label);
		fp_label = NULL;
	}

	return rval;
}

static int get_byte_num_from_bin(const char* filename, uint32_t *byte_num)
{
	FILE* fp = NULL;
	int rval = 0, num = 0;

	do {
		fp = fopen(filename, "rb");
		if (fp == NULL) {
			perror("get_byte_num_from_bin fopen error !\n");
			rval = -1;
			break;
		}

		if (fseek(fp, 0, SEEK_END) != 0) {
			perror("get_byte_num_from_bin fseek error !\n");
			rval = -1;
			break;
		}

		num = ftell(fp);
		if (num < 0) {
			perror("get_byte_num_from_bin ftell error !\n");
			rval = -1;
			break;
		}

		*byte_num = num;
	} while (0);

	if (fp != NULL) {
		fclose(fp);
		fp = NULL;
	}

	return rval;
}

static int get_float_from_bin(const char* filename, uint32_t byte_num, float* buffer)
{
	FILE* fp = NULL;
	int rval = 0;
	uint32_t read_num = 0;

	do {
		fp = fopen(filename, "rb");
		if (fp == NULL) {
			perror("get_float_from_bin fopen error !\n");
			rval = -1;
			break;
		}

		read_num = fread(buffer, 1, byte_num, fp);
		if (read_num != byte_num) {
			printf("get_float_from_bin fread error !\n");
			rval = -1;
			break;
		}

	} while (0);

	if (fp != NULL) {
		fclose(fp);
		fp = NULL;
	}

	return rval;
}

static int ssd_net_run_arm_nms_init(ssd_net_ctx_t *ssd_net_ctx, ssd_net_vp_result_info_t *vp_result_info)
{
	int rval = 0;
	int max_dproc_ssd_result_num = 0;
	uint32_t byte_num = 0;
	float *priorbox_buf = NULL;
	dproc_ssd_detection_output_config_t *dproc_ssd_config = NULL;

	dproc_config_t dproc_config;
	memset(&dproc_config, 0, sizeof(dproc_config_t));

	/* Each bbox is represented by 4 float parameters*/
	uint32_t loc_elements_num = (ea_tensor_shape(ssd_net_ctx->output_loc_tensor)[0] * \
						ea_tensor_shape(ssd_net_ctx->output_loc_tensor)[1] * \
						ea_tensor_shape(ssd_net_ctx->output_loc_tensor)[2] * \
						ea_tensor_shape(ssd_net_ctx->output_loc_tensor)[3]);
	ssd_net_ctx->all_bbox_num = loc_elements_num >> 2;

	do {
		RVAL_ASSERT_STRING_NULL(ssd_net_ctx->priorbox_path);
		if (get_byte_num_from_bin(ssd_net_ctx->priorbox_path, &byte_num) < 0) {
			printf("get_byte_num from priorbox error!\n");
			rval = -1;
			break;
		}

		priorbox_buf = (float *)malloc(byte_num);
		if (priorbox_buf == NULL) {
			printf("failed to malloc priorbox_buf\n");
			rval = -1;
			break;
		}
		memset(priorbox_buf, 0, byte_num);

		if (get_float_from_bin(ssd_net_ctx->priorbox_path, byte_num, priorbox_buf) < 0) {
			printf("fail to get_float from priorbox_path[%s] !\n", ssd_net_ctx->priorbox_path);
			rval = -1;
			break;
		}
		ssd_net_ctx->priorbox_buf = priorbox_buf;

		dproc_config.layer_type = DPROC_SSD_DETECTION_OUT;
		dproc_ssd_config = &dproc_config.arg.ssd_config;

		dproc_ssd_config->img_width = ssd_net_ctx->width;
		dproc_ssd_config->img_height = ssd_net_ctx->height;
		dproc_ssd_config->num_classes = ssd_net_ctx->class_num;
		dproc_ssd_config->background_label_id = ssd_net_ctx->background_label_id;
		dproc_ssd_config->mbox_loc_num_elements = loc_elements_num;
		dproc_ssd_config->num_all_bboxes = ssd_net_ctx->all_bbox_num;
		dproc_ssd_config->conf_threshold = ssd_net_ctx->conf_threshold;
		dproc_ssd_config->keep_top_k = ssd_net_ctx->keep_top_k;
		dproc_ssd_config->nms_threshold = ssd_net_ctx->nms_threshold;
		dproc_ssd_config->nms_top_k = ssd_net_ctx->nms_top_k;
		dproc_ssd_config->unnormalized = ssd_net_ctx->unnormalized;
		dproc_ssd_config->scale_factors.center_x_scale = ssd_net_ctx->scale_factors->center_x_scale;
		dproc_ssd_config->scale_factors.center_y_scale = ssd_net_ctx->scale_factors->center_y_scale;
		dproc_ssd_config->scale_factors.width_scale = ssd_net_ctx->scale_factors->width_scale;
		dproc_ssd_config->scale_factors.height_scale = ssd_net_ctx->scale_factors->height_scale;

		if (ssd_net_ctx->debug_en) {
			print_dproc_ssd_config(dproc_ssd_config);
			memcpy(&ssd_net_ctx->dproc_ssd_config, dproc_ssd_config, sizeof(dproc_ssd_detection_output_config_t));

			ssd_net_ctx->dproc_ssd_class_dbg_info = (dproc_ssd_class_dbg_info_t *)
				malloc(ssd_net_ctx->class_num * sizeof(dproc_ssd_class_dbg_info_t));
			if (ssd_net_ctx->dproc_ssd_class_dbg_info == NULL) {
				printf("failed to malloc dproc_ssd_class_dbg_info\n");
				rval = -1;
				break;
			}
		}

		max_dproc_ssd_result_num = dproc_init(&dproc_config);
		if (max_dproc_ssd_result_num < 0) {
			printf("dproc_init for ssd failed !\n");
			rval = -1;
			break;
		}
		vp_result_info->max_dproc_ssd_result_num = max_dproc_ssd_result_num;
	} while (0);

	if (rval < 0) {
		if (priorbox_buf) {
			free(priorbox_buf);
			priorbox_buf = NULL;
		}
	}

	return rval;
}

static int ssd_net_run_arm_nms_deinit(ssd_net_ctx_t *ssd_net_ctx)
{
	int rval = 0;
	float *priorbox_buf = ssd_net_ctx->priorbox_buf;
	dproc_ssd_class_dbg_info_t *dproc_ssd_class_dbg_info = ssd_net_ctx->dproc_ssd_class_dbg_info;

	do {
		if (dproc_deinit(DPROC_SSD_DETECTION_OUT) < 0){
			printf("dproc_deinit failed\n");
			rval = -1;
			break;
		}
		if (dproc_ssd_class_dbg_info) {
			free(dproc_ssd_class_dbg_info);
			dproc_ssd_class_dbg_info = NULL;
		}
		if (priorbox_buf) {
			free(priorbox_buf);
			priorbox_buf = NULL;
		}
	} while (0);

	return rval;
}

int ssd_net_init(const ssd_net_params_t *params, ssd_net_ctx_t *ssd_net_ctx,
	ssd_net_input_t *input, ssd_net_vp_result_info_t *vp_result_info)
{
	int i, ret = 0;
	int rval = 0;
	uint32_t label_count = 0;
	ea_net_params_t net_param = {0};

	do {
		memset(ssd_net_ctx, 0, sizeof(ssd_net_ctx_t));

		ssd_net_ctx->debug_en = params->debug_en;
		ssd_net_ctx->priorbox_path = params->priorbox_path;
		ssd_net_ctx->conf_threshold = params->conf_threshold;
		ssd_net_ctx->keep_top_k = params->keep_top_k;
		ssd_net_ctx->nms_threshold = params->nms_threshold;
		ssd_net_ctx->nms_top_k = params->nms_top_k;
		ssd_net_ctx->background_label_id = params->background_label_id;
		ssd_net_ctx->unnormalized = params->unnormalized;
		ssd_net_ctx->class_num = params->class_num;
		ssd_net_ctx->width = params->width;
		ssd_net_ctx->height = params->height;
		ssd_net_ctx->scale_factors = params->scale_factors;

		net_param.priority = params->priority;
		net_param.print_time = params->nnctrl_print_time;
		ssd_net_ctx->net = ea_net_new(&net_param);
		if (ssd_net_ctx->net == NULL) {
			printf("ea_net_new for ssd failed! \n");
			rval = -1;
			break;
		}

		RVAL_ASSERT_STRING_NULL(params->input_name);
		RVAL_ASSERT_STRING_NULL(params->output_loc);
		RVAL_ASSERT_STRING_NULL(params->output_conf);
		ea_net_config_input(ssd_net_ctx->net, params->input_name);
		ea_net_config_output(ssd_net_ctx->net, params->output_loc);
		ea_net_config_output(ssd_net_ctx->net, params->output_conf);

		RVAL_ASSERT_STRING_NULL(params->model_path);
		if (ea_net_load(ssd_net_ctx->net, EA_NET_LOAD_FILE, (void *)params->model_path, 1/*max_batch*/) < 0) {
			printf("ea_net_load for ssd failed! \n");
			rval = -1;
			break;
		}

		// get input and output
		input->tensor = ea_net_input(ssd_net_ctx->net, params->input_name);
		ssd_net_ctx->output_loc_tensor = ea_net_output(ssd_net_ctx->net, params->output_loc);
		ssd_net_ctx->output_conf_tensor = ea_net_output(ssd_net_ctx->net, params->output_conf);
		RVAL_ASSERT(input->tensor);
		RVAL_ASSERT(ssd_net_ctx->output_loc_tensor);
		RVAL_ASSERT(ssd_net_ctx->output_conf_tensor);

		// for circle Buffer
		// loc_dram_size  = all_box_num * 4 * sizeof(float), 32 alignment.
		// conf_dram_size = all_box_num * class_num * sizeof(float), 32 alignment.
		vp_result_info->loc_dram_addr = (uint8_t *)ea_tensor_data(ssd_net_ctx->output_loc_tensor);
		vp_result_info->conf_dram_addr = (uint8_t *)ea_tensor_data(ssd_net_ctx->output_conf_tensor);
		vp_result_info->loc_dram_size = ea_tensor_size(ssd_net_ctx->output_loc_tensor);
		vp_result_info->conf_dram_size = ea_tensor_size(ssd_net_ctx->output_conf_tensor);
		RVAL_ASSERT(vp_result_info->loc_dram_addr);
		RVAL_ASSERT(vp_result_info->conf_dram_addr);
		RVAL_ASSERT(vp_result_info->loc_dram_size);
		RVAL_ASSERT(vp_result_info->conf_dram_size);

		// loc_tensor: N is 1, C is 1, H is 1, W is all_box_num * 4
		// conf_tensor: N is 1, C is 1, H is 1, W is all_box_num * class_num
		if (params->class_num == 4) {
			printf("!!!!!!! Please make sure the output of network is correct  !!!!!!!\n");
		}
		if ((ea_tensor_shape(ssd_net_ctx->output_loc_tensor)[3] >> 2)
			!= (ea_tensor_shape(ssd_net_ctx->output_conf_tensor)[3] / params->class_num)) {
			printf("The output of network is wrong !\n");
			rval = -1;
			break;
		}

		// get label from file
		memset(ssd_net_ctx->labels, 0, SSD_NET_MAX_LABEL_NUM * SSD_NET_MAX_LABEL_LEN);
		if (params->label_path != NULL && strlen(params->label_path) != 0) {
			if (get_label_from_file(params->label_path, ssd_net_ctx->labels,
				&label_count) < 0) {
				printf("get_label_from_file for ssd failed! \n");
				rval = -1;
				break;
			}
		} else {
			for (i = 0; i < DEAFULT_CLASS_NUM; i++) {
				strcpy(ssd_net_ctx->labels[i], default_class_names[i]);
			}
		}

		// for arm nms
		ret = ssd_net_run_arm_nms_init(ssd_net_ctx, vp_result_info);
		if (ret < 0) {
			printf("ssd_net_run_arm_nms_init for ssd failed! \n");
			rval = -1;
			break;
		}
	} while (0);

	if (rval < 0) {
		if (ret == 0) {
			ssd_net_run_arm_nms_deinit(ssd_net_ctx);
		}

		if (ssd_net_ctx->net) {
			ea_net_free(ssd_net_ctx->net);
			ssd_net_ctx->net = NULL;
		}
	}

	return rval;
}

void ssd_net_deinit(ssd_net_ctx_t *ssd_net_ctx)
{
	ssd_net_run_arm_nms_deinit(ssd_net_ctx);

	if (ssd_net_ctx->net) {
		ea_net_free(ssd_net_ctx->net);
		ssd_net_ctx->net = NULL;
	}
}

int ssd_net_run_vp_forward(ssd_net_ctx_t *ssd_net_ctx)
{
	int rval = 0;

	do {
		if (ea_net_forward(ssd_net_ctx->net, 1/*batch*/) < 0) {
			printf("ea_net_forward for ssd failed !\n");
			rval = -1;
			break;
		}
	} while (0);

	return rval;
}

int ssd_net_run_arm_nms(ssd_net_ctx_t *ssd_net_ctx, uint8_t *loc_dram_addr,
	uint8_t *conf_dram_addr, ssd_net_final_result_t *ssd_net_result)
{
	int rval = 0, i = 0;
	int label_id = 0;
	int ssd_det_num = 0;

	dproc_run_param_t dproc_run_param;
	memset(&dproc_run_param, 0, sizeof(dproc_run_param_t));
	dproc_run_param.layer_type = DPROC_SSD_DETECTION_OUT;
	dproc_run_param.arg.ssd_param.mbox_priorbox = ssd_net_ctx->priorbox_buf;
	dproc_run_param.arg.ssd_param.mbox_loc = loc_dram_addr;
	dproc_run_param.arg.ssd_param.mbox_conf_flatten = conf_dram_addr;

	dproc_result_t dproc_result;
	memset(&dproc_result, 0, sizeof(dproc_result_t));
	dproc_result.arg.ssd_result = ssd_net_result->dproc_ssd_result;

	if (ssd_net_ctx->debug_en) { // for debug
		memset(ssd_net_ctx->dproc_ssd_class_dbg_info, 0, ssd_net_ctx->class_num * sizeof(dproc_ssd_class_dbg_info_t));
		dproc_result.arg_debug.ssd_debug_info = ssd_net_ctx->dproc_ssd_class_dbg_info;
	}

	do {
		ssd_det_num = dproc_run(&dproc_run_param, &dproc_result);
		if (ssd_det_num < 0) {
			printf("dproc run for ssd fail!\n");
			rval = -1;
			break;
		}
		ssd_net_result->ssd_det_num = ssd_det_num;

		for (i = 0; i < ssd_det_num; i++) {
			label_id = ssd_net_result->dproc_ssd_result[i].label;
			memcpy(ssd_net_result->labels[i], ssd_net_ctx->labels[label_id], SSD_NET_MAX_LABEL_LEN);
		}

		if (ssd_net_ctx->debug_en) { // for debug
			print_dproc_class_dbg_info(ssd_net_ctx->dproc_ssd_class_dbg_info, ssd_net_ctx->all_bbox_num, &ssd_net_ctx->dproc_ssd_config);
			print_dproc_ssd_results(ssd_net_result->dproc_ssd_result, ssd_det_num, ssd_net_ctx->unnormalized);
		}
	} while (0);

	return rval;
}

