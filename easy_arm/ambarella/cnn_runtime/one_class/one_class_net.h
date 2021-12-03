#pragma once

//opencv
#include <opencv2/core.hpp>

#include "cnn_runtime/cnn_common/cnn_data_structure.h"

class OneClassNet{
public:
    OneClassNet();
    ~OneClassNet();
    int init(const std::string &modelPath, const std::string &inputName, 
             const std::string &outputName, const float threshold=0.1f);
    int run(const cv::Mat &srcImage, const std::string &embedding_file, float *score);

private:
    float postprocess(const float *output,
                      const std::string &embedding_file);

private:
    cavalry_ctx_t cavalry_ctx;
    nnctrl_ctx_t nnctrl_ctx;
    float threshold;
    float *oneClassOutput;
};