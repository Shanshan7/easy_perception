/*******************************************************************************
 * yolov5.c
 *
 * History:
 *  2020/09/28  - [Du You] created
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
#include <math.h>
#include <float.h>

#include <signal.h>
#include <fcntl.h>

#include <eazyai.h>

#include "yolov5.h"

#define SIGMOID(x)	(1.0 / (1.0 + exp(-(x))))

EA_LOG_DECLARE_LOCAL(EA_LOG_LEVEL_NOTICE);

static float const yolov5_anchors[YOLOV5_FEATURE_MAP_NUM][YOLOV5_ANCHOR_NUM][2] = {
	{
		{116, 90}, {156, 198}, {373, 326}	// anchor box of feature map 1
	},
	{
		{30, 61}, {62, 45}, {59, 119}		// anchor box of feature map 2
	},
	{
		{10, 13}, {16, 30}, {33, 23}		// anchor box of feature map 3
	}
};

int yolov5_init(yolov5_t *yolov5, const yolov5_params_t *params)
{
	int rval = 0;
	FILE *fp_label = NULL;
	char *endl = NULL;
	int i;

	do {
		RVAL_OK(yolov5 != NULL);
		RVAL_ASSERT(params != NULL);
		RVAL_ASSERT(params->model_path != NULL);
		EA_LOG_SET_LOCAL(params->log_level);

		memset(yolov5, 0, sizeof(yolov5_t));
		yolov5->top_k = params->keep_top_k;
		yolov5->nms_threshold = params->nms_threshold;
		yolov5->conf_threshold = params->conf_threshold;
		yolov5->use_multi_cls = params->use_multi_cls;

		yolov5->net = ea_net_new(NULL);
		RVAL_ASSERT(yolov5->net != NULL);

		if (params->log_level == EA_LOG_LEVEL_VERBOSE) {
			ea_net_params(yolov5->net)->verbose_print = 1;
		}

		ea_net_config_input(yolov5->net, params->input_name);
		for (i = 0; i < YOLOV5_FEATURE_MAP_NUM; i++) {
			ea_net_config_output(yolov5->net, params->feature_map_names[i]);
		}
		RVAL_OK(ea_net_load(yolov5->net, EA_NET_LOAD_FILE, (void *)params->model_path, 1/*max_batch*/));
		yolov5->input_tensor = ea_net_input(yolov5->net, params->input_name);
		for (i = 0; i < YOLOV5_FEATURE_MAP_NUM; i++) {
			yolov5->feature_map_tensors[i] = ea_net_output(yolov5->net, params->feature_map_names[i]);
		}

		// load label from file
		fp_label = fopen(params->label_path, "r");
		if (fp_label == NULL) {
			EA_LOG_ERROR("can't open file %s\n", params->label_path);
			rval = -1;
			break;
		}

		yolov5->valid_label_count = 0;
		for (i = 0; i < YOLOV5_MAX_LABEL_NUM; i++) {
			if (fgets(yolov5->labels[i], YOLOV5_MAX_LABEL_LEN, fp_label) == NULL) {
				break;
			}

			if (strlen(yolov5->labels[i]) >= YOLOV5_MAX_LABEL_LEN - 1) {
				EA_LOG_ERROR("YOLOV5_MAX_LABEL_LEN %d is too small\n", YOLOV5_MAX_LABEL_LEN);
				rval = -1;
				break;
			}

			endl = strchr(yolov5->labels[i], '\n');
			if (endl) {
				endl[0] = '\0';
			}

			yolov5->valid_label_count++;
		}

		RVAL_BREAK();

		fclose(fp_label);
		fp_label = NULL;

		EA_LOG_NOTICE("label num: %d\n", yolov5->valid_label_count);
	} while (0);

	if (rval < 0) {
		if (fp_label) {
			free(fp_label);
			fp_label = NULL;
		}

		if (yolov5) {
			if (yolov5->net) {
				ea_net_free(yolov5->net);
				yolov5->net = NULL;
			}
		}
	}

	return rval;
}

void yolov5_deinit(yolov5_t *yolov5)
{
	if (yolov5) {
		if (yolov5->net) {
			ea_net_free(yolov5->net);
			yolov5->net = NULL;
		}
	}

	EA_LOG_NOTICE("yolov5_deinit\n");
}

ea_tensor_t *yolov5_input(yolov5_t *yolov5)
{
	return yolov5->input_tensor;
}

int yolov5_vp_forward(yolov5_t *yolov5)
{
	int rval = 0;

	do {
		RVAL_OK(ea_net_forward(yolov5->net, 1/*batch*/));
	} while (0);

	return rval;
}

int yolov5_arm_post_process(yolov5_t *yolov5, yolov5_result_t *result)
{
	int rval = 0;
	int class_num = ea_tensor_shape(yolov5->feature_map_tensors[0])[EA_C] / YOLOV5_ANCHOR_NUM - 5;
	int height[YOLOV5_FEATURE_MAP_NUM];
	int width[YOLOV5_FEATURE_MAP_NUM];
	int max_det_num_in_class = 0;
	uint8_t *feature_map_data;
	uint8_t *feature_map_data_in_anchor;
	uint8_t *x_data, *y_data, *w_data, *h_data, *box_conf_data, *cls_conf_data;
	uint8_t *x_data_w, *y_data_w, *w_data_w, *h_data_w, *box_conf_data_w, *cls_conf_data_w;
	const size_t *shape;
	size_t pitch;
	int stride_w, stride_h;
	float xywhscore[5];
	float cls_conf;
	float **x1y1x2y2score_in_class = NULL;
	int *valid_count_in_class = NULL;
	float **nms_x1y1x2y2score_in_class = NULL;
	int *nms_valid_count_in_class = NULL;
	int i, m, a, c, h, w;
#if YOLOV5_MULTI_CLASS_PER_ANCHOR
#else
	int best_cls;
	float best_cls_conf;
#endif

	do {
		for (m = 0; m < YOLOV5_FEATURE_MAP_NUM; m++) {
			RVAL_OK(ea_tensor_sync_cache(yolov5->feature_map_tensors[m], EA_VP, EA_CPU));
			height[m] = ea_tensor_shape(yolov5->feature_map_tensors[m])[EA_H];
			width[m] = ea_tensor_shape(yolov5->feature_map_tensors[m])[EA_W];
			max_det_num_in_class += height[m] * width[m] * YOLOV5_ANCHOR_NUM;
		}

		RVAL_BREAK();

		x1y1x2y2score_in_class = (float **)malloc(class_num * sizeof(float *));
		RVAL_ASSERT(x1y1x2y2score_in_class != NULL);
		for (c = 0; c < class_num; c++) {
			x1y1x2y2score_in_class[c] = (float *)malloc(max_det_num_in_class * 5 * sizeof(float));
			RVAL_ASSERT(x1y1x2y2score_in_class[c] != NULL);
		}

		RVAL_BREAK();

		valid_count_in_class = (int *)malloc(class_num * sizeof(int));
		RVAL_ASSERT(valid_count_in_class != NULL);
		memset(valid_count_in_class, 0, class_num * sizeof(int));

		for (m = 0; m < YOLOV5_FEATURE_MAP_NUM; m++) {
			feature_map_data = (uint8_t *)ea_tensor_data(yolov5->feature_map_tensors[m]);
			shape = ea_tensor_shape(yolov5->feature_map_tensors[m]);
			pitch = ea_tensor_pitch(yolov5->feature_map_tensors[m]);
			stride_w = ea_tensor_shape(yolov5->input_tensor)[EA_W] / shape[EA_W];
			stride_h = ea_tensor_shape(yolov5->input_tensor)[EA_H] / shape[EA_H];
			for (a = 0; a < YOLOV5_ANCHOR_NUM; a++) {
				feature_map_data_in_anchor = feature_map_data + shape[EA_H] * pitch * (class_num + 5) * a;
				x_data = feature_map_data_in_anchor;
				y_data = x_data + shape[EA_H] * pitch;
				w_data = y_data + shape[EA_H] * pitch;
				h_data = w_data + shape[EA_H] * pitch;
				box_conf_data = h_data + shape[EA_H] * pitch;
				if (yolov5->use_multi_cls) {
					for (c = 0; c < class_num; c++) {
						cls_conf_data = box_conf_data + shape[EA_H] * pitch + c * shape[EA_H] * pitch;
						for (h = 0; h < height[m]; h++) {
							x_data_w = x_data + h * pitch;
							y_data_w = y_data + h * pitch;
							w_data_w = w_data + h * pitch;
							h_data_w = h_data + h * pitch;
							x_data_w = x_data + h * pitch;
							box_conf_data_w = box_conf_data + h * pitch;
							cls_conf_data_w = cls_conf_data + h * pitch;
							for (w = 0; w < width[m]; w++) {
								xywhscore[4] = ((float *)box_conf_data_w)[w];
								xywhscore[4] = SIGMOID(xywhscore[4]);
								if (xywhscore[4] > yolov5->conf_threshold) {
									xywhscore[2] = ((float *)w_data_w)[w];
									xywhscore[2] = SIGMOID(xywhscore[2]);
									xywhscore[2] = pow(xywhscore[2] * 2.0, 2.0) * yolov5_anchors[m][a][0];
									if (xywhscore[2] > YOLOV5_MIN_WH && xywhscore[2] < YOLOV5_MAX_WH) {
										xywhscore[3] = ((float *)h_data_w)[w];
										xywhscore[3] = SIGMOID(xywhscore[3]);
										xywhscore[3] = pow(xywhscore[3] * 2.0, 2.0) * yolov5_anchors[m][a][1];
										if (xywhscore[3] > YOLOV5_MIN_WH && xywhscore[3] < YOLOV5_MAX_WH) {
											cls_conf = ((float *)cls_conf_data_w)[w];
											cls_conf = SIGMOID(cls_conf);

											xywhscore[4] = xywhscore[4] * cls_conf;
											if (xywhscore[4] > yolov5->conf_threshold) {
												xywhscore[0] = ((float *)x_data_w)[w];
												xywhscore[0] = SIGMOID(xywhscore[0]);
												xywhscore[0] = (xywhscore[0] * 2.0 - 0.5 + w) * stride_w;

												xywhscore[1] = ((float *)y_data_w)[w];
												xywhscore[1] = SIGMOID(xywhscore[1]);
												xywhscore[1] = (xywhscore[1] * 2.0 - 0.5 + h) * stride_h;
												x1y1x2y2score_in_class[c][valid_count_in_class[c] * 5 + 0] = xywhscore[0] - xywhscore[2] / 2.0;
												x1y1x2y2score_in_class[c][valid_count_in_class[c] * 5 + 1] = xywhscore[1] - xywhscore[3] / 2.0;
												x1y1x2y2score_in_class[c][valid_count_in_class[c] * 5 + 2] = xywhscore[0] + xywhscore[2] / 2.0;
												x1y1x2y2score_in_class[c][valid_count_in_class[c] * 5 + 3] = xywhscore[1] + xywhscore[3] / 2.0;
												x1y1x2y2score_in_class[c][valid_count_in_class[c] * 5 + 4] = xywhscore[4];
												valid_count_in_class[c]++;
												EA_LOG_DEBUG("%f %f %f %f %f\n", xywhscore[0], xywhscore[1], xywhscore[2], xywhscore[3], xywhscore[4]);
											}
										}
									}
								}
							}
						}
					}
				} else {
					for (h = 0; h < height[m]; h++) {
						x_data_w = x_data + h * pitch;
						y_data_w = y_data + h * pitch;
						w_data_w = w_data + h * pitch;
						h_data_w = h_data + h * pitch;
						x_data_w = x_data + h * pitch;
						box_conf_data_w = box_conf_data + h * pitch;
						for (w = 0; w < width[m]; w++) {
							xywhscore[4] = ((float *)box_conf_data_w)[w];
							xywhscore[4] = SIGMOID(xywhscore[4]);
							if (xywhscore[4] > yolov5->conf_threshold) {
								xywhscore[2] = ((float *)w_data_w)[w];
								xywhscore[2] = SIGMOID(xywhscore[2]);
								xywhscore[2] = pow(xywhscore[2] * 2.0, 2.0) * yolov5_anchors[m][a][0];
								if (xywhscore[2] > YOLOV5_MIN_WH && xywhscore[2] < YOLOV5_MAX_WH) {
									xywhscore[3] = ((float *)h_data_w)[w];
									xywhscore[3] = SIGMOID(xywhscore[3]);
									xywhscore[3] = pow(xywhscore[3] * 2.0, 2.0) * yolov5_anchors[m][a][1];
									if (xywhscore[3] > YOLOV5_MIN_WH && xywhscore[3] < YOLOV5_MAX_WH) {
										best_cls = 0;
										best_cls_conf = -FLT_MAX;
										for (c = 0; c < class_num; c++) {
											cls_conf_data = box_conf_data + shape[EA_H] * pitch + c * shape[EA_H] * pitch;
											cls_conf_data_w = cls_conf_data + h * pitch;
											cls_conf = ((float *)cls_conf_data_w)[w];
											if (best_cls_conf < cls_conf) {
												best_cls_conf = cls_conf;
												best_cls = c;
											}
										}

										c = best_cls;
										cls_conf = best_cls_conf;
										cls_conf = SIGMOID(cls_conf);

										xywhscore[4] = xywhscore[4] * cls_conf;
	#if 0	// the post process in python code doesn't have the following check on conf_threshold check.
										if (xywhscore[4] > yolov5->conf_threshold) {
	#endif
											xywhscore[0] = ((float *)x_data_w)[w];
											xywhscore[0] = SIGMOID(xywhscore[0]);
											xywhscore[0] = (xywhscore[0] * 2.0 - 0.5 + w) * stride_w;

											xywhscore[1] = ((float *)y_data_w)[w];
											xywhscore[1] = SIGMOID(xywhscore[1]);
											xywhscore[1] = (xywhscore[1] * 2.0 - 0.5 + h) * stride_h;
											x1y1x2y2score_in_class[c][valid_count_in_class[c] * 5 + 0] = xywhscore[0] - xywhscore[2] / 2.0;
											x1y1x2y2score_in_class[c][valid_count_in_class[c] * 5 + 1] = xywhscore[1] - xywhscore[3] / 2.0;
											x1y1x2y2score_in_class[c][valid_count_in_class[c] * 5 + 2] = xywhscore[0] + xywhscore[2] / 2.0;
											x1y1x2y2score_in_class[c][valid_count_in_class[c] * 5 + 3] = xywhscore[1] + xywhscore[3] / 2.0;
											x1y1x2y2score_in_class[c][valid_count_in_class[c] * 5 + 4] = xywhscore[4];
											valid_count_in_class[c]++;
											EA_LOG_DEBUG("%f %f %f %f %f\n", xywhscore[0], xywhscore[1], xywhscore[2], xywhscore[3], xywhscore[4]);
	#if 0
										}
	#endif
									}
								}
							}
						}
					}
				}
			}
		}

		nms_x1y1x2y2score_in_class = (float **)malloc(class_num * sizeof(float *));
		RVAL_ASSERT(nms_x1y1x2y2score_in_class != NULL);
		for (c = 0; c < class_num; c++) {
			nms_x1y1x2y2score_in_class[c] = (float *)malloc(valid_count_in_class[c] * 5 * sizeof(float));
			RVAL_ASSERT(nms_x1y1x2y2score_in_class[c] != NULL);
		}

		RVAL_BREAK();

		nms_valid_count_in_class = (int *)malloc(class_num * sizeof(int));
		RVAL_ASSERT(valid_count_in_class != NULL);

		for (c = 0; c < class_num; c++) {
			RVAL_OK(ea_nms(x1y1x2y2score_in_class[c], NULL, 0, valid_count_in_class[c], yolov5->nms_threshold, 0/*use_iou_min*/, yolov5->top_k/*tok_k*/,
				nms_x1y1x2y2score_in_class[c], NULL, &nms_valid_count_in_class[c]));
		}

		RVAL_BREAK();

		result->valid_det_count = 0;
		for (c = 0; c < class_num; c++) {
			for (i = 0; i < nms_valid_count_in_class[c]; i++) {
				if (nms_x1y1x2y2score_in_class[c][i * 5 + 4] > yolov5->conf_threshold) {
					result->detections[result->valid_det_count].id = c;
					result->detections[result->valid_det_count].score = nms_x1y1x2y2score_in_class[c][i * 5 + 4];

					result->detections[result->valid_det_count].x_start =
						nms_x1y1x2y2score_in_class[c][i * 5 + 0] / ea_tensor_shape(yolov5->input_tensor)[EA_W];
					result->detections[result->valid_det_count].x_start = max(0.0, result->detections[result->valid_det_count].x_start);
					result->detections[result->valid_det_count].x_start = min(1.0, result->detections[result->valid_det_count].x_start);

					result->detections[result->valid_det_count].y_start =
						nms_x1y1x2y2score_in_class[c][i * 5 + 1] / ea_tensor_shape(yolov5->input_tensor)[EA_H];
					result->detections[result->valid_det_count].y_start = max(0.0, result->detections[result->valid_det_count].y_start);
					result->detections[result->valid_det_count].y_start = min(1.0, result->detections[result->valid_det_count].y_start);

					result->detections[result->valid_det_count].x_end =
						nms_x1y1x2y2score_in_class[c][i * 5 + 2] / ea_tensor_shape(yolov5->input_tensor)[EA_W];
					result->detections[result->valid_det_count].x_end =
						max(result->detections[result->valid_det_count].x_start, result->detections[result->valid_det_count].x_end);
					result->detections[result->valid_det_count].x_end =
						min(1.0, result->detections[result->valid_det_count].x_end);

					result->detections[result->valid_det_count].y_end =
						nms_x1y1x2y2score_in_class[c][i * 5 + 3] / ea_tensor_shape(yolov5->input_tensor)[EA_H];
					result->detections[result->valid_det_count].y_end =
						max(result->detections[result->valid_det_count].y_start, result->detections[result->valid_det_count].y_end);
					result->detections[result->valid_det_count].y_end =
						min(1.0, result->detections[result->valid_det_count].y_end);

					strncpy(result->detections[result->valid_det_count].label, yolov5->labels[result->detections[result->valid_det_count].id],
						sizeof(result->detections[result->valid_det_count].label));
					EA_LOG_DEBUG("%f %f %f %f %f\n",
						nms_x1y1x2y2score_in_class[c][i * 5 + 0], nms_x1y1x2y2score_in_class[c][i * 5 + 1],
						nms_x1y1x2y2score_in_class[c][i * 5 + 2], nms_x1y1x2y2score_in_class[c][i * 5 + 3],
						nms_x1y1x2y2score_in_class[c][i * 5 + 4]);
					EA_LOG_DEBUG("%f %f %f %f %f\n",
						result->detections[result->valid_det_count].x_start, result->detections[result->valid_det_count].y_start,
						result->detections[result->valid_det_count].x_end, result->detections[result->valid_det_count].y_end,
						result->detections[result->valid_det_count].score);
					result->valid_det_count++;
					if (result->valid_det_count >= YOLOV5_MAX_OUT_NUM) {
						break;
					}
				}
			}

			if (result->valid_det_count >= YOLOV5_MAX_OUT_NUM) {
				break;
			}
		}
	} while (0);

	if (x1y1x2y2score_in_class) {
		for (c = 0; c < class_num; c++) {
			if (x1y1x2y2score_in_class[c]) {
				free(x1y1x2y2score_in_class[c]);
			}
		}

		free(x1y1x2y2score_in_class);
	}

	if (valid_count_in_class) {
		free(valid_count_in_class);
	}

	if (nms_x1y1x2y2score_in_class) {
		for (c = 0; c < class_num; c++) {
			if (nms_x1y1x2y2score_in_class[c]) {
				free(nms_x1y1x2y2score_in_class[c]);
			}
		}

		free(nms_x1y1x2y2score_in_class);
	}

	if (nms_valid_count_in_class) {
		free(nms_valid_count_in_class);
	}

	return rval;
}
