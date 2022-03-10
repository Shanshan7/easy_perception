#include <iostream>

// #include "yolov5rt/yolov5/detnet.h"
#include "yolov5rt/deepsort/deepsort.h"
#include "yolov5rt/yolov5/denetv2.h"
#include "yolov5rt/sde_tracker.h"
#include "common/data_struct.h"
#include "postprocess/calculate_trajectory.h"

#define CLASS_NUMBER (3)

static sde_track_ctx_t track_ctx;

static void sig_stop(int a)
{
	(void)a;
	track_ctx.sig_flag = 1;
}

int main(int argc, char** argv)
{
    int rval = 0;

    // model related
	const std::string detnet_model_path = "/data/detnet.bin";
	const std::vector<std::string> input_name = {"images"};
	const std::vector<std::string> output_name = {"326", "385", "444"};
    // const std::vector<std::string> output_name = {"804", "863", "922"};
	const char* class_name[CLASS_NUMBER] = {"car", "truck", "bus"};
    const std::string sort_model_path = "./deepsort.bin";

	// image related
	ea_tensor_t *img_tensor = NULL;

    amba_cv_env_init(&track_ctx);
    // detnet, track, traj initial
	DetNet denet_process;
    if (denet_process.init(detnet_model_path, input_name, output_name) < 0)
    {
		std::cout << "[Yolov5rt] DetNet init fail!" << std::endl;
    }
    DeepSort* DS = new DeepSort(sort_model_path, 128, 256, 0);
    CalculateTraj calculate_traj;

    // process signals to control program operation
    signal(SIGINT, sig_stop);
	signal(SIGQUIT, sig_stop);
	signal(SIGTERM, sig_stop);

    do {
        unsigned long start_time = get_current_time();
        RVAL_OK(ea_img_resource_hold_data(track_ctx.img_resource, &track_ctx.image_data));
        RVAL_ASSERT(track_ctx.image_data.tensor_group != NULL);
        RVAL_ASSERT(track_ctx.image_data.tensor_num >= 1);
        RVAL_ASSERT(track_ctx.image_data.tensor_group[0] != NULL);
        img_tensor = track_ctx.image_data.tensor_group[0];
        int width = ea_tensor_shape(img_tensor)[3];
        int height = ea_tensor_shape(img_tensor)[2];
        denet_process.run(img_tensor);
        std::cout << "[Yolov5 cost time " << (get_current_time() - start_time) / 1000 << " ms]" << std::endl;

        unsigned long time_start_sort = get_current_time();
        cv::Mat input_src(cv::Size(width, height), CV_8UC3);
        // tensor2mat(img_tensor, input_src, 3);
        DS->sort(input_src, denet_process.det_results);
        std::cout << "[deepsort cost time: " <<  (get_current_time() - time_start_sort) / 1000.0  << " ms]" << std::endl;
        ++loop_count;

        unsigned long time_start_calculate_tracking_trajectory = get_current_time();
        calculate_traj.calculate_trajectory(denet_process.det_results, track_ctx.loop_count, height);
        std::cout << "[calculate_tracking_trajectory cost time: " <<  (get_current_time() - time_start_calculate_tracking_trajectory) / 1000.0  << " ms]" << std::endl;
        amba_draw_detection(&track_ctx);
        RVAL_OK(ea_img_resource_drop_data(track_ctx.img_resource, &data));
        if (track_ctx.sig_flag) {
            break;
        }
    } while(1);

    denet_process.deinit();
    amba_cv_env_deinit(&track_ctx);

    return rval;
}