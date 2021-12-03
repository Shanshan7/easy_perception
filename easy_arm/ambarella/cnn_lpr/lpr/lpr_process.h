#ifndef _LPR_PROCESS_H_
#define _LPR_PROCESS_H_

#include "cnn_lpr/common/common_process.h"
#include "cnn_lpr/lpr/lpr.hpp"
#include "cnn_lpr/lpr/det_process.h"

#define CHINESE_LICENSE_STR_LEN		(9)
#define DRAW_LICNESE_UPSCALE_H		(1.0f)
#define DRAW_LICNESE_UPSCALE_W		(0.2f)

int ssd_critical_resource(
	dproc_ssd_detection_output_result_t *amba_ssd_result,
	ea_img_resource_data_t* imgs_data_addr, int ssd_result_num,
	state_buffer_t *ssd_mid_buf, global_control_param_t *G_param);

int lpr_critical_resource(uint16_t *license_num, bbox_param_t *bbox_param,
	state_buffer_t *ssd_mid_buf, global_control_param_t *G_param);

int init_LPR(LPR_ctx_t *LPR_ctx, global_control_param_t *G_param);

void draw_overlay_preprocess(draw_plate_list_t *draw_plate_list,
	license_list_t *license_result, bbox_param_t *bbox_param, global_control_param_t *G_param);

#endif // _LPR_PROCESS_H_