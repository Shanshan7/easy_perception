#include "utils.h"


// tensor: size is n,c,h,w
// mat: size is h, w, c
void tensor2mat(ea_tensor_t *input_tensor, cv::Mat output_mat, int channel_convert) {
    int batch_n = ea_tensor_shape(input_tensor)[0];
    int input_channel = ea_tensor_shape(input_tensor)[1];
    int input_height = ea_tensor_shape(input_tensor)[2];
    int input_width = ea_tensor_shape(input_tensor)[3];
    int input_pitch = ea_tensor_pitch(input_tensor);
    LOG(INFO) << ("Tensor shape: %d, %d, %d, %d, %d\n", batch_n, input_channel, input_height, input_width, input_pitch);

    // sync vp to cpu
    uint8_t *c1_data = NULL;
    ea_tensor_sync_cache(input_tensor, EA_VP, EA_CPU);
    c1_data = (uint8_t *)ea_tensor_data(input_tensor);

	// void *c1_data = c_data ;
	void *c2_data = c1_data + ea_tensor_shape(input_tensor)[2] * ea_tensor_pitch(input_tensor);
	void *c3_data = c1_data + ea_tensor_shape(input_tensor)[2] * ea_tensor_pitch(input_tensor) * 2;

    cv::Mat c1(input_height, input_width, CV_8UC1, c1_data, ea_tensor_pitch(input_tensor));
	cv::Mat c2(input_height, input_width, CV_8UC1, c2_data, ea_tensor_pitch(input_tensor));
	cv::Mat c3(input_height, input_width, CV_8UC1, c3_data, ea_tensor_pitch(input_tensor));
    std::vector<cv::Mat> channels;

    if (input_channel == 1) {
        channels.push_back(c1);
    }
    else if (input_channel == 3)
    {
        channels.push_back(c1);
        channels.push_back(c2);
        channels.push_back(c3);
    }
    else {
        LOG(ERROR) << ("channel number %lu is not 1 or 3 for saving to jpeg\n", input_channel);
        // rval = -1;
        // break;
    }

    cv::merge(channels, output_mat);
}

// void mat2tensor() {

// }

unsigned long get_current_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec*1000000 + tv.tv_usec);
}