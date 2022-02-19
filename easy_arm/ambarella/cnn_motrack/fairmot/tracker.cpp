#include "tracker.h"
#include "../postprocess/calculate_trajectory.h"

int amba_cv_env_init(track_ctx_t *track_ctx)
{
    int rval = 0;
	int features = 0;

	do {
		features = EA_ENV_ENABLE_IAV
			| EA_ENV_ENABLE_CAVALRY
			| EA_ENV_ENABLE_VPROC
			| EA_ENV_ENABLE_NNCTRL;

        features |= EA_ENV_ENABLE_OSD_VOUT 
            | EA_ENV_ENABLE_OSD_STREAM 
            | EA_ENV_ENABLE_OSD_JPEG;

		RVAL_OK(ea_env_open(features));

		track_ctx->img_resource = ea_img_resource_new(EA_CANVAS, (void *)(unsigned long)track_ctx->params.canvas_id);
		RVAL_ASSERT(track_ctx->img_resource != NULL);

        // track_ctx->display = ea_display_new(EA_DISPLAY_JPEG, EA_COLOR_YUV2RGB_NV12, EA_DISPLAY_BBOX_TEXTBOX, (void *)track_ctx->params.output_dir);
        // track_ctx->display = ea_display_new(EA_DISPLAY_STREAM, track_ctx->params.stream_id, EA_DISPLAY_BBOX_TEXTBOX, NULL);
		track_ctx->display = ea_display_new(EA_DISPLAY_VOUT, EA_DISPLAY_ANALOG_VOUT, EA_DISPLAY_BBOX_TEXTBOX, NULL);
		RVAL_ASSERT(track_ctx->display != NULL);
	} while(0);

	if (rval < 0) {
		if (track_ctx->display) {
			ea_display_free(track_ctx->display);
			track_ctx->display = NULL;
		}

		if (track_ctx->img_resource) {
			ea_img_resource_free(track_ctx->img_resource);
			track_ctx->img_resource = NULL;
		}
	}
	
	return rval;
}

int amba_track_init(track_ctx_t *track_ctx, track_params_t *params)
{
	int rval = 0;
	fairmot_params_t net_params;
	mot_params_t mot_params;
	char model_path[MAX_PATH_STRLEN + 1] = {0};
	char file_path[MAX_PATH_STRLEN + 1] = {0};

	memset(params, 0, sizeof(track_params_t));

	params->log_level = EA_LOG_LEVEL_NOTICE;
	params->draw_mode = 0;
	params->canvas_id = 1;
	params->stream_id = 0;
	params->det_threshold = 0.24f;                  // detection threshold
	params->det_min_area = 0.0038f;
	params->nms_threshold = 0.7f;
	params->top_k = 80;                             // maximum number of detection objects
	// params->measure_time = 0;
	params->mot_image_width = 1920;
	params->mot_image_height = 1080;
	params->embedding_reliable_use_iou = 0;
	params->embedding_reliable_distance = 0.2f;
	params->embedding_cosine_distance = 0.35f;
	params->overlap_reliable_use_iou = 1;
	params->overlap_reliable_distance = 0.3f;
	params->overlap_iou_distance = 0.6f;
	params->confirming_use_iou = 1;
	params->confirming_distance = 0.8f;
	params->duplicate_iou_distance = 0.1f;
	params->max_lost_age = 35;                      // maximum tracking time
	params->fuse_lambda = 0.99f;
	params->use_match_priority = 0;
	params->merge_match_priority = 0;
	params->match_priority_thresh = 6;
	params->delta_time = 0.1f;
	params->use_smooth_feature = 0;
	params->feature_buffer_size = 20;
	params->smooth_alpha = 0.5f;
	params->tentative_thresh = 10;
	params->mot_verbose = 0;

	do {
        RVAL_OK(amba_cv_env_init(track_ctx));
		LOG(INFO) << "[AMBA ENV] AMBA CV environment initial success!";

		memset(&net_params, 0, sizeof(fairmot_params_t));
		net_params.log_level = params->log_level;
		snprintf(model_path, sizeof(model_path), "%s", FAIRMOT_MODEL_PATH);
		// strncat(model_path, FAIRMOT_MODEL_FILE_NAME, MAX_PATH_STRLEN - strlen(model_path));

        net_params.model_path = model_path;
        net_params.input_name = FAIRMOT_INPUT_NAME;
        net_params.hm_name = FAIRMOT_HEATMAP_NAME;
        net_params.hmax_name = FAIRMOT_HEATMAP_MAX_NAME;
        net_params.wh_name = FAIRMOT_WH_NAME;
        net_params.reg_name = FAIRMOT_REG_NAME;
        net_params.id_name = FAIRMOT_ID_NAME;
		net_params.det_threshold = params->det_threshold;
		net_params.det_min_area = params->det_min_area;
		net_params.nms_threshold = params->nms_threshold;
		net_params.keep_top_k = params->top_k;
		// net_params.measure_time = params->measure_time;
		RVAL_OK(fairmot_init(&track_ctx->fairmot, &net_params));
		LOG(INFO) << "[AMBA Track] AMBA fairmot initial success!";

		memset(&mot_params, 0, sizeof(mot_params_t));
		mot_params.log_level = params->log_level;
		// mot_params.measure_time = params->measure_time;
		mot_params.image_width = params->mot_image_width;
		mot_params.image_height = params->mot_image_height;
		mot_params.embedding_reliable_use_iou = params->embedding_reliable_use_iou;
		mot_params.embedding_reliable_distance = params->embedding_reliable_distance;
		mot_params.embedding_cosine_distance = params->embedding_cosine_distance;
		mot_params.overlap_reliable_use_iou = params->overlap_reliable_use_iou;
		mot_params.overlap_reliable_distance = params->overlap_reliable_distance;
		mot_params.overlap_iou_distance = params->overlap_iou_distance;
		mot_params.confirming_use_iou = params->confirming_use_iou;
		mot_params.confirming_distance = params->confirming_distance;
		mot_params.duplicate_iou_distance = params->duplicate_iou_distance;
		mot_params.max_lost_age = params->max_lost_age;
		mot_params.fuse_lambda = params->fuse_lambda;
		mot_params.use_match_priority = params->use_match_priority;
		mot_params.merge_match_priority = params->merge_match_priority;
		mot_params.match_priority_thresh = params->match_priority_thresh;
		mot_params.dt = params->delta_time;
		mot_params.use_smooth_feature = params->use_smooth_feature;
		mot_params.feature_buffer_size = params->feature_buffer_size;
		mot_params.smooth_alpha = params->smooth_alpha;
		mot_params.tentative_thresh = params->tentative_thresh;
		mot_params.verbose = params->mot_verbose;
		RVAL_OK(mot_init(&track_ctx->mot, &mot_params));
		LOG(INFO) << "[AMBA Track] AMBA multi-object track initial success!";

// #ifdef IS_SEND_DATA
// 		SAVE_LOG_PROCESS(network_transmission.socket_init(), "[net_trans] Network Transmission initial");
// #endif

        RVAL_OK(pthread_create(&track_ctx->det_tidp, NULL,
            det_thread_func, track_ctx));
		LOG(INFO) << "[AMBA Track] AMBA detection thread initial success!";
	} while (0);

	return rval;
}

void *det_thread_func(void *arg)
{
	int rval = 0;
	track_ctx_t *ctx = (track_ctx_t *)arg;
	track_params_t *params = &ctx->params;
	ea_img_resource_data_t data;

	do {
		RVAL_OK(ea_img_resource_hold_data(ctx->img_resource, &data));
		LOG(INFO) << "[AMBA Track] AMBA hold image data success!";

        RVAL_ASSERT(data.tensor_group != NULL);
        RVAL_ASSERT(data.tensor_num >= 1);
        RVAL_ASSERT(data.tensor_group[0] != NULL);
        RVAL_OK(ea_cvt_color_resize(data.tensor_group[0], fairmot_input(&ctx->fairmot), EA_COLOR_YUV2RGB_NV12, EA_VP));
		// LOG(INFO) << "[AMBA Track] AMBA resize and convert color success!";

		// it is mainly used to cache data
		ctx->img_data[ctx->tail] = data;
		ctx->tail++;
		if (ctx->tail == IMG_DATA_QUEUE_SIZE) {
			ctx->tail = 0;
		}

		RVAL_OK(fairmot_vp_forward_in_parallel(&ctx->fairmot));
		// LOG(INFO) << "[AMBA Track] AMBA VP detect object success!";
	} while (ctx->det_stop_flag == 0);

	return (void *)(unsigned long)rval;
}

int amba_draw_detection(std::map<int, TrajectoryParams> track_idx_map, track_ctx_t *track_ctx, uint32_t dsp_pts)
{
	int rval = 0;
	char text[MAX_LABEL_LEN];
	int border_thickness;

	ea_display_obj_params(track_ctx->display)->obj_win_w = 1.0;
	ea_display_obj_params(track_ctx->display)->obj_win_h = 1.0;
	if (ea_display_obj_params(track_ctx->display)->dis_win_w < HIGH_RESOLUTION_BEGIN) {
		ea_display_obj_params(track_ctx->display)->border_thickness = 2;
		ea_display_obj_params(track_ctx->display)->font_size = 18;
	} else {
		ea_display_obj_params(track_ctx->display)->border_thickness = 5;
		ea_display_obj_params(track_ctx->display)->font_size = 32;
	}

	ea_display_obj_params(track_ctx->display)->text_color = EA_16_COLORS_WHITE;

	do {
		snprintf(text, MAX_LABEL_LEN, "Frame %d", track_ctx->mot_result.frame_id, track_ctx->mot_result.valid_track_count);
		border_thickness = ea_display_obj_params(track_ctx->display)->border_thickness;
		ea_display_obj_params(track_ctx->display)->border_thickness = 0;
		ea_display_set_textbox(track_ctx->display, text, 20 / ea_display_obj_params(track_ctx->display)->dis_win_w, 
		20 / ea_display_obj_params(track_ctx->display)->dis_win_h,
			ea_display_obj_params(track_ctx->display)->font_size * strlen(text) * 2 / ea_display_obj_params(track_ctx->display)->dis_win_w,
			ea_display_obj_params(track_ctx->display)->font_size * 2 / ea_display_obj_params(track_ctx->display)->dis_win_h);

		snprintf(text, MAX_LABEL_LEN, "Counted %d", track_ctx->mot_result.valid_track_count);
		ea_display_obj_params(track_ctx->display)->border_thickness = 0;
		ea_display_set_textbox(track_ctx->display, text, 
		    20 / ea_display_obj_params(track_ctx->display)->dis_win_w, 60 / ea_display_obj_params(track_ctx->display)->dis_win_h,
			ea_display_obj_params(track_ctx->display)->font_size * strlen(text) * 2 / ea_display_obj_params(track_ctx->display)->dis_win_w,
			ea_display_obj_params(track_ctx->display)->font_size * 2 / ea_display_obj_params(track_ctx->display)->dis_win_h);

		// draw person box
		ea_display_obj_params(track_ctx->display)->border_thickness = border_thickness;
		ea_display_obj_params(track_ctx->display)->font_size = 8;
		for (int i = 0; i < track_ctx->mot_result.valid_track_count; i++) {
			snprintf(text, MAX_LABEL_LEN, "ID %d V %.02f", track_ctx->mot_result.tracks[i].track_id, 
			track_idx_map[track_ctx->mot_result.tracks[i].track_id].mean_velocity); // track_ctx->mot_result.tracks[i].x_start

			ea_display_obj_params(track_ctx->display)->box_color = (ea_16_colors_t)(track_ctx->mot_result.tracks[i].track_id % EA_16_COLORS_MAX_NUM);
			ea_display_set_bbox(track_ctx->display, text,
				track_ctx->mot_result.tracks[i].x_start, track_ctx->mot_result.tracks[i].y_start,
				track_ctx->mot_result.tracks[i].x_end - track_ctx->mot_result.tracks[i].x_start,
				track_ctx->mot_result.tracks[i].y_end - track_ctx->mot_result.tracks[i].y_start);
		}

		// draw ID trajectory
		for (std::map<int, TrajectoryParams>::iterator it = track_idx_map.begin(); it != track_idx_map.end(); ++it)
		{
			if(it->second.draw_flag == 1) {
				ea_display_obj_params(track_ctx->display)->border_thickness = 2;
				ea_display_obj_params(track_ctx->display)->box_color = (ea_16_colors_t)(it->first % EA_16_COLORS_MAX_NUM);
				snprintf(text, MAX_LABEL_LEN, "");
				for (int j = 0; j < it->second.trajectory_position.size(); j++) {
					if (it->second.trajectory_position[j].x <= 1.0 - 2 / ea_display_obj_params(track_ctx->display)->dis_win_w&& 
						it->second.trajectory_position[j].y <= 1.0 - 2 / ea_display_obj_params(track_ctx->display)->dis_win_h) {
						ea_display_set_bbox(track_ctx->display, text,
						it->second.trajectory_position[j].x, it->second.trajectory_position[j].y,
						2 / ea_display_obj_params(track_ctx->display)->dis_win_w,
						2 / ea_display_obj_params(track_ctx->display)->dis_win_h);
					}
				}
			}
		}

		ea_display_refresh(track_ctx->display, (void *)(unsigned long)dsp_pts);
	} while (0);

	return rval;
}

int amba_track_run(track_ctx_t *track_ctx, std::map<int, TrajectoryParams> &track_idx_map) // track_params_t *params
{
	int rval = 0;
	// ea_calc_fps_ctx_t calc_fps_ctx;
	// float fps;
	mot_input_t mot_input;

	// memset(&calc_fps_ctx, 0, sizeof(ea_calc_fps_ctx_t));
	// calc_fps_ctx.count_period = 100;

	do {
        RVAL_OK(fairmot_arm_post_process_in_parallel(&track_ctx->fairmot, &track_ctx->net_result));
		// LOG(INFO) << "[AMBA Track] AMBA Arm Post Process success!";

        if (track_ctx->img_data[track_ctx->head].tensor_group == NULL) {
            break;
        }

        track_ctx->loop_count++;
        memset(&mot_input, 0, sizeof(mot_input_t));
        RVAL_ASSERT(track_ctx->net_result.reid_dim == MOT_REID_DIM);
        mot_input.valid_det_count = track_ctx->net_result.valid_det_count;
        for (int i = 0; i < track_ctx->net_result.valid_det_count; i++) {
            mot_input.detections[i].class_id = track_ctx->net_result.detections[i].id;
            mot_input.detections[i].score = track_ctx->net_result.detections[i].score;
            mot_input.detections[i].x_start = track_ctx->net_result.detections[i].x_start;
            mot_input.detections[i].x_end = track_ctx->net_result.detections[i].x_end;
            mot_input.detections[i].y_start = track_ctx->net_result.detections[i].y_start;
            mot_input.detections[i].y_end = track_ctx->net_result.detections[i].y_end;
            memcpy(mot_input.detections[i].reid, track_ctx->net_result.detections[i].reid, sizeof(mot_input.detections[i]));
        }

        RVAL_OK(mot_process(&track_ctx->mot, &mot_input, &track_ctx->mot_result));

        // save_result(live_ctx); // save fairmot result
		CalculateTraj calculate_traj;
		SAVE_LOG_PROCESS(calculate_traj.calculate_tracking_trajectory(&track_ctx->mot_result, track_idx_map, ea_display_obj_params(track_ctx->display)->dis_win_h), "[Postprocess] Calculate Tracking trajectory");

		// draw result
#ifdef IS_SHOW
		amba_draw_detection(track_idx_map, track_ctx, track_ctx->img_data[track_ctx->head].dsp_pts);
#endif

        ea_img_resource_drop_data(track_ctx->img_resource, &track_ctx->img_data[track_ctx->head]);
        track_ctx->head++;
        if (track_ctx->head == IMG_DATA_QUEUE_SIZE) {
            track_ctx->head = 0;
        }

        // if (track_ctx->sig_flag) {
        //     break;
        // }
	} while (0);

    // track_ctx->det_stop_flag = 1;
    // fairmot_vp_forward_signal(&track_ctx->fairmot);
    // pthread_join(track_ctx->det_tidp, NULL);

	// LOG(INFO) << "Track stops after " << track_ctx->loop_count << " loops";

	return rval;
}

void amba_cv_env_deinit(track_ctx_t *track_ctx)
{
	ea_display_free(track_ctx->display);
	track_ctx->display = NULL;
	ea_img_resource_free(track_ctx->img_resource);
	track_ctx->img_resource = NULL;
	ea_env_close();
}

void amba_track_deinit(track_ctx_t *track_ctx)
{
	mot_deinit(&track_ctx->mot);
	fairmot_deinit(&track_ctx->fairmot);
    amba_cv_env_deinit(track_ctx);
	EA_LOG_NOTICE("live_deinit\n");
}