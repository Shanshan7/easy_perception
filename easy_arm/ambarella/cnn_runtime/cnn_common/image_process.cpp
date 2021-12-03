#include "image_process.h"
#include <iostream>
#include <math.h>
#include "cnn_runtime/cnn_common/blob_define.h"

void get_square_size(const cv::Size src_size, const cv::Size dst_size, float &ratio, cv::Size &pad_size)
{
    const int src_width = src_size.width;
    const int src_height = src_size.height;
    const int dst_width = dst_size.width;
    const int dst_height = dst_size.height;
    ratio = std::min(static_cast<float>(dst_width) / src_width, \
                     static_cast<float>(dst_height) / src_height);
    const int new_width = static_cast<int>(src_width * ratio);
    const int new_height = static_cast<int>(src_height * ratio);
    const int pad_width = dst_width - new_width;
    const int pad_height = dst_height - new_height;
    pad_size.width = pad_width;
    pad_size.height = pad_height;
}

void image_resize_square(const cv::Mat &src, const cv::Size dst_size, cv::Mat &dst_image)
{
    const int src_width = src.cols;
    const int src_height = src.rows;
    float ratio;
    cv::Size pad_size;
    get_square_size(cv::Size(src_width, src_height), dst_size, ratio, pad_size);
    int new_width = static_cast<int>(src_width * ratio);
    int new_height = static_cast<int>(src_height * ratio);
    const int top = pad_size.height / 2;
    const int bottom = pad_size.height - (pad_size.height / 2);
    const int left = pad_size.width / 2;
    const int right = pad_size.width - (pad_size.width / 2);
    cv::Mat resize_mat;
    cv::resize(src, resize_mat, cv::Size(new_width, new_height), 0, 0, cv::INTER_LINEAR);
    cv::copyMakeBorder(resize_mat, dst_image, top, bottom, left, right, cv::INTER_LINEAR, cv::Scalar(0, 0, 0));
}

void resize_with_specific_height(const cv::Mat &src, const cv::Size dst_size, cv::Mat &dst_image)
{
    const int src_width = src.cols;
    const int src_height = src.rows;
    float ratio = src_width / float(src_height);
    int resize_w = ceil(ratio * dst_size.height);
    if(resize_w >= dst_size.width){
        resize_w = dst_size.width;
        cv::resize(src, dst_image, cv::Size(resize_w, dst_size.height), 0, 0, cv::INTER_LINEAR);
    }
    else
    {
        cv::Mat temp_mat;
        dst_image = cv::Mat::zeros(dst_size.height, dst_size.width, CV_8UC3);
        cv::resize(src, temp_mat, cv::Size(resize_w, dst_size.height), 0, 0, cv::INTER_LINEAR);
        temp_mat.copyTo(dst_image(cv::Rect(0, 0, resize_w, dst_size.height)));
        // cv::imwrite("li.png", dst_image);
    } 
}

cv::Size get_input_size(nnctrl_ctx_t *nnctrl_ctx)
{
    // int channel = nnctrl_ctx->net.net_in.in_desc[0].dim.depth;
    int height = nnctrl_ctx->net.net_in.in_desc[0].dim.height;
    int width = nnctrl_ctx->net.net_in.in_desc[0].dim.width;
    // std::cout << "input --height: " << height << "--width: " << width << std::endl;
    cv::Size dst_size(width, height);
    return dst_size;
}

cv::Size get_output_size(nnctrl_ctx_t *nnctrl_ctx)
{
    // int channel = nnctrl_ctx->net.net_in.in_desc[0].dim.depth;
    int output_h = nnctrl_ctx->net.net_out.out_desc[0].dim.height;
    int output_w = nnctrl_ctx->net.net_out.out_desc[0].dim.width;
    cv::Size dst_size(output_w, output_h);
    return dst_size;
}

int get_output_channel(nnctrl_ctx_t *nnctrl_ctx)
{
    int channel = nnctrl_ctx->net.net_out.out_desc[0].dim.depth;
    return channel;
}

int get_input_pitch(nnctrl_ctx_t *nnctrl_ctx)
{
    int pitch = nnctrl_ctx->net.net_in.in_desc[0].dim.pitch;
    // std::cout << "input --pitch: " << pitch << "--" << std::endl;
    return pitch;
}

int get_output_pitch(nnctrl_ctx_t *nnctrl_ctx)
{
    int width = nnctrl_ctx->net.net_in.in_desc[0].dim.width;
    int pitch = LAYER_P(width);
    // std::cout << "output --pitch: " << pitch << "--" << std::endl;
    return pitch;
}

void preprocess(nnctrl_ctx_t *nnctrl_ctx, const cv::Mat &src_mat, const int resize_type)
{
    if(src_mat.empty()){
        return;
    }
    cv::Size dst_size = get_input_size(nnctrl_ctx);
    int width = nnctrl_ctx->net.net_in.in_desc[0].dim.width;
    int pitch = get_input_pitch(nnctrl_ctx) / 1;
    cv::Mat dst_mat;
    std::vector<cv::Mat> channel_s;
    if(resize_type == 0){
        cv::resize(src_mat, dst_mat, dst_size, 0, 0, cv::INTER_LINEAR);
    }
    else if(resize_type == 1){
        image_resize_square(src_mat, dst_size, dst_mat);
    }
    else if(resize_type == 2){
        resize_with_specific_height(src_mat, dst_size, dst_mat);
    }
    
    cv::split(dst_mat, channel_s);

    for (int i=0; i<3; i++)
    {
        for (int h=0; h< dst_size.height; h++)
        {
            memcpy(nnctrl_ctx->net.net_in.in_desc[0].virt + i * dst_size.height * pitch + h * pitch, 
                   channel_s[2-i].data + h * dst_size.width, dst_size.width);
        }
    }

    // sycn address
    cavalry_mem_sync_cache(nnctrl_ctx->net.net_in.in_desc[0].size, nnctrl_ctx->net.net_in.in_desc[0].addr, 1, 0);
}