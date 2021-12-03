#pragma once

//opencv
#include <opencv2/core.hpp>

#include "cnn_runtime/cnn_common/cnn_data_structure.h"


class TextNet{
public:
    TextNet();
    ~TextNet();
    int init(const std::string &modelPath, const std::string &inputName, 
             const std::string &outputName, const float threshold=0.3f,
             const int charCount=0, const std::string charType="");
    cv::Mat cropImageROI(const cv::Mat &srcImage, const std::vector<cv::Point> &polygon);
    std::string run(const cv::Mat &srcImage);

private:
    cavalry_ctx_t cavalry_ctx;
    nnctrl_ctx_t nnctrl_ctx;
    std::string charType;
    int charCount;
    float threshold;
    float *textnetOutput;
};