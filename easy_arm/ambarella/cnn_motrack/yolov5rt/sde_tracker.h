#ifndef _SDE_TRACKER_H_
#define _SDE_TRACKER_H_

#include <data_struct.h>
#include <eazyai.h>

#define MAX_PATH_STRLEN			    256
#define MAX_LABEL_LEN				128

typedef struct sde_track_ctx_s {
	ea_img_resource_t *img_resource;
	ea_display_t *display;

	int canvas_id;
    int stream_id;
    int sig_flag;
    int run_write_video_file;

    char input_dir[MAX_PATH_STRLEN + 1];
    char output_dir[MAX_PATH_STRLEN + 1];
} sde_track_ctx_t;

int amba_cv_env_init(sde_track_ctx_t *track_ctx);
void amba_cv_env_deinit(sde_track_ctx_t *track_ctx);
// int amba_draw_detection(sde_track_ctx_t *track_ctx, std::vector<DetectBox> &det_results, uint32_t dsp_pts);
int amba_draw_detection(sde_track_ctx_t *track_ctx, std::vector<DetectBox> &det_results, 
                        std::map<int, TrajectoryParams> track_idx_map, ea_tensor_t *img_tensor, uint32_t dsp_pts);

#endif // _SDE_TRACKER_H_
