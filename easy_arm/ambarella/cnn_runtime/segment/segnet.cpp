#include "cnn_runtime/segment/segnet.h"
#include "cnn_runtime/cnn_common/net_process.h"
#include "cnn_runtime/cnn_common/blob_define.h"
#include "cnn_runtime/cnn_common/image_process.h"
#include <iostream>

#define THRESH (0.5f)

SegNet::SegNet()
{
    memset(&cavalry_ctx, 0, sizeof(cavalry_ctx_t));
    memset(&nnctrl_ctx, 0, sizeof(nnctrl_ctx_t));
    threshold = 0;
    segnetOutput = NULL;
}

SegNet::~SegNet()
{
    deinit_net_context(&nnctrl_ctx, &cavalry_ctx);
    DPRINT_NOTICE("mtcnn_deinit\n");
    if(segnetOutput != NULL)
    {
        delete[] segnetOutput;
        segnetOutput = NULL;
    }
}

int SegNet::init(const std::string &modelPath, const std::string &inputName, \
                   const std::string &outputName, const float threshold)
{
    int rval = 0;
    set_net_param(&nnctrl_ctx, modelPath.c_str(), \
                    inputName.c_str(), outputName.c_str());
    rval = cnn_init(&nnctrl_ctx, &cavalry_ctx);
    this->threshold = threshold;
    this->inputSize = get_input_size(&nnctrl_ctx);
    this->segnetOutput = new float[this->inputSize.height * this->inputSize.width];

    return rval;
}

cv::Mat SegNet::run(const cv::Mat &srcImage)
{
    cv::Mat result;
    cv::Size srcSize = cv::Size(srcImage.cols, srcImage.rows);
    float *tempOutput[1] = {NULL};
    preprocess(&nnctrl_ctx, srcImage, 0);
    cnn_run(&nnctrl_ctx, tempOutput, 1);
    int output_c = nnctrl_ctx.net.net_out.out_desc[0].dim.depth;
    int output_h = nnctrl_ctx.net.net_out.out_desc[0].dim.height;
    int output_w = nnctrl_ctx.net.net_out.out_desc[0].dim.width;
    int output_p = nnctrl_ctx.net.net_out.out_desc[0].dim.pitch / 4;

    std::cout << "output size: " << "--output_c: " << output_c << "--output_h: " << output_h << "--output_w: " \
                                  << output_w << "--output_p: " << output_p << "--" << std::endl;

    for (int h = 0; h < output_h; h++)
    {
        memcpy(segnetOutput + h * output_w, tempOutput[0] + h * output_p, output_w * sizeof(float));
    }

    // std::ofstream ouF;
    // ouF.open("./score.bin", std::ofstream::binary);
    // ouF.write(reinterpret_cast<const char*>(tempOutput[0]), sizeof(float) * CLASS_NUM);
    // ouF.close();
    postprocess(segnetOutput, 0, srcSize, this->inputSize, result);
    return result;
}

void SegNet::postprocess(const float* output, const int flag, const cv::Size srcSize, 
                         const cv::Size inputSize, cv::Mat &result)
{
    cv::Mat seg_mat(inputSize.height, inputSize.width, CV_8UC3);
    uint8_t colorB[] = {0, 0};
    uint8_t colorG[] = {0, 0};
    uint8_t colorR[] = {0, 255};
    for (int row = 0; row < seg_mat.rows; row++) {
        for (int col = 0; col < seg_mat.cols; col++) {
            int posit;
            if (output[row * seg_mat.cols + col] > THRESH) {
                posit = 1;
            }
            else {
                posit = 0;
            }
            if(flag == 0){
                seg_mat.at<cv::Vec3b>(row, col) = cv::Vec3b(posit, posit, posit);
            }
            else if (flag == 1){
                seg_mat.at<cv::Vec3b>(row, col) = cv::Vec3b(colorB[posit], colorG[posit], colorR[posit]);
            }
        }
    }
    cv::resize(seg_mat, result, srcSize, 0, 0, cv::INTER_NEAREST);
}