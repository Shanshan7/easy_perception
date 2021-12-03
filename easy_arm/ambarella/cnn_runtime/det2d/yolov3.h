/*
  The following source code derives from Darknet
*/

#ifndef YOLOV3_H__
#define YOLOV3_H__

#include <algorithm>
#include <iomanip>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <math.h>

std::vector<std::vector<float>> yolo_run(const int classNumber, float* node0, float* node1, float* node2);

int entry_index(int loc, int anchorC, int w, int h, int lWidth, int lHeight);
float overlap(float x1, float w1, float x2, float w2);
float cal_iou(std::vector<float> box, std::vector<float>truth);

//for amba version
void detect(std::vector<std::vector<float>> &boxes, const float* lOutput, \
			      int lHeight, int lWidth, int num, \
			      int sHeight, int sWidth);
std::vector<std::vector<float>> applyNMS(std::vector<std::vector<float>>& boxes,
							                           const float thres, bool sign_nms=true);

#endif  //YOLOV3_H__
