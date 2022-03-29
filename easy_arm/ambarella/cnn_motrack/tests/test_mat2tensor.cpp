#include "common/utils.h"
#include <opencv2/imgcodecs.hpp>


#define MAX_PATH_STRLEN			    256

static int cv_env_init() {
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
	} while(0);

	return rval;

}

static void cv_env_deinit()
{
	ea_env_close();
}

int main() {
    int rval = 0;
    int frame_id = 0;
    EA_MEASURE_TIME_DECLARE();

    cv_env_init();
    for (int i = 0; i < 1000; i++)
    {
        cv::Mat input_image = cv::imread("/sdcard/input.jpg");
        size_t shape[4];
        shape[0] = 1;
        shape[1] = input_image.channels();
        shape[2] = input_image.rows;
        shape[3] = input_image.cols;

        ea_tensor_t *output_tensor = NULL;
        output_tensor = ea_tensor_new(EA_U8, shape, 0);
        EA_MEASURE_TIME_START();
        mat2tensor(input_image, output_tensor);
        EA_MEASURE_TIME_END("[IMAGE] mat to tensor time");

        std::stringstream save_path;
        save_path.str("");
        save_path << "/sdcard/output" << frame_id << ".jpg";
        ea_tensor_to_jpeg(output_tensor, EA_TENSOR_COLOR_MODE_BGR, save_path.str().c_str());
        frame_id += 1;
        ea_tensor_free(output_tensor);
    }

    cv_env_deinit();

    return rval;
}
