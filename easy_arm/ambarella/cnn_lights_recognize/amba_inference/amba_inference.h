#pragma once
#include <unistd.h>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string.h>
#include "net.h"
#include <iostream>

class Amba_Inference
{
public:
    Amba_Inference();
    ~Amba_Inference();
    int AmbaEntry()；
    void AmbaExit(NET_INFO_ST* pstNet)；
    int Init(NET_INFO_ST* pstNet)；
    void PostProcess(NET_INFO_ST* pstNet, vector<float>& output)；
    void LoadImgFile(IN Mat& img, OUT IMAGE_INFO_ST* pstImage)；
    int Net(IN Mat& img, OUT vector<float>& output)；
    std::vector<float> amba_pred();
    
private：
    /* 模型信息，在编译之前要自定义修改 */
    static int   netInNum  = 2;                                           //输入层个数
    static char  netInName[NET_IN_MAX][STRING_MAX] = {"data", "data_uv"}; //输入层名
    static int   netOutNum = 1;                                           //输出层个数
    static char  netOutName[NET_OUT_MAX][STRING_MAX] = {"prob"};          //输出层名
    static char  netFile[STRING_MAX] = "cavalry_googlenet_yuv.bin";       //模型路径

    /* vproc.bin路径，在编译之前要自定义修改 */
    static char VPROC_BIN_PATH[STRING_MAX] = "/usr/local/vproc/vproc.bin";

    static int             fdCavalry = -1; // cavalry设备句柄，不要改
    static uint8_t         verbose  = 0;
    static int             nnCnt     = 0;  // 算子计数器，退出安霸环境时需要
    static int             fdFlag    = 0;  // cavalry设备句柄
    static struct net_mem  stBinMem  = {0};// vproc.bin
}
