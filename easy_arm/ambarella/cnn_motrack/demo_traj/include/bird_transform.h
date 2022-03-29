#ifndef _BIRD_TRANSFORM_H_
#define _BIRD_TRANSFORM_H_

#include <json/json.h>
#include <fstream> 
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>


class BirdTransform
{
public:
    BirdTransform();
    ~BirdTransform();

public:
    int bird_view_matrix_calculate();
    int img2bird(cv::Mat &input_image, cv::Mat &output_bird);
    int bird2img(cv::Mat &output_bird, cv::Mat &input_image);
    int projection_on_bird(cv::Point2f &point_image, cv::Point2f &point_bird);
    int projection_on_image(cv::Point2f &point_bird, cv::Point2f &point_image);

    cv::Mat transferI2B;
    cv::Mat transferB2I;
    float pixel2world_distance;
    float time_interval;
    int camera_position_pixel;
    float translation_position;

private:
    std::string camera_calibration_file;
    cv::Point2f point_image[4];
    cv::Point2f point_bird[4];
};

#endif // _BIRD_TRANSFORM_H_