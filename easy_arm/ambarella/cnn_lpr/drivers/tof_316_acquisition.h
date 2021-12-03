#ifndef _TOF_ACQUISITION_H_
#define _TOF_ACQUISITION_H_

#include <vector>
#include <pthread.h>
#include <sys/prctl.h>

#include <glog/logging.h>
#include <glog/raw_logging.h>

//opencv
#include <opencv2/core.hpp>

#define TOF_BUFFER_SIZE (3)
#define MAX_POINT_CLOUD (240*180)

#define DEPTH_WIDTH (240)
#define DEPTH_HEIGTH (180)
#define TOF_SIZE		(DEPTH_WIDTH * DEPTH_HEIGTH * 4) //float type

struct TOFBuffer  
{  	
    // float buffer_x[TOF_BUFFER_SIZE][MAX_POINT_CLOUD];
	// float buffer_y[TOF_BUFFER_SIZE][MAX_POINT_CLOUD];
	float buffer_z[TOF_BUFFER_SIZE][MAX_POINT_CLOUD];
    pthread_mutex_t lock; /* 互斥体lock 用于对缓冲区的互斥操作 */  
    int readpos, writepos; /* 读写指针*/  
    pthread_cond_t notempty; /* 缓冲区非空的条件变量 */  
    pthread_cond_t notfull; /* 缓冲区未满的条件变量 */  
};

class TOF316Acquisition
{
public:
    struct Point
    {
        float x;
        float y;
        float z;
        int index;
    };
    typedef std::vector<Point> PointCloud;

    TOF316Acquisition();
    ~TOF316Acquisition();

    int open_tof();

    int start();
    int stop();

    void set_up();
    void set_sleep();

    void get_tof_depth_map(cv::Mat &depth_map);

    void get_tof_Z(unsigned char* addr);

    // void get_tof_pc(PointCloud &point_cloud);

    // void get_tof_data(PointCloud &point_cloud, cv::Mat &depth_map);

    int dump_ply(const char* save_path, const PointCloud &src_cloud);
    int dump_bin(const std::string &save_path, const PointCloud &src_cloud);
    int read_bin(const std::string &file_path, PointCloud &result_cloud);

private:
    pthread_t pthread_id;
};

#endif // _TOF_ACQUISITION_H_