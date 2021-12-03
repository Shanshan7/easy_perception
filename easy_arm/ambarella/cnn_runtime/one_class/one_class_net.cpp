#include "cnn_runtime/one_class/one_class_net.h"
#include "cnn_runtime/cnn_common/net_process.h"
#include "cnn_runtime/cnn_common/blob_define.h"
#include "cnn_runtime/cnn_common/image_process.h"
#include "cnn_runtime/one_class/patchcore_postprocess.h"
#include <iostream>



#define KNEIGHBOURS (9)

OneClassNet::OneClassNet()
{
    memset(&cavalry_ctx, 0, sizeof(cavalry_ctx_t));
    memset(&nnctrl_ctx, 0, sizeof(nnctrl_ctx_t));
    threshold = 0;
    oneClassOutput = NULL;
}

OneClassNet::~OneClassNet()
{
    deinit_net_context(&nnctrl_ctx, &cavalry_ctx);
    DPRINT_NOTICE("mtcnn_deinit\n");
    if(oneClassOutput != NULL)
    {
        delete[] oneClassOutput;
        oneClassOutput = NULL;
    }
}

int OneClassNet::init(const std::string &modelPath, const std::string &inputName, \
                   const std::string &outputName, const float threshold)
{
    int rval = 0;
    cv::Size outputSize;
    int outputChannel;
    set_net_param(&nnctrl_ctx, modelPath.c_str(), \
                    inputName.c_str(), outputName.c_str());
    rval = cnn_init(&nnctrl_ctx, &cavalry_ctx);
    this->threshold = threshold;
    outputChannel = get_output_channel(&nnctrl_ctx);
    outputSize = get_output_size(&nnctrl_ctx);
    this->oneClassOutput = new float[outputChannel * outputSize.height * outputSize.width];

    return rval;
}

int OneClassNet::run(const cv::Mat &srcImage, const std::string &embedding_file, float *score)
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

    for (int c = 0; c < output_c; c++)
    {
        for (int h = 0; h < output_h; h++)
        {
            memcpy(oneClassOutput + c * output_h * output_w + h * output_w, \
            tempOutput[0] + c * output_h * output_p  + h * output_p, output_w * sizeof(float));
        }
    }

    *score = postprocess(oneClassOutput, embedding_file);
    if(*score > this->threshold) {
        result = 1;
    }
    else {
        result = 0;
    }
    std::cout << "result: " << result << std::endl;
    return result;
}

float OneClassNet::postprocess(const float *output,
                        const std::string &embedding_file)
{
    int out_channel = nnctrl_ctx.net.net_out.out_desc[0].dim.depth;
    int out_height = nnctrl_ctx.net.net_out.out_desc[0].dim.height;
    int out_width = nnctrl_ctx.net.net_out.out_desc[0].dim.width;
    cv::Mat embedding_train = train_embedding_process(embedding_file, out_channel);
    cv::Mat embedding_test = reshape_embedding(output, out_channel, out_height, out_width);
    cv::Mat distances_mat = knn_process(embedding_train, embedding_test);
    int distanceMatWidth = distances_mat.size[0];
    int distanceMatHeight = distances_mat.size[1];
    // std::cout << "result: " << distances_mat.size[0] << " " << distances_mat.size[1] << std::endl;

    // reshape distances from 784 * 9 --> 9 * 784
    float* distances = new float[distanceMatWidth*distanceMatHeight];
    for (int d = 0; d < distanceMatHeight; d++)
    {
        for (int c = 0; c < distanceMatWidth; c++)
        {
            distances[d*distanceMatWidth + c] = ((float*)distances_mat.data)[c*distanceMatHeight + d];
            // memcpy(distances + d*784 + c, distances_mat.data + c*9 + d, sizeof(float));
        }
    }

    int max_posit = std::max_element(distances, \
				                distances + distanceMatWidth) - distances; // - distances.data;

    // std::cout << "max_posit: " << max_posit << std::endl;

    float* N_b = new float[distanceMatHeight];
    for(int i = 0; i < distanceMatHeight; i++)
    {
        N_b[i] = distances[i * distanceMatWidth + max_posit];
    }

    float w, sum_N_b = 0;
    for(int j = 0; j < distanceMatHeight; j++)
    {
        sum_N_b += exp(N_b[j]);
    }
    float max_N_b = *std::max_element(N_b, N_b + distanceMatHeight);
    w = (1 - exp(max_N_b) / sum_N_b);

    float score;
    score = w * distances[max_posit];

    return score;
}