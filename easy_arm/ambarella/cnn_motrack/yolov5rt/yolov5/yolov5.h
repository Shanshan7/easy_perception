#ifndef _YOLOV5_H_
#define _YOLOV5_H_

#include "../../common/data_struct.h"
#include "../../common/utils.h"


#define YOLOV5_FEATURE_MAP_NUM			3

EA_LOG_DECLARE_LOCAL(EA_LOG_LEVEL_NOTICE);

#define IDX(o) (entry_index(o,anchor,j,i,grid_x,grid_y))

struct yolov5_s {
	ea_net_t *net;
	ea_tensor_t *input_tensor;
	ea_tensor_t *feature_map_tensors[YOLOV5_FEATURE_MAP_NUM];
	std::vector<cv::Mat> outputs;
	float obj_thresh;
	float nms_threshold;
	float conf_threshold;

	const float anchors[3][6] = {{10.0, 13.0, 16.0, 30.0, 33.0, 23.0}, {30.0, 61.0, 62.0, 45.0, 59.0, 119.0},{116.0, 90.0, 156.0, 198.0, 373.0, 326.0}};
	const float stride[3] = { 8.0, 16.0, 32.0 };
	const int inpWidth = 576;
	const int inpHeight = 352;
	int src_width;
	int src_height;
	std::vector<std::string> classes;
};
typedef struct yolov5_s yolov5_t;

typedef struct yolov5_params_s {
	// int log_level;

	const char *model_path;
	const char *label_path;
	const char *input_name;
	const char *feature_map_names[YOLOV5_FEATURE_MAP_NUM];

	float conf_threshold;	/*!< the threshold for filter_bboxes */
	int keep_top_k;			/*!< Keep the maximum number of objects */
	float nms_threshold;	/*!< the threshold for nms */
	int use_multi_cls;		/*!< use multi class or best class in post process. 0=best class, 1=multi class */
} yolov5_params_t;

int yolov5_init(yolov5_t *yolov5, const yolov5_params_t *params);
void yolov5_deinit(yolov5_t *yolov5);
ea_tensor_t *yolov5_input(yolov5_t *yolov5);
int entry_index(int loc, int anchorC, int w, int h, int lWidth, int lHeight);
int yolov5_vp_forward(yolov5_t *yolov5);
int yolov5_postprocess(yolov5_t *yolov5, std::vector<DetectBox> &det_results);


#endif // _YOLOV5_H_