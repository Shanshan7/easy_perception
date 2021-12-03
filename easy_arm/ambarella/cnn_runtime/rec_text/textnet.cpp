#include "cnn_runtime/rec_text/textnet.h"
#include "cnn_runtime/cnn_common/net_process.h"
#include "cnn_runtime/cnn_common/cnn_function.h"
#include "cnn_runtime/cnn_common/image_process.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>

const static int maxTextLength = 32;
const static int classNumber = 37;
// const static char characterSet[classNumber] = {' ', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 
//                                                'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 
//                                                's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 
//                                                'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 
//                                                'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 
//                                                'W', 'X', 'Y', 'Z', '1', '2', '3', '4', '5', '6', 
//                                                '7', '8', '9', '0', '!', '"', '#', '$', '%', '&', 
//                                                '\\', '\'', '(', ')', '*', '+', ',', '-', '.', '/', 
//                                                ':', ';', '<', '=', '>', '?', '@', '[', ']', '^', 
//                                                '_', '`', '{', '}', '~', '|', ' '};
const static char characterSet[classNumber] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 
                                               'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 
                                               'U', 'V', 'W', 'X', 'Y', 'Z', '1', '2', '3', '4', '5', 
                                               '6', '7', '8', '9', '0'};

TextNet::TextNet()
{
    memset(&cavalry_ctx, 0, sizeof(cavalry_ctx_t));
    memset(&nnctrl_ctx, 0, sizeof(nnctrl_ctx_t));
    threshold = 0;
    textnetOutput = NULL;
}

TextNet::~TextNet()
{
    deinit_net_context(&nnctrl_ctx, &cavalry_ctx);
    DPRINT_NOTICE("mtcnn_deinit\n");
    if(textnetOutput != NULL)
    {
        delete[] textnetOutput;
        textnetOutput = NULL;
    }
}

int TextNet::init(const std::string &modelPath, const std::string &inputName, \
                  const std::string &outputName, const float threshold, \
                  const int charCount, const std::string charType)
{
    int rval = 0;
    set_net_param(&nnctrl_ctx, modelPath.c_str(), \
                    inputName.c_str(), outputName.c_str());
    rval = cnn_init(&nnctrl_ctx, &cavalry_ctx);
    this->charType = charType;
    this->charCount = charCount;
    this->threshold = threshold;
    this->textnetOutput = new float[maxTextLength * classNumber];

    return rval;
}

cv::Mat TextNet::cropImageROI(const cv::Mat &srcImage, const std::vector<cv::Point> &polygon)
{
    cv::Scalar borderValue;
    if(srcImage.channels() == 1)
    {
        borderValue = cv::Scalar(0);
    }
    else
    {
        borderValue = cv::Scalar(0, 0, 0);
    }
    cv::Point2f srcpoint[4];//存放变换前四顶点
    cv::Point2f dstpoint[4];//存放变换后四顶点
    cv::RotatedRect rect = cv::minAreaRect(polygon);
    float width = rect.size.width;
    float height = rect.size.height;
    rect.points(srcpoint);//获取最小外接矩形四顶点坐标
    dstpoint[0]= cv::Point2f(0, height);
    dstpoint[1] = cv::Point2f(0, 0);
    dstpoint[2] = cv::Point2f(width, 0);
    dstpoint[3] = cv::Point2f(width, height);
    cv::Mat M = cv::getPerspectiveTransform(srcpoint, dstpoint);
    cv::Mat result = cv::Mat::zeros(cv::Size(width, height), CV_8UC3);
    cv::warpPerspective(srcImage, result, M, result.size(), cv::INTER_LINEAR, cv::BORDER_CONSTANT, borderValue);
    std::cout << "width:" << result.cols << " height:" << result.rows << std::endl;
    if((result.rows * 1.0f / result.cols) >= 2.0f)
    {
        // cv::Point2f center;
        // center.x = float(result.cols / 2.0);
        // center.y = float(result.rows / 2.0);
        // int length = cv::sqrt(result.cols * result.cols + result.rows * result.rows);
        // cv::Mat M = cv::getRotationMatrix2D(center, -90, 1);
        // cv::warpAffine(src, src_rotate, M, Size(length, length), 1, 0, Scalar(0, 0, 0));
        cv::Mat temp;
        cv::transpose(result, temp);
        cv::flip(temp, result, 0);
    }
    return result;
}

std::string TextNet::run(const cv::Mat &srcImage)
{
    int count = 0;
    std::string result = "";
    float max_score = 0.0f;
    int pre_max_index = 0;
    int max_index = 0;
    float *tempOutput[1] = {NULL};
    // cv::Mat filterMat;
    // cv::medianBlur(srcImage, filterMat, 5);
    // cv::bilateralFilter(srcImage, filterMat, 10, 10 * 2, 10 / 2);
    preprocess(&nnctrl_ctx, srcImage, 2);
    cnn_run(&nnctrl_ctx, tempOutput, 1);
    int output_c = nnctrl_ctx.net.net_out.out_desc[0].dim.depth;
    int output_h = nnctrl_ctx.net.net_out.out_desc[0].dim.height;
    int output_w = nnctrl_ctx.net.net_out.out_desc[0].dim.width;
    int output_p = nnctrl_ctx.net.net_out.out_desc[0].dim.pitch / 4;

    std::cout << "output size: " << "--output_c: " << output_c << "--output_h: " << output_h << "--output_w: " \
                                  << output_w << "--output_p: " << output_p << "--" << std::endl;

    for (int h = 0; h < output_h; h++)
    {
        memcpy(textnetOutput + h * output_w, tempOutput[0] + h * output_p, output_w * sizeof(float));
    }
    // std::ofstream ouF;
    // ouF.open("./score.bin", std::ofstream::binary);
    // ouF.write(reinterpret_cast<const char*>(textnetOutput), sizeof(float) * maxTextLength * classNumber);
    // ouF.close();
    for (int row = 0; row < maxTextLength; row++) {
        float* output = textnetOutput + row * classNumber;
        softmax(classNumber, output);
        max_score = this->threshold;
        max_index = 0;
        for (int col = 0; col < classNumber; col++) {
            if (output[col] > max_score) {
                max_score = output[col];
                max_index = col;
            }
        }
        if((max_index > 0) && !(row > 0 && max_index == pre_max_index))
        {
            if(this->charCount > 0 && this->charType.at(count) == 'A')
            {
                if(max_index <= 26)
                {
                    result += characterSet[max_index];
                    count++;
                }
                else
                {   
                    int second_max_score = 0;
                    int second_max_index = 0;
                    for (int col = 0; col < classNumber; col++) {
                        if (output[col] > second_max_score && col != max_index) {
                            second_max_score = output[col];
                            second_max_index = col;
                        }
                    }
                    if(second_max_index <= 26)
                    {
                        result += characterSet[second_max_index];
                        count++;
                    }
                    else if(row > 0)
                    {
                        result += ' ';
                        count++;
                    }
                }
            }
            else if(this->charCount > 0 && this->charType.at(count) == 'B')
            {
                if(max_index > 26)
                {
                    result += characterSet[max_index];
                    count++;
                }
                else
                {
                    int second_max_score = 0;
                    int second_max_index = 0;
                    for (int col = 0; col < classNumber; col++) {
                        if (output[col] > second_max_score && col != max_index) {
                            second_max_score = output[col];
                            second_max_index = col;
                        }
                    }
                    if(second_max_index > 26)
                    {
                        result += characterSet[second_max_index];
                        count++;
                    }
                    else if(row > 0)
                    {
                        result += ' ';
                        count++;
                    }
                }
            }
            else
            {
                result += characterSet[max_index];
                count++;
            }
            // std::cout << max_score << std::endl;
            if(this->charCount > 0 && count >= this->charCount)
            {
                break;
            }
        }
        pre_max_index = max_index;
    }
    return result;
}