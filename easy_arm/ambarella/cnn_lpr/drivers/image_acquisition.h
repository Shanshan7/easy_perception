#ifndef _IMAGE_ACQUISITION_H_
#define _IMAGE_ACQUISITION_H_

#include <pthread.h>
#include <sys/prctl.h>

#include <glog/logging.h>
#include <glog/raw_logging.h>

//opencv
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>

#include "utility/utils.h"

//#define ONLY_SAVE_DATA
//#define ONLY_SEND_DATA

#if defined(ONLY_SAVE_DATA) || defined(ONLY_SEND_DATA)
#define IMAGE_BUFFER_SIZE (4)
#else
#define IMAGE_BUFFER_SIZE (2)
#endif
#define IMAGE_WIDTH (1920)
#define IMAGE_HEIGHT (1080)
#define IMAGE_YUV_SIZE		(IMAGE_WIDTH * IMAGE_HEIGHT * 3 / 2) //yuv420

struct ImageBuffer  
{  	
	unsigned char buffer[IMAGE_BUFFER_SIZE][IMAGE_YUV_SIZE];
    pthread_mutex_t lock; /* 互斥体lock 用于对缓冲区的互斥操作 */  
    int readpos, writepos; /* 读写指针*/  
    pthread_cond_t notempty; /* 缓冲区非空的条件变量 */  
    pthread_cond_t notfull; /* 缓冲区未满的条件变量 */  
};

class ImageAcquisition
{
public:

    ImageAcquisition();
    ~ImageAcquisition();

    int open_camera();

    int start();
    int stop();

    void get_image(cv::Mat &src_image);

    void get_yuv(unsigned char* addr);

private:
    pthread_t pthread_id;
};

#endif // _IMAGE_ACQUISITION_H_