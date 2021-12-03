#pragma once

//opencv
#include <opencv2/core.hpp>

#include "cnn_runtime/cnn_common/cnn_data_structure.h"

class SegNet{
public:
    SegNet();
    ~SegNet();
    int init(const std::string &modelPath, const std::string &inputName, 
             const std::string &outputName, const float threshold=0.1f);
    cv::Mat run(const cv::Mat &srcImage);

private:
    void postprocess(const float* output, const int flag, const cv::Size srcSize, 
                    const cv::Size inputSize, cv::Mat &result);

private:
    cavalry_ctx_t cavalry_ctx;
    nnctrl_ctx_t nnctrl_ctx;
    float threshold;
    float *segnetOutput;
    cv::Size inputSize;
};