#include <iostream>
#include <unistd.h>

#include "fairmot/tracker.h"
#include "common/data_struct.h"

static track_params_t track_params;
static track_ctx_t track_ctx;


static void sig_stop(int a)
{
	(void)a;
	track_ctx.sig_flag = 1;
}

int main() {
    int rval = 0;

    // process signals to control program operation
    signal(SIGINT, sig_stop);
	signal(SIGQUIT, sig_stop);
	signal(SIGTERM, sig_stop);

    memset(&track_ctx, 0, sizeof(track_ctx_t));

	// create struct of track output
	TrackOutPut track_output;
	memset(&track_output, 0, sizeof(TrackOutPut));

	// create track idx map
	std::map<int, TrajectoryParams> track_idx_map;
	track_idx_map.clear();

    do {
        RVAL_OK(amba_track_init(&track_ctx, &track_ctx.params));
		RVAL_OK(amba_track_run_loop(&track_ctx, &track_output, track_idx_map));
    } while (0);

    amba_track_deinit(&track_ctx);

    return rval;
}


