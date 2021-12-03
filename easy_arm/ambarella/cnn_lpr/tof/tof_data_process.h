#ifndef _TOF_DATA_PROCESS_H_
#define _TOF_DATA_PROCESS_H_

//opencv
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "cnn_lpr/drivers/tof_316_acquisition.h"

bool is_file_exists(const std::string& name);

int filter_point_cloud( const cv::Mat &depth_map, TOF316Acquisition::PointCloud &src_cloud);
int compute_point_count(const TOF316Acquisition::PointCloud &bg_cloud, TOF316Acquisition::PointCloud &src_cloud);

void init_background(const std::string &bg_path, cv::Mat &bg_map);

int compute_depth_map(const cv::Mat &bg_map, const cv::Mat &depth_map);
int vote_in_out(const std::vector<int> &point_cout_list);
int get_in_out(const std::vector<int> &result_list);

#endif // _TOF_DATA_PROCESS_H_