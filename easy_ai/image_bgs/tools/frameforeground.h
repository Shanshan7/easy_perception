#ifndef FRAMEFOREGROUND_H
#define FRAMEFOREGROUND_H

#include <vector>
#include "image_bgs/bgs/MixtureOfGaussianV2BGS.h"
#include "image_bgs/bgs/PixelBasedAdaptiveSegmenter.h"
#include "image_bgs/bgs/vibebgs.h"

class FrameForeground
{
public:
    FrameForeground();
    ~FrameForeground();

    void initData();//初始化参数

    std::vector<cv::Rect> getFrameForegroundRect(const cv::Mat& inFrame,float minBox=250.0f);//处理视频帧得到前景目标
    std::vector<cv::Point2f> getFrameForegroundCenter(const cv::Mat& inFrame,float minBox=250.f);//处理视频帧得到前景目标的中心
    std::vector<cv::Point2f> getFrameForegroundCentroid(const cv::Mat& inFrame,float minBox=250.0f);//处理视频帧得到目标的质心

    //得到前景图像
    void getFrameForeground(const cv::Mat& inFrame, cv::Mat& foregroundFrame);
    //得到目标的多边形区域前景图像
    void getFrameForeground(const cv::Mat& inFrame, cv::Mat &foregroundFrame, float minBox);

private:
    IBGS *bgs;//背景分割算法
    bool isFirstRun;//第一次运行
    std::string bgsName;//背景分割算法名称
    std::vector< std::vector<cv::Point> > objectContours;//目标多边形轮廓
    cv::Mat elementBGS;//自定义核，用于背景检测形态学操作

    int flags;//使用前景提取的算法
    bool enableMorphology;//是否进行形态学操作
    int elementSizeBGS;//背景检测形态学操作尺寸大小
    int cannyThreshold;//canny算子的低阈值
    bool enableShowForeground;//是否显示前景

private:
    //计算目标的多边形轮廓
    void calculateFrameForegroundContours(const cv::Mat& inFrame);

    void init();
    void saveConfig();
    void loadConfig();
};

#endif // FRAMEFOREGROUND_H
