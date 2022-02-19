#include "common/utils.h"
#include <opencv2/imgcodecs.hpp>


#define MAX_PATH_STRLEN			    256


typedef struct save_data_s {
    int canvas_id;
    int sig_flag;

    int frame_number;

    char input_dir[MAX_PATH_STRLEN + 1];
    char output_dir[MAX_PATH_STRLEN + 1];

	ea_img_resource_t *img_resource;
	ea_display_t *display;

} save_data_t;
static save_data_t save_data;

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

		// save_data->img_resource = ea_img_resource_new(EA_JPEG_FOLDER, (void *)save_data->input_dir);
        save_data->img_resource = ea_img_resource_new(EA_CANVAS, (void *)(unsigned long)save_data->canvas_id);
		RVAL_ASSERT(save_data->img_resource != NULL);

        save_data->display = ea_display_new(EA_DISPLAY_JPEG, EA_TENSOR_COLOR_MODE_BGR, EA_DISPLAY_BBOX_TEXTBOX, (void *)save_data->output_dir);
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
    std::stringstream save_path;
    size_t input_shape[4] = {1, 3, 352, 640};
    ea_tensor_t *in_tensor = NULL;
    in_tensor = ea_tensor_new(EA_U8, input_shape, 640);

    EA_MEASURE_TIME_DECLARE();

    do {
        RVAL_OK(ea_img_resource_hold_data(save_data->img_resource, &data));
        if (data.tensor_group != NULL) {
            RVAL_ASSERT(data.tensor_group[0] != NULL);
            //--------------------------test-----------------------------------------
            RVAL_OK(ea_cvt_color_resize(data.tensor_group[0], in_tensor, EA_COLOR_YUV2BGR_NV12, EA_VP));

            save_path.str("");
            save_path << "/tmp/fairmot/out/src_mat_" << save_data->frame_number << ".jpg";

            EA_MEASURE_TIME_START();
            cv::Mat input_mat(cv::Size(640, 352), CV_8UC3);
            tensor2mat(in_tensor, input_mat, 0);
            EA_MEASURE_TIME_END("[IMAGE] yuv to bgr time");
            // cv::imwrite(save_path.str(), input_mat);
            save_data->frame_number++;
            //-----------------------------------------------------------------------
            // EA_LOG_NOTICE("Save path: %s\n", save_path.str().c_str());
			// RVAL_OK(ea_tensor_to_jpeg(in_tensor, EA_TENSOR_COLOR_MODE_BGR, save_path.str().c_str()));
            ea_img_resource_drop_data(save_data->img_resource, &data);
            EA_LOG_NOTICE("Frame number: %d\n", save_data->frame_number);
        }
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

    do {
        save_data.canvas_id = 0;
        save_data.frame_number = 0;
        snprintf(save_data.input_dir, sizeof(save_data.input_dir), "%s", \
        "/tmp/fairmot/in");
        snprintf(save_data.output_dir, sizeof(save_data.output_dir), "%s", \
        "/tmp/fairmot/out");
        RVAL_OK(cv_env_init(&save_data));
        RVAL_OK(cv_img_acquisition(&save_data));

    } while(0);
    cv_env_deinit(&save_data);

    return rval;
}
