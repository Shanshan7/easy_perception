#ifndef YOLOV5_H
#define YOLOV5_H

#include <fstream>
#include <sstream>
#include <iostream>
#include <opencv2/dnn.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include <datatype.h>

// using namespace cv;
// using namespace dnn;
using namespace std;

#define MOT_REID_DIM    128
#define g_classificationCnt 80

#define IDX(o) (entry_index(o,anchor,j,i,grid_x,grid_y))

struct Net_config
{
	float confThreshold; // class Confidence threshold
	float nmsThreshold;  // Non-maximum suppression threshold
	float objThreshold;  //Object Confidence threshold
	string netname;
};

// struct DetResults {
// 	int class_id; // detection class id
// 	float confidence;// detection confidence
// 	cv::Rect box; // detection box(x,y,w,h)
// 	float reid[MOT_REID_DIM]; //REID information
// };

class YOLO
{
	public:
		YOLO(string model_path);
		void run(cv::Mat& frame, std::vector<DetectBox> &det_results);
	private:
		const float anchors[3][6] = {{10.0, 13.0, 16.0, 30.0, 33.0, 23.0}, {30.0, 61.0, 62.0, 45.0, 59.0, 119.0},{116.0, 90.0, 156.0, 198.0, 373.0, 326.0}};
		const float stride[3] = { 8.0, 16.0, 32.0 };
		const string classesFile = "coco.names";
		const int inpWidth = 640;
		const int inpHeight = 640;
		float confThreshold = 0.45;
		float nmsThreshold = 0.45;
		float objThreshold = 0.45;
		int src_width;
		int src_height;
		
		char netname[20];
		std::vector<string> classes;
		cv::dnn::Net net;
		int postprocess(std::vector<cv::Mat> &outputs, std::vector<DetectBox> &det_results);
		void sigmoid(cv::Mat* out, int length);
};

static inline float sigmoid_x(float x)
{
	return static_cast<float>(1.f / (1.f + exp(-x)));
}

static int entry_index(int loc, int anchorC, int w, int h, int lWidth, int lHeight)
{
    return ((anchorC *(g_classificationCnt+5) + loc) * lHeight * lWidth + h * lWidth + w);
}

// Net_config yolo_nets[4] = {
// 	{0.25, 0.45, 0.25, "yolov5s"},
// 	{0.5, 0.5, 0.5,  "yolov5m"},
// 	{0.5, 0.5, 0.5, "yolov5l"},
// 	{0.5, 0.5, 0.5, "yolov5x"}
// };

#endif // YOLOV5_H