#pragma once

//opencv
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>

#include "cnn_runtime/cnn_common/cnn_data_structure.h"

class PoseNet{
public:
    PoseNet();
    ~PoseNet();
    int init(const std::string &modelPath, const std::string &inputName, 
             const std::string &outputName, const float threshold=0.1f);
    std::vector<std::vector<cv::Point>> run(const cv::Mat &srcImage);

    static int show(cv::Mat &image, const std::vector<std::vector<cv::Point>> &result,
                    const int pointSize=20, const int lineWidth=10);

private:
    std::vector<std::vector<cv::Point>> postprocess(const cv::Size src_size, const cv::Size dst_size, 
                                                    const std::vector<cv::Mat>& netOutputParts);

private:
    cavalry_ctx_t cavalry_ctx;
    nnctrl_ctx_t nnctrl_ctx;
    float threshold;
};