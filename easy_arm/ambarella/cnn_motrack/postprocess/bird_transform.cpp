#include "bird_transform.h"


BirdTransform::BirdTransform()
{
    camera_calibration_file = "/sdcard/traj/calibration_paramater.json";

    Json::Reader reader;
	Json::Value root;
 
	std::ifstream camera_calibration_in(camera_calibration_file, std::ios::binary);
 
	if (!camera_calibration_in.is_open())
	{
		std::cout << "Error opening file\n";
		return;
	}

    if (reader.parse(camera_calibration_in, root))
	{
        Json::Value transferI2B_array;
        transferI2B_array = root["paramater"]['transfer_Image2Bird'];
        for(int i = 0; i < transferI2B_array.size(); i++){
            std::cout << transferI2B_array[i].asDouble() << std::endl;
        }
        // transferB2I = root["paramater"]['transfer_Bird2Image']
        pixel2world_distance = root["paramater"]['pixel2world_distance'].asDouble();
        time_interval = root["paramater"]['time_interval'].asDouble();
    }
    camera_calibration_in.close();
}

int BirdTransform::bird_view_matrix_calculate()
{
    this->transferI2B = cv::getPerspectiveTransform(this->point_image, this->point_bird);
    return 0;
}

int BirdTransform::img2bird(cv::Mat &input_image, cv::Mat &output_bird)
{
    int width = input_image.cols;
    int height = input_image.rows;
    cv::warpPerspective(input_image, output_bird, this->transferI2B, cv::Size(width, height));
    return 0;
}

int BirdTransform::projection_on_bird(cv::Point2f &point_image, cv::Point2f &point_bird)
{
    point_bird.x = (this->transferI2B.at<float>(0,0) * point_image.x + this->transferI2B.at<float>(0,1) * point_image.y \
                     + this->transferI2B.at<float>(0,2)) / (this->transferI2B.at<float>(2,0) * point_image.x \
                     + this->transferI2B.at<float>(2,1) * point_image.y + this->transferI2B.at<float>(2,2));

    point_bird.y = (this->transferI2B.at<float>(1,0) * point_image.x + this->transferI2B.at<float>(1,1) * point_image.y \
                     + this->transferI2B.at<float>(1,2)) / (this->transferI2B.at<float>(2,0) * point_image.x \
                     + this->transferI2B.at<float>(2,1) * point_image.y + this->transferI2B.at<float>(2,2));

    return 0;
}