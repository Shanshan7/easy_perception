#include <iostream>

#include "sde_tracker.h"


int amba_cv_env_init(sde_track_ctx_t *track_ctx)
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

		// track_ctx->img_resource = ea_img_resource_new(EA_CANVAS, (void *)(unsigned long)track_ctx->params.canvas_id);
        track_ctx->img_resource = ea_img_resource_new(EA_JPEG_FOLDER, (void *)track_ctx->input_dir);
		RVAL_ASSERT(track_ctx->img_resource != NULL);

        track_ctx->display = ea_display_new(EA_DISPLAY_JPEG, EA_TENSOR_COLOR_MODE_BGR, EA_DISPLAY_BBOX_TEXTBOX, (void *)track_ctx->output_dir);
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

void amba_cv_env_deinit(sde_track_ctx_t *track_ctx)
{
	ea_display_free(track_ctx->display);
	track_ctx->display = NULL;
	ea_img_resource_free(track_ctx->img_resource);
	track_ctx->img_resource = NULL;
	ea_env_close();
}

int amba_draw_detection(sde_track_ctx_t *track_ctx, std::vector<DetectBox> &det_results, 
                        std::map<int, TrajectoryParams> track_idx_map, ea_tensor_t *img_tensor)
{
	int rval = 0;
    char text[MAX_LABEL_LEN];
	int border_thickness;

	ea_display_obj_params(track_ctx->display)->obj_win_w = 1.0;
	ea_display_obj_params(track_ctx->display)->obj_win_h = 1.0;
    ea_display_obj_params(track_ctx->display)->border_thickness = 2;
    ea_display_obj_params(track_ctx->display)->font_size = 18;

	ea_display_obj_params(track_ctx->display)->text_color = EA_16_COLORS_YELLOW;

	do {
		// draw person box
		ea_display_obj_params(track_ctx->display)->border_thickness = 2;
		ea_display_obj_params(track_ctx->display)->font_size = 8;
		for (int i = 0; i < det_results.size(); i++) {
			snprintf(text, MAX_LABEL_LEN, "");
			// track_idx_map[track_ctx->mot_result.tracks[i].track_id].mean_velocity); // track_ctx->mot_result.tracks[i].x_start

			ea_display_obj_params(track_ctx->display)->box_color = EA_16_COLORS_YELLOW;
			ea_display_set_bbox(track_ctx->display, text,
				det_results[i].x1 / 960, det_results[i].y1 / 540,
				(det_results[i].x2 - det_results[i].x1) / 960,
				(det_results[i].y2 - det_results[i].y1) / 540);
		}

		// draw ID trajectory
		for (std::map<int, TrajectoryParams>::iterator it = track_idx_map.begin(); it != track_idx_map.end(); ++it)
		{
			if(it->second.draw_flag == 1) {
				ea_display_obj_params(track_ctx->display)->border_thickness = 2;
				ea_display_obj_params(track_ctx->display)->box_color = (ea_16_colors_t)(it->first % EA_16_COLORS_MAX_NUM);
				snprintf(text, MAX_LABEL_LEN, "");
				for (int j = 0; j < it->second.trajectory_position.size(); j++) {
					if (it->second.trajectory_position[j].x / 960 <= 1.0 - 2 / 960 && 
						it->second.trajectory_position[j].y / 540 <= 1.0 - 2 / 540) {
						ea_display_set_bbox(track_ctx->display, text,
						it->second.trajectory_position[j].x  / 960, it->second.trajectory_position[j].y / 540,
						2 / 960,
						2 / 540);
					}
				}
			}
		}

		ea_display_refresh(track_ctx->display, (void *)img_tensor);
	} while (0);

	return rval;
}