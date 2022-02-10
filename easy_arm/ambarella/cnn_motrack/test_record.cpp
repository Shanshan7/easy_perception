#include <iostream>

// #include "yolov5rt/yolov5/detnet.h"
//#include "yolov5rt/deepsort/deepsort.h"
//#include "yolov5rt/yolov5/denetv2.h"
#include "yolov5rt/sde_tracker.h"
//#include "common/data_struct.h"
#include "postprocess/calculate_trajectory.h"

#define CLASS_NUMBER (3)

static sde_track_ctx_t track_ctx;


int main(int argc, char **argv)
{
	int ret = 0;
	int err = 0;

    amba_cv_env_init(&track_ctx);
	pthread_t capture_encoded_video_tid;

	// register signal handler for Ctrl+C,  Ctrl+'\'  ,  and "kill" sys cmd
	signal(SIGINT, sigstop);
	signal(SIGQUIT, sigstop);
	signal(SIGTERM, sigstop);

	if (init_data() < 0)
	{
		fprintf(stderr, "data initiation failed!\n");
		ret = -1;
	}

	if (ret == 0)
	{
		err = pthread_create(&capture_encoded_video_tid, NULL, capture_encoded_video, NULL);
		if (err != 0)
		{
			fprintf(stderr, "capture encoded video failed: %s\n", strerror(err));
			ret = -1;
		}
	}

	if (capture_encoded_video_tid)
	{
		pthread_join(capture_encoded_video_tid, NULL);
	}
	
	deinit_data();
    amba_cv_env_deinit(&track_ctx);

	return ret;
}