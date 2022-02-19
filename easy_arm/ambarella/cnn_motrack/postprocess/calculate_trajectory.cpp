#include <iostream>
#include "calculate_trajectory.h"


CalculateTraj::CalculateTraj()
{
	run_flag = 0;
	dTs = 1.0 / 25.0;
	pixel2world_distance = 3.0 / 40.0;
	save_txt_dir = "/home/edge/traj/h3c_result/"; // "/sdcard/traj/h3c_result/";
	camera_calibration_file = "/sdcard/traj/";
	track_idx_map.clear();

    point_image[0].x = 0;
    point_image[0].y = 0;
    point_image[1].x = 960;
    point_image[1].y = 0;
    point_image[2].x = 0;
    point_image[2].y = 540;
    point_image[3].x = 960;
    point_image[3].y = 540;

    // bird point
    point_bird[0].x = 180;
    point_bird[0].y = 162;
    point_bird[1].x = 618;
    point_bird[1].y = 0;
    point_bird[2].x = 552;
    point_bird[2].y = 540;
    point_bird[3].x = 682;
    point_bird[3].y = 464;
    
    transferI2B = cv::Mat(3, 3, CV_32F, cv::Scalar::all(0));
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

int CalculateTraj::bird_view_matrix_calculate()
{
    this->transferI2B = cv::getPerspectiveTransform(this->point_image, this->point_bird);
    return 0;
}

int CalculateTraj::projection_on_bird(cv::Point2f &point_image, cv::Point2f &point_bird)
{
    point_bird.x = (this->transferI2B.at<float>(0,0) * point_image.x + this->transferI2B.at<float>(0,1) * point_image.y \
                     + this->transferI2B.at<float>(0,2)) / (this->transferI2B.at<float>(2,0) * point_image.x \
                     + this->transferI2B.at<float>(2,1) * point_image.y + this->transferI2B.at<float>(2,2));

    point_bird.y = (this->transferI2B.at<float>(1,0) * point_image.x + this->transferI2B.at<float>(1,1) * point_image.y \
                     + this->transferI2B.at<float>(1,2)) / (this->transferI2B.at<float>(2,0) * point_image.x \
                     + this->transferI2B.at<float>(2,1) * point_image.y + this->transferI2B.at<float>(2,2));

    return 0;
}

// int CalculateTraj::bird_view_transform(DetectResultInfo &det_result_info)
// {
//     int width;
//     int height;
//     cv::warpPerspective(this->input_img_bgr, this->input_img_bird_eye, this->transferI2B, cv::Size(width, height));
//     return 0;
// }

int CalculateTraj::calculate_tracking_trajectory(mot_result_t *mot_result, std::map<int, TrajectoryParams> &track_idx_map, int data_height) {
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
		float pedestrian_location[4] = {x_start, y_start, x_end, y_end};
		float head_loc[4] = {0, 0, 0, 0};

		// printf("id: %d, distancex: %f, distancey: %f\n", track_id, x_start, y_start);
		cv::Point2f trajectory_position_current = cv::Point2f(x_start + (x_end - x_start) / 2, y_end);
        cv::Point2f trajectory_position_bird;
        this->projection_on_bird(trajectory_position_current, trajectory_position_bird);

		std::map<int, TrajectoryParams>::iterator iter;
		iter = track_idx_map.find(track_id); 
		if (iter != track_idx_map.end()) {
			// calculate the latest velocity
			float trajectory_bird_position_latest_x = this->track_idx_map[track_id].trajectory_bird_position.back().x;
            float trajectory_bird_position_latest_y = this->track_idx_map[track_id].trajectory_bird_position.back().y;

            float move_distance = sqrt(pow((trajectory_bird_position_latest_x - trajectory_position_bird.x), 2) + \
                                    pow((trajectory_bird_position_latest_y - trajectory_position_bird.y), 2));

            this->track_idx_map[track_id].trajectory_position.push_back(trajectory_position_current);
            this->track_idx_map[track_id].trajectory_bird_position.push_back(trajectory_position_bird);
            this->track_idx_map[track_id].draw_flag = 1;
            this->track_idx_map[track_id].latest_frame_id = mot_result->frame_id;
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
            this->track_idx_map[track_id].pedestrian_x_start.push_back(x_start);
			this->track_idx_map[track_id].pedestrian_y_start.push_back(y_start);
			this->track_idx_map[track_id].pedestrian_x_end.push_back(x_end);
			this->track_idx_map[track_id].pedestrian_y_end.push_back(y_end);
		}
		else {
			TrajectoryParams trajector_param;
			trajector_param.draw_flag = 1;
			trajector_param.latest_frame_id = mot_result->frame_id;
            trajector_param.trajectory_position.push_back(trajectory_position_current);
            trajector_param.trajectory_bird_position.push_back(trajectory_position_bird);
            trajector_param.relative_distance = (data_height - trajectory_position_bird.y) \
                                                                * this->pixel2world_distance;
            memcpy(trajector_param.pedestrian_location, pedestrian_location, sizeof(pedestrian_location));
            memcpy(trajector_param.head_location, head_loc, sizeof(head_loc));
			trajector_param.velocity_vector.push_back(1.38);
			trajector_param.mean_velocity = 1.38;
            trajector_param.pedestrian_x_start.push_back(x_start);
			trajector_param.pedestrian_y_start.push_back(y_start);
			trajector_param.pedestrian_x_end.push_back(x_end);
			trajector_param.pedestrian_y_end.push_back(y_end);

			track_idx_map.insert(std::pair<int, TrajectoryParams>(track_id, trajector_param));
		}
	}
	return rval;
}

int CalculateTraj::calculate_tracking_trajectory(std::vector<DetectBox>& boxes, int frame_id, int data_height)
{
	int rval = 0;
    int current_frame = frame_id;

	// calculate bird view matrix
	// this->bird_view_matrix_calculate();

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
			this->track_idx_map[track_id].pedestrian_x_start.push_back(x_start);
			this->track_idx_map[track_id].pedestrian_y_start.push_back(y_start);
			this->track_idx_map[track_id].pedestrian_x_end.push_back(x_end);
			this->track_idx_map[track_id].pedestrian_y_end.push_back(y_end);
		}
		else {
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
            trajector_param.pedestrian_x_start.push_back(x_start);
			trajector_param.pedestrian_y_start.push_back(y_start);
			trajector_param.pedestrian_x_end.push_back(x_end);
			trajector_param.pedestrian_y_end.push_back(y_end);

			this->track_idx_map.insert(std::pair<int, TrajectoryParams>(track_id, trajector_param));
		}
	}
	return rval;
}