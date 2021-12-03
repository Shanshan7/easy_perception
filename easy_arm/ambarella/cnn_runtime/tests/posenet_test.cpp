#include <signal.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <opencv2/imgcodecs.hpp>
#include "utility/utils.h"
#include "cnn_runtime/pose/posenet.h"

#include "IOdevice.hpp"

//#define TEST_R01TH
//#define TEST_I05C
//#define TEST_I05M
#define TEST_I08C
//#define TOFPROCESS

//#define TEST_VIDEO

const static std::string model_path = "./posenet.bin";
const static std::string input_name = "data";
const static std::string output_name = "concat_stage7";

static volatile bool stop = false;

static void sig_int_handler(int)
{
   stop = true;
}

void image_txt_infer(const std::string &image_dir, const std::string &image_txt_path)
{
    unsigned long frame_index = 0;
    unsigned long time_start, time_end;
    std::vector<std::vector<cv::Point>> result;
    std::ifstream read_txt;
    std::string line_data;
    cv::Mat src_image;
    PoseNet pose_process;
    if(pose_process.init(model_path, input_name, output_name) < 0)
    {
        std::cout << "ClassNet init fail!" << std::endl;
        return;
    }

    read_txt.open(image_txt_path.data());
    if(!read_txt.is_open()){
        std::cout << image_txt_path << " not exits" << std::endl;
        return;
    }
    
    while(std::getline(read_txt, line_data)){
        result.clear();
        if(line_data.empty()){
            continue;
        }
        size_t index = line_data.find_first_of(' ', 0);
        std::string image_name = line_data.substr(0, index);
        std::stringstream image_path;
        image_path << image_dir << image_name;
        std::cout << frame_index << image_path.str() << std::endl;
        src_image = cv::imread(image_path.str());
        time_start = get_current_time();
        result = pose_process.run(src_image);
        time_end = get_current_time();
        std::cout << "posenet cost time: " <<  (time_end - time_start)/1000.0  << "ms" << std::endl;

        pose_process.show(src_image, result);

        std::stringstream save_path;
        save_path << frame_index << "_li.png";
        cv::imwrite(save_path.str(), src_image);
        frame_index++;
    }
    read_txt.close();
}

void camera_infer()
{
    unsigned long frame_index = 0;
    unsigned long time_start, time_end;
    CameraDevice cam;
    std::vector<std::vector<cv::Point>> result;
    cv::Mat src_image;
    PoseNet pose_process;
    if(pose_process.init(model_path, input_name, output_name) < 0)
    {
        std::cout << "ClassNet init fail!" << std::endl;
        return;
    }

#ifdef TEST_VIDEO
    cv::VideoCapture capture;
    capture.open("/sdcard/video.avi", cv::CAP_OPENCV_MJPEG);
	if (capture.isOpened())
	{
		std::cout << "open video failed..\n" << std::endl;
		return;
	}
#endif

#ifdef TEST_I05C
	cam.loadCamera(CameraDevice::I05C,CameraDevice::CameraImSize::I05C_2432x2048);
#endif

#ifdef TEST_I05M
//	cam.loadCamera(CameraDevice::I05M,CameraDevice::CameraImSize::I05M_2432x2048);
	cam.loadCamera(CameraDevice::I05M,CameraDevice::CameraImSize::I05M_1920x1080);
#endif

#ifdef TEST_R01TH
	cam.loadCamera(CameraDevice::R01TH,CameraDevice::CameraImSize::R01TH_1280x800);
#endif

#ifdef TEST_I08C
	cam.loadCamera(CameraDevice::I08C,CameraDevice::CameraImSize::I08C_3840x2160);
#endif

	HDMIDevice hdmi;
	hdmi.HDMIInit();

    std::cout << "HDMI init success" << std::endl;

    signal(SIGINT, sig_int_handler);
    signal(SIGTERM, sig_int_handler);
    signal(SIGKILL, sig_int_handler);
    signal(SIGQUIT, sig_int_handler);

    while(1){
        result.clear();
		// grab image
		time_start = get_current_time();
#ifdef TEST_VIDEO
        if(!capture.read(src_image))
        {
            break;
        }
#else
        // std::cout << "grap capture" << std::endl;
		src_image = cam.grabImage();
        // std::cout << "image size:" << src_image.channels() << std::endl;
#endif
        time_end = get_current_time();
		double dt_grab = (time_end - time_start) / 1000.0;

		time_start = get_current_time();
        result = pose_process.run(src_image);
        time_end = get_current_time();
        std::cout << "posenet cost time: " <<  (time_end - time_start)/1000.0  << "ms" << std::endl;

        pose_process.show(src_image, result);

		// display image
		time_start = get_current_time();
		hdmi.HDMIShow(src_image);
        time_end = get_current_time();
		double dt_disp = (time_end - time_start) / 1000.0;

		// printf("%d %d -> grabbing display time/fq : %1.3lf/%2.1lf %1.3lf/%2.1lf %d\n",src_image.cols,src_image.rows,dt_grab,1.0/dt_grab,dt_disp,1.0/dt_disp,src_image.channels());

        // std::stringstream save_path;
        // save_path << frame_index << "_li.png";
        // cv::imwrite(save_path.str(), src_image);
        // frame_index++;

		if (stop){break;}
	}
}


int main()
{
    std::cout << "start..." << std::endl;
    const std::string image_dir = "./images/";
    const std::string image_txt_path = "img.txt";
    image_txt_infer(image_dir, image_txt_path);
    // camera_infer();
    std::cout << "End of game!!!" << std::endl;
    return 0;
}
