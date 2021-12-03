#ifndef OPENPOSE_POSTPORCESS_H
#define OPENPOSE_POSTPORCESS_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>

struct KeyPoint{
    KeyPoint(cv::Point point,float probability){
        this->id = -1;
        this->point = point;
        this->probability = probability;
    }

    int id;
    cv::Point point;
    float probability;
};

struct ValidPair{
    ValidPair(int aId,int bId,float score){
        this->aId = aId;
        this->bId = bId;
        this->score = score;
    }

    int aId;
    int bId;
    float score;
};

void getOpenposeResult(const std::vector<cv::Mat>& netOutputParts, std::vector<std::vector<KeyPoint>> &result);

#endif // POESNET_POSTPORCESS_H
