#include "cnn_runtime/det2d/denet.h"
#include "cnn_runtime/cnn_common/net_process.h"
#include "cnn_runtime/cnn_common/image_process.h"
#include "cnn_runtime/det2d/yolov3.h"
#include <iostream>
#include <string.h>
#include <math.h>

#define OUTPUT_NUM (3)

DeNet::DeNet()
{
    memset(&cavalry_ctx, 0, sizeof(cavalry_ctx_t));
    memset(&nnctrl_ctx, 0, sizeof(nnctrl_ctx_t));
    threshold = 0;
}

DeNet::~DeNet()
{
    deinit_net_context(&nnctrl_ctx, &cavalry_ctx);
    DPRINT_NOTICE("mtcnn_deinit\n");
}

int DeNet::init(const std::string &modelPath, const std::vector<std::string> &inputName, 
             const std::vector<std::string> &outputName, const int classNumber, const float threshold)
{
    int rval = 0;
    int inputCount = static_cast<int>(inputName.size());
    int outputCount = static_cast<int>(outputName.size());
    char ** inNames = new char*[inputCount];
    char ** outNames = new char*[outputCount];
    for(size_t i = 0; i < inputName.size(); i++){
        inNames[i] = new char[inputName[i].size() + 1];
        strcpy(inNames[i], inputName[i].c_str());
    }
    for(size_t i = 0; i < outputName.size(); i++){
        outNames[i] = new char[outputName[i].size() + 1];
        strcpy(outNames[i], outputName[i].c_str());
    }
    set_net_multi_param(&nnctrl_ctx, modelPath.c_str(), \
                       (const char**)inNames, inputCount, \
                       (const char**)outNames, outputCount);
    rval = cnn_init(&nnctrl_ctx, &cavalry_ctx);
    this->classNumber = classNumber;
    this->threshold = threshold;
    
    for(size_t i = 0; i < inputName.size(); i++){
        delete [] inNames[i];
    }
    delete [] inNames;
    for(size_t i = 0; i < outputName.size(); i++){
        delete [] outNames[i];
    }
    delete [] outNames;

    return rval;
}

std::vector<std::vector<float>> DeNet::run(const cv::Mat &srcImage)
{
    std::vector<std::vector<float>> boxes;
    cv::Size srcSize = cv::Size(srcImage.cols, srcImage.rows);
    cv::Size inputSize = get_input_size(&nnctrl_ctx);
    float *output[OUTPUT_NUM] = {NULL};
    boxes.clear();
    preprocess(&nnctrl_ctx, srcImage, 1);
    cnn_run(&nnctrl_ctx, output, OUTPUT_NUM);
    int output_c = nnctrl_ctx.net.net_out.out_desc[0].dim.depth;
    int output_h = nnctrl_ctx.net.net_out.out_desc[0].dim.height;
    int output_w = nnctrl_ctx.net.net_out.out_desc[0].dim.width;
    int output_p = nnctrl_ctx.net.net_out.out_desc[0].dim.pitch / 4;

    // std::cout << "output size: " << "--output_c: " << output_c << "--output_h: " << output_h << "--output_w: " \
    //                               << output_w << "--output_p: " << output_p << "--" << std::endl;

    // std::ofstream ouF;
    // ouF.open("./score.bin", std::ofstream::binary);
    // ouF.write(reinterpret_cast<const char*>(output[0]), sizeof(float) * CLASS_NUM);
    // ouF.close();
    boxes = postprocess(srcSize, inputSize, output);
    return boxes;
}

std::vector<std::vector<float>> DeNet::postprocess(const cv::Size src_size, const cv::Size dst_size, \
                                                   float *output[OUTPUT_NUM])
{
    float ratio;
    cv::Size pad_size;

    std::vector<std::vector<float>> final_results;
	final_results = yolo_run(this->classNumber, output[0], output[1], output[2]);

    get_square_size(src_size, dst_size, ratio, pad_size);

    for (size_t i = 0; i < final_results.size(); ++i)
    {
        final_results[i][0] = (final_results[i][0] - floor((float)pad_size.width / 2)) / ratio;
        final_results[i][1] = (final_results[i][1] - floor((float)pad_size.height / 2)) / ratio;
        final_results[i][2] = final_results[i][2] / ratio;
        final_results[i][3] = final_results[i][3] / ratio;

        if (final_results[i][0] < 1.0) {
            final_results[i][0] = 1.0;
        }
        if (final_results[i][1] < 1.0) {
            final_results[i][1] = 1.0;
        }
        if (final_results[i][0] + final_results[i][2] >= src_size.width) {
            final_results[i][2] = floor((src_size.width - final_results[i][0]));
        }
        if (final_results[i][1] + final_results[i][3] >= src_size.height) {
            final_results[i][3] = floor((src_size.height - final_results[i][1]));
        }
    }
    // std::cout << "final results size: " << final_results.size() << std::endl;

    return final_results;
}