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

int calculate_tracking_trajectory(std::vector<DetectBox>& boxes, int frame_id, int data_height)
{
	int rval = 0;
    int current_frame = frame_id;

	// calculate bird view matrix
	this->bird_view_matrix_calculate();

	// read lost_frame_count
    for (std::map<int, TrajectoryParams>::iterator it = this->track_idx_map.begin(); it != this->track_idx_map.end();)
    {
		if (it->second.latest_frame_id != -1 && (current_frame - it->second.latest_frame_id) > 3) {
			this->track_idx_map.erase(it++);
		}
		else {
			++it;
		}
		it->second.draw_flag = 0;
    }

	// write id in
	for (int i = 0; i < boxes.size(); i++) {
		int track_id = boxes[i].trackID;
		float pedestrian_location[4] = {boxes[i].x1, boxes[i].y1, boxes[i].x2, boxes[i].y2};
		float x_start = boxes[i].x1;
		float y_start = boxes[i].y1;
        float x_end = boxes[i].x2;
        float y_end = boxes[i].y2;
        float head_loc[4] = {0, 0, 0, 0};
		// memcpy(head_loc, detect_result_vector[i].head_location, sizeof(detect_result_vector[i].head_location));

		// printf("id: %d, distancex: %f, distancey: %f\n", track_id, x_start, y_start);
		cv::Point2f trajectory_position_current = cv::Point2f(x_start + (x_end - x_start) / 2, y_end);
        cv::Point2f trajectory_position_bird;
        this->projection_on_bird(trajectory_position_current, trajectory_position_bird);

		std::map<int, TrajectoryParams>::iterator iter;
		iter = track_idx_map.find(track_id); 
		if (iter != track_idx_map.end()) {
            float trajectory_bird_position_latest_x = this->track_idx_map[track_id].trajectory_bird_position.back().x;
            float trajectory_bird_position_latest_y = this->track_idx_map[track_id].trajectory_bird_position.back().y;

            float move_distance = sqrt(pow((trajectory_bird_position_latest_x - trajectory_position_bird.x), 2) + \
                                    pow((trajectory_bird_position_latest_y - trajectory_position_bird.y), 2));

            this->track_idx_map[track_id].trajectory_position.push_back(trajectory_position_current);
            this->track_idx_map[track_id].trajectory_bird_position.push_back(trajectory_position_bird);
            this->track_idx_map[track_id].draw_flag = 1;
            this->track_idx_map[track_id].latest_frame_id = current_frame;
            this->track_idx_map[track_id].pedestrian_direction = -1;
            this->track_idx_map[track_id].relative_distance = (data_height - trajectory_position_bird.y) \
                                                                * this->pixel2world_distance;

            float velocity_current = move_distance * this->pixel2world_distance / this->dTs;
            memcpy(this->track_idx_map[track_id].pedestrian_location, pedestrian_location, sizeof(pedestrian_location));
            memcpy(this->track_idx_map[track_id].head_location, head_loc, sizeof(head_loc));
            this->track_idx_map[track_id].velocity_vector.push_back(velocity_current);
            if (this->track_idx_map[track_id].trajectory_bird_position.size() < 3)
            {
                this->track_idx_map[track_id].mean_velocity = 1.38;
            }
            else
            {
                std::vector<float> velocity_vector_tmp = this->track_idx_map[track_id].velocity_vector;
                this->track_idx_map[track_id].mean_velocity = (velocity_vector_tmp.at(velocity_vector_tmp.size()-1) + \
                                                               velocity_vector_tmp.at(velocity_vector_tmp.size()-2) + \
                                                               velocity_vector_tmp.at(velocity_vector_tmp.size()-3)) / 3;
            }
		}
		else {
            LOG(INFO) << "No track id find!!!";
            TrajectoryParams trajector_param;
            trajector_param.draw_flag = 1;
            trajector_param.latest_frame_id = current_frame;
            trajector_param.trajectory_position.push_back(trajectory_position_current);
            trajector_param.trajectory_bird_position.push_back(trajectory_position_bird);
            trajector_param.pedestrian_direction = -1;
            trajector_param.relative_distance = (data_height - trajectory_position_bird.y) \
                                                                * this->pixel2world_distance;
            memcpy(trajector_param.pedestrian_location, pedestrian_location, sizeof(pedestrian_location));
            memcpy(trajector_param.head_location, head_loc, sizeof(head_loc));
            trajector_param.velocity_vector.push_back(1.38);
            trajector_param.mean_velocity = 1.38;

			this->track_idx_map.insert(std::pair<int, TrajectoryParams>(track_id, trajector_param));
		}
	}
	return rval;
}