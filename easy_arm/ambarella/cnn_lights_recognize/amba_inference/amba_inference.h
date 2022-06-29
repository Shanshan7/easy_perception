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
    int get_input_pitch(net_input_cfg* pstNetIn);
    cv::Size get_input_size(net_input_cfg* pstNetIn);
    void LoadAndPreprocess(IN cv::Mat& img,OUT struct net_input_cfg* pstNetIn);
    int Net(IN cv::Mat& img, OUT std::vector<float>& output);
    std::vector<float> amba_pred(cv::Mat img,std::string amba_path);
    
private:
    /* 模型信息，在编译之前要自定义修改 */
    int   netInNum  ;                                           //输入层个数
    char  netInName[NET_IN_MAX][STRING_MAX] ; //输入层名
    int   netOutNum ;                                           //输出层个数
    char  netOutName[NET_OUT_MAX][STRING_MAX] ;          //输出层名
    char  netFile[STRING_MAX] ;       //模型路径

    /* vproc.bin路径，在编译之前要自定义修改 */
    char VPROC_BIN_PATH[STRING_MAX] ;

    int             fdCavalry ; // cavalry设备句柄，不要改
    uint8_t         verbose  ;
    int             nnCnt     ;  // 算子计数器，退出安霸环境时需要
    int             fdFlag    ;  // cavalry设备句柄
    struct net_mem  stBinMem  ;// vproc.bin

    std::string amba_path;
    std::string json_path="/sdcard/hyp.json";
};
