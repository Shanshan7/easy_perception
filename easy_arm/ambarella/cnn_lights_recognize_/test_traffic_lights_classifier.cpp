#include "ialg_error_code.h"
#include "ialg_open_app.h"
#include "analyser.h"
#include "json11.hpp"

#include "traffic_lights_classifier.h"

#include <memory>
#include <iostream>

#include <time.h>
#include <stdarg.h>
#include <string.h>


int main(int argc, char** argv)
{
    int rval = 0;

    if(argc != 2)
	{
        printf("usage: ./test_yolov5rt image_name\n");
        exit(0);
    }

    /* 设置日志回调函数 */
    IALG_SetLogFunc(IALG_DEBUG);

    /* 创建算法分析引擎 */
    IALG_CONTEXT stAmpCtx;
    IALG_CreateEngine(&stAmpCtx);
    /* 获取算法分析能力 */
    IALG_Capability stCap;
    IALG_GetCapability(&stCap);
    /*
    ** accomplished by software framework.
    ** TODO: open data stream connection.
    - 43 -*/
    /* 申请视频流分析通道 */
    int32_t iChlNo = -1;
    const char pcConfigJson[] = ”
    {/"roi/":[{/"objType/":4,/"leftTopX/":12,/"leftTopY/":25,/"rightBtmX/":50,/"rightBtmY/":562}]}”;
    int32_t iChlNo = IALG_CreateChannel();
    /* 分析通道参数配置 */
    rval = IALG_SetChannelConfig(iChlNo, pcConfigJson);
    do {
        /*
        ** accomplished by software framework.
        ** TODO: fetch origin data.
        */
        /*
        ** accomplished by software framework.
        ** TODO: decode data into YUV/NV12 format.
        */
        IALG_IMAGE_INFO_S stImg;
        IALG_FREE_IMAGE_INFO_MEM pfnFreeMem;
        rval = IALG_PutFrame(iChlNo, &stImg, pfnFreeMem, NULL);
        /* our algorithm to detect traffic lights */
        cv::Mat rgb_image;
        TrafficLightsClassifier traffic_lights_classifier;

        rgb_image = cv::imread(argv[1]);
        traffic_lights_classifier.red_green_yellow(rgb_image);


        /*
        ** accomplished by software framework.
        ** TODO: break the loop when the data stream is closed.
        */
    } while (true);

    /* 释放视频流分析通道 */
    IALG_ReleaseChannel(iChlNo);
    /* 销毁算法分析引擎 */
    IALG_ShutdownEngine();

    return rval;
}