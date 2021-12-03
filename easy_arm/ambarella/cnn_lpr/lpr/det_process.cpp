#include "det_process.h"

const static std::string ssd_model_path = "/data/lpr/mobilenetv1_ssd_cavalry.bin";
const static std::string ssd_priorbox_path = "/data/lpr/lpr_priorbox_fp32.bin";
const static std::vector<std::string> ssd_input_name = {"data"};
const static std::vector<std::string> ssd_output_name = {"mbox_loc", "mbox_conf_flatten"};

void upscale_normalized_rectangle(float x_min, float y_min,
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


int init_ssd(SSD_ctx_t *SSD_ctx, global_control_param_t *G_param,
	uint32_t buffer_h, uint32_t buffer_w)
{
	int rval = 0;
	ssd_net_params_t ssd_net_params;
	ssd_tf_scale_factors_t scale_factors;
	LOG(INFO) << "init_ssd";
	do {
		memset(&ssd_net_params, 0, sizeof(ssd_net_params));
		// set params for ssd_net
		ssd_net_params.model_path = ssd_model_path.c_str();
		ssd_net_params.priorbox_path = ssd_priorbox_path.c_str();
		ssd_net_params.label_path = NULL;
		ssd_net_params.input_name = ssd_input_name[0].c_str();
		ssd_net_params.output_loc = ssd_output_name[0].c_str();
		ssd_net_params.output_conf = ssd_output_name[1].c_str();
		ssd_net_params.width = buffer_w;
		ssd_net_params.height = buffer_h;
		ssd_net_params.conf_threshold = G_param->conf_threshold;
		ssd_net_params.keep_top_k = G_param->keep_top_k;
		ssd_net_params.nms_threshold = G_param->nms_threshold;
		ssd_net_params.nms_top_k = G_param->nms_top_k;
		ssd_net_params.background_label_id = G_param->background_label_id;
		ssd_net_params.unnormalized = 0;
		ssd_net_params.class_num = G_param->num_classes;
		ssd_net_params.priority = SSD_PRIORITY;
		ssd_net_params.debug_en = (G_param->debug_en >= INFO_LEVEL);
		ssd_net_params.nnctrl_print_time = (G_param->verbose);
		scale_factors.center_x_scale = 0;
		scale_factors.center_y_scale = 0;
		scale_factors.height_scale = 0;
		scale_factors.width_scale = 0;
		ssd_net_params.scale_factors = &scale_factors;
		RVAL_OK(ssd_net_init(&ssd_net_params, &SSD_ctx->ssd_net_ctx,
			&SSD_ctx->net_input, &SSD_ctx->vp_result_info));
	} while (0);
	LOG(INFO) << "init_ssd success";
	return rval;
}
