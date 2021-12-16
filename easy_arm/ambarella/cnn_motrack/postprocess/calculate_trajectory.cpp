#include "calculate_trajectory.h"


int calculate_tracking_trajectory(mot_result_t *mot_result, std::map<int, TrajectoryParams> &track_idx_map) {
	int rval = 0;
	// read lost_frame_count
    for (std::map<int, TrajectoryParams>::iterator it = track_idx_map.begin(); it != track_idx_map.end();)
    {
		if (it->second.latest_frame_id != -1 && (mot_result->frame_id - it->second.latest_frame_id) > 3) {
			track_idx_map.erase(it++);
		}
		else {
			++it;
		}
		it->second.draw_flag = 0;
    }

	// write id in
	for (int i = 0; i < mot_result->valid_track_count; i++) {
		int track_id = mot_result->tracks[i].track_id;
		float x_start = mot_result->tracks[i].x_start;
		float y_start = mot_result->tracks[i].y_start;
        float x_end = mot_result->tracks[i].x_end;
        float y_end = mot_result->tracks[i].y_end;
		// printf("id: %d, distancex: %f, distancey: %f\n", track_id, x_start, y_start);
		cv::Point2f trajectory_position_current = cv::Point2f(x_start + (x_end - x_start) / 2, y_end);

		std::map<int, TrajectoryParams>::iterator iter;
		iter = track_idx_map.find(track_id); 
		if (iter != track_idx_map.end()) {
			track_idx_map[track_id].trajectory_position.push_back(trajectory_position_current);
			// calculate the latest velocity
			float latest_frame_x_start = track_idx_map[track_id].pedestrian_x_start.back();
			float latest_frame_y_start = track_idx_map[track_id].pedestrian_y_start.back();

			// float velocity_current = sqrt(pow((latest_frame_x_start - x_start) * ea_display_obj_params(live_ctx->display)->dis_win_w, 2) + \
			// pow((y_start - latest_frame_y_start) * ea_display_obj_params(live_ctx->display)->dis_win_h, 2)) / 0.04 * 0.017; // s per meter m/s

			track_idx_map[track_id].pedestrian_x_start.push_back(x_start);
			track_idx_map[track_id].pedestrian_y_start.push_back(y_start);
			track_idx_map[track_id].pedestrian_x_end.push_back(x_end);
			track_idx_map[track_id].pedestrian_y_end.push_back(y_end);
			track_idx_map[track_id].velocity_vector.push_back(1.38);
			track_idx_map[track_id].mean_velocity = 1.38;
			// if (track_idx_map[track_id].velocity_vector.size() < 3) {
			// 	track_idx_map[track_id].mean_velocity = 1.38;
			// }
			// else {

			// }
			track_idx_map[track_id].draw_flag = 1;
			track_idx_map[track_id].latest_frame_id = mot_result->frame_id;
            // printf("track id size: %d\n", track_idx_map.size());
		}
		else {
            // printf("No track id find!!!\n");
			TrajectoryParams track_idx_param;
			track_idx_param.draw_flag = 1;
			// printf("Frame id: %d\n", track_output->nframe_index);
			// printf("latest_frame_id: %d\n", track_idx_param.latest_frame_id);
			track_idx_param.latest_frame_id = mot_result->frame_id;
			track_idx_param.pedestrian_x_start.push_back(x_start);
			track_idx_param.pedestrian_y_start.push_back(y_start);
			track_idx_param.pedestrian_x_end.push_back(x_end);
			track_idx_param.pedestrian_y_end.push_back(y_end);
			track_idx_param.velocity_vector.push_back(1.38);
			track_idx_param.mean_velocity = 1.38;
			track_idx_param.trajectory_position.push_back(trajectory_position_current);
			track_idx_map.insert(std::pair<int, TrajectoryParams>(track_id, track_idx_param));
		}
	}
	return rval;
}