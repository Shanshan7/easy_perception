#include "ialg_error_code.h"
#include "ialg_open_app.h"
#include "analyser.h"
#include "json11.hpp"

#include <memory>
#include <iostream>

#include <time.h>
#include <stdarg.h>
#include <string.h>

static void printFunc(int32_t level, const char *format, ...)
{
    char logBuf[1024];
    memset(&logBuf, 0, sizeof(logBuf) / sizeof(logBuf[0]));

    va_list valist;
    va_start(valist, format);
    vsnprintf(logBuf, sizeof(logBuf) / sizeof(logBuf[0]) - 1, format, valist);
    va_end(valist);

    std::cout << logBuf << std::endl;
}

int32_t IALG_SetLogFunc(const LOG_FUNC_CB pfnCb)
{
    analyser::instance().registerPrintCb(printFunc);
    return 0;
}

/*****************************************************************************************************
* @Name   IALG_StartupEngine
* @Brief  : 启动算法分析引擎
* @Param  : pstCtx  上下文信息
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
int32_t IALG_StartupEngine(IALG_CONTEXT stCtx)
{
    auto &als = analyser::instance();
    als.registerOdCb(stCtx.pfnAnalyzeOdRstCb);
    als.registerOaCb(stCtx.pfnAnalyzeRstCbPerObj);
    als.initQueue(10);

    return 0;
}

/*****************************************************************************************************
* @Name   IALG_GetCapability
* @Brief  : 获取算法插件运算能力
* @Param  : OUT pAmpCap  算法能力
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
int32_t IALG_GetCapability(IALG_Capability_S *const pAmpCap)
{
    pAmpCap->iAlgoSupplier = 5;        // 算法厂商
    pAmpCap->iAlgoType = 47;           // 算法类型
    pAmpCap->pcVersion = "0.0.1"; // 算法版本信息
    // pAmpCap->pfnFreeMem = nullptr;   // 释放算法版本信息回调
    // pAmpCap->iMaxTPS = 1;            // 图片流最大并发数
    // pAmpCap->iMaxCVS = 1;            // 视频流最大并发数
    // pAmpCap->iITD;               // 图片流解码后图像数据位置
    // pAmpCap->iVTD;               // 视频流解码后图像数据位置
    // pAmpCap->iImageFmts;         // 图片流所支持解码格式能力列表
    // pAmpCap->iVideoFmts;         // 视频流所支持解码格式能力列表
    // pAmpCap->iOptimize;          // 是否优选 0：算法不优选 1：算法优选

    return 0;
}

/*****************************************************************************************************
* @Name   IALG_AnalyzeImage_V1
* @Brief  : 视频流特征
* @Param  : pstImgInfo    图片信息
* @Param  : pcConfigJson  图片参数配置
* @Param  : pPrivateData  软件框架传入的私有数据
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
int32_t IALG_AnalyzeImage_V1(const IALG_IMAGE_INFO_S *pstImgInfo, const char *pcConfigJson, IALG_FREE_IMAGE_INFO_MEM pfnFreeMem, void *pPrivateData)
{
    return 0;
}

/*****************************************************************************************************
* @Name   IALG_CreateChannel
* @Brief  : 申请通道资源
* @Return : 大于等0,表示返回对应的资源通道号， 失败:errorcode
******************************************************************************************************/
int32_t IALG_CreateChannel(void)
{
    return 0;
}

/*****************************************************************************************************
* @Name   IALG_SetChannelConfig
* @Brief  : 设置通道配置
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
int32_t IALG_SetChannelConfig(int32_t iChannelNo, const char *pcConfigJson)
{
    return 0;
}

/*****************************************************************************************************
* @Name   IALG_PutFrame
* @Brief  : 向指定通道推送解码后的数据
* @Param  : iChannelNo       算法通道号
* @Param  : pstImgIALG_INFO  图片信息
* @Param  : pfnFreeMem       释放图片数据的函数指针(算法优选时使用；算法不优选时为NULL，不使用)
* @Param  : pPrivateData     框架传进的私有数据，回调时透传回去
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
int32_t IALG_PutFrame(int32_t iChannelNo, const IALG_IMAGE_INFO_S *pstImgInfo, IALG_FREE_IMAGE_INFO_MEM pfnFreeMem, void *pPrivateData)
{
    static uint32_t objId = 0;

    analyser::instance().addTask([=]() {
        // 1.对图像进行分析

        // 2.根据分析结果填充结果字段
        json11::Json resultJson = json11::Json::object {
            {"EventSort", "0x02050001"},
            {"VideoLabelID", "VideoLabelID"},
            {"CreateTimeAbs", (int)(time(nullptr))},
            {"BehaviorBeginTime", "BehaviorBeginTime"},
            {"AlgoOptimization", "AlgoOptimization"},
            {"Abandoned", "Abandoned"},
            {"BackType", "BackType"}
        };
        auto jsonstr = resultJson.dump();

        auto spOaRes = std::make_shared<IALG_REG_RST_S>();
        spOaRes->pcResultJson = const_cast<char *>(jsonstr.c_str());

        IALG_OBJ_IMAGE_S ImageInfo;
        ImageInfo.uiLftX = 100;
        ImageInfo.uiLftY = 200;
        ImageInfo.uiWidth = 400;
        ImageInfo.uiHeight = 400;

        auto &objInfo = spOaRes->stObjInfo;
        objInfo.pstObjImageInfo = &ImageInfo;
        objInfo.uiObjImageNum = 1;
        objInfo.uiObjID = ++objId;
        objInfo.enObjType = IALG_FACE_OBJ;

        analyser::instance().pushOaResult(iChannelNo, spOaRes.get(), pstImgInfo, pPrivateData);

        if (pfnFreeMem)
        {
            pfnFreeMem(pstImgInfo, pPrivateData);
        }
    });

    return 0;
}

/*****************************************************************************************************
* @Name   IALG_ReleaseChannel
* @Brief  : 释放通道
* @Param  : iChannelNo  通道号
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
int32_t IALG_ReleaseChannel(int32_t iChannelNo)
{
    return 0;
}

/*****************************************************************************************************
* @Name   IALG_ShutdownEngine
* @Brief  : 关闭算法分析引擎
* @Param  : iMode  引擎模式	1:代表图片流 2：代表视频流
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
int32_t IALG_ShutdownEngine(int32_t iMode)
{
    return 0;
}
