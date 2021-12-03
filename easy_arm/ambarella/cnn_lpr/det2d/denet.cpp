#include "cnn_lpr/det2d/denet.h"
#include <eazyai.h>
#include "yolov5.h"

#define MAX_FILE_NAME_LEN			(128)
#define MAX_LABEL_LEN				(128)

#define YOLOV5_INPUT_NAME			"images"
#define YOLOV5S_FEATURE_MAP_1_NAME	"1037"	// the smallest size
#define YOLOV5S_FEATURE_MAP_2_NAME	"1017"	// the medium size
#define YOLOV5S_FEATURE_MAP_3_NAME	"997"	// the biggest size

#define YOLOV5M_FEATURE_MAP_1_NAME	"1428"	// the smallest size
#define YOLOV5M_FEATURE_MAP_2_NAME	"1408"	// the medium size
#define YOLOV5M_FEATURE_MAP_3_NAME	"1388"	// the biggest size

#define YOLOV5L_FEATURE_MAP_1_NAME	"1819"	// the smallest size
#define YOLOV5L_FEATURE_MAP_2_NAME	"1799"	// the medium size
#define YOLOV5L_FEATURE_MAP_3_NAME	"1779"	// the biggest size

#define YOLOV5X_FEATURE_MAP_1_NAME	"2210"	// the smallest size
#define YOLOV5X_FEATURE_MAP_2_NAME	"2190"	// the medium size
#define YOLOV5X_FEATURE_MAP_3_NAME	"2170"	// the biggest size

enum {
	DRAW_MODE_VOUT,
	DRAW_MODE_STREAM,
};

enum {
	RUN_MODE_LIVE,
	RUN_MODE_FILE,
};

typedef struct live_params_s {
	int run_mode;
	char input_dir[MAX_FILE_NAME_LEN + 2]; // may plus "/\0"
	int log_level;
	int draw_mode;
	int canvas_id;
	int stream_id;
	int use_pyramid;
	int pyramid[2];
} live_params_t;

typedef struct live_ctx_s {
	int sig_flag;

	ea_img_resource_t *img_resource;
	uint32_t dsp_pts;
	yolov5_t yolov5;
	yolov5_result_t net_result;
	ea_display_t *display;

	DIR *image_dirp;
} live_ctx_t;

EA_LOG_DECLARE_LOCAL(EA_LOG_LEVEL_NOTICE);
EA_MEASURE_TIME_DECLARE();
static live_ctx_t live_ctx;

static int cv_env_init(live_ctx_t *live_ctx, live_params_t *params)
{
	int rval = 0;
	int features = 0;

	do {
		features = EA_ENV_ENABLE_IAV
			| EA_ENV_ENABLE_CAVALRY
			| EA_ENV_ENABLE_VPROC
			| EA_ENV_ENABLE_NNCTRL;
		if (params->run_mode == RUN_MODE_LIVE) {
			if (params->draw_mode == DRAW_MODE_VOUT) {
				features |= EA_ENV_ENABLE_OSD_VOUT;
			} else {
				features |= EA_ENV_ENABLE_OSD_STREAM;
			}
		} else {
			features |= EA_ENV_ENABLE_OSD_JPEG;
		}

		RVAL_OK(ea_env_open(features));

		if (params->run_mode == RUN_MODE_LIVE) {
			if (params->use_pyramid) {
				live_ctx->img_resource = ea_img_resource_new(EA_PYRAMID, (void *)(unsigned long)params->pyramid[0]);
			} else {
				live_ctx->img_resource = ea_img_resource_new(EA_CANVAS, (void *)(unsigned long)params->canvas_id);
			}
		} else {
			live_ctx->img_resource = ea_img_resource_new(EA_JPEG_FOLDER, (void *)params->input_dir);
		}

		RVAL_ASSERT(live_ctx->img_resource != NULL);

		if (params->run_mode == RUN_MODE_LIVE) {
			if (params->draw_mode == DRAW_MODE_VOUT) {
				live_ctx->display = ea_display_new(EA_DISPLAY_VOUT, EA_DISPLAY_ANALOG_VOUT, EA_DISPLAY_BBOX_TEXTBOX, NULL);
			} else {
				live_ctx->display = ea_display_new(EA_DISPLAY_STREAM, params->stream_id, EA_DISPLAY_BBOX_TEXTBOX, NULL);
			}
		} else {
			live_ctx->display = ea_display_new(EA_DISPLAY_JPEG, EA_TENSOR_COLOR_MODE_BGR, EA_DISPLAY_BBOX_TEXTBOX, (void *)params->output_dir);
		}

		RVAL_ASSERT(live_ctx->display != NULL);
	} while(0);

	if (rval < 0) {
		if (live_ctx->display) {
			ea_display_free(live_ctx->display);
			live_ctx->display = NULL;
		}

		if (live_ctx->img_resource) {
			ea_img_resource_free(live_ctx->img_resource);
			live_ctx->img_resource = NULL;
		}
	}

	return rval;
}

static void cv_env_deinit(live_ctx_t *live_ctx, live_params_t *params)
{
	ea_display_free(live_ctx->display);
	live_ctx->display = NULL;
	ea_img_resource_free(live_ctx->img_resource);
	live_ctx->img_resource = NULL;
	ea_env_close();
}

static int draw_detection(live_ctx_t *live_ctx, live_params_t *params, const ea_tensor_t *tensor)
{
	int rval = 0;
	char text[MAX_LABEL_LEN];
	int i;

	ea_display_obj_params(live_ctx->display)->obj_win_w = 1.0;
	ea_display_obj_params(live_ctx->display)->obj_win_h = 1.0;
	if (ea_display_obj_params(live_ctx->display)->dis_win_w < HIGH_RESOLUTION_BEGIN) {
		ea_display_obj_params(live_ctx->display)->border_thickness = 2;
		ea_display_obj_params(live_ctx->display)->font_size = 18;
	} else {
		ea_display_obj_params(live_ctx->display)->border_thickness = 5;
		ea_display_obj_params(live_ctx->display)->font_size = 32;
	}

	ea_display_obj_params(live_ctx->display)->text_color = EA_16_COLORS_WHITE;

	do {
		for (i = 0; i < live_ctx->net_result.valid_det_count; i++) {
			if (strstr(live_ctx->net_result.detections[i].label, "invalid") == NULL) {
				snprintf(text, MAX_LABEL_LEN, "%.3f %s",
					live_ctx->net_result.detections[i].score,
					live_ctx->net_result.detections[i].label);
				ea_display_obj_params(live_ctx->display)->box_color = live_ctx->net_result.detections[i].id % EA_16_COLORS_MAX_NUM;
				ea_display_set_bbox(live_ctx->display, text,
					live_ctx->net_result.detections[i].x_start, live_ctx->net_result.detections[i].y_start,
					live_ctx->net_result.detections[i].x_end - live_ctx->net_result.detections[i].x_start,
					live_ctx->net_result.detections[i].y_end - live_ctx->net_result.detections[i].y_start);
			}
		}

		if (params->run_mode == RUN_MODE_LIVE) {
			ea_display_refresh(live_ctx->display, (void *)(unsigned long)0/*dsp_pts*/); // loop time is too long to sync with pts, so pass 0 to refresh without pts
		} else {
			ea_display_refresh(live_ctx->display, (void *)tensor/*tensor*/);
		}
	} while (0);

	return rval;
}

DeNet::DeNet()
{
    threshold = 0;
    nms_threshold = 0.3f;
    top_k = 100;
	use_multi_cls = 0;
}

DeNet::~DeNet()
{
    
}

int DeNet::init(const std::string &modelPath, const std::string &labelPath,
                const std::vector<std::string> &inputName, const std::vector<std::string> &outputName, 
                const int classNumber, const float threshold=0.1f)
{
    int rval = 0;
    live_params_t params;
	yolov5_params_t net_params;
    int inputCount = static_cast<int>(inputName.size());
    int outputCount = static_cast<int>(outputName.size());
    char ** inNames = new char*[inputCount];
    char ** outNames = new char*[outputCount];

    for(size_t i = 0; i < inputName.size(); i++){
        inNames[i] = new char[inputName[i].size() + 1];
        strcpy(inNames[i], inputName[i].c_str());
    }
    for(size_t i = 0; i < outputName.size(); i++){
        outNames[i] = new char[outputName[i].size() + 1];
        strcpy(outNames[i], outputName[i].c_str());
    }

    params.run_mode = RUN_MODE_LIVE;
	strcpy(params->input_dir, "/sdcard/yolov5/in/");
	params.log_level = EA_LOG_LEVEL_NOTICE;
	params.draw_mode = 0;
	params.canvas_id = 1;
	params.stream_id = 0;
    params.use_pyramid = 0;

    do {
		RVAL_OK(cv_env_init(live_ctx, &params));

		memset(&net_params, 0, sizeof(yolov5_params_t));
		net_params.log_level = params.log_level;
        net_params.feature_map_names[0] = outNames[0];
		net_params.feature_map_names[1] = outNames[1];
		net_params.feature_map_names[2] = outNames[2];
		
		RVAL_BREAK();

		net_params.model_path = modelPath.c_str();
		net_params.label_path = labelPath.c_str();
		net_params.input_name = inNames[0];

		net_params.conf_threshold = threshold;
		net_params.nms_threshold = nms_threshold;
		net_params.keep_top_k = top_k;
		net_params.use_multi_cls = use_multi_cls;
		RVAL_OK(yolov5_init(&live_ctx->yolov5, &net_params));

		if (params->run_mode == RUN_MODE_FILE) {
			live_ctx->image_dirp = opendir(params->input_dir);
			if (live_ctx->image_dirp == NULL) {
				EA_LOG_ERROR("opendir() failed on %s\n", params->input_dir);
				rval = -1;
				break;
			}
		}
	} while (0);

    this->classNumber = classNumber;
    this->threshold = threshold;

    for(size_t i = 0; i < inputName.size(); i++){
        delete [] inNames[i];
    }
    delete [] inNames;
    for(size_t i = 0; i < outputName.size(); i++){
        delete [] outNames[i];
    }
    delete [] outNames;

    return rval;
}