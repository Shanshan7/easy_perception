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

std::vector<std::vector<float>> applyNMS(std::vector<std::vector<float>>& boxes,
	                                    const float thres) 
{    
    std::vector<std::vector<float>> result;
    std::vector<bool> exist_box(boxes.size(), true);

    int n = 0;
    for (size_t _i = 0; _i < boxes.size(); ++_i) 
	{
        if (!exist_box[_i]) 
			continue;
        n = 0;
        for (size_t _j = _i + 1; _j < boxes.size(); ++_j)
		{
            // different class name
            if (!exist_box[_j] || boxes[_i][4] != boxes[_j][4]) 
				continue;
            float ovr = cal_iou(boxes[_j], boxes[_i]);
            if (ovr >= thres) 
            {
                if (boxes[_j][5] <= boxes[_i][5])
                {
                    exist_box[_j] = false;
                }
                else
                {
                    n++;   // have object_score bigger than boxes[_i]
                    exist_box[_i] = false;
                    break;
                }
            }
        }
        //if (n) exist_box[_i] = false;
		if (n == 0) 
		{
			result.push_back(boxes[_i]);
		}			
    }

    return result;
}

float sigmoid_x(float x)
{
	return static_cast<float>(1.f / (1.f + exp(-x)));
}

float overlap(float x1, float w1, float x2, float w2)
{
    float left = max(x1 - w1 / 2.0f, x2 - w2 / 2.0f);
    float right = min(x1 + w1 / 2.0f, x2 + w2 / 2.0f);
    return right - left;
}

float cal_iou(std::vector<float> box, std::vector<float>truth)
{
    float w = overlap(box[0], box[2], truth[0], truth[2]);
    float h = overlap(box[1], box[3], truth[1], truth[3]);
    if(w < 0 || h < 0) return 0;

    float inter_area = w * h;
    float union_area = box[2] * box[3] + truth[2] * truth[3] - inter_area;
    return inter_area * 1.0f / union_area;
}

unsigned long get_current_time(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec*1000000 + tv.tv_usec);
}