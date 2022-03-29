#ifndef _UTILS_H_
#define _UTILS_H_

#include "data_struct.h"

#define TIME_DECLARE() \
	static unsigned long ea_mt_start

#define TIME_START() ea_mt_start = get_current_time()
#define TIME_END(s) \
	LOG(INFO) << s << " " << (get_current_time() - ea_mt_start) / 1000 << " ms";
#define TIME_END_AVG(s, loop) \
    LOG(INFO) << s << " " << (get_current_time() - ea_mt_start) / 1000 / loop << " ms";

#define SAVE_LOG_PROCESS(rval, s) \
    LOG_IF(ERROR, rval < 0) << s << " failed, return " << rval;

// void tensor2mat(ea_tensor_t *input_tensor, cv::Mat output_mat, int channel_convert);
// void mat2tensor(cv::Mat input_mat, ea_tensor_t *output_tensor);
std::vector<std::vector<float>> applyNMS(std::vector<std::vector<float>>& boxes,
	                                    const float thres);
float overlap(float x1, float w1, float x2, float w2);
float cal_iou(std::vector<float> box, std::vector<float>truth);
float sigmoid_x(float x);
std::vector<std::vector<float>> martrix_multiply_num(std::vector<std::vector<float>> matrix, float num);
void array_reverse(float* array, int array_length);

unsigned long get_current_time(void);
bool copy_file(std::string src, std::string dest);
void conv1d(float *input, float *kernel, int input_length, int kernel_size, float *output);
void conv1d(std::vector<float> &input, float *kernel, int input_length, int kernel_size, float *output);

#endif // _UTILS_H_