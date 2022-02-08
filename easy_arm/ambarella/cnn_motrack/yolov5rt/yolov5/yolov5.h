#ifndef _YOLOV5_H_
#define _YOLOV5_H_

#ifdef __cplusplus
extern "C" {
#endif

#define YOLOV5_MAX_LABEL_NUM		183
#define YOLOV5_MAX_LABEL_LEN		48
#define YOLOV5_MAX_OUT_NUM			200

#define YOLOV5_FEATURE_MAP_NUM			3
#define YOLOV5_ANCHOR_NUM				3
#define YOLOV5_MIN_WH					2
#define YOLOV5_MAX_WH					4096

typedef struct ea_tensor_s ea_tensor_t;

struct yolov5_s {
	char labels[YOLOV5_MAX_LABEL_NUM][YOLOV5_MAX_LABEL_LEN];
	int valid_label_count;

	ea_net_t *net;
	ea_tensor_t *input_tensor;
	ea_tensor_t *feature_map_tensors[YOLOV5_FEATURE_MAP_NUM];
	int top_k;
	float nms_threshold;
	float conf_threshold;
	int use_multi_cls;
};
typedef struct yolov5_s yolov5_t;

typedef struct yolov5_params_s {
	int log_level;

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
int yolov5_vp_forward(yolov5_t *yolov5);

typedef struct yolov5_det_s {
	float score;
	int id;
	char label[YOLOV5_MAX_LABEL_LEN];
	float x_start; // normalized value
	float y_start;
	float x_end;
	float y_end;
} yolov5_det_t;

typedef struct  yolov5_result_s {
	yolov5_det_t detections[YOLOV5_MAX_OUT_NUM];
	int valid_det_count;
} yolov5_result_t;

int yolov5_arm_post_process(yolov5_t *yolov5, yolov5_result_t *result);

#ifdef __cplusplus
}
#endif

#endif
