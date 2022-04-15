#include "traffic_lights_classifier.h"


TrafficLightsClassifier::TrafficLightsClassifier()
{
    traffic_lights_results.clear();
}

TrafficLightsClassifier:: ~TrafficLightsClassifier()
{

}

void TrafficLightsClassifier::red_green_yellow(cv::Mat rgb_image)
{
    cv::Mat resize_rgb_image, hsv_image;
    cv::resize(rgb_image, resize_rgb_image, cv::Size(64, 64));
    cv::cvtColor(resize_rgb_image, hsv_image, cv::COLOR_BGR2HSV);
    std::vector<cv::Mat> hsv_split;
    cv::split(hsv_image, hsv_split);
    cv::Scalar sum_saturation = cv::sum(hsv_split[2]);  // Sum the brightness values
    int area = 64 * 64;
    float avg_saturation = sum_saturation[0] / area;

    int sat_low = (int)(avg_saturation * 1.3);
    int val_low = 140;

    cv::Mat green_mask, yellow_mask, red_mask_1, red_mask_2;
    // Green
    cv::Scalar lower_green = cv::Scalar(60, sat_low, val_low);
    cv::Scalar upper_green = cv::Scalar(100, 255, 255);
	cv::inRange(hsv_image, lower_green, upper_green, green_mask);
    int sum_green = cv::countNonZero(green_mask);
    // Yellow
    cv::Scalar lower_yellow = cv::Scalar(15, sat_low, val_low);
    cv::Scalar upper_yellow = cv::Scalar(60, 255, 255);
	cv::inRange(hsv_image, lower_yellow, upper_yellow, yellow_mask);
    int sum_yellow = cv::countNonZero(yellow_mask);
    // Red
    cv::Scalar red_min_1 = cv::Scalar(0, sat_low, val_low);
    cv::Scalar red_max_1 = cv::Scalar(8, 255, 255);
    cv::inRange(hsv_image, red_min_1, red_max_1, red_mask_1);
    cv::Scalar red_min_2 = cv::Scalar(150, sat_low, val_low);
    cv::Scalar red_max_2 = cv::Scalar(180, 255, 255);
    cv::inRange(hsv_image, red_min_2, red_max_2, red_mask_2);
    int sum_red = cv::countNonZero(red_mask_1) + cv::countNonZero(red_mask_2);

    int sum_all = sum_green + sum_red + sum_yellow;

    if(sum_all > 5)
    {
        if(sum_red >= sum_yellow && sum_red >= sum_green)
        {
            // this->traffic_lights_params.traffic_lights_type = 0;
            std::cout << "light type: Red" << std::endl;
        }
        else if (sum_yellow >= sum_green)
        {
            std::cout << "light type: yellow" << std::endl;
        }
        else
        {
            std::cout << "light type: green" << std::endl;
        }
    }
    else
    {
        std::cout << "light type: DontKnown" << std::endl;
    }
}