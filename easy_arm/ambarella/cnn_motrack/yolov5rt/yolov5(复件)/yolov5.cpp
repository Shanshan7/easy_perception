#include "yolov5.h"

#define IDX(o) (entry_index(o,anchor,j,i,grid_x,grid_y))


int yolov5_init(yolov5_t *yolov5, const yolov5_params_t *params)
{
	int rval = 0;
	// FILE *fp_label = NULL;
	// char *endl = NULL;

	do {
		RVAL_OK(yolov5 != NULL);
		RVAL_ASSERT(params != NULL);
		RVAL_ASSERT(params->model_path != NULL);

		memset(yolov5, 0, sizeof(yolov5_t));
		yolov5->nms_threshold = params->nms_threshold;
		yolov5->conf_threshold = params->conf_threshold;
		yolov5->obj_thresh = 0.8;
		// yolov5->anchors = {{10.0, 13.0, 16.0, 30.0, 33.0, 23.0}, {30.0, 61.0, 62.0, 45.0, 59.0, 119.0},{116.0, 90.0, 156.0, 198.0, 373.0, 326.0}};
		// yolov5->stride = {8.0, 16.0, 32.0};
		yolov5->classes = {"car"};
		yolov5->inpWidth = 416;
		yolov5->inpHeight = 416;
		yolov5->src_width = 960;
		yolov5->src_height = 540;

		yolov5->net = ea_net_new(NULL);
		RVAL_ASSERT(yolov5->net != NULL);

		ea_net_config_input(yolov5->net, params->input_name);
		for (int i = 0; i < YOLOV5_FEATURE_MAP_NUM; i++) {
			ea_net_config_output(yolov5->net, params->feature_map_names[i]);
		}
		RVAL_OK(ea_net_load(yolov5->net, EA_NET_LOAD_FILE, (void *)params->model_path, 1/*max_batch*/));
		yolov5->input_tensor = ea_net_input(yolov5->net, params->input_name);
		for (int i = 0; i < YOLOV5_FEATURE_MAP_NUM; i++) {
			yolov5->feature_map_tensors[i] = ea_net_output(yolov5->net, params->feature_map_names[i]);
			ea_tensor_sync_cache(yolov5->feature_map_tensors[i], EA_VP, EA_CPU);
		}

		RVAL_BREAK();

		// fclose(fp_label);
		// fp_label = NULL;

		// EA_LOG_NOTICE("label num: %d\n", yolov5->valid_label_count);
	} while (0);

	if (rval < 0) {
		// if (fp_label) {
		// 	fclose(fp_label);
		// 	fp_label = NULL;
		// }

		if (yolov5) {
			if (yolov5->net) {
				ea_net_free(yolov5->net);
				yolov5->net = NULL;
			}
		}
	}

	return rval;
}

void yolov5_deinit(yolov5_t *yolov5)
{
	if (yolov5) {
		if (yolov5->net) {
			ea_net_free(yolov5->net);
			yolov5->net = NULL;
		}
	}

	EA_LOG_NOTICE("yolov5_deinit\n");
}

int yolov5_vp_forward(yolov5_t *yolov5)
{
	int rval = 0;

	do {
		RVAL_OK(ea_net_forward(yolov5->net, 1/*batch*/));
	} while (0);

	return rval;
}

ea_tensor_t *yolov5_input(yolov5_t *yolov5)
{
	return yolov5->input_tensor;
}

int entry_index(int loc, int anchorC, int w, int h, int lWidth, int lHeight)
{
    return ((anchorC *(YOLOV5_FEATURE_MAP_NUM+5) + loc) * lHeight * lWidth + h * lWidth + w);
}

int yolov5_postprocess(yolov5_t *yolov5, std::vector<DetectBox> &det_results) // std::vector<cv::Mat> &outputs, 
{
	int rval = 0;

	const float anchors_[3][6] = {{10.0, 13.0, 16.0, 30.0, 33.0, 23.0}, {30.0, 61.0, 62.0, 45.0, 59.0, 119.0},{116.0, 90.0, 156.0, 198.0, 373.0, 326.0}};
	const float stride_[3] = { 8.0, 16.0, 32.0 };

	std::vector<int> classIds;//结果id数组
	std::vector<float> confidences;//结果每个id对应置信度数组
	// std::vector<cv::Rect> boxes;//每个id矩形框
	std::vector<std::vector<float>> boxes;
	float ratio_h = (float)yolov5->src_height / yolov5->inpHeight;
	float ratio_w = (float)yolov5->src_width / yolov5->inpWidth;
	int net_width = yolov5->classes.size() + 5;  //输出的网络宽度是类别数+5
	std::cout << net_width << std::endl;

	for (int stride = 0; stride < 3; stride++) 
	{    //stride
		std::vector<float> box;
		// ea_tensor_sync_cache(yolov5->feature_map_tensors[stride], EA_VP, EA_CPU);
		float* pdata = (float*)ea_tensor_data(yolov5->feature_map_tensors[stride]);
		std::cout << yolov5->obj_thresh << std::endl;
		int grid_x = (int)(yolov5->inpWidth / stride_[stride]);
		int grid_y = (int)(yolov5->inpHeight / stride_[stride]);
		float max_class_socre;
    	int cls = 0;
		for (int anchor = 0; anchor < 3; anchor++) { //anchors
			const float anchor_w = anchors_[stride][anchor * 2];
			const float anchor_h = anchors_[stride][anchor * 2 + 1];
			for (int i = 0; i < grid_y; i++) {
				for (int j = 0; j < grid_x; j++) {
					float box_score = sigmoid_x(pdata[IDX(4)]);//获取每一行的box框中含有某个物体的概率
					if (box_score > yolov5->obj_thresh) {
						// cv::Mat scores(1, this->classes.size(), CV_32FC1, pdata + 5);
						// Point classIdPoint;
						// double max_class_socre;
						// minMaxLoc(scores, 0, &max_class_socre, 0, &classIdPoint);
						max_class_socre = -1e10;
						cls = 0;
						for (int p = 0; p < YOLOV5_FEATURE_MAP_NUM; p++) {
							if (pdata[IDX(5 + p)] >= max_class_socre) {
								max_class_socre = pdata[IDX(5 + p)];
								cls = p;
							}
						}
						
						max_class_socre = sigmoid_x(max_class_socre);
						box_score *= max_class_socre;

						// max_class_socre = sigmoid_x((float)max_class_socre); // (float)max_class_socre;
						if (box_score > yolov5->conf_threshold) {
							std::cout << "box_score: " << box_score << std::endl;
							// rect [x,y,w,h]
							box.clear();
							float x = (sigmoid_x(pdata[IDX(0)]) * 2.f - 0.5f + j) * stride_[stride]; //x pdata[0];   // 
							float y = (sigmoid_x(pdata[IDX(1)]) * 2.f - 0.5f + i) * stride_[stride]; //y pdata[1]; //    
							float w = powf(sigmoid_x(pdata[IDX(2)]) * 2.f, 2.f) * anchor_w; //w pdata[2]; //  
							float h = powf(sigmoid_x(pdata[IDX(3)]) * 2.f, 2.f) * anchor_h; //h pdata[3]; //  
							int left = (x - 0.5*w)*ratio_w;
							int top = (y - 0.5*h)*ratio_h;
							classIds.push_back(cls);
							confidences.push_back(box_score);
							box.push_back(float(left));
							box.push_back(float(top));
							box.push_back(w*ratio_w);
							box.push_back(h*ratio_h);
							box.push_back(float(cls));
							box.push_back(float(box_score));
							boxes.push_back(box);
						}
					}
				}
			}
		}
	}
	std::cout << "0000000000000000000000000" << std::endl;

	//执行非最大抑制以消除具有较低置信度的冗余重叠框（NMS）
	// vector<int> nms_result;
	// NMSBoxes(boxes, confidences, this->confThreshold, this->nmsThreshold, nms_result);
	std::cout << yolov5->nms_threshold << " " << boxes.size() << std::endl;
	boxes = applyNMS(boxes, yolov5->nms_threshold);
	std::cout << "11111111111111111111111111" << std::endl;
	for (int i = 0; i < boxes.size(); i++) {
		DetectBox result;
		result.classID = boxes[i][4];
		// if (result.classID == 0) 
		// {
		result.confidence = boxes[i][5];
		result.x1 = boxes[i][0];
		// if (result.x1 < 1) {continue;}
		result.y1 = boxes[i][1];
		// if (result.y1 < 1) {continue;}
		result.x2 = boxes[i][0] + boxes[i][2];
		// if (result.x2 >= this->src_width) {result.x2 = this->src_width - 1;}
		result.y2 = boxes[i][1] + boxes[i][3];
		// if (result.y2 >= 1080) {result.y2 = 1080 - 1;}
		std::cout << result.x1 << " " << result.y1 << " " << result.x2 << " " << result.y2 << std::endl;
		det_results.push_back(result);
		// }
	}

	return rval;
}