#include "led_process.h"
#include <iostream>

LEDProcess::LEDProcess()
{
	is_night = 0;
    led_device = -1;
}

LEDProcess::~LEDProcess()
{
	if(led_device >= 0)
    {
		write(led_device, "0", sizeof(char));
        close(led_device);
		led_device = -1;
		LOG(INFO) << "close led";
    }
}

int LEDProcess::led_open()
{
	led_device = open("/sys/devices/platform/e4000000.n_apb/e4008000.i2c/i2c-0/0-0064/leds/lm36011:torch/brightness", O_RDWR, 0);
	if (led_device < 0) {
		LOG(ERROR) << "open led fail";
        return -1;
	}
	return 0;
}

int LEDProcess::write_process(const cv::Mat &bgr)
{
	if(led_device > 0)
	{
		is_night = night_process(bgr);
		if(is_night > 0)
		{
			// std::cout << "is_night:" << is_night << std::endl;
			write(led_device, "20", sizeof(char));
		}
		else
		{
			write(led_device, "0", sizeof(char));
		}
	}
	return 0;
}

int LEDProcess::write_close()
{
	write(led_device, "0", sizeof(char));
	return 0;
}

int LEDProcess::night_process(const cv::Mat &bgr)
{
	cv::Mat gray;
	cv::cvtColor(bgr, gray, CV_BGR2GRAY);
	//std::cout << "image size:" << gray.cols << " " << gray.rows << std::endl;
	cv::Scalar left_mean = cv::mean(gray(cv::Rect(0, 0, 300, 300)));  
	cv::Scalar right_mean = cv::mean(gray(cv::Rect(gray.cols-1-300, 0, 300, 300)));
	//std::cout << "left:" << left_mean.val[0] << " right:" << right_mean.val[0] << std::endl;
	if(left_mean.val[0] < 50 && right_mean.val[0] < 50)
	{
		return 1;
	}
	else
	{
		return 0;
	} 
}