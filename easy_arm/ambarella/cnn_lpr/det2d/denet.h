#pragma once

class DeNet{
public:
    DeNet();
    ~DeNet();
    int init(const std::string &modelPath, const std::string &labelPath,
             const std::vector<std::string> &inputName, const std::vector<std::string> &outputName, 
             const int classNumber, const float threshold=0.1f);
    std::vector<std::vector<float>> run(const cv::Mat &srcImage);

private:
    std::vector<std::vector<float>> postprocess(const cv::Size src_size, const cv::Size dst_size, float *output[]);

private:
    cavalry_ctx_t cavalry_ctx;
    nnctrl_ctx_t nnctrl_ctx;
    float threshold;
    float nms_threshold;
    int top_k;
	int use_multi_cls;
    int classNumber;
};