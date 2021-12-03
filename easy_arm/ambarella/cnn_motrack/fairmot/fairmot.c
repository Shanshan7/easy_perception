/*******************************************************************************
 * fairmot.c
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
#include <math.h>

#include <signal.h>
#include <fcntl.h>
#include <pthread.h>

#include <eazyai.h>

#include "fairmot.h"

EA_LOG_DECLARE_LOCAL(EA_LOG_LEVEL_NOTICE);

int fairmot_init(fairmot_t *fairmot, const fairmot_params_t *params)
{
	int rval = 0;

	do {
		RVAL_OK(fairmot != NULL);
		RVAL_ASSERT(params != NULL);
		RVAL_ASSERT(params->model_path != NULL);
		EA_LOG_SET_LOCAL(params->log_level);

		memset(fairmot, 0, sizeof(fairmot_t));
		fairmot->top_k = params->keep_top_k;
		fairmot->nms_threshold = params->nms_threshold;
		fairmot->det_threshold = params->det_threshold;
		fairmot->det_min_area = params->det_min_area;
		fairmot->measure_time = params->measure_time;

		fairmot->net = ea_net_new(NULL);
		RVAL_ASSERT(fairmot->net != NULL);

		if (params->log_level == EA_LOG_LEVEL_VERBOSE) {
			ea_net_params(fairmot->net)->verbose_print = 1;
		}

		ea_net_config_input(fairmot->net, params->input_name);
		ea_net_config_output(fairmot->net, params->hm_name);
		ea_net_config_output(fairmot->net, params->hmax_name);
		ea_net_config_output(fairmot->net, params->wh_name);
		ea_net_config_output(fairmot->net, params->reg_name);
		ea_net_config_output(fairmot->net, params->id_name);
		RVAL_OK(ea_net_load(fairmot->net, EA_NET_LOAD_FILE, (void *)params->model_path, 1/*max_batch*/));
		fairmot->input_tensor = ea_net_input(fairmot->net, params->input_name);
		fairmot->hm_tensor = ea_net_output(fairmot->net, params->hm_name);
		fairmot->hmax_tensor = ea_net_output(fairmot->net, params->hmax_name);
		fairmot->wh_tensor = ea_net_output(fairmot->net, params->wh_name);
		fairmot->reg_tensor = ea_net_output(fairmot->net, params->reg_name);
		fairmot->id_tensor = ea_net_output(fairmot->net, params->id_name);

		RVAL_ASSERT(ea_tensor_shape(fairmot->id_tensor)[EA_C] <= FAIRMOT_MAX_REID_DIM);

		fairmot->mutex = malloc(sizeof(pthread_mutex_t));
		RVAL_ASSERT(fairmot->mutex != NULL);
		fairmot->cond = malloc(sizeof(pthread_cond_t));
		RVAL_ASSERT(fairmot->cond != NULL);
		RVAL_OK(pthread_mutex_init((pthread_mutex_t *)fairmot->mutex, NULL));
		RVAL_OK(pthread_cond_init((pthread_cond_t *)fairmot->cond, NULL));
		fairmot->flag = 0;
	} while (0);

	if (rval < 0) {
		if (fairmot) {
			if (fairmot->net) {
				ea_net_free(fairmot->net);
				fairmot->net = NULL;
			}
		}

		if (fairmot->mutex) {
			free(fairmot->mutex);
			fairmot->mutex = NULL;
		}

		if (fairmot->cond) {
			free(fairmot->cond);
			fairmot->cond = NULL;
		}
	}

	return rval;
}

void fairmot_deinit(fairmot_t *fairmot)
{
	if (fairmot) {
		if (fairmot->net) {
			ea_net_free(fairmot->net);
			fairmot->net = NULL;
		}

		if (fairmot->mutex) {
			pthread_mutex_destroy((pthread_mutex_t *)fairmot->mutex);
			free(fairmot->mutex);
			fairmot->mutex = NULL;
		}

		if (fairmot->cond) {
			pthread_cond_destroy((pthread_cond_t *)fairmot->cond);
			free(fairmot->cond);
			fairmot->cond = NULL;
		}
	}

	EA_LOG_NOTICE("fairmot_deinit\n");
}

ea_tensor_t *fairmot_input(fairmot_t *fairmot)
{
	return fairmot->input_tensor;
}

int fairmot_vp_forward_in_parallel(fairmot_t *fairmot)
{
	int rval = 0;
	EA_MEASURE_TIME_DECLARE();

	do {
		RVAL_OK(pthread_mutex_lock((pthread_mutex_t *)fairmot->mutex));
		while (fairmot->flag != 0) {
			RVAL_OK(pthread_cond_wait((pthread_cond_t *)fairmot->cond,
				(pthread_mutex_t *)fairmot->mutex));
		}

		if (fairmot->measure_time) {
			EA_MEASURE_TIME_START();
		}

		RVAL_OK(ea_net_forward(fairmot->net, 1/*batch*/));

		if (fairmot->measure_time) {
			EA_MEASURE_TIME_END("forward:");
		}

		fairmot->flag = 1;
		RVAL_OK(pthread_cond_signal((pthread_cond_t *)fairmot->cond));
		RVAL_OK(pthread_mutex_unlock((pthread_mutex_t *)fairmot->mutex));
	} while (0);

	return rval;
}

int fairmot_vp_forward_signal(fairmot_t *fairmot)
{
	int rval = 0;

	do {
		fairmot->flag = 0;
		RVAL_OK(pthread_cond_signal((pthread_cond_t *)fairmot->cond));
		RVAL_OK(pthread_mutex_unlock((pthread_mutex_t *)fairmot->mutex));
	} while (0);

	return rval;
}

static int top_k_with_threshold(float *data, int num, int k, float threshold, int *top_k_ind)
{
	int rval = 0;
	int *ind = NULL;
	int temp;
	int i, p;

	do {
		RVAL_ASSERT(top_k_ind != NULL);

		ind = (int *)malloc(sizeof(int) * num);
		RVAL_ASSERT(ind != NULL);
		for (i = 0; i < num; i++) {
			ind[i] = i;
		}

		for (i = 0; i < num - 1; i++) {
			if (i == k) {
				break;
			}

			for (p = i + 1; p < num; p++) {
				if (data[ind[i]] < data[ind[p]]) {
					temp = ind[i];
					ind[i] = ind[p];
					ind[p] = temp;
				}
			}

			if (data[ind[i]] < threshold) {
				break;
			}
		}

		for (i = 0; i < k && i < num; i++) {
			top_k_ind[i] = ind[i];
		}

		free(ind);
		ind = NULL;
	} while (0);

	if (rval < 0) {
		if (ind) {
			free(ind);
			ind = NULL;
		}
	}

	return rval;
}

static int top_k(float *data, int num, int k, int *top_k_ind)
{
	int rval = 0;
	int *ind = NULL;
	int temp;
	int i, p;

	do {
		RVAL_ASSERT(top_k_ind != NULL);

		ind = (int *)malloc(sizeof(int) * num);
		RVAL_ASSERT(ind != NULL);
		for (i = 0; i < num; i++) {
			ind[i] = i;
		}

		for (i = 0; i < num - 1; i++) {
			if (i == k) {
				break;
			}

			for (p = i + 1; p < num; p++) {
				if (data[ind[i]] < data[ind[p]]) {
					temp = ind[i];
					ind[i] = ind[p];
					ind[p] = temp;
				}
			}
		}

		for (i = 0; i < k && i < num; i++) {
			top_k_ind[i] = ind[i];
		}

		free(ind);
		ind = NULL;
	} while (0);

	if (rval < 0) {
		if (ind) {
			free(ind);
			ind = NULL;
		}
	}

	return rval;
}

int fairmot_arm_post_process_in_parallel(fairmot_t *fairmot, fairmot_result_t *result)
{
	int rval = 0;
	int class_num = ea_tensor_shape(fairmot->hm_tensor)[1];
	int height = ea_tensor_shape(fairmot->hm_tensor)[2];
	int width = ea_tensor_shape(fairmot->hm_tensor)[3];
	int reid_dim = ea_tensor_shape(fairmot->id_tensor)[1];
	float *hm = NULL;
	float *hmax = NULL;
	float *wh_wl = NULL;
	float *wh_ht = NULL;
	float *wh_wr = NULL;
	float *wh_hb = NULL;
	float *reg_x = NULL;
	float *reg_y = NULL;
	float *reid = NULL;
	uint8_t *hm_src = NULL;
	uint8_t *hmax_src = NULL;
	uint8_t *wh_wl_src = NULL;
	uint8_t *wh_ht_src = NULL;
	uint8_t *wh_wr_src = NULL;
	uint8_t *wh_hb_src = NULL;
	uint8_t *reg_x_src = NULL;
	uint8_t *reg_y_src = NULL;
	uint8_t *reid_src = NULL;
	int *per_class_score_top_k_ind = NULL;
	float *per_class_score_top_k = NULL;
	int *total_top_k_ind = NULL;

	float *x1y1x2y2score = NULL;
	int *reid_ind = NULL;
	int *valid_count = NULL;
	float *nms_x1y1x2y2score = NULL;
	int *nms_reid_ind = NULL;
	int *nms_valid_count = NULL;
	int c, h, w, k, ind, d;
	float x, y, x1, y1, x2, y2;
	float score;

	do {
		RVAL_OK(pthread_mutex_lock((pthread_mutex_t *)fairmot->mutex));
		while (fairmot->flag == 0) {
			RVAL_OK(pthread_cond_wait((pthread_cond_t *)fairmot->cond,
				(pthread_mutex_t *)fairmot->mutex));
		}

		RVAL_OK(ea_tensor_sync_cache(fairmot->hm_tensor, EA_VP, EA_CPU));
		RVAL_OK(ea_tensor_sync_cache(fairmot->hmax_tensor, EA_VP, EA_CPU));
		RVAL_OK(ea_tensor_sync_cache(fairmot->wh_tensor, EA_VP, EA_CPU));
		RVAL_OK(ea_tensor_sync_cache(fairmot->reg_tensor, EA_VP, EA_CPU));
		RVAL_OK(ea_tensor_sync_cache(fairmot->id_tensor, EA_VP, EA_CPU));

		hm = (float *)malloc(sizeof(float) * class_num * height * width);
		RVAL_ASSERT(hm != NULL);

		hmax = (float *)malloc(sizeof(float) * class_num * height * width);
		RVAL_ASSERT(hmax != NULL);

		wh_wl = (float *)malloc(sizeof(float) * height * width);
		RVAL_ASSERT(wh_wl != NULL);
		wh_ht = (float *)malloc(sizeof(float) * height * width);
		RVAL_ASSERT(wh_ht != NULL);
		wh_wr = (float *)malloc(sizeof(float) * height * width);
		RVAL_ASSERT(wh_wr != NULL);
		wh_hb = (float *)malloc(sizeof(float) * height * width);
		RVAL_ASSERT(wh_hb != NULL);
		reg_x = (float *)malloc(sizeof(float) * height * width);
		RVAL_ASSERT(reg_x != NULL);
		reg_y = (float *)malloc(sizeof(float) * height * width);
		RVAL_ASSERT(reg_y != NULL);
		reid = (float *)malloc(sizeof(float) * reid_dim * height * width);
		RVAL_ASSERT(reid != NULL);

		hm_src = (uint8_t *)ea_tensor_data(fairmot->hm_tensor);
		hmax_src = (uint8_t *)ea_tensor_data(fairmot->hmax_tensor);
		for (c = 0; c < class_num; c++) {
			for (h = 0; h < height; h++) {
				memcpy(&hm[c * height * width + h * width], hm_src, width * sizeof(float));
				hm_src += ea_tensor_pitch(fairmot->hm_tensor);
				memcpy(&hmax[c * height * width + h * width], hmax_src, width * sizeof(float));
				hmax_src += ea_tensor_pitch(fairmot->hmax_tensor);

			}
		}

		wh_wl_src = (uint8_t *)ea_tensor_data(fairmot->wh_tensor);
		wh_ht_src = (uint8_t *)ea_tensor_data(fairmot->wh_tensor) + height * ea_tensor_pitch(fairmot->wh_tensor);
		wh_wr_src = (uint8_t *)ea_tensor_data(fairmot->wh_tensor) + height * ea_tensor_pitch(fairmot->wh_tensor) * 2;
		wh_hb_src = (uint8_t *)ea_tensor_data(fairmot->wh_tensor) + height * ea_tensor_pitch(fairmot->wh_tensor) * 3;
		reg_x_src = (uint8_t *)ea_tensor_data(fairmot->reg_tensor);
		reg_y_src = (uint8_t *)ea_tensor_data(fairmot->reg_tensor) + height * ea_tensor_pitch(fairmot->reg_tensor);
		reid_src = (uint8_t *)ea_tensor_data(fairmot->id_tensor);
		for (h = 0; h < height; h++) {
			memcpy(&wh_wl[h * width], wh_wl_src, width * sizeof(float));
			wh_wl_src += ea_tensor_pitch(fairmot->wh_tensor);
			memcpy(&wh_ht[h * width], wh_ht_src, width * sizeof(float));
			wh_ht_src += ea_tensor_pitch(fairmot->wh_tensor);
			memcpy(&wh_wr[h * width], wh_wr_src, width * sizeof(float));
			wh_wr_src += ea_tensor_pitch(fairmot->wh_tensor);
			memcpy(&wh_hb[h * width], wh_hb_src, width * sizeof(float));
			wh_hb_src += ea_tensor_pitch(fairmot->wh_tensor);
			memcpy(&reg_x[h * width], reg_x_src, width * sizeof(float));
			reg_x_src += ea_tensor_pitch(fairmot->reg_tensor);
			memcpy(&reg_y[h * width], reg_y_src, width * sizeof(float));
			reg_y_src += ea_tensor_pitch(fairmot->reg_tensor);
		}

		for (d = 0; d < reid_dim; d++) {
			for (h = 0; h < height; h++) {
				memcpy(&reid[d * height * width + h * width], reid_src, width * sizeof(float));
				reid_src += ea_tensor_pitch(fairmot->id_tensor);
			}
		}
#if 0 // load from file
		FILE *f = fopen("id.bin", "rb");
		fread(reid, sizeof(float), 128*80*144, f);
		fclose(f);
		f = fopen("hm.bin", "rb");
		fread(hm, sizeof(float), 1*80*144, f);
		fclose(f);
		f = fopen("hmax.bin", "rb");
		fread(hmax, sizeof(float), 1*80*144, f);
		fclose(f);
		f = fopen("wh.bin", "rb");
		fread(wh_wl, sizeof(float), 1*80*144, f);
		fread(wh_ht, sizeof(float), 1*80*144, f);
		fread(wh_wr, sizeof(float), 1*80*144, f);
		fread(wh_hb, sizeof(float), 1*80*144, f);
		fclose(f);
		f = fopen("reg.bin", "rb");
		fread(reg_x, sizeof(float), 1*80*144, f);
		fread(reg_y, sizeof(float), 1*80*144, f);
		fclose(f);
#endif
		fairmot->flag = 0;
		RVAL_OK(pthread_cond_signal((pthread_cond_t *)fairmot->cond));
		RVAL_OK(pthread_mutex_unlock((pthread_mutex_t *)fairmot->mutex));

		// keep = (heat_max == heat).float()
		// heat = heat * keep
		for (c = 0; c < class_num; c++) {
			for (h = 0; h < height; h++) {
				for (w = 0; w < width; w++) {
					if (hm[c * height * width + h * width + w] != hmax[c * height * width + h * width + w]) {
						hmax[c * height * width + h * width + w] = 0.0;
					}
				}
			}
		}

		if (hmax == ea_tensor_data(fairmot->hmax_tensor)) {
			// clean the written data in cache
			RVAL_OK(ea_tensor_sync_cache(fairmot->hmax_tensor, EA_CPU, EA_VP));
		}

		// topk_scores, topk_inds = torch.topk(scores.view(batch, cat, -1), K)
		per_class_score_top_k_ind = (int *)malloc(sizeof(int) * class_num * fairmot->top_k);
		RVAL_ASSERT(per_class_score_top_k_ind != NULL);
		for (c = 0; c < class_num; c++) {
			RVAL_OK(top_k_with_threshold(&hmax[c * height * width], height * width, fairmot->top_k, fairmot->det_threshold, &per_class_score_top_k_ind[c * fairmot->top_k]));
		}

		RVAL_BREAK();

		per_class_score_top_k = (float *)malloc(sizeof(float) * class_num * fairmot->top_k);
		for (c = 0; c < class_num; c++) {
			for (k = 0; k < fairmot->top_k; k++) {
				per_class_score_top_k[c * fairmot->top_k + k] = (&hmax[c * height * width])[per_class_score_top_k_ind[c * fairmot->top_k + k]];
			}
		}

		// topk_score, topk_ind = torch.topk(topk_scores.view(batch, -1), K)
		total_top_k_ind = (int *)malloc(sizeof(int) * fairmot->top_k);
		RVAL_OK(top_k(per_class_score_top_k, class_num * fairmot->top_k, fairmot->top_k, total_top_k_ind));

		// xs = xs.view(batch, K, 1) + reg[:, :, 0:1]
		// ys = ys.view(batch, K, 1) + reg[:, :, 1:2]

		// wh = _transpose_and_gather_feat(wh, inds)
		// wh = wh.view(batch, K, 4)
		// clses  = clses.view(batch, K, 1).float()
		// scores = scores.view(batch, K, 1)
		// bboxes = torch.cat([xs - wh[..., 0:1],
		// 					ys - wh[..., 1:2],
		// 					xs + wh[..., 2:3],
		// 					ys + wh[..., 3:4]], dim=2)
		x1y1x2y2score = (float *)malloc(sizeof(float) * class_num * fairmot->top_k * 5);
		reid_ind = (int *)malloc(sizeof(int) * class_num * fairmot->top_k);
		valid_count = (int *)malloc(sizeof(int) * class_num);
		memset(valid_count, 0, sizeof(int) * class_num);
		nms_x1y1x2y2score = (float *)malloc(sizeof(float) * class_num * fairmot->top_k * 5);
		nms_reid_ind = (int *)malloc(sizeof(int) * class_num * fairmot->top_k);
		nms_valid_count = (int *)malloc(sizeof(int) * class_num);
		memset(nms_valid_count, 0, sizeof(int) * class_num);

		for (k = 0; k < fairmot->top_k; k++) {
			c = total_top_k_ind[k] / fairmot->top_k;
			ind = per_class_score_top_k_ind[total_top_k_ind[k]];
			reid_ind[c * fairmot->top_k + valid_count[c]] = ind;

			x = (float)(ind % width);
			x += reg_x[ind];
			y = (float)(ind / width);
			y += reg_y[ind];
			x1 = max(0.0, x - wh_wl[ind]);
			y1 = max(0.0, y - wh_ht[ind]);
			x2 = max(0.0, x + wh_wr[ind]);
			y2 = max(0.0, y + wh_hb[ind]);
			score = hmax[c * height * width + ind];
			per_class_score_top_k[c * fairmot->top_k + k] = (&hmax[c * height * width])[per_class_score_top_k_ind[c * fairmot->top_k + k]];
			(&x1y1x2y2score[c * fairmot->top_k * 5 + valid_count[c] * 5])[0] = min(x1 / (width - 1), 1.0);
			(&x1y1x2y2score[c * fairmot->top_k * 5 + valid_count[c] * 5])[1] = min(y1 / (height - 1), 1.0);
			(&x1y1x2y2score[c * fairmot->top_k * 5 + valid_count[c] * 5])[2] = min(x2 / (width - 1), 1.0);
			(&x1y1x2y2score[c * fairmot->top_k * 5 + valid_count[c] * 5])[3] = min(y2 / (height - 1), 1.0);
			(&x1y1x2y2score[c * fairmot->top_k * 5 + valid_count[c] * 5])[4] = score;
			valid_count[c]++;
		}

		for (c = 0; c < class_num; c++) {
			RVAL_OK(ea_nms(&x1y1x2y2score[c * fairmot->top_k * 5], &reid_ind[c * fairmot->top_k], sizeof(int), valid_count[c], fairmot->nms_threshold, 0/*use_iou_min*/, 0/*top_k*/,
				&nms_x1y1x2y2score[c * fairmot->top_k * 5], &nms_reid_ind[c * fairmot->top_k], &nms_valid_count[c]));
		}

		RVAL_BREAK();

		result->valid_det_count = 0;
		result->reid_dim = reid_dim;
		for (c = 0; c < class_num; c++) {
			for (k = 0; k < nms_valid_count[c]; k++) {
				if ((&nms_x1y1x2y2score[c * fairmot->top_k * 5 + k * 5])[4] < fairmot->det_threshold) {
					continue;
				}

				result->detections[result->valid_det_count].id = c;
				result->detections[result->valid_det_count].score = (&nms_x1y1x2y2score[c * fairmot->top_k * 5 + k * 5])[4];
				x1 = (&nms_x1y1x2y2score[c * fairmot->top_k * 5 + k * 5])[0];
				y1 = (&nms_x1y1x2y2score[c * fairmot->top_k * 5 + k * 5])[1];
				x2 = (&nms_x1y1x2y2score[c * fairmot->top_k * 5 + k * 5])[2];
				y2 = (&nms_x1y1x2y2score[c * fairmot->top_k * 5 + k * 5])[3];
				if ((x2 - x1) * (y2 - y1) < fairmot->det_min_area) {
					continue;
				}

				result->detections[result->valid_det_count].x_start = x1;
				result->detections[result->valid_det_count].y_start = y1;
				result->detections[result->valid_det_count].x_end = x2;
				result->detections[result->valid_det_count].y_end = y2;
				for (d = 0; d < reid_dim; d++) {
					result->detections[result->valid_det_count].reid[d] = reid[d * height * width + nms_reid_ind[c * fairmot->top_k + k]];
				}

				result->valid_det_count++;
				if (result->valid_det_count >= FAIRMOT_MAX_OUT_NUM) {
					break;
				}
			}

			if (result->valid_det_count >= FAIRMOT_MAX_OUT_NUM) {
				break;
			}
		}
	} while (0);

	if (hm) {
		free(hm);
	}

	if (hmax) {
		free(hmax);
	}

	if (wh_wl) {
		free(wh_wl);
	}

	if (wh_ht) {
		free(wh_ht);
	}

	if (wh_wr) {
		free(wh_wr);
	}

	if (wh_hb) {
		free(wh_hb);
	}

	if (reg_x) {
		free(reg_x);
	}

	if (reg_y) {
		free(reg_y);
	}

	if (reid) {
		free(reid);
	}

	if (per_class_score_top_k_ind) {
		free(per_class_score_top_k_ind);
	}

	if (per_class_score_top_k) {
		free(per_class_score_top_k);
	}

	if (total_top_k_ind) {
		free(total_top_k_ind);
	}

	if (x1y1x2y2score){
		free(x1y1x2y2score);
	}

	if (reid_ind) {
		free(reid_ind);
	}

	if (valid_count) {
		free(valid_count);
	}

	if (nms_x1y1x2y2score) {
		free(nms_x1y1x2y2score);
	}

	if (nms_reid_ind) {
		free(nms_reid_ind);
	}

	if (nms_valid_count) {
		free(nms_valid_count);
	}

	return rval;
}
