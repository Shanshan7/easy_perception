#include <iostream>
#include <unistd.h>

#include "fairmot/tracker.h"
#include "common/data_struct.h"
#include "common/utils.h"
#include "network/network_transmission.h"


static track_params_t track_params;
static track_ctx_t track_ctx;
static NetWorkTransmission network_transmission;

int run_flag = 1;

static void *run_send_message_pthread(void *thread_send_message_params)
{
	int rval = 0;
	prctl(PR_SET_NAME, "run_send_message_pthread");

	GlobalControlParam *global_param_ = (GlobalControlParam*)thread_send_message_params;
	// std::map<int, TrajectoryParams> track_result_map = global_param_->track_idx_map;

	while(run_flag)
	{
		char send_data[100];
		std::stringstream send_data_;

		for (std::map<int, TrajectoryParams>::iterator it = global_param_->track_idx_map.begin(); \
		                                               it != global_param_->track_idx_map.end(); ++it) 
		{
			send_data_.str("");
			send_data_ << global_param_->current_frame << "|" \
				<< it->first << "|" \
				<< it->second.npedestrian_direction << "|" \
				<< it->second.mean_velocity << "\n";
			snprintf(send_data, sizeof(send_data), "%s", \
					send_data_.str().c_str());
		}

		rval = network_transmission.send_data(send_data, sizeof(send_data));
		if (rval < 0)
		{
			run_flag = 0;
			LOG(ERROR) << "[net_trans] Network Transmission send data failed!";
		}
	}

	LOG(WARNING) << "process_recv_pthread quit！";
	return NULL;
}

static int start_process(track_ctx_t *track_ctx, GlobalControlParam *global_param)
{
	int rval = 0;

#ifdef IS_SEND_DATA
	pthread_t send_message_pthread_id = 0;
	pthread_create(&send_message_pthread_id, NULL, run_send_message_pthread, (void*)&global_param);
#endif

	while (global_param->run_flag)
	{
		amba_track_run(track_ctx, global_param->track_idx_map);
		global_param->current_frame = track_ctx->mot_result.frame_id;
		std::cout << global_param->current_frame << std::endl;
	}
	track_ctx->det_stop_flag = 1;
    fairmot_vp_forward_signal(&track_ctx->fairmot);
    pthread_join(track_ctx->det_tidp, NULL);

#ifdef IS_SEND_DATA
	pthread_join(send_message_pthread_id, NULL);
	LOG(WARNING) << "send_message_pthread pthread release";
#endif

	LOG(INFO) << "Track stops after " << track_ctx->loop_count << " loops";
	return rval;
}

static void sig_stop(int a)
{
	(void)a;
	track_ctx.sig_flag = 1;
}

int main(int argc, char **argv) {
    int rval = 0;

	// initialize log file
	std::string log_dir = "/sdcard/log/";
	std::string command_log = "mkdir -p " + log_dir;
    system(command_log.c_str());
    google::InitGoogleLogging(argv[0]);
    google::SetLogDestination(google::INFO, log_dir.c_str());
    // google::InstallFailureSignalHandler();
	// google::InstallFailureWriter(&SignalHandle); 

    // process signals to control program operation
    signal(SIGINT, sig_stop);
	signal(SIGQUIT, sig_stop);
	signal(SIGTERM, sig_stop);

    memset(&track_ctx, 0, sizeof(track_ctx_t));

	// create struct of global control param
	GlobalControlParam global_param;
	memset(&global_param, 0, sizeof(GlobalControlParam));

#ifdef IS_SEND_DATA
	rval = network_transmission.socket_init();
	if (rval < 0)
	{
		run_flag = 0;
		LOG(ERROR) << "[net_trans] Network Transmission initial failed!";
		return -1;
	}
#endif

	SAVE_LOG_PROCESS(amba_track_init(&track_ctx, &track_ctx.params), "[main] Track initial");

	TIME_DECLARE();
	TIME_START();
	SAVE_LOG_PROCESS(start_process(&track_ctx, &global_param), "[main] Track process run");
	TIME_END_AVG("[main] Track process run", track_ctx.loop_count);

    amba_track_deinit(&track_ctx);
	// google::ShutdownGoogleLogging();
	LOG(INFO) << "[main] All process done!";

    return rval;
}


