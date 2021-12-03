/*******************************************************************************
 * fairmot.h
 *
 * History:
 *  2020/12/14 - [Du You] create file
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

#ifndef _FAIRMOT_H_
#define _FAIRMOT_H_

#ifdef __cplusplus
extern "C" {
#endif

#define FAIRMOT_MAX_LABEL_NUM		183
#define FAIRMOT_MAX_LABEL_LEN		48
#define FAIRMOT_MAX_OUT_NUM			100
#define FAIRMOT_MAX_REID_DIM		128

typedef struct ea_tensor_s ea_tensor_t;

struct fairmot_s {
	ea_net_t *net;
	ea_tensor_t *input_tensor;
	ea_tensor_t *hm_tensor;
	ea_tensor_t *hmax_tensor;
	ea_tensor_t *wh_tensor;
	ea_tensor_t *reg_tensor;
	ea_tensor_t *id_tensor;
	int top_k;
	float nms_threshold;
	float det_threshold;
	float det_min_area;
	int measure_time;

	void *mutex;
	void *cond;
	int flag;
};
typedef struct fairmot_s fairmot_t;

typedef struct fairmot_params_s {
	int log_level;
	int measure_time;

	const char *model_path;
	const char *input_name;
	const char *hm_name;
	const char *hmax_name;
	const char *wh_name;
	const char *reg_name;
	const char *id_name;

	float det_threshold;	/*!< the threshold for valid bboxes */
	float det_min_area;		/*!< the min area for valid bboxes */
	int keep_top_k;			/*!< the maximum number of objects to keep */
	float nms_threshold;	/*!< the threshold for nms */
} fairmot_params_t;

int fairmot_init(fairmot_t *fairmot, const fairmot_params_t *params);
void fairmot_deinit(fairmot_t *fairmot);
ea_tensor_t *fairmot_input(fairmot_t *fairmot);
int fairmot_vp_forward_in_parallel(fairmot_t *fairmot);
int fairmot_vp_forward_signal(fairmot_t *fairmot);

typedef struct fairmot_det_s {
	float score;
	int id;
	float x_start; // normalized value
	float y_start;
	float x_end;
	float y_end;
	float reid[FAIRMOT_MAX_REID_DIM];
} fairmot_det_t;

typedef struct  fairmot_result_s {
	fairmot_det_t detections[FAIRMOT_MAX_OUT_NUM];
	int reid_dim;
	int valid_det_count;
} fairmot_result_t;

int fairmot_arm_post_process_in_parallel(fairmot_t *fairmot, fairmot_result_t *result);

#ifdef __cplusplus
}
#endif

#endif
