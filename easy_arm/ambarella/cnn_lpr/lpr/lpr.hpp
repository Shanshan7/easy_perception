/*******************************************************************************
 * lpr.hpp
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

#ifndef __LPR_H__
#define __LPR_H__

// MAX_LICENSE_LENGTH is decided by net output feature.
#define MAX_LICENSE_LENGTH			(20)
#define MAX_DETECTED_LICENSE_NUM	(30)

typedef struct license_info_s {
	char text[MAX_LICENSE_LENGTH];
	float conf;
} license_info_t;

typedef struct license_list_s {
	license_info_t license_info[MAX_DETECTED_LICENSE_NUM];
	uint32_t license_num;
} license_list_t;

typedef struct LPR_net_ctx_s {
	char *net_name;
	char *input_name;
	char *output_name;
	ea_net_t *net;
	ea_tensor_t *input_tensor;
	ea_tensor_t *output_tensor;
	ea_net_params_t net_param;
	int net_verbose;
} LPR_net_ctx_t;

typedef struct LPR_ctx_s {
	void *prc;
	int debug_en;
	uint16_t img_h;
	uint16_t img_w;
	LPR_net_ctx_t LPR_net_ctx;
	LPR_net_ctx_t LPHM_net_ctx;
	ea_tensor_t *rgb_img;
	ea_tensor_t *cropped_rgb_img[MAX_DETECTED_LICENSE_NUM];
	ea_tensor_t *cropped_mat_img[MAX_DETECTED_LICENSE_NUM];
} LPR_ctx_t;

int LPR_init(LPR_ctx_t *LPR_ctx);
void LPR_deinit(LPR_ctx_t *LPR_ctx);
int LPR_run(LPR_ctx_t *LPR_ctx, ea_tensor_t *input_tensor, uint16_t license_num,
	void *bbox_param, license_list_t *license_result);
#endif

