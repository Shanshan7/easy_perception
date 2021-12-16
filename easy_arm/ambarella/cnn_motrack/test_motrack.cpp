#include <iostream>
#include <unistd.h>

#include "fairmot/tracker.h"
#include "common/data_struct.h"
#include "common/utils.h"

static track_params_t track_params;
static track_ctx_t track_ctx;


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

	// create struct of track output
	// TrackOutPut track_output;
	// memset(&track_output, 0, sizeof(TrackOutPut));

	// create track idx map
	std::map<int, TrajectoryParams> track_idx_map;
	track_idx_map.clear();

	SAVE_LOG_PROCESS(amba_track_init(&track_ctx, &track_ctx.params), "[main] Track initial");

	TIME_DECLARE();
	TIME_START();
	SAVE_LOG_PROCESS(amba_track_run_loop(&track_ctx, track_idx_map), "[main] Track process run");
	TIME_END_AVG("[main] Track process run", track_ctx.loop_count);

    amba_track_deinit(&track_ctx);
	LOG(INFO) << "[main] All process done!";

    return rval;
}


