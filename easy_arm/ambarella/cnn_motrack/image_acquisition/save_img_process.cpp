#include <sys/time.h> // system time

#include "save_img_process.h"

SaveImageProcess::SaveImageProcess()
{
    image_pthread_id = 0;
    save_dir = "/sdcard/fairmot/";
    image_frame_number = 0;
}

SaveImageProcess::~SaveImageProcess()
{

}   

int SaveImageProcess::init_save_dir()
{
    struct timeval tv;  
    char time_str[64];
	save_path.str("");
    std::string command;
    gettimeofday(&tv, NULL); 
	strftime(time_str, sizeof(time_str)-1, "%Y_%m_%d_%H_%M_%S", localtime(&tv.tv_sec)); 
	save_path << save_dir << time_str << "/";
    command = "mkdir -p " + save_path.str();
    system(command.c_str());

    return 0;
}

int SaveImageProcess::cv_env_init(save_data_t *save_data) {
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

int SaveImageProcess::cv_img_acquisition(save_data_t *save_data) {
    int rval = 0;
    ea_img_resource_data_t data;

    EA_LOG_NOTICE("image is acquisition...\n");
    do {
        RVAL_OK(ea_img_resource_hold_data(save_data->img_resource, &data));
        RVAL_ASSERT(data.tensor_group[0] != NULL);
        // RVAL_OK(ea_tensor_to_jpeg(data.tensor_group[0], EA_TENSOR_COLOR_MODE_YUV_NV12, "/tmp/fairmot/out/src.jpg"));
        ea_display_refresh(save_data->display, (void *)data.tensor_group[0]);
        ea_img_resource_drop_data(save_data->img_resource, &data);
		image_frame_number++;
    } while(0);


    return rval;
}

int SaveImageProcess::cv_video_acquisition() {
    int rval = 0;

    return rval;
}

void SaveImageProcess::cv_env_deinit(save_data_t *save_data)
{
	ea_display_free(save_data->display);
	save_data->display = NULL;
	ea_img_resource_free(save_data->img_resource);
	save_data->img_resource = NULL;
	ea_env_close();
}