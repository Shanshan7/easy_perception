#ifndef _LED_PROCESS_H_
#define _LED_PROCESS_H_

//opencv
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include <glog/logging.h>
#include <glog/raw_logging.h>

class LEDProcess
{
public:
    LEDProcess();
    ~LEDProcess();
    int led_open();
    int write_process(const cv::Mat &bgr);
    int write_close();
    int night_process(const cv::Mat &bgr);

private:
    int is_night;
    int led_device;
};

#endif // _LED_PROCESS_H_