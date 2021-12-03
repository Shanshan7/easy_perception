#include "cnn_runtime/pose/posenet.h"
#include "cnn_runtime/cnn_common/net_process.h"
#include "cnn_runtime/cnn_common/blob_define.h"
#include "cnn_runtime/cnn_common/image_process.h"
#include "cnn_runtime/pose/openpose_postprocess.h"
#include <iostream>

const static int nPoints = 18;

const static std::vector<std::pair<int,int>> posePairs1 = {
    {1,2}, {1,5}, {2,3}, {3,4}, {5,6}, {6,7},
    {1,8}, {8,9}, {9,10}, {1,11}, {11,12}, {12,13},
    {1,0}, {0,14}, {14,16}, {0,15}, {15,17}, {2,17},
    {5,16}
};

const static std::vector<cv::Scalar> colors = {
    cv::Scalar(255, 0, 0), cv::Scalar(0, 255, 0), cv::Scalar(0, 0, 255),
    cv::Scalar(255, 255, 0), cv::Scalar(0, 255, 255), cv::Scalar(255, 0, 255),
    cv::Scalar(142, 229, 238), cv::Scalar(128, 0, 0), cv::Scalar(0, 100, 0),
    cv::Scalar(0, 0, 128), cv::Scalar(128, 128, 0), cv::Scalar(0, 128, 128),
    cv::Scalar(65, 105, 225), cv::Scalar(132, 112, 255), cv::Scalar(238, 122, 233),
    cv::Scalar(28, 134, 238), cv::Scalar(178, 58, 238), cv::Scalar(150, 62, 255),
};

PoseNet::PoseNet()
{
    memset(&cavalry_ctx, 0, sizeof(cavalry_ctx_t));
    memset(&nnctrl_ctx, 0, sizeof(nnctrl_ctx_t));
    threshold = 0;
}

PoseNet::~PoseNet()
{
    deinit_net_context(&nnctrl_ctx, &cavalry_ctx);
    DPRINT_NOTICE("mtcnn_deinit\n");
}

int PoseNet::init(const std::string &modelPath, const std::string &inputName, \
                   const std::string &outputName, const float threshold)
{
    int rval = 0;
    set_net_param(&nnctrl_ctx, modelPath.c_str(), \
                    inputName.c_str(), outputName.c_str());
    rval = cnn_init(&nnctrl_ctx, &cavalry_ctx);
    this->threshold = threshold;
    return rval;
}

std::vector<std::vector<cv::Point>> PoseNet::run(const cv::Mat &srcImage)
{
    std::vector<std::vector<cv::Point>> result;
    std::vector<cv::Mat> netOutputParts;
    cv::Size srcSize = cv::Size(srcImage.cols, srcImage.rows);
    cv::Size inputSize = get_input_size(&nnctrl_ctx);
    float *tempOutput[1] = {NULL};
    cv::Mat bgrImage;
    if(srcImage.channels() == 1)
    {
        cv::cvtColor(srcImage, bgrImage, cv::COLOR_GRAY2BGR);
    }
    else
    {
        bgrImage = srcImage;
    }
    preprocess(&nnctrl_ctx, bgrImage, 0);
    cnn_run(&nnctrl_ctx, tempOutput, 1);
    int output_c = nnctrl_ctx.net.net_out.out_desc[0].dim.depth;
    int output_h = nnctrl_ctx.net.net_out.out_desc[0].dim.height;
    int output_w = nnctrl_ctx.net.net_out.out_desc[0].dim.width;
    int output_p = nnctrl_ctx.net.net_out.out_desc[0].dim.pitch / 4;
    int layer_count = output_h * output_p;
    float *scoreAddr = tempOutput[0];

    std::cout << "output size: " << "--output_c: " << output_c << "--output_h: " << output_h << "--output_w: " \
                                  << output_w << "--output_p: " << output_p << "--" << std::endl;

    netOutputParts.clear();
    for(int c = 0; c < output_c; ++c)
    {
        cv::Mat resizedPart;
        cv::Mat part(output_h, output_w, CV_32F, cv::Scalar(255));
        memcpy(part.data, scoreAddr, output_h *  output_p * sizeof(float));
        scoreAddr = scoreAddr + layer_count;
        cv::resize(part, resizedPart, inputSize, cv::INTER_NEAREST);
        netOutputParts.push_back(resizedPart);
    }

    // std::ofstream ouF;
    // ouF.open("./score.bin", std::ofstream::binary);
    // ouF.write(reinterpret_cast<const char*>(classnetOutput), sizeof(float) * CLASS_NUM);
    // ouF.close();
    result = postprocess(srcSize, inputSize, netOutputParts);
    return result;
}

int PoseNet::show(cv::Mat &image, const std::vector<std::vector<cv::Point>> &result,
                         const int pointSize, const int lineWidth)
{
    if(image.channels() == 1)
    {
        cv::cvtColor(image, image, cv::COLOR_GRAY2BGR);
    }
    for(size_t i = 0; i < nPoints;++i){
        for(size_t j = 0; j < result.size(); ++j){
            const cv::Point& kp = result[j][i];
            if(kp.x < 0 || kp.y < 0){
                    continue;
            }
            cv::circle(image, kp, pointSize, colors[i], -1, cv::LINE_AA);
        }
    }

    for(size_t i = 0; i< nPoints-1;++i){
        for(size_t n  = 0; n < result.size();++n){
            const std::pair<int,int>& posePair = posePairs1[i];
            const cv::Point& kpA = result[n][posePair.first];
            const cv::Point& kpB = result[n][posePair.second];
            if(kpA.x < 0 || kpA.y < 0 || kpB.x < 0 || kpB.y < 0){
                continue;
            }
            cv::line(image, kpA, kpB, colors[i], lineWidth, cv::LINE_AA);
        }
    }
    return 0;
}

std::vector<std::vector<cv::Point>> PoseNet::postprocess(const cv::Size src_size, const cv::Size dst_size, \
                                                         const std::vector<cv::Mat>& netOutputParts)
{
    float scale_w = static_cast<float>(src_size.width) / dst_size.width;
    float scale_h = static_cast<float>(src_size.height) / dst_size.height;
    std::vector<std::vector<cv::Point>> result;
    std::vector<std::vector<KeyPoint>> tempResult;
    tempResult.clear();
    result.clear();
    getOpenposeResult(netOutputParts, tempResult);
    for(size_t n  = 0; n < tempResult.size();++n)
    {
        std::vector<cv::Point> keypoints;
        keypoints.clear();
        for(size_t p = 0; p < tempResult[0].size(); p++)
        {
            int x = static_cast<int>(tempResult[n][p].point.x * scale_w);
            int y = static_cast<int>(tempResult[n][p].point.y * scale_h);
            if(x > src_size.width)
            {
                x = src_size.width;
            }
            if(y > src_size.height)
            {
                y = src_size.height;
            }
            tempResult[n][p].point.x = x;
            tempResult[n][p].point.y = y;
            if(tempResult[n][p].probability < 0)
            {
                keypoints.push_back(cv::Point(-1, -1));
                // std::cout << "pose point: -1, -1" << std::endl;
            }
            else
            {
                keypoints.push_back(cv::Point(x, y));
                // std::cout << "pose point:" << x << " " << y << std::endl;
            }
        }
        // std::cout << "pose count:" << keypoints.size() << std::endl;
        result.push_back(keypoints);
    }
    return result;
}
