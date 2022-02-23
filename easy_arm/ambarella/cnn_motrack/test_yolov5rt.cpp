#include <iostream>
#include <json/json.h>
#include <record_stream.h>
#include <deepsort.h>
#include <denetv2.h>
#include <sde_tracker.h>
#include <data_struct.h>
#include <calculate_trajectory.h>
#include <network_process.h>

#define CLASS_NUMBER (3)
#define ONE_MINUTE_TO_MISECOND (60000)

static sde_track_ctx_t track_ctx;
static NetWorkProcess network_process;
static pthread_rwlock_t rwlock = PTHREAD_RWLOCK_INITIALIZER;

int run_result_process = 1;
int run_yolov5_deepsort = 1;
int run_write_video_file = 1;
int run_receive_message = 1;
int run_result_process_flag = 0;
int run_json_save_flag = 0;

static void sig_stop(int a)
{
	(void)a;
	track_ctx.sig_flag = 1;
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
            std::cout << "[error: can not find or create the file which named \" ***.json\"]." << std::endl;
        os << sw.write(root);
        os.close();
    }
 
	//缩进输出  
	// std::cout << "StyledWriter:" << std::endl;
	// std::cout << sw.write(root) << endl << std::endl;
}

static void *run_image_pthread(void *thread_params)
{
    int rval = 0;
    u64 total_frames = 0;
	u64 total_bytes = 0;

    RecordStream record_stream;
    if (record_stream.init_data() < 0)
	{
		fprintf(stderr, "data initiation failed!\n");
		rval = -1;
	}

	while (run_write_video_file)
	{
		if ((rval = record_stream.write_stream(&total_frames, &total_bytes)) < 0)
		{
			if (rval == -1)
			{
				usleep(100 * 1000);
				record_stream.show_waiting();
			}
			else
			{
				fprintf(stderr, "write_stream err code %d \n", rval);
				break;
			}
			continue;
		}
		if (md5_idr_number == 0)
		{
			md5_idr_number = -1;
			/// statistics_run = 0;
			break;
		}
	}

    record_stream.deinit_data();

	return NULL;
}


static void* result_process_thread(void* argv)
{
    int rval = 0;
    struct timeval pre;
    gettimeofday(&pre, NULL);

    while (run_result_process)
    {
        if (run_result_process_flag < 1)
        {
            continue;
        }
        else
        {
            unsigned long start_time_draw = get_current_time();
            pthread_rwlock_rdlock(&rwlock);
            amba_draw_detection(&track_ctx);
            if (run_json_save_flag == 1)
            {
                save_json_result(pre, track_ctx.track_idx_map);
            }
            pthread_rwlock_unlock(&rwlock);
            run_result_process_flag = 0;
            std::cout << "[Result process cost time: " << (get_current_time() - start_time_draw) / 1000 << " ms]" << std::endl;
        }
    }
    
    return NULL;
}

static void* yolov5_deepsort_thread(void* argv)
{
    int rval = 0;

    // model related
	const std::string detnet_model_path = "/data/detnet.bin";
	const std::vector<std::string> input_name = {"images"};
	// const std::vector<std::string> output_name = {"326", "385", "444"};
    const std::vector<std::string> output_name = {"804", "863", "922"};
	const char* class_name[CLASS_NUMBER] = {"car", "truck", "bus"};
    const std::string sort_model_path = "./deepsort.bin";

	// image related
	ea_tensor_t *img_tensor = NULL;

    // detnet, track, traj initial
	DetNet denet_process;
    if (denet_process.init(detnet_model_path, input_name, output_name) < 0)
    {
		std::cout << "DetNet init fail!" << std::endl;
    }
    DeepSort* DS = new DeepSort(sort_model_path, 128, 256, 0);
    CalculateTraj calculate_traj;

    while (run_yolov5_deepsort)
    {
        unsigned long start_time = get_current_time();
        RVAL_OK(ea_img_resource_hold_data(track_ctx.img_resource, &track_ctx.image_data));
        RVAL_ASSERT(track_ctx.image_data.tensor_group != NULL);
        RVAL_ASSERT(track_ctx.image_data.tensor_num >= 1);
        RVAL_ASSERT(track_ctx.image_data.tensor_group[0] != NULL);
        img_tensor = track_ctx.image_data.tensor_group[0];
        int width = ea_tensor_shape(img_tensor)[3];
        int height = ea_tensor_shape(img_tensor)[2];
        std::cout << "[main] image width: " << width << ", image height: " << height << std::endl;

        denet_process.run(img_tensor);
        std::cout << "[Yolov5 cost time: " << (get_current_time() - start_time) / 1000 << " ms]" << std::endl;

        unsigned long time_start_sort = get_current_time();
        cv::Mat input_src(cv::Size(width, height), CV_8UC3);
        // tensor2mat(img_tensor, input_src, 3);
        DS->sort(input_src, denet_process.det_results);
        std::cout << "[deepsort cost time: " <<  (get_current_time() - time_start_sort) / 1000.0  << " ms]" << std::endl;

        unsigned long time_start_calculate_tracking_trajectory = get_current_time();
        calculate_traj.calculate_tracking_trajectory(denet_process.det_results, track_ctx.loop_count, input_src.rows);
        pthread_rwlock_wrlock(&rwlock);
        track_ctx.image_width = width;
        track_ctx.image_height = height;
        track_ctx.track_idx_map = calculate_traj.track_idx_map;
        pthread_rwlock_unlock(&rwlock);
        std::cout << "[calculate_tracking_trajectory cost time: " <<  (get_current_time() - time_start_calculate_tracking_trajectory) / 1000.0  << " ms]" << std::endl;

        std::cout << "[Loop: "<< track_ctx.loop_count << ", all process cost time: " << (get_current_time() - start_time) / 1000 << " ms]" << std::endl;
        RVAL_OK(ea_img_resource_drop_data(track_ctx.img_resource, &track_ctx.image_data));
        run_result_process_flag = 1;
        track_ctx.loop_count++;
    }
    denet_process.deinit();
    
    return NULL;
}

static void* receive_message_thread(void* argv)
{
    int rval = 0;

    while (run_receive_message)
    {
        rval = network_process.receive_message();
        if (network_process.receive_code == 0)
        {
            run_json_save_flag = 0;
        }
        else
        {
            run_json_save_flag = 1;
        }
    }
    
    return NULL;
}


int main(int argc, char** argv)
{
    int rval = 0;

    track_ctx.canvas_id = 1;
    track_ctx.loop_count = 0;
    track_ctx.track_idx_map.clear();

    memset(&track_ctx.image_data, 0, sizeof(track_ctx.image_data));

    if(network_process.init_network() < 0)
	{
        std::cout << "[Network] process initial fail!" << std::endl;
		return -1;
	}

    // pthread_t capture_encoded_video_tid = 0;
    pthread_t result_process_thread_pid = 1;
    pthread_t yolov5_deepsort_thread_pid = 2;
    pthread_t receive_message_thread_pid = 3;

    // process signals to control program operation
    signal(SIGINT, sig_stop);
	signal(SIGQUIT, sig_stop);
	signal(SIGTERM, sig_stop);

    amba_cv_env_init(&track_ctx);

    // pthread_create(&capture_encoded_video_tid, NULL, run_image_pthread, NULL);
    pthread_create(&result_process_thread_pid, NULL, result_process_thread, NULL);
    pthread_create(&yolov5_deepsort_thread_pid, NULL, yolov5_deepsort_thread, NULL);
    pthread_create(&receive_message_thread_pid, NULL, receive_message_thread, NULL);
 
    // pthread_join(capture_encoded_video_tid, NULL);
    pthread_join(result_process_thread_pid, NULL);
    pthread_join(yolov5_deepsort_thread_pid, NULL);
    pthread_join(receive_message_thread_pid, NULL);
    
    // network_process.stop();
    amba_cv_env_deinit(&track_ctx);

    return rval;
}
