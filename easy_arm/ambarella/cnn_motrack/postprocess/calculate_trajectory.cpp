#include "calculate_trajectory.h"


CalculateTraj::CalculateTraj()
{
	run_flag = 0;
	save_txt_dir = "/home/edge/traj/h3c_result/"; // "/sdcard/traj/h3c_result/";
	track_idx_map.clear();
}

CalculateTraj::~CalculateTraj()
{
	// if(point_image != NULL)
    // {
    //     delete[] point_image;
    //     point_image = NULL;
    // }
	// if(point_bird != NULL)
    // {
    //     delete[] point_bird;
    //     point_bird = NULL;
    // }
}

int CalculateTraj::init_save_dir()
{
	int rval = 0;

    std::string command;
    command = "mkdir -p " + this->save_txt_dir;
    system(command.c_str());

	this->det_txt_path.str("");

	struct timeval tv;
	char time_str[64];
    gettimeofday(&tv, NULL); 
	strftime(time_str, sizeof(time_str)-1, "%Y_%m_%d_%H_%M_%S", localtime(&tv.tv_sec)); 
	det_txt_path << this->save_txt_dir << time_str << ".txt";

    return 0;
}

// int CalculateTraj::calculate_trajectory(mot_result_t *mot_result, std::map<int, TrajectoryParams> &track_idx_map, int data_height) {
// 	int rval = 0;
//     BirdTransform bird_transform;

// 	// read lost_frame_count
//     for (std::map<int, TrajectoryParams>::iterator it = track_idx_map.begin(); it != track_idx_map.end();)
//     {
// 		if (it->second.latest_frame_id != -1 && (mot_result->frame_id - it->second.latest_frame_id) > 3) {
// 			track_idx_map.erase(it++);
// 		}
// 		else {
// 			++it;
// 		}
// 		it->second.draw_flag = 0;
//     }

// 	// write id in
// 	for (int i = 0; i < mot_result->valid_track_count; i++) {
// 		int track_id = mot_result->tracks[i].track_id;
// 		float x_start = mot_result->tracks[i].x_start;
// 		float y_start = mot_result->tracks[i].y_start;
//         float x_end = mot_result->tracks[i].x_end;
//         float y_end = mot_result->tracks[i].y_end;
// 		float pedestrian_location[4] = {x_start, y_start, x_end, y_end};
// 		float head_loc[4] = {0, 0, 0, 0};

// 		// printf("id: %d, distancex: %f, distancey: %f\n", track_id, x_start, y_start);
// 		cv::Point2f trajectory_position_current = cv::Point2f(x_start + (x_end - x_start) / 2, y_end);
//         cv::Point2f trajectory_position_bird;
//         bird_transform.projection_on_bird(trajectory_position_current, trajectory_position_bird);

// 		std::map<int, TrajectoryParams>::iterator iter;
// 		iter = track_idx_map.find(track_id); 
// 		if (iter != track_idx_map.end()) {
// 			// calculate the latest velocity
// 			float trajectory_bird_position_latest_x = this->track_idx_map[track_id].trajectory_bird_position.back().x;
//             float trajectory_bird_position_latest_y = this->track_idx_map[track_id].trajectory_bird_position.back().y;

//             float move_distance = sqrt(pow((trajectory_bird_position_latest_x - trajectory_position_bird.x), 2) + \
//                                     pow((trajectory_bird_position_latest_y - trajectory_position_bird.y), 2));

//             this->track_idx_map[track_id].trajectory_position.push_back(trajectory_position_current);
//             this->track_idx_map[track_id].trajectory_bird_position.push_back(trajectory_position_bird);
//             this->track_idx_map[track_id].draw_flag = 1;
//             this->track_idx_map[track_id].latest_frame_id = mot_result->frame_id;
//             this->track_idx_map[track_id].object_direction = -1;
//             this->track_idx_map[track_id].relative_distance = (data_height - trajectory_position_bird.y) \
//                                                                 * bird_transform.pixel2world_distance;

//             float velocity_current = move_distance * bird_transform.pixel2world_distance / bird_transform.time_interval;
//             memcpy(this->track_idx_map[track_id].pedestrian_location, pedestrian_location, sizeof(pedestrian_location));
//             memcpy(this->track_idx_map[track_id].head_location, head_loc, sizeof(head_loc));
//             this->track_idx_map[track_id].velocity_vector.push_back(velocity_current);
//             if (this->track_idx_map[track_id].trajectory_bird_position.size() < 3)
//             {
//                 this->track_idx_map[track_id].mean_velocity = 1.38;
//             }
//             else
//             {
//                 std::vector<float> velocity_vector_tmp = this->track_idx_map[track_id].velocity_vector;
//                 this->track_idx_map[track_id].mean_velocity = (velocity_vector_tmp.at(velocity_vector_tmp.size()-1) + \
//                                                                velocity_vector_tmp.at(velocity_vector_tmp.size()-2) + \
//                                                                velocity_vector_tmp.at(velocity_vector_tmp.size()-3)) / 3;
// 			}
//             this->track_idx_map[track_id].pedestrian_x_start.push_back(x_start);
// 			this->track_idx_map[track_id].pedestrian_y_start.push_back(y_start);
// 			this->track_idx_map[track_id].pedestrian_x_end.push_back(x_end);
// 			this->track_idx_map[track_id].pedestrian_y_end.push_back(y_end);
// 		}
// 		else {
// 			TrajectoryParams trajector_param;
// 			trajector_param.draw_flag = 1;
// 			trajector_param.latest_frame_id = mot_result->frame_id;
//             trajector_param.trajectory_position.push_back(trajectory_position_current);
//             trajector_param.trajectory_bird_position.push_back(trajectory_position_bird);
//             trajector_param.relative_distance = (data_height - trajectory_position_bird.y) \
//                                                                 * bird_transform.pixel2world_distance;
//             memcpy(trajector_param.pedestrian_location, pedestrian_location, sizeof(pedestrian_location));
//             memcpy(trajector_param.head_location, head_loc, sizeof(head_loc));
// 			trajector_param.velocity_vector.push_back(1.38);
// 			trajector_param.mean_velocity = 1.38;
//             trajector_param.pedestrian_x_start.push_back(x_start);
// 			trajector_param.pedestrian_y_start.push_back(y_start);
// 			trajector_param.pedestrian_x_end.push_back(x_end);
// 			trajector_param.pedestrian_y_end.push_back(y_end);

// 			track_idx_map.insert(std::pair<int, TrajectoryParams>(track_id, trajector_param));
// 		}
// 	}
// 	return rval;
// }

int CalculateTraj::calculate_trajectory(std::vector<DetectBox>& boxes, int frame_id)
{
	int rval = 0;
    int current_frame = frame_id;

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
        int class_id = boxes[i].classID;
        float confidence = boxes[i].confidence;
		float pedestrian_location[4] = {boxes[i].x1, boxes[i].y1, boxes[i].x2, boxes[i].y2};
		float x_start = boxes[i].x1;
		float y_start = boxes[i].y1;
        float x_end = boxes[i].x2;
        float y_end = boxes[i].y2;
        float head_loc[4] = {0, 0, 0, 0};
		// memcpy(head_loc, detect_result_vector[i].head_location, sizeof(detect_result_vector[i].head_location));

        std::vector<std::vector<float>> position_value;
        std::vector<std::vector<float>> position_value_slide;
        cv::Point2f trajectory_position_current;
		std::map<int, TrajectoryParams>::iterator iter_;
		iter_ = track_idx_map.find(track_id); 
		if (iter_ != track_idx_map.end()) {
			this->track_idx_map[track_id].pedestrian_x_start.push_back(x_start);
			this->track_idx_map[track_id].pedestrian_y_start.push_back(y_start);
			this->track_idx_map[track_id].pedestrian_x_end.push_back(x_end);
			this->track_idx_map[track_id].pedestrian_y_end.push_back(y_end);

            position_value.push_back(this->track_idx_map[track_id].pedestrian_x_start);
            position_value.push_back(this->track_idx_map[track_id].pedestrian_y_start);
            position_value.push_back(this->track_idx_map[track_id].pedestrian_x_end);
            position_value.push_back(this->track_idx_map[track_id].pedestrian_y_end);
            norm_trajectory.trajectory_smoothing(position_value, position_value_slide, 2560);
            trajectory_position_current = cv::Point2f(position_value_slide[0].back() + \
                                        (position_value_slide[2].back() - position_value_slide[0].back()) / 2, position_value_slide[3].back());
        }
        else
        {
            trajectory_position_current = cv::Point2f(x_start + (x_end - x_start) / 2, y_end);
        }

		// cv::Point2f trajectory_position_current = cv::Point2f(x_start + (x_end - x_start) / 2, y_end);
        cv::Point2f trajectory_position_bird;
        bird_transform.projection_on_bird(trajectory_position_current, trajectory_position_bird);

		std::map<int, TrajectoryParams>::iterator iter;
		iter = track_idx_map.find(track_id); 
		if (iter != track_idx_map.end()) {
            float trajectory_bird_position_latest_x = this->track_idx_map[track_id].trajectory_bird_position.back().x;
            float trajectory_bird_position_latest_y = this->track_idx_map[track_id].trajectory_bird_position.back().y;

            float move_distance = sqrt(pow((trajectory_bird_position_latest_y - trajectory_position_bird.y), 2));

            // 根据移动位置的变化量大小来判断方向
            if (trajectory_bird_position_latest_y > trajectory_position_bird.y)
            {
                this->track_idx_map[track_id].object_direction = 1; // backward
            }
            else
            {
                this->track_idx_map[track_id].object_direction = 0; // forward
            }

            this->track_idx_map[track_id].trajectory_position.push_back(trajectory_position_current);
            this->track_idx_map[track_id].trajectory_bird_position.push_back(trajectory_position_bird);
            this->track_idx_map[track_id].draw_flag = 1;
            this->track_idx_map[track_id].latest_frame_id = current_frame;
            this->track_idx_map[track_id].relative_distance = (bird_transform.camera_position_pixel - trajectory_position_bird.y) \
                                                            * bird_transform.pixel2world_distance + bird_transform.translation_position;

            float velocity_current = move_distance * bird_transform.pixel2world_distance / bird_transform.time_interval;
            std::cout << "move_distance: " << move_distance * bird_transform.pixel2world_distance \
                      << " bird_transform.time_interval: " << bird_transform.time_interval \
                      << " velocity_current: " << velocity_current << std::endl;
            memcpy(this->track_idx_map[track_id].pedestrian_location, pedestrian_location, sizeof(pedestrian_location));
            memcpy(this->track_idx_map[track_id].head_location, head_loc, sizeof(head_loc));

            // 速度平滑 通过卷积进行数据平滑
            this->track_idx_map[track_id].velocity_vector.push_back(velocity_current);
            float kernel_v_slide[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25};
            int input_length = this->track_idx_map[track_id].velocity_vector.size();
            int kernel_size = sizeof(kernel_v_slide) / sizeof(float);
            array_reverse(kernel_v_slide, kernel_size);
            int output_length = input_length + kernel_size - 1;
            float *velocity_y_conv = new float[output_length];
            memset(velocity_y_conv, 0, sizeof(velocity_y_conv)); //注意一定要清零

            conv1d(this->track_idx_map[track_id].velocity_vector, kernel_v_slide, input_length, kernel_size, velocity_y_conv);
            this->track_idx_map[track_id].mean_velocity = velocity_y_conv[output_length - 25] / 325; // 倒数第25个值
            delete [] velocity_y_conv;

			// this->track_idx_map[track_id].pedestrian_x_start.push_back(x_start);
			// this->track_idx_map[track_id].pedestrian_y_start.push_back(y_start);
			// this->track_idx_map[track_id].pedestrian_x_end.push_back(x_end);
			// this->track_idx_map[track_id].pedestrian_y_end.push_back(y_end);
            this->track_idx_map[track_id].confidence = confidence;
            this->track_idx_map[track_id].class_id = class_id;
		}
		else {
            TrajectoryParams trajector_param;
            trajector_param.draw_flag = 1;
            trajector_param.latest_frame_id = current_frame;
            trajector_param.trajectory_position.push_back(trajectory_position_current);
            trajector_param.trajectory_bird_position.push_back(trajectory_position_bird);
            trajector_param.object_direction = -1;
            trajector_param.relative_distance = (bird_transform.camera_position_pixel - trajectory_position_bird.y) \
                                                * bird_transform.pixel2world_distance + bird_transform.translation_position;
            memcpy(trajector_param.pedestrian_location, pedestrian_location, sizeof(pedestrian_location));
            memcpy(trajector_param.head_location, head_loc, sizeof(head_loc));
            // trajector_param.velocity_vector.push_back(0.6);
            // 前26frame取第1frame的值
            trajector_param.velocity_vector.insert(trajector_param.velocity_vector.end(), 26, 0.6); 
            trajector_param.mean_velocity = 0.6;
            trajector_param.pedestrian_x_start.insert(trajector_param.pedestrian_x_start.end(), 26, x_start);
			trajector_param.pedestrian_y_start.insert(trajector_param.pedestrian_y_start.end(), 26, y_start);
			trajector_param.pedestrian_x_end.insert(trajector_param.pedestrian_x_end.end(), 26, x_end);
			trajector_param.pedestrian_y_end.insert(trajector_param.pedestrian_y_end.end(), 26, y_end);
            trajector_param.confidence = confidence;
            trajector_param.class_id = class_id;

			this->track_idx_map.insert(std::pair<int, TrajectoryParams>(track_id, trajector_param));
		}
	}
	return rval;
}

int CalculateTraj::calculate_trajectory(DetectResultInfo &det_result_info)
{
	int rval = 0;
    int current_frame = det_result_info.current_frame;

    std::vector<DetectResult> detect_result_vector = det_result_info.detect_result_vector;

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

	for (int i = 0; i < detect_result_vector.size(); i++) {
		int track_id = detect_result_vector[i].track_id;
        int class_id = detect_result_vector[i].class_id;
        float confidence = detect_result_vector[i].confidence;
        float pedestrian_location[4];
        memcpy(pedestrian_location, detect_result_vector[i].pedestrian_location, sizeof(detect_result_vector[i].pedestrian_location));
        float automobile_location[4];
        memcpy(automobile_location, detect_result_vector[i].automobile_location, sizeof(detect_result_vector[i].automobile_location));
        float x_start, y_start, x_end, y_end;
        if (class_id == 1)
        {
            x_start = pedestrian_location[0];
            y_start = pedestrian_location[1];
            x_end = pedestrian_location[2];
            y_end = pedestrian_location[3];
        }
        else if (class_id == 2)
        {
            x_start = automobile_location[0];
            y_start = automobile_location[1];
            x_end = automobile_location[2];
            y_end = automobile_location[3];
        }
        
        float head_loc[4];
		memcpy(head_loc, detect_result_vector[i].head_location, sizeof(detect_result_vector[i].head_location));

        std::vector<std::vector<float>> position_value;
        std::vector<std::vector<float>> position_value_slide;
        cv::Point2f trajectory_position_current;
		std::map<int, TrajectoryParams>::iterator iter_;
		iter_ = track_idx_map.find(track_id); 
		if (iter_ != track_idx_map.end()) {
			this->track_idx_map[track_id].pedestrian_x_start.push_back(x_start);
			this->track_idx_map[track_id].pedestrian_y_start.push_back(y_start);
			this->track_idx_map[track_id].pedestrian_x_end.push_back(x_end);
			this->track_idx_map[track_id].pedestrian_y_end.push_back(y_end);

            position_value.push_back(this->track_idx_map[track_id].pedestrian_x_start);
            position_value.push_back(this->track_idx_map[track_id].pedestrian_y_start);
            position_value.push_back(this->track_idx_map[track_id].pedestrian_x_end);
            position_value.push_back(this->track_idx_map[track_id].pedestrian_y_end);
            norm_trajectory.trajectory_smoothing(position_value, position_value_slide, det_result_info.yuv_data.data_width);
            trajectory_position_current = cv::Point2f(position_value_slide[0].back() + \
                                        (position_value_slide[2].back() - position_value_slide[0].back()) / 2, position_value_slide[3].back());
        }
        else
        {
            trajectory_position_current = cv::Point2f(x_start + (x_end - x_start) / 2, y_end);
        }

        cv::Point2f trajectory_position_bird;
        bird_transform.projection_on_bird(trajectory_position_current, trajectory_position_bird);

		std::map<int, TrajectoryParams>::iterator iter;
		iter = track_idx_map.find(track_id); 
		if (iter != track_idx_map.end()) {
			float trajectory_bird_position_latest_x = this->track_idx_map[track_id].trajectory_bird_position.back().x;
            float trajectory_bird_position_latest_y = this->track_idx_map[track_id].trajectory_bird_position.back().y;

            float move_distance = sqrt(pow((trajectory_bird_position_latest_y - trajectory_position_bird.y), 2));

            // 根据移动位置的变化量大小来判断方向
            if (trajectory_bird_position_latest_y > trajectory_position_bird.y)
            {
                this->track_idx_map[track_id].object_direction = 1; // backward
            }
            else
            {
                this->track_idx_map[track_id].object_direction = 0; // forward
            }

            this->track_idx_map[track_id].trajectory_position.push_back(trajectory_position_current);
            this->track_idx_map[track_id].trajectory_bird_position.push_back(trajectory_position_bird);
            this->track_idx_map[track_id].draw_flag = 1;
            this->track_idx_map[track_id].latest_frame_id = current_frame;
            this->track_idx_map[track_id].relative_distance = (bird_transform.camera_position_pixel - trajectory_position_bird.y) \
                                                            * bird_transform.pixel2world_distance + bird_transform.translation_position;

            float velocity_current = move_distance * bird_transform.pixel2world_distance / bird_transform.time_interval;
            memcpy(this->track_idx_map[track_id].pedestrian_location, pedestrian_location, sizeof(pedestrian_location));
            memcpy(this->track_idx_map[track_id].head_location, head_loc, sizeof(head_loc));
            // 速度平滑 通过卷积进行数据平滑
            this->track_idx_map[track_id].velocity_vector.push_back(velocity_current);
            float kernel_v_slide[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25};
            int input_length = this->track_idx_map[track_id].velocity_vector.size();
            int kernel_size = sizeof(kernel_v_slide) / sizeof(float);
            array_reverse(kernel_v_slide, kernel_size);
            int output_length = input_length + kernel_size - 1;
            float *velocity_y_conv = new float[output_length];
            memset(velocity_y_conv, 0, sizeof(velocity_y_conv)); //注意一定要清零

            conv1d(this->track_idx_map[track_id].velocity_vector, kernel_v_slide, input_length, kernel_size, velocity_y_conv);
            this->track_idx_map[track_id].mean_velocity = velocity_y_conv[output_length - 25] / 325; // 倒数第25个值
            delete [] velocity_y_conv;

            this->track_idx_map[track_id].confidence = confidence;
            this->track_idx_map[track_id].class_id = class_id;
		}
		else {
            TrajectoryParams trajector_param;
            trajector_param.draw_flag = 1;
            trajector_param.latest_frame_id = current_frame;
            trajector_param.trajectory_position.push_back(trajectory_position_current);
            trajector_param.trajectory_bird_position.push_back(trajectory_position_bird);
            trajector_param.object_direction = -1;
            trajector_param.relative_distance = (bird_transform.camera_position_pixel - trajectory_position_bird.y) \
                                                * bird_transform.pixel2world_distance + bird_transform.translation_position;
            memcpy(trajector_param.pedestrian_location, pedestrian_location, sizeof(pedestrian_location));
            memcpy(trajector_param.automobile_location, automobile_location, sizeof(automobile_location));
            memcpy(trajector_param.head_location, head_loc, sizeof(head_loc));
            // 前26frame取第1frame的值
            trajector_param.velocity_vector.insert(trajector_param.velocity_vector.end(), 26, 0.6); 
            trajector_param.mean_velocity = 0.6;
            trajector_param.pedestrian_x_start.insert(trajector_param.pedestrian_x_start.end(), 26, x_start);
			trajector_param.pedestrian_y_start.insert(trajector_param.pedestrian_y_start.end(), 26, y_start);
			trajector_param.pedestrian_x_end.insert(trajector_param.pedestrian_x_end.end(), 26, x_end);
			trajector_param.pedestrian_y_end.insert(trajector_param.pedestrian_y_end.end(), 26, y_end);
 
            trajector_param.confidence = confidence;
            trajector_param.class_id = class_id;

			this->track_idx_map.insert(std::pair<int, TrajectoryParams>(track_id, trajector_param));
		}
	}
	return rval;
}

int CalculateTraj::save_det_result(std::vector<DetectBox>& boxes, int frame_id)
{
    int rval = 0;

	std::ofstream save_result;
	save_result.open(this->det_txt_path.str(), std::ios::app); // 

	for (int i = 0; i < boxes.size(); i++)
	{
		save_result << frame_id << " " \
		                   << boxes[i].trackID << " " \
						   << boxes[i].x1 << " " \
						   << boxes[i].y1 << " " \
						   << boxes[i].x2 << " " \
						   << boxes[i].y2 << " " \
						   << boxes[i].confidence << " " \
						   << -1 << " " << -1 << "\n";
	}
	save_result.close();

	return rval;
}