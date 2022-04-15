#include <iostream>
#include <opencv2/opencv.hpp>

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

		snprintf(track_ctx->output_dir, sizeof(track_ctx->output_dir), "%s", "/data");
        track_ctx->display_jpeg = ea_display_new(EA_DISPLAY_JPEG, EA_TENSOR_COLOR_MODE_BGR, EA_DISPLAY_BBOX_TEXTBOX, (void *)track_ctx->output_dir);
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

int amba_draw_detection(sde_track_ctx_t *track_ctx)
{
	int rval = 0;
    char text[MAX_LABEL_LEN];

	uint32_t dsp_pts = track_ctx->image_data.dsp_pts;
	int width = track_ctx->image_width;
    int height = track_ctx->image_height;
	std::map<int, TrajectoryParams> track_idx_map = track_ctx->track_idx_map;

	ea_display_obj_params(track_ctx->display)->obj_win_w = 1.0;
	ea_display_obj_params(track_ctx->display)->obj_win_h = 1.0;

	do {
		for (std::map<int, TrajectoryParams>::iterator it = track_idx_map.begin(); it != track_idx_map.end(); ++it)
		{
			if(it->second.draw_flag == 1) {
				// draw ID trajectory
				ea_display_obj_params(track_ctx->display)->border_thickness = 2;
				ea_display_obj_params(track_ctx->display)->font_size = 8;
				ea_display_obj_params(track_ctx->display)->text_color = EA_16_COLORS_YELLOW;
				ea_display_obj_params(track_ctx->display)->box_color = EA_16_COLORS_YELLOW;
				snprintf(text, MAX_LABEL_LEN, "ID %d", (int)it->first);
				float xmin = (it->second.pedestrian_x_start.back() / width < 0.0)? 1 / width:it->second.pedestrian_x_start.back() / width;
				float ymin = (it->second.pedestrian_y_start.back() / height < 0.0)? 1 / height:it->second.pedestrian_y_start.back() / height;
				float box_width = (it->second.pedestrian_x_end.back() / width > 1.0)? 1.0 - xmin:it->second.pedestrian_x_end.back() / width - xmin;
				float box_height = (it->second.pedestrian_y_end.back() / height > 1.0)? 1.0 - ymin:it->second.pedestrian_y_end.back() / height - ymin;
				ea_display_set_bbox(track_ctx->display, text,
					xmin, ymin, box_width, box_height);

				// draw ID trajectory
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

		ea_display_refresh(track_ctx->display, (void *)(unsigned long)dsp_pts); // (void *) img_tensor

	} while (0);

	return rval;
}


int amba_draw_detection(sde_track_ctx_t *track_ctx, std::vector<DetectBox> &det_results)
{
	int rval = 0;
    char text[MAX_LABEL_LEN];

	uint32_t dsp_pts = track_ctx->image_data.dsp_pts;
	int width = track_ctx->image_width;
    int height = track_ctx->image_height;
	std::map<int, TrajectoryParams> track_idx_map = track_ctx->track_idx_map;

	ea_display_obj_params(track_ctx->display)->obj_win_w = 1.0;
	ea_display_obj_params(track_ctx->display)->obj_win_h = 1.0;

	do {
		ea_display_obj_params(track_ctx->display)->border_thickness = 2;
		ea_display_obj_params(track_ctx->display)->font_size = 8;
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

		ea_display_refresh(track_ctx->display, (void *)(unsigned long)dsp_pts); // (void *) img_tensor
	} while (0);

	return rval;
}


int amba_draw_detection_jpeg(sde_track_ctx_t *track_ctx, std::string file_name) // 
{
	int rval = 0;
	int draw_result = 0;
    std::stringstream save_path;
	save_path << "/data/" << file_name << ".jpg";

	ea_tensor_t *img_tensor = track_ctx->image_data.tensor_group[0];
	ea_tensor_t *out_tensor = NULL;

	int width = track_ctx->image_width;
    int height = track_ctx->image_height;
	std::map<int, TrajectoryParams> track_idx_map = track_ctx->track_idx_map;
	cv::Mat input_src(cv::Size(width, height), CV_8UC3);

	size_t shape[4];
	shape[0] = 1;
	shape[1] = input_src.channels();
	shape[2] = input_src.rows;
	shape[3] = input_src.cols;

	out_tensor = ea_tensor_new(EA_U8, shape, 0);

	ea_cvt_color_resize(img_tensor, out_tensor, EA_COLOR_YUV2BGR_NV12, EA_VP);
    tensor2mat(out_tensor, input_src, 3);
	if (draw_result > 0)
	{
		for (std::map<int, TrajectoryParams>::iterator it = track_idx_map.begin(); it != track_idx_map.end(); ++it)
		{
			if(it->second.draw_flag == 1) {
				cv::Point lt(it->second.pedestrian_location[0], it->second.pedestrian_location[1]);
				cv::Point br(it->second.pedestrian_location[2], it->second.pedestrian_location[3]);
				cv::rectangle(input_src, lt, br, cv::Scalar(0, 255, 255), 1);
				std::string lbl = cv::format("ID:%d",(int)it->first);
				cv::putText(input_src, lbl, lt, cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(0,255,255));
				for (int j = 0; j < it->second.trajectory_position.size(); j++) {
					cv::Point p(it->second.trajectory_position[j].x, it->second.trajectory_position[j].y);
					cv::circle(input_src, p, 2, cv::Scalar(0, 0, 255), -1);
				}
			}
		}
	}
	cv::imwrite(save_path.str(), input_src);
	copy_file(save_path.str(), "/data/newest.jpg");
	ea_tensor_free(out_tensor);
}