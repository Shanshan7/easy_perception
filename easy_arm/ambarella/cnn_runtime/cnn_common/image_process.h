#ifndef IMAGEPROCESS_H
#define IMAGEPROCESS_H

#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "cnn_runtime/cnn_common/cnn_data_structure.h"

void get_square_size(const cv::Size src_size, const cv::Size dst_size, float &ratio, cv::Size &pad_size);

void image_resize_square(const cv::Mat &src, cv::Size dst_size, cv::Mat &dst_image);

void resize_with_specific_height(const cv::Mat &src, const cv::Size dst_size, cv::Mat &dst_image);

cv::Size get_input_size(nnctrl_ctx_t *nnctrl_ctx);
cv::Size get_output_size(nnctrl_ctx_t *nnctrl_ctx);

int get_input_pitch(nnctrl_ctx_t *nnctrl_ctx);
int get_output_pitch(nnctrl_ctx_t *nnctrl_ctx);
int get_output_channel(nnctrl_ctx_t *nnctrl_ctx);

void preprocess(nnctrl_ctx_t *nnctrl_ctx, const cv::Mat &src_mat, const int resize_type);

#endif //IMAGEPROCESS_H