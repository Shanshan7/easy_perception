#include "calculate_trajectory.h"


// void calculate_tracking_trajectory(TrackOutPut *track_output, std::map<int, TrajectoryParams> &track_idx_map) {
// 	// read lost_frame_count
//     for (std::map<int, TrajectoryParams>::iterator it = track_idx_map.begin(); it != track_idx_map.end();)
//     {
// 		if (it->second.latest_frame_id != -1 && (track_output->nframe_index - it->second.latest_frame_id) > 3) {
// 			track_idx_map.erase(it++);
// 		}
// 		else {
// 			++it;
// 		}
// 		it->second.draw_flag = 0;
//     }

// 	// write id in
// 	for (int i = 0; i < track_output->nvalid_track_count; i++) {
// 		int track_id = track_output->track_attri[i].ntrack_id;
// 		float x_start = track_output->track_attri[i].fobject_loc[0];
// 		float y_start = track_output->track_attri[i].fobject_loc[1];
//         float x_end = track_output->track_attri[i].fobject_loc[2];
//         float y_end = track_output->track_attri[i].fobject_loc[3];
// 		// printf("id: %d, distancex: %f, distancey: %f\n", track_id, x_start, y_start);
// 		cv::Point2f trajectory_position_current = cv::Point2f(x_start + (x_end - x_start) / 2, y_end);

// 		std::map<int, TrajectoryParams>::iterator iter;
// 		iter = track_idx_map.find(track_id); 
// 		if (iter != track_idx_map.end()) {
// 			track_idx_map[track_id].trajectory_position.push_back(trajectory_position_current);
// 			// calculate the latest velocity
// 			float latest_frame_x_start = track_idx_map[track_id].pedestrian_x_start.back();
// 			float latest_frame_y_start = track_idx_map[track_id].pedestrian_y_start.back();

// 			// float velocity_current = sqrt(pow((latest_frame_x_start - x_start) * ea_display_obj_params(live_ctx->display)->dis_win_w, 2) + \
// 			// pow((y_start - latest_frame_y_start) * ea_display_obj_params(live_ctx->display)->dis_win_h, 2)) / 0.04 * 0.017; // s per meter m/s

// 			track_idx_map[track_id].pedestrian_x_start.push_back(x_start);
// 			track_idx_map[track_id].pedestrian_y_start.push_back(y_start);
// 			track_idx_map[track_id].velocity_vector.push_back(1.38);
// 			track_idx_map[track_id].mean_velocity = 1.38;
// 			// if (track_idx_map[track_id].velocity_vector.size() < 3) {
// 			// 	track_idx_map[track_id].mean_velocity = 1.38;
// 			// }
// 			// else {

// 			// }
// 			track_idx_map[track_id].draw_flag = 1;
// 			track_idx_map[track_id].latest_frame_id = track_output->nframe_index;
//             // printf("track id size: %d\n", track_idx_map.size());
// 		}
// 		else {
//             // printf("No track id find!!!\n");
// 			TrajectoryParams track_idx_param;
// 			track_idx_param.draw_flag = 1;
// 			// printf("Frame id: %d\n", track_output->nframe_index);
// 			// printf("latest_frame_id: %d\n", track_idx_param.latest_frame_id);
// 			track_idx_param.latest_frame_id = track_output->nframe_index;
// 			track_idx_param.pedestrian_x_start.push_back(x_start);
// 			track_idx_param.pedestrian_y_start.push_back(y_start);
// 			track_idx_param.velocity_vector.push_back(1.38);
// 			track_idx_param.mean_velocity = 1.38;
// 			track_idx_param.trajectory_position.push_back(trajectory_position_current);
// 			track_idx_map.insert(std::pair<int, TrajectoryParams>(track_id, track_idx_param));
// 		}
// 	}
// }

void calculate_tracking_trajectory(std::vector<DetectBox>& boxes, std::map<int, TrajectoryParams> &track_idx_map, int frame_id)
{
	for (std::map<int, TrajectoryParams>::iterator it = track_idx_map.begin(); it != track_idx_map.end();)
    {
		if (it->second.latest_frame_id != -1 && (frame_id - it->second.latest_frame_id) > 3) {
			track_idx_map.erase(it++);
		}
		else {
			++it;
		}
		it->second.draw_flag = 0;
    }
	// write id in
	for (int i = 0; i < boxes.size(); i++) {
		int track_id = boxes[i].trackID;
		float x_start = boxes[i].x1;
		float y_start = boxes[i].y1;
        float x_end = boxes[i].x2;
        float y_end = boxes[i].y2;
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
			track_idx_map[track_id].latest_frame_id = frame_id;
            // printf("track id size: %d\n", track_idx_map.size());
		}
		else {
            LOG(INFO) << "No track id find!!!";
			TrajectoryParams track_idx_param;
			track_idx_param.draw_flag = 1;
			track_idx_param.latest_frame_id = frame_id;
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
}