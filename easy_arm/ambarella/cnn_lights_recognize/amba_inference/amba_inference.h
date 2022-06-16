#pragma once
#include <unistd.h>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string.h>

#include "net.h"
#include "json/json.h"

class Amba_Inference
{
public:
    Amba_Inference();
    ~Amba_Inference();
    int AmbaEntry();
    void AmbaExit(NET_INFO_ST* pstNet);
    int Init(NET_INFO_ST* pstNet);
    void PostProcess(NET_INFO_ST* pstNet, std::vector<float>& output);
    void LoadImgFile(IN cv::Mat& img, OUT IMAGE_INFO_ST* pstImage);
    void usage();
    int Net(IN cv::Mat& img, OUT std::vector<float>& output);
    std::vector<float> amba_pred(cv::Mat img,std::string amba_path);
    
private:
    /* 模型信息，在编译之前要自定义修改 */
    static int   netInNum  ;                                           //输入层个数
    static char  netInName[NET_IN_MAX][STRING_MAX] ; //输入层名
    static int   netOutNum ;                                           //输出层个数
    static char  netOutName[NET_OUT_MAX][STRING_MAX] ;          //输出层名
    static char  netFile[STRING_MAX] ;       //模型路径

    /* vproc.bin路径，在编译之前要自定义修改 */
    static char VPROC_BIN_PATH[STRING_MAX] ;

    static int             fdCavalry ; // cavalry设备句柄，不要改
    static uint8_t         verbose  ;
    static int             nnCnt     ;  // 算子计数器，退出安霸环境时需要
    static int             fdFlag    ;  // cavalry设备句柄
    static struct net_mem  stBinMem  ;// vproc.bin

    std::string amba_path;
    std::string json_path="/home/lpj/Desktop/easy_perception/easy_arm/ambarella/cnn_lights_recognize/hyp.json";
};
