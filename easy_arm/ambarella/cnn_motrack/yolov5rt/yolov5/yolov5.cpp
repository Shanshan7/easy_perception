#include "yolov5.h"

YOLO::YOLO(string model_path)
{
	// cout << "Net use " << config.netname << endl;
	// this->confThreshold = config.confThreshold;
	// this->nmsThreshold = config.nmsThreshold;
	// this->objThreshold = config.objThreshold;
	// strcpy(this->netname, config.netname.c_str());

	ifstream ifs(this->classesFile.c_str());
	string line;
	while (getline(ifs, line)) this->classes.push_back(line);

	// string modelFile = this->netname;
	// modelFile += ".onnx";
	this->net = cv::dnn::readNet(model_path);
}

void YOLO::run(cv::Mat& frame, std::vector<DetectBox> &det_results)
{
	cv::Mat blob;
	int col = frame.cols;
	int row = frame.rows;
	int maxLen = MAX(col, row);
	cv::Mat netInputImg = frame.clone();
	if (maxLen > 1.2*col || maxLen > 1.2*row) {
		cv::Mat resizeImg = cv::Mat::zeros(maxLen, maxLen, CV_8UC3);
		frame.copyTo(resizeImg(cv::Rect(0, 0, col, row)));
		netInputImg = resizeImg;
	}
	this->src_height = netInputImg.rows;
	this->src_width = netInputImg.cols;
	cv::dnn::blobFromImage(netInputImg, blob, 1 / 255.0, cv::Size(this->inpWidth, this->inpHeight), cv::Scalar(104, 117,123), true, false);
	this->net.setInput(blob);
	std::vector<cv::Mat> netOutputImg;
	this->net.forward(netOutputImg, net.getUnconnectedOutLayersNames());

	// for (int c = 0; c < netOutputImg.size(); c++)
	// {
	// 	std::cout << "i: " << c << " shape: " << netOutputImg[c].size[1] << " " << netOutputImg[c].size[2] << " " \
	//               << netOutputImg[c].size[3] << std::endl;
	// }
	
	postprocess(netOutputImg, det_results);
}

int YOLO::postprocess(std::vector<cv::Mat> &outputs, std::vector<DetectBox> &det_results)
{
	int rval = 0;

	std::vector<int> classIds;//结果id数组
	std::vector<float> confidences;//结果每个id对应置信度数组
	std::vector<cv::Rect> boxes;//每个id矩形框
	float ratio_h = (float)this->src_height / this->inpHeight;
	float ratio_w = (float)this->src_width / this->inpWidth;
	int net_width = this->classes.size() + 5;  //输出的网络宽度是类别数+5

	for (int stride =0; stride < 3; stride++) 
	{    //stride
		float* pdata = (float*)outputs[stride].data;
		int grid_x = (int)(this->inpWidth / this->stride[stride]);
		int grid_y = (int)(this->inpHeight / this->stride[stride]);
		float max_class_socre;
    	int cls = 0;
		for (int anchor = 0; anchor < 3; anchor++) { //anchors
			const float anchor_w = this->anchors[stride][anchor * 2];
			const float anchor_h = this->anchors[stride][anchor * 2 + 1];
			for (int i = 0; i < grid_y; i++) {
				for (int j = 0; j < grid_x; j++) {
					float box_score = sigmoid_x(pdata[IDX(4)]);//获取每一行的box框中含有某个物体的概率
					if (box_score > this->objThreshold) {
						// cv::Mat scores(1, this->classes.size(), CV_32FC1, pdata + 5);
						// Point classIdPoint;
						// double max_class_socre;
						// minMaxLoc(scores, 0, &max_class_socre, 0, &classIdPoint);
						max_class_socre = -1e10;
						cls = 0;
						for (int p = 0; p < g_classificationCnt; p++) {
							if (pdata[IDX(5 + p)] >= max_class_socre) {
								max_class_socre = pdata[IDX(5 + p)];
								cls = p;
							}
						}
						
						max_class_socre = sigmoid_x(max_class_socre);
						box_score *= max_class_socre;

						// max_class_socre = sigmoid_x((float)max_class_socre); // (float)max_class_socre;
						if (box_score > this->confThreshold) {
							// rect [x,y,w,h]
							float x = (sigmoid_x(pdata[IDX(0)]) * 2.f - 0.5f + j) * this->stride[stride]; //x pdata[0];   // 
							float y = (sigmoid_x(pdata[IDX(1)]) * 2.f - 0.5f + i) * this->stride[stride]; //y pdata[1]; //    
							float w = powf(sigmoid_x(pdata[IDX(2)]) * 2.f, 2.f) * anchor_w; //w pdata[2]; //  
							float h = powf(sigmoid_x(pdata[IDX(3)]) * 2.f, 2.f) * anchor_h; //h pdata[3]; //  
							int left = (x - 0.5*w)*ratio_w;
							int top = (y - 0.5*h)*ratio_h;
							classIds.push_back(cls);
							confidences.push_back(box_score);
							boxes.push_back(cv::Rect(left, top, int(w*ratio_w), int(h*ratio_h)));
						}
					}
					// pdata += net_width;//下一行
				}
			}
		}
	}

	//执行非最大抑制以消除具有较低置信度的冗余重叠框（NMS）
	vector<int> nms_result;
	cv::dnn::NMSBoxes(boxes, confidences, this->confThreshold, this->nmsThreshold, nms_result);
	for (int i = 0; i < nms_result.size(); i++) {
		int idx = nms_result[i];
		DetectBox result;
		result.classID = classIds[idx];
		if (result.classID == 0) 
		{
			result.confidence = confidences[idx];
			result.x1 = boxes[idx].x;
			// if (result.x1 < 1) {continue;}
			result.y1 = boxes[idx].y;
			// if (result.y1 < 1) {continue;}
			result.x2 = boxes[idx].x + boxes[idx].width;
			// if (result.x2 >= this->src_width) {result.x2 = this->src_width - 1;}
			result.y2 = boxes[idx].y + boxes[idx].height;
			// if (result.y2 >= 1080) {result.y2 = 1080 - 1;}
			// std::cout << result.x1 << " " << result.y1 << " " << result.x2 << " " << result.y2 << std::endl;
			det_results.push_back(result);
		}
	}

	return rval;
}