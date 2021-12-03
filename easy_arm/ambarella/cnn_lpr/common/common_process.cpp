#include "common_process.h"

#define DEFAULT_STATE_BUF_NUM		(3)
#define DEFAULT_STREAM_ID			(2)
#define DEFAULT_CHANNEL_ID			(2)
#define DEFAULT_SSD_LAYER_ID		(1)
#define DEFAULT_LPR_LAYER_ID		(0)
#define DEFAULT_RGB_TYPE			(1) /* 0: RGB, 1:BGR */
#define DEFAULT_SSD_CLASS_NUM		(2) /* For license and background */
#define DEFAULT_BACKGROUND_ID		(0)
#define DEFAULT_KEEP_TOP_K			(50)
#define DEFAULT_NMS_TOP_K			(100)
#define DEFAULT_NMS_THRES			(0.45f)
#define DEFAULT_SSD_CONF_THRES		(0.3f)
#define DEFAULT_LPR_CONF_THRES		(0.9f)

int init_param(global_control_param_t *G_param)
{
	int rval = 0;
	LOG(INFO) << "init ...";
	memset(G_param, 0, sizeof(global_control_param_t));

	G_param->channel_id = DEFAULT_CHANNEL_ID;
	G_param->stream_id = DEFAULT_STREAM_ID;

	G_param->ssd_pyd_idx= DEFAULT_SSD_LAYER_ID;
	G_param->lpr_pyd_idx= DEFAULT_LPR_LAYER_ID;
	G_param->rgb_type = DEFAULT_RGB_TYPE;
	G_param->state_buf_num = DEFAULT_STATE_BUF_NUM;
	G_param->num_classes = DEFAULT_SSD_CLASS_NUM;
	G_param->background_label_id = DEFAULT_BACKGROUND_ID;
	G_param->keep_top_k = DEFAULT_KEEP_TOP_K;
	G_param->nms_top_k = DEFAULT_NMS_TOP_K;
	G_param->nms_threshold = DEFAULT_NMS_THRES;
	G_param->conf_threshold = DEFAULT_SSD_CONF_THRES;
	G_param->recg_threshold = DEFAULT_LPR_CONF_THRES;
	G_param->overlay_x_offset = DEFAULT_X_OFFSET;
	G_param->overlay_highlight_sec = DEFAULT_HIGHLIGHT_SEC;
	G_param->overlay_clear_sec = DEFAULT_CLEAR_SEC;
	G_param->overlay_text_width_ratio = DEFAULT_WIDTH_RATIO;
	G_param->draw_plate_num = DEFAULT_OVERLAY_LICENSE_NUM;
	G_param->debug_en = 0;
	G_param->verbose = 0;

	LOG(INFO) << "init sucess";

	return rval;
}

int env_init(global_control_param_t *G_param)
{
	int rval = 0;
	LOG(INFO) << "env_init";
	do {
		RVAL_OK(ea_env_open(EA_ENV_ENABLE_IAV
			| EA_ENV_ENABLE_CAVALRY
			| EA_ENV_ENABLE_VPROC
			| EA_ENV_ENABLE_NNCTRL
			| EA_ENV_ENABLE_OSD_STREAM));
		G_param->img_resource = ea_img_resource_new(EA_PYRAMID,
			(void *)(unsigned long)G_param->channel_id);
		RVAL_ASSERT(G_param->img_resource != NULL);
		RVAL_OK(init_overlay_tool(G_param->stream_id,
			G_param->overlay_x_offset, G_param->overlay_highlight_sec,
			G_param->overlay_clear_sec, G_param->overlay_text_width_ratio,
			G_param->draw_plate_num, G_param->debug_en));
		RVAL_OK(init_state_buffer_param(&G_param->ssd_result_buf,
			G_param->state_buf_num, (uint16_t)sizeof(ea_img_resource_data_t),
			30, (G_param->debug_en >= INFO_LEVEL)));
		pthread_mutex_init(&G_param->access_buffer_mutex, NULL);
		sem_init(&G_param->sem_readable_buf, 0, 0);
		RVAL_OK(ea_set_preprocess_priority_on_current_process(VPROC_PRIORITY, EA_VP));
	} while(0);
	LOG(INFO) << "env_init success";
	return rval;
}

void env_deinit(global_control_param_t *G_param)
{
	pthread_mutex_destroy(&G_param->access_buffer_mutex);
	sem_destroy(&G_param->sem_readable_buf);
	deinit_state_buffer_param(&G_param->ssd_result_buf);
	deinit_overlay_tool();
	ea_img_resource_free(G_param->img_resource);
	G_param->img_resource = NULL;
	ea_env_close();
}

int tensor2mat_yuv2bgr_nv12(ea_tensor_t *tensor, cv::Mat &bgr)
{
	int rval = EA_SUCCESS;
	cv::Mat nv12(ea_tensor_shape(tensor)[2] +
		(ea_tensor_related(tensor) == NULL ? 0 : ea_tensor_shape(ea_tensor_related(tensor))[2]),
		ea_tensor_shape(tensor)[3], CV_8UC1);
	uint8_t *p_src = NULL;
	uint8_t *p_dst = NULL;
	size_t h;

	do {
		RVAL_ASSERT(ea_tensor_shape(tensor)[1] == 1);

		p_src = (uint8_t *)ea_tensor_data_for_read(tensor, EA_CPU);
		p_dst = nv12.data;
		for (h = 0; h < ea_tensor_shape(tensor)[2]; h++) {
			memcpy(p_dst, p_src, ea_tensor_shape(tensor)[3]);
			p_src += ea_tensor_pitch(tensor);
			p_dst += ea_tensor_shape(tensor)[3];
		}

		if (ea_tensor_related(tensor)) {
			p_src = (uint8_t *)ea_tensor_data_for_read(ea_tensor_related(tensor), EA_CPU);
			for (h = 0; h < ea_tensor_shape(ea_tensor_related(tensor))[2]; h++) {
				memcpy(p_dst, p_src, ea_tensor_shape(ea_tensor_related(tensor))[3]);
				p_src += ea_tensor_pitch(ea_tensor_related(tensor));
				p_dst += ea_tensor_shape(ea_tensor_related(tensor))[3];
			}
		}

		#if CV_VERSION_MAJOR < 4
			cv::cvtColor(nv12, bgr, CV_YUV2BGR_NV12);
		#else
			cv::cvtColor(nv12, bgr, COLOR_YUV2BGR_NV12);
		#endif
	} while (0);

	return rval;
}

void fill_data(unsigned char* addr, int data)
{
	addr[0] = (data >> 24) & 0xFF;
	addr[1] = (data >> 16) & 0xFF;
	addr[2] = (data >>  8) & 0xFF;
	addr[3] = (data >>  0) & 0xFF;
}