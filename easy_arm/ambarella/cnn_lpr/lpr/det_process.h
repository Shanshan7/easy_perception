#ifndef _DET_PROCESS_H_
#define _DET_PROCESS_H_

#include "cnn_lpr/common/common_process.h"
#include "cnn_lpr/lpr/ssd.h"

typedef struct SSD_ctx_s {
	ea_net_t *net;
	ssd_net_ctx_t ssd_net_ctx;
	ssd_net_input_t net_input;
	ssd_net_vp_result_info_t vp_result_info;
} SSD_ctx_t;

typedef struct lpr_thread_params_s {
	uint16_t width;
	uint16_t height;
	uint16_t pitch;
	uint16_t reserved;
	global_control_param_t *G_param;
} ssd_lpr_thread_params_t;

void upscale_normalized_rectangle(float x_min, float y_min,
	float x_max, float y_max, float w_ratio, float h_ratio,
	bbox_param_t *bbox_scaled);

int init_ssd(SSD_ctx_t *SSD_ctx, global_control_param_t *G_param,
	uint32_t buffer_h, uint32_t buffer_w);

#endif // _DET_PROCESS_H_