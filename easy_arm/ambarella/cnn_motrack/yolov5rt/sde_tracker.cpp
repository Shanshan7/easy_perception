#include <iostream>

#include "sde_tracker.h"
#include "../common/utils.h"


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

		track_ctx->img_resource = ea_img_resource_new(EA_CANVAS, (void *)(unsigned long)track_ctx->canvas_id);
        // track_ctx->img_resource = ea_img_resource_new(EA_JPEG_FOLDER, (void *)track_ctx->input_dir);
		RVAL_ASSERT(track_ctx->img_resource != NULL);

        // track_ctx->display = ea_display_new(EA_DISPLAY_JPEG, EA_TENSOR_COLOR_MODE_BGR, EA_DISPLAY_BBOX_TEXTBOX, (void *)track_ctx->output_dir);
		// track_ctx->display = ea_display_new(EA_DISPLAY_STREAM, track_ctx->stream_id, EA_DISPLAY_BBOX_TEXTBOX, NULL);
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

void amba_cv_env_deinit(sde_track_ctx_t *track_ctx)
{
	ea_display_free(track_ctx->display);
	track_ctx->display = NULL;
	ea_img_resource_free(track_ctx->img_resource);
	track_ctx->img_resource = NULL;
	ea_env_close();
}

int amba_draw_detection(sde_track_ctx_t *track_ctx, std::vector<DetectBox> &det_results, 
                        std::map<int, TrajectoryParams> track_idx_map, ea_tensor_t *img_tensor, uint32_t dsp_pts)
{
	int rval = 0;
    char text[MAX_LABEL_LEN];

	int width = ea_tensor_shape(img_tensor)[3];
    int height = ea_tensor_shape(img_tensor)[2];

	ea_display_obj_params(track_ctx->display)->obj_win_w = 1.0;
	ea_display_obj_params(track_ctx->display)->obj_win_h = 1.0;

	ea_display_obj_params(track_ctx->display)->text_color = EA_16_COLORS_YELLOW;

	do {
		// draw person box
		ea_display_obj_params(track_ctx->display)->border_thickness = 2;
		ea_display_obj_params(track_ctx->display)->font_size = 8;
		unsigned long start_time = get_current_time();
		for (int i = 0; i < det_results.size(); i++) {
			snprintf(text, MAX_LABEL_LEN, "ID %d V %.02f", (int)det_results[i].trackID, det_results[i].confidence);
			// track_idx_map[track_ctx->mot_result.tracks[i].track_id].mean_velocity); // track_ctx->mot_result.tracks[i].x_start

			ea_display_obj_params(track_ctx->display)->box_color = EA_16_COLORS_YELLOW;
			float xmin = (det_results[i].x1 / width < 0.0)? 1 / width:det_results[i].x1 / width;
			float ymin = (det_results[i].y1 / height < 0.0)? 1 / height:det_results[i].y1 / height;
			float box_width = (det_results[i].x2 / width > 1.0)? 1.0 - xmin:det_results[i].x2 / width - xmin;
			float box_height = (det_results[i].y2 / height > 1.0)? 1.0 - ymin:det_results[i].y2 / height - ymin;
			ea_display_set_bbox(track_ctx->display, text,
				xmin, ymin, box_width, box_height);
		}
		std::cout << "[Box display cost time: " << (get_current_time() - start_time) / 1000 << " ms]" << std::endl;

		unsigned long start_time_traj = get_current_time();
		// draw ID trajectory
		for (std::map<int, TrajectoryParams>::iterator it = track_idx_map.begin(); it != track_idx_map.end(); ++it)
		{
			if(it->second.draw_flag == 1) {
				ea_display_obj_params(track_ctx->display)->border_thickness = 2;
				ea_display_obj_params(track_ctx->display)->box_color = EA_16_COLORS_RED;
				snprintf(text, MAX_LABEL_LEN, "");
				for (int j = 0; j < it->second.trajectory_position.size(); j++) {
					float track_xmin = (it->second.trajectory_position[j].x / width < 0.0)? 1 / width:it->second.trajectory_position[j].x / width;
					float track_ymin = (it->second.trajectory_position[j].y / height < 0.0)? 1 / height:it->second.trajectory_position[j].y / height;
					if (it->second.trajectory_position[j].x / width <= 1.0 - 2.0 / width && 
						it->second.trajectory_position[j].y / height <= 1.0 - 2.0 / height) {
						ea_display_set_bbox(track_ctx->display, text,
						track_xmin, track_ymin,
						2.0 / width, 2.0 / height);
					}
				}
			}
		}
		std::cout << "[Traj display cost time: " << (get_current_time() - start_time_traj) / 1000 << " ms]" << std::endl;

		ea_display_refresh(track_ctx->display, (void *)(unsigned long)dsp_pts); // (void *) img_tensor
	} while (0);

	return rval;
}