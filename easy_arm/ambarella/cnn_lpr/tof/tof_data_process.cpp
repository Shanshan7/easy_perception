#include "tof_data_process.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

bool is_file_exists(const std::string& name) 
{
    std::ifstream f(name.c_str());
    return f.good();
}

int filter_point_cloud( const cv::Mat &depth_map, TOF316Acquisition::PointCloud &src_cloud)
{
	const uchar* data = depth_map.ptr<uchar>(0);
	for (auto it = src_cloud.begin(); it != src_cloud.end();)
    {
		int index = it->index;
		if(*(data + index) == 0)
		{
			it = src_cloud.erase(it);
		}
        else
		{
            ++it;
        }
    }
}

int compute_point_count(const TOF316Acquisition::PointCloud &bg_cloud, TOF316Acquisition::PointCloud &src_cloud)
{
	int result = 0;
	float min_dist = 1000;
	float temp_dist = 0;
	for (size_t i = 0; i < src_cloud.size(); i++)
	{
		min_dist = 1000;
		for (size_t j = 0; j < bg_cloud.size(); j++)
		{
			temp_dist = abs(src_cloud[i].z - bg_cloud[j].z);
			if (temp_dist < min_dist)
			{
				min_dist = temp_dist;
			}
		}
		if(min_dist >= 0.1f)
		{
			result++;
		}
	}
	return result;
}

void init_background(const std::string &bg_path, cv::Mat &bg_map)
{
	if(!is_file_exists(bg_path))
	{
		cv::Mat filter_map;
		cv::medianBlur(bg_map, filter_map, 5);
		cv::imwrite(bg_path, filter_map);
		bg_map = filter_map;
	}
	else
	{
		bg_map = cv::imread(bg_path.c_str());
	}
}

int compute_depth_map(const cv::Mat &bg_map, const cv::Mat &depth_map)
{
	int result = 0;
	int diff = 0;
	for (int i = 0; i < depth_map.rows; i++)
    {
        const uchar* bg_data = bg_map.ptr<uchar>(i);
        const uchar* data = depth_map.ptr<uchar>(i);
        for (int j=0; j<depth_map.cols; j++)
        {
			diff = abs(bg_data[j] - data[j]);
			if(diff > 80)
			{
				result++;
			}
        }
    }
	return result;
}

int vote_in_out(const std::vector<int> &point_cout_list)
{
	int result = -1;
	int diff_count = 0;
	int all_count = 0;
	size_t point_size = point_cout_list.size();
	if(point_size == 1)
	{
		all_count = 0 - point_cout_list[0];
	}
	for(size_t i = 1; i < point_cout_list.size();i++)
	{
		diff_count = point_cout_list[i] - point_cout_list[i-1];
		if(diff_count > 20)
		{
			all_count += diff_count * 3;
		}
		else if(diff_count < -20)
		{
			all_count += (diff_count / 3);
		}
	}
	
	if(all_count > 0)
	{
		return 0;
	}
	else if(all_count < 0)
	{
		return 1;
	}
	return result;
}

int get_in_out(const std::vector<int> &result_list)
{
	// cv::Mat result = cv::Mat::zeros(cv::Size(1, 3), CV_64FC1);
	// cv::Mat A = cv::Mat::zeros(cv::Size(3, result_list.size()), CV_64FC1);
	// cv::Mat b = cv::Mat::zeros(cv::Size(1, result_list.size()), CV_64FC1);
	// cv::Mat c;
	// cv::Mat d;
	// double axis = 0;
	// if(result_list.size() < 3)
	// 	return -1;
    // for (int i = 0; i < result_list.size(); i++)
    // {
    //     A.at<double>(i, 0) = 1;
    //     A.at<double>(i, 1) = i;
    //     A.at<double>(i, 2) = i * i;
    // }
    
    // for (int i = 0; i < result_list.size(); i++)
    // {
    //     b.at<double>(i, 0) = (double)result_list[i];
    // }

    // c = A.t() * A;
    // d = A.t() * b;
	// axis = -result.at<double>(1, 0) / result.at<double>(2, 0) / 2;
	// std::cout << "axis:" << axis << std::endl;
	// if(axis > 3)
	// {
	// 	return 0;
	// }
	// else if(axis > 0)
	// {
	// 	return 1;
	// }
	// else
	// {
	// 	return -1;
	// }
	int in_count = 0;
	int out_count = 0;
	for(size_t i = 1; i < result_list.size(); i++)
	{
		if(result_list[i] == 0)
		{
			in_count++;
		}
		else if(result_list[i] == 1)
		{
			out_count++;
		}
	}
	if(in_count > out_count)
	{
		return 0;
	}
	else if(in_count < out_count)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}