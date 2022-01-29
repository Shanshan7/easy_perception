#include <iostream>

#include "common/data_struct.h"
#include "yolov5rt/yolov5/detnet.h"
#include "yolov5rt/deepsort/deepsort.h"
#include "postprocess/calculate_trajectory.h"


#define CLASS_NUMBER (3)

int main(int argc, char** argv)
{
	const std::string model_path = "./denet.bin";
	const std::vector<std::string> input_name = {"images"};
	const std::vector<std::string> output_name = {"444", "385", "326"};
	const char* class_name[CLASS_NUMBER] = {"car", "truck", "bus"};
	// global_control_param_t *G_param = (global_control_param_t*)thread_params;
	int rval = 0;
	std::vector<std::vector<float>> boxes;
	// image related
    ea_img_resource_t *img_resource;
	ea_tensor_t *img_tensor = NULL;
	ea_img_resource_data_t data;
	uint32_t dsp_pts = 0;
	// bbox_list_t bbox_list = {0};
	cv::Mat src_image;
	DetNet denet_process;
    DeepSort* DS = new DeepSort(sort_path, 128, 256, 0);
	// Time measurement
	uint64_t start_time = 0;
	uint64_t debug_time = 0;
	float sum_time = 0.0f;
	uint32_t loop_count = 1;

	int width = 0;
	int height = 0;

	if(denet_process.init(model_path, input_name, output_name, CLASS_NUMBER) < 0)
    {
		std::cout << "DeNet init fail!" << std::endl;
    }
	memset(&data, 0, sizeof(data));

    do {
        // start_time = gettimeus();
    #if defined(OFFLINE_DATA)
        save_process.get_image(src_image);
        if(src_image.empty())
        {
            LOG(ERROR) << "DeNet get image fail!";
            continue;
        }
    #else
        // image_geter.get_image(src_image);
        // if(src_image.empty())
        // {
        // 	LOG(ERROR) << "DeNet get image fail!";
        // 	continue;
        // }
        
        RVAL_OK(ea_img_resource_hold_data(img_resource, &data));
        RVAL_ASSERT(data.tensor_group != NULL);
        RVAL_ASSERT(data.tensor_num >= 1);
        img_tensor = data.tensor_group[0];
        dsp_pts = data.dsp_pts;
        width = ea_tensor_shape(img_tensor)[3];
        height = ea_tensor_shape(img_tensor)[2];
        // RVAL_OK(ea_tensor_to_jpeg(img_tensor, EA_TENSOR_COLOR_MODE_YUV_NV12, "image.jpg"));
    #endif
        denet_process.run(img_tensor);
        RVAL_OK(ea_img_resource_drop_data(img_resource, &data));
        // sum_time += (gettimeus() - start_time);
        ++loop_count;
        // if (loop_count == TIME_MEASURE_LOOPS) {
        //     // LOG(WARNING) << "Car det average time [per " << TIME_MEASURE_LOOPS << " loops]:" << sum_time / (1000 * TIME_MEASURE_LOOPS) << "ms";
        //     sum_time = 0;
        //     loop_count = 1;
        // }

        DS->sort(src_image, denet_process.det_results);
        // time_end_sort = get_current_time();
        // LOG(INFO) << "[deepsort] deepsort cost time: " <<  (time_end_sort - time_start_sort)/1000.0  << "ms";
        // LOG(INFO) << "[deepsort] Deepsort process Done!!!";
        // showDetection(frame, det_results);
        // frame_id = index;
        calculate_tracking_trajectory(denet_process.det_results, src_image, src_image.rows, loop_count);
    } while(0);

    return rval;
}
