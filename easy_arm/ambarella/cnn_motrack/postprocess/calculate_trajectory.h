#ifndef _CALCULATE_TRAJECTORY_H_
#define _CALCULATE_TRAJECTORY_H_

#include <sys/time.h> // system time
#include <fstream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <glog/logging.h>
#include <map>
#include "../common/data_struct.h"
#include "../fairmot/mot.h"


/********************************************
Function：calculate trajectory
Description:  Track the target and output speed, direction and position
Input:	det_result_info
OutPut: track_idx_map
********************************************/

class CalculateTraj{
public:
    CalculateTraj();
    ~CalculateTraj();
    int init_save_dir();
	// int calculate_trajectory(DetectResultInfo &det_result_info);
    int calculate_tracking_trajectory(mot_result_t *mot_result, std::map<int, TrajectoryParams> &track_idx_map);
    int calculate_tracking_trajectory(std::vector<DetectBox>& boxes, int frame_id, int data_height);
    // int calculate_trajectory(std::vector<DetectBox>& boxes, int frame_id, int data_height);
	// int save_det_result(DetectResultInfo &det_result_info);               
    int save_det_result(std::vector<DetectBox>& boxes, int frame_id);
	// save detect result like: <frame>, <id>, <bb_left>, <bb_top>, <bb_width>, <bb_height>, <0/1忽略>, <cls>, <>

public:
	std::map<int, TrajectoryParams> track_idx_map;

private:
    int bird_view_matrix_calculate();
    // int bird_view_transform(DetectResultInfo &det_result_info);
    int projection_on_bird(cv::Point2f &point_image, cv::Point2f &point_bird);
    // int DetectBox_to_DetectResultInfo(std::vector<DetectBox>& boxes, DetectResultInfo &det_result_info);

private:
	int run_flag;
	int dTs;
    cv::Mat input_img_bgr;
    cv::Mat input_img_bird_eye;
	float pixel2world_distance;
    cv::Point2f point_image[4];
    cv::Point2f point_bird[4];
    cv::Mat transferI2B;
	std::string save_txt_dir;
    std::stringstream det_txt_path;
	std::string camera_calibration_file;
};


#endif // _CALCULATE_TRAJECTORY_H_