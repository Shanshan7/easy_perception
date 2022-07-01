#include "ctx.h"
#include "traffic_lights_classifier.h"


TrafficLightsClassifier::TrafficLightsClassifier()
{
    traffic_lights_results.clear();
}

TrafficLightsClassifier:: ~TrafficLightsClassifier()
{

}

void TrafficLightsClassifier::red_green_yellow(const cv::Mat &rgb_image, const std::vector<float> traffic_lights_locations, const int32_t f_threshold)
{
    auto &ctx = Detail::Ctx::instance();
    cv::Mat rgb_image_roi, rgb_image_resize, hsv_image;
    rgb_image_roi = rgb_image(cv::Rect(traffic_lights_locations[0], traffic_lights_locations[1], traffic_lights_locations[2], \
                                        traffic_lights_locations[3]));
    cv::resize(rgb_image_roi, rgb_image_resize, cv::Size(64, 64));
    cv::cvtColor(rgb_image_resize, hsv_image, cv::COLOR_BGR2HSV);
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
    // ctx.log()(0, "CCCCCCCCCCCCCCCCCCCCCCCCCCCCC");

    int sum_all = sum_green + sum_red + sum_yellow;

    TrafficLightsParams traffic_lights_params;
    traffic_lights_params.traffic_lights_location[0] = traffic_lights_locations[0];
    traffic_lights_params.traffic_lights_location[1] = traffic_lights_locations[1];
    traffic_lights_params.traffic_lights_location[2] = traffic_lights_locations[2];
    traffic_lights_params.traffic_lights_location[3] = traffic_lights_locations[3];
    ctx.log()(0, "f_threshold: %d", f_threshold);

    if(sum_all > f_threshold)
    {
        if(sum_red >= sum_yellow && sum_red >= sum_green)
        {
            traffic_lights_params.traffic_lights_type = E_TRAFFIC_LIGHTS_TYPE_RED;
            ctx.log()(0, "light type: Red");
        }
        else if (sum_yellow >= sum_green)
        {
            traffic_lights_params.traffic_lights_type = E_TRAFFIC_LIGHTS_TYPE_GREEN;
            ctx.log()(0, "light type: Green");
        }
        else
        {
            traffic_lights_params.traffic_lights_type = E_TRAFFIC_LIGHTS_TYPE_YELLOW;
            ctx.log()(0, "light type: Yellow");
        }
    }
    else
    {
        traffic_lights_params.traffic_lights_type = E_TRAFFIC_LIGHTS_TYPE_DONT_KNOWN;
        ctx.log()(0, "light type: DontKnown");
    }
    this->traffic_lights_results.push_back(traffic_lights_params);
}