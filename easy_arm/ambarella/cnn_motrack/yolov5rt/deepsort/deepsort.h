#include <iostream>
#include <opencv2/opencv.hpp>
#include "../../common/datatype.h"
#include "../../common/data_struct.h"
#include "featuretensor.h"
#include "tracker.h"
#include <vector>


//using nvinfer1::ILogger;

// class DLL_API DeepSort {
class DeepSort {
public:    
    //DeepSort(std::string modelPath, int batchSize, int featureDim, int gpuID, ILogger* gLogger);
    DeepSort(std::string modelPath, int batchSize, int featureDim, int gpuID);

    ~DeepSort();

public:
    void sort(cv::Mat& frame, std::vector<DetectBox>& dets);
     
private:
    void sort(cv::Mat& frame, DETECTIONS& detections);
    void sort(cv::Mat& frame, DETECTIONSV2& detectionsv2);    
    void sort(std::vector<DetectBox>& dets);
    void sort(DETECTIONS& detections);
    void init();

private:
    std::string enginePath;
    int batchSize;
    int featureDim;
    cv::Size imgShape;
    float confThres;
    float nmsThres;
    int maxBudget;
    float maxCosineDist;

private:
    std::vector<RESULT_DATA> result;
    std::vector<std::pair<CLSCONF, DETECTBOX>> results;
    tracker* objTracker;
    FeatureTensor* featureExtractor;
    //ILogger* gLogger;
    int gpuID;
};