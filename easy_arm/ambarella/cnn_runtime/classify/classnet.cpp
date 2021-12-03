#include "cnn_runtime/classify/classnet.h"
#include "cnn_runtime/cnn_common/net_process.h"
#include "cnn_runtime/cnn_common/blob_define.h"
#include "cnn_runtime/cnn_common/image_process.h"
#include <iostream>

ClassNet::ClassNet()
{
    memset(&cavalry_ctx, 0, sizeof(cavalry_ctx_t));
    memset(&nnctrl_ctx, 0, sizeof(nnctrl_ctx_t));
    class_number = 0;
    threshold = 0;
    classnetOutput = NULL;
}

ClassNet::~ClassNet()
{
    deinit_net_context(&nnctrl_ctx, &cavalry_ctx);
    DPRINT_NOTICE("mtcnn_deinit\n");
    if(classnetOutput != NULL)
    {
        delete[] classnetOutput;
        classnetOutput = NULL;
    }
}

int ClassNet::init(const std::string &modelPath, const std::string &inputName, \
                   const std::string &outputName, const int class_num, const float threshold)
{
    int rval = 0;
    set_net_param(&nnctrl_ctx, modelPath.c_str(), \
                    inputName.c_str(), outputName.c_str());
    rval = cnn_init(&nnctrl_ctx, &cavalry_ctx);
    this->class_number = class_num;
    this->threshold = threshold;
    this->classnetOutput = new float[class_number];

    return rval;
}

int ClassNet::run(const cv::Mat &srcImage)
{
    int result = -1;
    float *tempOutput[1] = {NULL};
    preprocess(&nnctrl_ctx, srcImage, 0);
    cnn_run(&nnctrl_ctx, tempOutput, 1);
    int output_c = nnctrl_ctx.net.net_out.out_desc[0].dim.depth;
    int output_h = nnctrl_ctx.net.net_out.out_desc[0].dim.height;
    int output_w = nnctrl_ctx.net.net_out.out_desc[0].dim.width;
    int output_p = nnctrl_ctx.net.net_out.out_desc[0].dim.pitch / 4;

    std::cout << "output size: " << "--output_c: " << output_c << "--output_h: " << output_h << "--output_w: " \
                                  << output_w << "--output_p: " << output_p << "--" << std::endl;

    for (int i=0; i < class_number; i++) 
    {
        classnetOutput[i] = DIM1_DATA(tempOutput[0], i);
    }

    // std::ofstream ouF;
    // ouF.open("./score.bin", std::ofstream::binary);
    // ouF.write(reinterpret_cast<const char*>(classnetOutput), sizeof(float) * class_number);
    // ouF.close();
    result = postprocess(classnetOutput);
    return result;
}

int ClassNet::postprocess(const float *output)
{
    float id_max = -100;
    int class_idx = 0;
    for (int id = 0; id < class_number; id++) {
        if (output[id] > id_max) {
            id_max = output[id];
            class_idx = id;
        }
    }
    return class_idx;
}