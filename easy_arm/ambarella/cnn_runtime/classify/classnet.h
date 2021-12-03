#pragma once

//opencv
#include <opencv2/core.hpp>

#include "cnn_runtime/cnn_common/cnn_data_structure.h"

class ClassNet{
public:
    ClassNet();
    ~ClassNet();
    int init(const std::string &modelPath, 
             const std::string &inputName, 
             const std::string &outputName, 
             const int class_num=100,
             const float threshold=0.1f);
    int run(const cv::Mat &srcImage);

private:
    int postprocess(const float *output);

private:
    cavalry_ctx_t cavalry_ctx;
    nnctrl_ctx_t nnctrl_ctx;
    int class_number;
    float threshold;
    float *classnetOutput;
};