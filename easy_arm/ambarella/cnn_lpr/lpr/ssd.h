/*******************************************************************************
 * ssd.h
 *
 * History:
 *  2020/07/28 - [Qiangqiang Zhang] create file
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

#ifndef __SSD_H__
#define __SSD_H__

#ifdef __cplusplus
extern "C" {
#endif

#define IN
#define OUT
#define SSD_NET_MAX_LABEL_NUM		1000
#define SSD_NET_MAX_LABEL_LEN		50

typedef struct ssd_net_params_s {
	ssd_tf_scale_factors_t *scale_factors;

	const char *model_path;
	const char *priorbox_path;
	const char *label_path;
	const char *input_name;
	const char *output_loc;
	const char *output_conf;

	uint32_t priority; /* The range is from 0 to 31. 0:lowest(default), 31:highest */
	uint32_t width;
	uint32_t height;
	float conf_threshold;
	uint32_t keep_top_k;
	float nms_threshold;
	uint32_t nms_top_k;
	uint32_t background_label_id;
	uint32_t class_num;
	uint32_t unnormalized;

	uint32_t debug_en : 1;
	uint32_t nnctrl_print_time : 1;
	uint32_t reserved : 30;
} ssd_net_params_t;

typedef struct ssd_net_ctx_s {
	char labels[SSD_NET_MAX_LABEL_NUM][SSD_NET_MAX_LABEL_LEN];

	dproc_ssd_detection_output_config_t dproc_ssd_config; // only for debug
	dproc_ssd_class_dbg_info_t *dproc_ssd_class_dbg_info; // only for debug

	ea_net_t *net;
	ea_tensor_t *output_loc_tensor;
	ea_tensor_t *output_conf_tensor;

	// for arm nms
	const char *priorbox_path;
	ssd_tf_scale_factors_t *scale_factors;
	float *priorbox_buf;

	uint32_t all_bbox_num;
	uint32_t class_num;
	uint32_t background_label_id;
	uint32_t width;
	uint32_t height;
	uint32_t nms_top_k;
	float nms_threshold;
	uint32_t keep_top_k;
	float conf_threshold;
	uint32_t unnormalized;
	uint32_t debug_en;
} ssd_net_ctx_t;

typedef struct ea_tensor_s ea_tensor_t;
typedef struct ssd_net_input_s {
	ea_tensor_t *tensor; /* CAFFE: BGR; TF: RGB*/
} ssd_net_input_t;

typedef struct ssd_net_vp_result_info_s {
	uint8_t *loc_dram_addr;
	uint8_t *conf_dram_addr;
	size_t loc_dram_size;
	size_t conf_dram_size;
	int max_dproc_ssd_result_num;
} ssd_net_vp_result_info_t;

typedef struct ssd_net_final_result_s {
	char labels[SSD_NET_MAX_LABEL_NUM][SSD_NET_MAX_LABEL_LEN];
	dproc_ssd_detection_output_result_t *dproc_ssd_result;
	int ssd_det_num;
} ssd_net_final_result_t;

int ssd_net_init(IN const ssd_net_params_t *params, IN ssd_net_ctx_t *ssd_net_ctx,
	OUT ssd_net_input_t *input, OUT ssd_net_vp_result_info_t *vp_result_info);

void ssd_net_deinit(ssd_net_ctx_t *ssd_net_ctx);

int ssd_net_run_vp_forward(ssd_net_ctx_t *ssd_net_ctx);

int ssd_net_run_arm_nms(IN ssd_net_ctx_t *ssd_net_ctx, IN uint8_t *loc_dram_addr,
	IN uint8_t *conf_dram_addr, OUT ssd_net_final_result_t *ssd_net_result);

#ifdef __cplusplus
}
#endif

#endif

