#include <iostream>

// #include "yolov5rt/yolov5/detnet.h"
#include "yolov5rt/deepsort/deepsort.h"
#include "yolov5rt/yolov5/denetv2.h"
#include "yolov5rt/sde_tracker.h"
#include "common/data_struct.h"
#include "postprocess/calculate_trajectory.h"

#define CLASS_NUMBER (1)

static sde_track_ctx_t track_ctx;

static void sig_stop(int a)
{
	(void)a;
	track_ctx.sig_flag = 1;
}

int main(int argc, char** argv)
{
    int rval = 0;
	// const std::string model_path = "./detnet.bin";
	// const std::vector<std::string> input_name = {"images"};
	// const std::vector<std::string> output_name = {"611", "670", "729"};
	const std::string model_path = "./denet.bin";
	const std::vector<std::string> input_name = {"images"};
	const std::vector<std::string> output_name = {"326", "385", "444"};
	// const std::string model_path = "./onnx_yolov5s_cavalry.bin";
	// const std::vector<std::string> input_name = {"images"};
	// const std::vector<std::string> output_name = {"1017", "1037", "997"};
	// const std::string model_path = "./onnx_yolov5plate_cavalry.bin";
	// const std::vector<std::string> input_name = {"data"};
	// const std::vector<std::string> output_name = {"949", "969", "989"};
	const char* class_name[CLASS_NUMBER] = {"car"}; // {"car", "truck", "bus"};
    std::string input_dir = "./in";
    std::string output_dir = "./out/";
    snprintf(track_ctx.input_dir, sizeof(track_ctx.input_dir), "%s", \
            input_dir.c_str());
    snprintf(track_ctx.output_dir, sizeof(track_ctx.output_dir), "%s", \
            output_dir.c_str());

    std::cout << "input_dir: " << input_dir.c_str() << " " \
              << "output_dir: " << output_dir.c_str() << std::endl;

    // process signals to control program operation
    signal(SIGINT, sig_stop);
	signal(SIGQUIT, sig_stop);
	signal(SIGTERM, sig_stop);

    amba_cv_env_init(&track_ctx);
	// image related
	ea_tensor_t *img_tensor = NULL;
	ea_img_resource_data_t data;
	uint32_t dsp_pts = 0;
	DetNet denet_process;
    DeepSort* DS = new DeepSort("./yolov5rt/deepsort/deepsort.onnx", 128, 256, 0);
    CalculateTraj calculate_traj;
	// Time measurement
	uint32_t loop_count = 1;

	int width = 0;
	int height = 0;

	if(denet_process.init(model_path, input_name, output_name, CLASS_NUMBER) < 0)
    {
		std::cout << "DetNet init fail!" << std::endl;
    }
    else
    {
        std::cout << "DetNet init success!" << std::endl;   
    }

	memset(&data, 0, sizeof(data));

    do {
        unsigned long start_time = get_current_time();
        RVAL_OK(ea_img_resource_hold_data(track_ctx.img_resource, &data));
        RVAL_ASSERT(data.tensor_group[0] != NULL);
        img_tensor = data.tensor_group[0];
        dsp_pts = data.dsp_pts;
        width = ea_tensor_shape(img_tensor)[3];
        height = ea_tensor_shape(img_tensor)[2];
        std::cout << "width: " << width << " height: " << height << std::endl;
        denet_process.run(img_tensor);
        
        // ea_display_refresh(track_ctx.display, (void *)data.tensor_group[0]);
        std::cout << "[Yolov5 cost time " << (get_current_time() - start_time) / 1000 << " ms]" << std::endl;

        unsigned long time_start_sort = get_current_time();
        cv::Mat input_src(cv::Size(width, height), CV_8UC3);
        tensor2mat(img_tensor, input_src, 3);
        DS->sort(input_src, denet_process.det_results);
        std::cout << "[deepsort cost time: " <<  (get_current_time() - time_start_sort) / 1000.0  << " ms]" << std::endl;
        // LOG(INFO) << "[deepsort] Deepsort process Done!!!";
        // showDetection(frame, det_results);
        // frame_id = index;
        ++loop_count;

        unsigned long time_start_calculate_tracking_trajectory = get_current_time();
        calculate_traj.calculate_tracking_trajectory(denet_process.det_results, input_src.rows, loop_count);
        std::cout << "[calculate_tracking_trajectory cost time: " <<  (get_current_time() - time_start_calculate_tracking_trajectory) / 1000.0  << " ms]" << std::endl;
        amba_draw_detection(&track_ctx, denet_process.det_results, calculate_traj.track_idx_map, img_tensor);
        RVAL_OK(ea_img_resource_drop_data(track_ctx.img_resource, &data));
        // if (loop_count == TIME_MEASURE_LOOPS) {
        //     // LOG(WARNING) << "Car det average time [per " << TIME_MEASURE_LOOPS << " loops]:" << sum_time / (1000 * TIME_MEASURE_LOOPS) << "ms";
        //     sum_time = 0;
        //     loop_count = 1;
        // }
        if (track_ctx.sig_flag) {
            break;
        }
    } while(1);

    amba_cv_env_deinit(&track_ctx);

    return rval;
}
