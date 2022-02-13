#ifndef _DENETV2_H_
#define _DENETV2_H_

#include "../../common/data_struct.h"
#include "../../common/utils.h"

#include "yolov5.h"

class DetNet{
public:
    DetNet();
    ~DetNet();
    int init(const std::string &modelPath, const std::vector<std::string> &inputName, 
             const std::vector<std::string> &outputName);
    int run(ea_tensor_t *img_tensor);
    void deinit();
    std::vector<DetectBox> det_results;

private:
    float threshold;
    float nms_threshold;
    int top_k;
	int use_multi_cls;

    int log_level;
    yolov5_t yolov5_ctx;
};

#endif // _DENETV2_H_