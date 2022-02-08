#ifndef _DETNET_H_
#define _DETNET_H_

#include "yolov5.h"

class DetNet{
public:
    DetNet();
    ~DetNet();
    // void set_log(int debug_en = 0);
    int init(const std::string &modelPath, const std::vector<std::string> &inputName, 
             const std::vector<std::string> &outputName, 
             const int classNumber);
    void run(ea_tensor_t *img_tensor);

    std::vector<DetectBox> det_results;

private:
    float conf_threshold;
    float nms_threshold;
    float obj_thresh;
    // int top_k;
	// int use_multi_cls;
    int classNumber;

    // int log_level;
    yolov5_t yolov5_ctx;
};

#endif // _DETNET_H_