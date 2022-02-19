#include <iostream>
// #include <time.h>
#include <json/json.h>
#include <record_stream.h>
#include <deepsort.h>
#include <denetv2.h>
#include <sde_tracker.h>
#include <data_struct.h>
#include <calculate_trajectory.h>

#define CLASS_NUMBER (3)
#define ONE_MINUTE_TO_MISECOND (60000)

static sde_track_ctx_t track_ctx;


static void sig_stop(int a)
{
	(void)a;
	track_ctx.sig_flag = 1;
    track_ctx.run_write_video_file = 0;
}

static void *run_image_pthread(void *thread_params)
{
    int rval = 0;
    // int run_write_video_file = 1;
    int run_write_video_file = *(int *)thread_params;

    RecordStream record_stream;
    if (record_stream.init_data() < 0)
	{
		fprintf(stderr, "data initiation failed!\n");
		rval = -1;
	}

    printf("run_write_video_file: %d", run_write_video_file);
    rval = record_stream.capture_encoded_video(run_write_video_file);
    if (rval != 0)
    {
        fprintf(stderr, "capture encoded video failed: %s\n", strerror(rval));
        rval = -1;
    }

    record_stream.deinit_data();

	return NULL;
}

static void save_json_result(struct timeval &pre, std::map<int, TrajectoryParams> track_idx_map)
{
    struct timeval curr;
    char time_str[64];
    std::stringstream save_path;
    save_path.str("");

    gettimeofday(&curr, NULL);

    //根节点  
    Json::StyledWriter sw;
	Json::Value root;

    for (std::map<int, TrajectoryParams>::iterator it = track_idx_map.begin(); it != track_idx_map.end(); ++it)
    {
        Json::Value targets;
        targets["record_id"] = Json::Value((int)(pre.tv_sec * 1000));
        targets["target_id"] = Json::Value(it->first);
        targets["target_type"] = Json::Value(1);
        targets["snap_time"] = Json::Value((int)(curr.tv_sec * 1000));
        targets["target_speed"] = Json::Value(0.0);
        targets["move_direction"] = Json::Value(-1);
        targets["distance"] = Json::Value(0.0);
        if (it->second.trajectory_position.size() > 0)
        {
            targets["track_point"]["x"] = Json::Value(it->second.trajectory_position.back().x);
            targets["track_point"]["y"] = Json::Value(it->second.trajectory_position.back().y);
        }
        targets["target_rect"]["left_top_x"] = Json::Value(it->second.pedestrian_x_start.back());
        targets["target_rect"]["left_top_y"] = Json::Value(it->second.pedestrian_y_start.back());
        targets["target_rect"]["right_btm_x"] = Json::Value(it->second.pedestrian_x_end.back());
        targets["target_rect"]["right_btm_y"] = Json::Value(it->second.pedestrian_y_end.back());
        root["event_type"]["target_list"].append(targets);
    }
 
    // save json file
	if (((curr.tv_sec - pre.tv_sec) * 1000 + (curr.tv_usec - pre.tv_usec) / 1000) >= ONE_MINUTE_TO_MISECOND)
    {
        pre.tv_sec = curr.tv_sec;
		pre.tv_usec = curr.tv_usec;

        strftime(time_str, sizeof(time_str)-1, "%Y_%m_%d_%H_%M_%S", localtime(&pre.tv_sec)); 
        save_path << "/data/" << time_str << ".json";

        //输出到文件  
        std::ofstream os;
        os.open(save_path.str(), std::ios::out | std::ios::app);
        if (!os.is_open())
            std::cout << "error：can not find or create the file which named \" demo.json\"." << std::endl;
        os << sw.write(root);
        os.close();
    }
 
	//缩进输出  
	// std::cout << "StyledWriter:" << std::endl;
	// std::cout << sw.write(root) << endl << std::endl;
}

int main(int argc, char** argv)
{
    int rval = 0;
    track_ctx.run_write_video_file = 1;
    track_ctx.canvas_id = 1;
    int run_track = 1;

    struct timeval pre;
    gettimeofday(&pre, NULL);

	const std::string detnet_model_path = "/data/detnet.bin";
	const std::vector<std::string> input_name = {"images"};
	const std::vector<std::string> output_name = {"326", "385", "444"};
	const char* class_name[CLASS_NUMBER] = {"car", "truck", "bus"};
    
    const std::string sort_model_path = "./deepsort.bin";
    std::string input_dir = "/sdcard/in";
    std::string output_dir = "/sdcard/out/";
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

    // detnet init
	DetNet denet_process;
	if(denet_process.init(detnet_model_path, input_name, output_name) < 0)
    {
		std::cout << "DetNet init fail!" << std::endl;
    }
    else
    {
        std::cout << "DetNet init success!" << std::endl;   
    }
    DeepSort* DS = new DeepSort(sort_model_path, 128, 256, 0);
    CalculateTraj calculate_traj;
	// Time measurement
	uint32_t loop_count = 0;

	int width = 0;
	int height = 0;

	memset(&data, 0, sizeof(data));

    // pthread_t capture_encoded_video_tid = 0;
    // rval = pthread_create(&capture_encoded_video_tid, NULL, run_image_pthread, &track_ctx.run_write_video_file);
	// if (capture_encoded_video_tid)
	// {
	// 	pthread_join(capture_encoded_video_tid, NULL);
	// }

    do {
        unsigned long start_time = get_current_time();
        RVAL_OK(ea_img_resource_hold_data(track_ctx.img_resource, &data));
        RVAL_ASSERT(data.tensor_group != NULL);
        RVAL_ASSERT(data.tensor_num >= 1);
        RVAL_ASSERT(data.tensor_group[0] != NULL);
        img_tensor = data.tensor_group[0];
        dsp_pts = data.dsp_pts;
        width = ea_tensor_shape(img_tensor)[3];
        height = ea_tensor_shape(img_tensor)[2];
        EA_LOG_NOTICE("[main] image width: %d, image height: %d\n", width, height);
        denet_process.run(img_tensor);
        std::cout << "[Yolov5 cost time: " << (get_current_time() - start_time) / 1000 << " ms]" << std::endl;

        unsigned long time_start_sort = get_current_time();
        cv::Mat input_src(cv::Size(width, height), CV_8UC3);
        tensor2mat(img_tensor, input_src, 3);
        DS->sort(input_src, denet_process.det_results);
        std::cout << "[deepsort cost time: " <<  (get_current_time() - time_start_sort) / 1000.0  << " ms]" << std::endl;

        unsigned long time_start_calculate_tracking_trajectory = get_current_time();
        calculate_traj.calculate_tracking_trajectory(denet_process.det_results, loop_count, input_src.rows);
        std::cout << "[calculate_tracking_trajectory cost time: " <<  (get_current_time() - time_start_calculate_tracking_trajectory) / 1000.0  << " ms]" << std::endl;

        amba_draw_detection(&track_ctx, denet_process.det_results, calculate_traj.track_idx_map, img_tensor, dsp_pts);
        RVAL_OK(ea_img_resource_drop_data(track_ctx.img_resource, &data));
        std::cout << "[Loop: "<< loop_count << ", all process cost time: " << (get_current_time() - start_time) / 1000 << " ms]" << std::endl;
        loop_count++;

        save_json_result(pre, calculate_traj.track_idx_map);

        if (track_ctx.sig_flag) {
            break;
        }
    } while(1);

    denet_process.deinit();
    amba_cv_env_deinit(&track_ctx);

    return rval;
}
