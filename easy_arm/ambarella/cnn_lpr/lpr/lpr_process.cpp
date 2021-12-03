#include "lpr_process.h"

const static std::string lpr_model_path = "/data/lpr/segfree_inception_cavalry.bin";
const static std::vector<std::string> lpr_input_name = {"data"};
const static std::vector<std::string> lpr_output_name = {"prob"};

const static std::string lphm_model_path = "/data/lpr/LPHM_cavalry.bin";
const static std::vector<std::string> lphm_input_name = {"data"};
const static std::vector<std::string> lphm_output_name = {"dense"};

int ssd_critical_resource(
	dproc_ssd_detection_output_result_t *amba_ssd_result,
	ea_img_resource_data_t* imgs_data_addr, int ssd_result_num,
	state_buffer_t *ssd_mid_buf, global_control_param_t *G_param)
{
	int i, rval = 0;
	ea_img_resource_data_t covered_imgs_addr;
	uint8_t buffer_covered = 0;

	do {
		ssd_result_num = min(MAX_DETECTED_LICENSE_NUM, ssd_result_num);
		for (i = 0; i < ssd_result_num; i++) {
			ssd_mid_buf->bbox_param[i].norm_max_x =
				amba_ssd_result[i].bbox.x_max;
			ssd_mid_buf->bbox_param[i].norm_max_y =
				amba_ssd_result[i].bbox.y_max;
			ssd_mid_buf->bbox_param[i].norm_min_x =
				amba_ssd_result[i].bbox.x_min;
			ssd_mid_buf->bbox_param[i].norm_min_y =
				amba_ssd_result[i].bbox.y_min;
		}
		ssd_mid_buf->object_num = ssd_result_num;
		memcpy(ssd_mid_buf->img_resource_addr, imgs_data_addr,
			G_param->ssd_result_buf.img_resource_len);
		RVAL_OK(write_state_buffer(&G_param->ssd_result_buf, ssd_mid_buf,
			&G_param->access_buffer_mutex, &G_param->sem_readable_buf,
			(void*)&covered_imgs_addr, &buffer_covered));
		if (buffer_covered) {
			RVAL_OK(ea_img_resource_drop_data(G_param->img_resource, &covered_imgs_addr));
		}
	} while (0);

	return rval;
}

int lpr_critical_resource(uint16_t *license_num, bbox_param_t *bbox_param,
	state_buffer_t *ssd_mid_buf, global_control_param_t *G_param)
{
	int i, rval = 0;

	do {
		RVAL_OK(read_state_buffer(ssd_mid_buf, &G_param->ssd_result_buf,
			&G_param->access_buffer_mutex, &G_param->sem_readable_buf));
		*license_num = ssd_mid_buf->object_num;
		if (*license_num > MAX_DETECTED_LICENSE_NUM) {
			LOG(ERROR) << "license_num " << *license_num << " > " << MAX_DETECTED_LICENSE_NUM;
			rval = -1;
			break;
		}
		for (i = 0; i < *license_num; ++i) {
			bbox_param[i].norm_min_x = ssd_mid_buf->bbox_param[i].norm_min_x;
			bbox_param[i].norm_min_y = ssd_mid_buf->bbox_param[i].norm_min_y;
			bbox_param[i].norm_max_x = ssd_mid_buf->bbox_param[i].norm_max_x;
			bbox_param[i].norm_max_y = ssd_mid_buf->bbox_param[i].norm_max_y;
		}
		if (G_param->debug_en >= INFO_LEVEL) {
			LOG(INFO) << "------------------------------";
			LOG(INFO) << "LPR got bboxes:";
			for (i = 0; i < *license_num; ++i) {
				LOG(INFO) << i << " " << bbox_param[i].norm_min_x << "," << bbox_param[i].norm_min_y << "|" << \
				bbox_param[i].norm_max_x << "," << bbox_param[i].norm_max_y;
			}
			LOG(INFO) << "------------------------------";
		}
	} while (0);

	return rval;
}

int init_LPR(LPR_ctx_t *LPR_ctx, global_control_param_t *G_param)
{
	int rval = 0;
	LOG(INFO) << "init_LPR";
	do {
		LPR_ctx->LPHM_net_ctx.input_name = const_cast<char*>(lphm_input_name[0].c_str());
	    LPR_ctx->LPHM_net_ctx.output_name = const_cast<char*>(lphm_output_name[0].c_str());
		LPR_ctx->LPHM_net_ctx.net_name = const_cast<char*>(lphm_model_path.c_str());
		LPR_ctx->LPHM_net_ctx.net_verbose = G_param->verbose;
		LPR_ctx->LPHM_net_ctx.net_param.priority = LPR_PRIORITY;

		LPR_ctx->LPR_net_ctx.input_name = const_cast<char*>(lpr_input_name[0].c_str());
		LPR_ctx->LPR_net_ctx.output_name = const_cast<char*>(lpr_output_name[0].c_str());
		LPR_ctx->LPR_net_ctx.net_name = const_cast<char*>(lpr_model_path.c_str());
		LPR_ctx->LPR_net_ctx.net_verbose = G_param->verbose;
		LPR_ctx->LPR_net_ctx.net_param.priority = LPR_PRIORITY;

		LPR_ctx->debug_en = G_param->debug_en;
		RVAL_OK(LPR_init(LPR_ctx));
	} while (0);
	LOG(INFO) << "init_LPR success";
	return rval;
}

void draw_overlay_preprocess(draw_plate_list_t *draw_plate_list,
	license_list_t *license_result, bbox_param_t *bbox_param, global_control_param_t *G_param)
{
	uint32_t i = 0;
	int draw_num = 0;
	bbox_param_t scaled_bbox_draw[MAX_DETECTED_LICENSE_NUM];
	license_plate_t *plates = draw_plate_list->license_plate;
	license_info_t *license_info = license_result->license_info;

	license_result->license_num = min(license_result->license_num, MAX_OVERLAY_PLATE_NUM);
	for (i = 0; i < license_result->license_num; ++i) {
		if (license_info[i].conf > G_param->recg_threshold &&
			strlen(license_info[i].text) == CHINESE_LICENSE_STR_LEN) {
			upscale_normalized_rectangle(bbox_param[i].norm_min_x, bbox_param[i].norm_min_y,
			bbox_param[i].norm_max_x, bbox_param[i].norm_max_y,
				DRAW_LICNESE_UPSCALE_W, DRAW_LICNESE_UPSCALE_H, &scaled_bbox_draw[i]);
			plates[draw_num].bbox.norm_min_x = scaled_bbox_draw[i].norm_min_x;
			plates[draw_num].bbox.norm_min_y = scaled_bbox_draw[i].norm_min_y;
			plates[draw_num].bbox.norm_max_x = scaled_bbox_draw[i].norm_max_x;
			plates[draw_num].bbox.norm_max_y = scaled_bbox_draw[i].norm_max_y;
			plates[draw_num].conf = license_info[i].conf;
			memset(plates[draw_num].text, 0, sizeof(plates[draw_num].text));
			strncpy(plates[draw_num].text, license_info[i].text,
				sizeof(plates[draw_num].text));
			plates[draw_num].text[sizeof(plates[draw_num].text) - 1] = '\0';
			++draw_num;
			if (G_param->debug_en > 0) {
				LOG(INFO) << "Drawed license:" << license_info[i].text << "," << license_info[i].conf;
			}
		}
	}
	draw_plate_list->license_num = draw_num;

	return;
}