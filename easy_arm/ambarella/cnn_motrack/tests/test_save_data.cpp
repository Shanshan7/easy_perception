#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <signal.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <string>
#include <sstream>
#include <sys/time.h> // system time

#include <eazyai.h>


#define MAX_PATH_STRLEN			    256

typedef struct save_data_s {
	int canvas_id;

	int sig_flag;
    char output_dir[MAX_PATH_STRLEN + 1];

	ea_img_resource_t *img_resource;
	ea_display_t *display;

} save_data_t;
static save_data_t save_data;
EA_LOG_DECLARE_LOCAL(EA_LOG_LEVEL_VERBOSE);


static int cv_env_init(save_data_t *save_data) {
	int rval = 0;
	int features = 0;

	do {
		features = EA_ENV_ENABLE_IAV
			| EA_ENV_ENABLE_CAVALRY
			| EA_ENV_ENABLE_VPROC
			| EA_ENV_ENABLE_NNCTRL;

        features |= EA_ENV_ENABLE_OSD_VOUT 
            | EA_ENV_ENABLE_OSD_STREAM 
            | EA_ENV_ENABLE_OSD_JPEG;

		RVAL_OK(ea_env_open(features));

		save_data->img_resource = ea_img_resource_new(EA_CANVAS, (void *)(unsigned long)save_data->canvas_id);
		RVAL_ASSERT(save_data->img_resource != NULL);

        save_data->display = ea_display_new(EA_DISPLAY_JPEG, EA_COLOR_YUV2RGB_NV12, EA_DISPLAY_BBOX_TEXTBOX, (void *)save_data->output_dir);
		RVAL_ASSERT(save_data->display != NULL);
	} while(0);

	if (rval < 0) {
		if (save_data->display) {
			ea_display_free(save_data->display);
			save_data->display = NULL;
		}

		if (save_data->img_resource) {
			ea_img_resource_free(save_data->img_resource);
			save_data->img_resource = NULL;
		}
	}

	return rval;

}

static int cv_img_acquisition(save_data_t *save_data) {
    int rval = 0;
    ea_img_resource_data_t data;

    do {
        EA_LOG_NOTICE("image is acquisition...\n");
        RVAL_OK(ea_img_resource_hold_data(save_data->img_resource, &data));
        RVAL_ASSERT(data.tensor_group[0] != NULL);
        // RVAL_OK(ea_tensor_to_jpeg(data.tensor_group[0], EA_TENSOR_COLOR_MODE_YUV_NV12, "/tmp/fairmot/out/src.jpg"));
        ea_display_refresh(save_data->display, (void *)data.tensor_group[0]);
        ea_img_resource_drop_data(save_data->img_resource, &data);

        if (save_data->sig_flag) {
			break;
		}
    } while(1);

    return rval;
}

static void cv_env_deinit(save_data_t *save_data)
{
	ea_display_free(save_data->display);
	save_data->display = NULL;
	ea_img_resource_free(save_data->img_resource);
	save_data->img_resource = NULL;
	ea_env_close();
}

static void sig_stop(int a)
{
	(void)a;
	save_data.sig_flag = 1;
}


int main() {
    int rval = 0;

    // process signals to control program operation
    signal(SIGINT, sig_stop);
	signal(SIGQUIT, sig_stop);
	signal(SIGTERM, sig_stop);

    memset(&save_data, 0, sizeof(save_data_t));

    // set param
    std::string command;
    std::stringstream save_path;
    std::string output_dir = "/sdcard/fairmot/";
    struct timeval tv;
    char time_str[64];
    gettimeofday(&tv, NULL); 
	strftime(time_str, sizeof(time_str)-1, "%Y_%m_%d_%H_%M_%S", localtime(&tv.tv_sec));
    save_path << output_dir << time_str << "/";

    save_data.canvas_id = 0;
    snprintf(save_data.output_dir, sizeof(save_data.output_dir), "%s", save_path.str().c_str());

    command = "mkdir -p " + save_path.str();
    system(command.c_str());

    do {
        RVAL_OK(cv_env_init(&save_data));
        RVAL_OK(cv_img_acquisition(&save_data));

    } while (0);

    cv_env_deinit(&save_data);
    EA_LOG_NOTICE("Save image success!!!");

    return rval;
}