#include "ialg_error_code.h"
#include "ialg_open_app.h"

#include "ctx.h"
#include "channel.h"

#include "json11.hpp"
#include "image.h"
#include "traffic_lights_classifier.h"

#include <memory>
#include <iostream>

#include <time.h>
#include <stdarg.h>
#include <string.h>

/**
 * @brief
            IALG_系列接口都由算法引擎来调用,调用顺序如下
            (同层的接口如果不带序号表明无顺序限制)
            1. IALG_SetLogFunc
            2. IALG_StartupEngine
               IALG_GetCapability
               IALG_AnalyzeImage_V1
      
               a. IALG_CreateChannel
                    IALG_PutFrame
                    IALG_SetChannelConfig
               b. IALG_ReleaseChannel

            3. IALG_ShutdownEngine
 * @note
 * @param  pfnCb:
 * @retval
 */

static const int MAX_SUPPORT_CHANNEL = 3;
static Detail::ChannelManager *gpt_chnManager{ nullptr };
auto &ctx = Detail::Ctx::instance();

int32_t IALG_SetLogFunc(const LOG_FUNC_CB pfnCb)
{
    ctx.registerLogFunc(pfnCb);
    return IALG_OK;
}

/*****************************************************************************************************
* @Name   IALG_StartupEngine
* @Brief  : 启动算法分析引擎
* @Param  : pstCtx  上下文信息
*           包括一些回调信息
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
int32_t IALG_StartupEngine(IALG_CONTEXT stCtx)
{
    ctx.log()(0, "[%s:%d] start init", __func__, __LINE__);

    ctx.registerOdFunc(stCtx.pfnAnalyzeOdRstCb);     //保存目标检测结果回调
    ctx.registerOaFunc(stCtx.pfnAnalyzeRstCbPerObj); //保存目标属性分析结果回调
    ctx.initQueue(10); //初始化任务队列

    gpt_chnManager = new Detail::ChannelManager(MAX_SUPPORT_CHANNEL);

    ctx.log()(0, "[%s:%d] complete init", __func__, __LINE__);

    return IALG_OK;
}

/*****************************************************************************************************
* @Name   IALG_GetCapability
* @Brief  : 获取算法插件运算能力
* @Param  : OUT pAmpCap  算法能力
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
int32_t IALG_GetCapability(IALG_Capability_S *const pAmpCap)
{
    pAmpCap->iAlgoSupplier = 5;    // 算法厂商
    pAmpCap->iAlgoType = 47;       // 算法类型
    pAmpCap->pcVersion = "0.0.1";  // 算法版本信息
    // pAmpCap->pfnFreeMem = nullptr; // 释放算法版本信息回调
    // pAmpCap->iMaxTPS = 1;          // 图片流最大并发数
    // pAmpCap->iMaxCVS = 1;          // 视频流最大并发数
    // pAmpCap->iITD;                 // 图片流解码后图像数据位置
    // pAmpCap->iVTD;                 // 视频流解码后图像数据位置
    // pAmpCap->iImageFmts;           // 图片流所支持解码格式能力列表
    // pAmpCap->iVideoFmts;           // 视频流所支持解码格式能力列表
    // pAmpCap->iOptimize;            // 是否优选 0：算法不优选 1：算法优选

    return IALG_OK;
}



/*****************************************************************************************************
* @Name   IALG_CreateChannel
* @Brief  : 申请通道资源
* @note   :
            由算法包内部创建一个分析通道,返回的通道号可以唯一标识该通道
            算法引擎根据该通道号进行操作：通道配置IALG_SetChannelConfig、推图IALG_PutFrame、释放通道IALG_ReleaseChannel
* @Return : 大于等0,表示返回对应的资源通道号， 失败:errorcode
******************************************************************************************************/
int32_t IALG_CreateChannel(void)
{
    return gpt_chnManager->allocChn();
}

/*****************************************************************************************************
* @Name   IALG_ReleaseChannel
* @Brief  : 释放通道
* @Param  : iChannelNo  通道号
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
int32_t IALG_ReleaseChannel(int32_t iChannelNo)
{
    gpt_chnManager->releaseChn(iChannelNo);
    return IALG_OK;
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
    // pstImgInfo->enImageFormat;//图片格式
    // pstImgInfo->uiWidth;//图片宽
    // pstImgInfo->uiHeight;//图片高
    // pstImgInfo->pPlanar[0];//Y分量的虚拟地址
    // pstImgInfo->pPlanar[1];//UV分量的虚拟地址
    // pstImgInfo->uiStride[0];//stride0
    // pstImgInfo->uiStride[1];//stride1
    // pstImgInfo->pPlanarExt[0];//Y分量的物理地址
    // pstImgInfo->pPlanarExt[1];//UV分量的物理地址
    // pstImgInfo->dulTimestamp;//帧的时间戳(单位ms)
    // pstImgInfo->uiFrameIdx;//帧号
    // pstImgInfo->ulFrameRate;//帧率
    // pstImgInfo->uiPoolId;//缓冲池ID，-1-非缓冲池图像内存块

    Image image_process;
    TrafficLightsClassifier traffic_lights_classifier;

    std::string err;
    auto js = json11::Json::parse(pcConfigJson, err);
    std::vector<std::vector<float>> dect_region_vector; 
    json11::Json::array dect_region_array = js["dect_region"].array_items(); 
    dect_region_vector.resize(dect_region_array.size()); 
    for (int i = 0; i < dect_region_array.size(); i++) 
    {
        json11::Json::array tmp = dect_region_array[i].array_items();  
        dect_region_vector[i].resize(tmp.size());  
        for (int j = 0; j < tmp.size(); j++) 
        { 
            dect_region_vector[i][j] = tmp[j].int_value();  
        }
    } 
    int32_t f_threshold = js["threshold"].int_value();

    cv::Mat input_image;
    image_process.IALG_to_mat(pstImgInfo, input_image);

    for (int i = 0; i < dect_region_vector.size(); i++)
    {
        traffic_lights_classifier.red_green_yellow(input_image, dect_region_vector[i], f_threshold);
    }

    // 以上传入的关于图像的信息,可按需使用,参数说明和注意事项详见<<华智解析平台多算法SDK开放接口描述>>

    ctx.log()(0, "[%s:%d] async analyze image", __func__, __LINE__);//根据需要选择数据做计算

    if (ctx.addTask([=]() {
        // 此处为算法实现
        // 推入数据队列后立即返回，
        // 算法内部的数据处理由另外的线程完成

        // 根据分析结果填充结果字段
        json11::Json::array resultJson_array;
        for (int k = 0; k < traffic_lights_classifier.traffic_lights_results.size(); k++)
        {
            json11::Json resultJson_object = json11::Json::object{
                {"target_id", traffic_lights_classifier.traffic_lights_results[k].target_id},
                {"target_type", traffic_lights_classifier.traffic_lights_results[k].traffic_lights_type},
                {"target_rect_height", traffic_lights_classifier.traffic_lights_results[k].traffic_lights_location[2]},
                {"target_rect_width", traffic_lights_classifier.traffic_lights_results[k].traffic_lights_location[3]},
                {"target_rect_x", traffic_lights_classifier.traffic_lights_results[k].traffic_lights_location[0]},
                {"target_rect_y", traffic_lights_classifier.traffic_lights_results[k].traffic_lights_location[1]},
            };
            resultJson_array.push_back(resultJson_object);
        }
        
        json11::Json resultJson(resultJson_array);
        auto jsonstr = resultJson.dump();

        auto spResult = std::make_shared<IALG_REG_RST_S>();
        spResult->pcResultJson = const_cast<char *>(jsonstr.c_str());

        // IALG_OBJ_IMAGE_S ImageInfo;
        // ImageInfo.uiLftX = 100;
        // ImageInfo.uiLftY = 200;
        // ImageInfo.uiWidth = 300;
        // ImageInfo.uiHeight = 400;

        // auto &objInfo = spResult->stObjInfo;
        // objInfo.pstObjImageInfo = &ImageInfo;
        // objInfo.uiObjImageNum = 1;
        // objInfo.uiObjID = ++objId;
        // objInfo.enObjType = IALG_IMAGE_OBJ;

        // 此处根据具体分析结论,填充spResult
        ctx.pushOaResult(-1, spResult.get(), pstImgInfo, pPrivateData);//将分析结果spResult通过注册的回调推给算法引擎

        // 须确保资源释放,如果有结果要推送,应该在推送后释放资源
        if (pfnFreeMem)
        {
            pfnFreeMem(pstImgInfo, pPrivateData);
        }
    }))
    {
        return IALG_NOK;
    }

    // 推入数据队列失败时,须确保资源释放
    if (pfnFreeMem)
    {
        pfnFreeMem(pstImgInfo, pPrivateData);
    }
    return IALG_OK;
}

/*****************************************************************************************************
* @Name   IALG_SetChannelConfig
* @Brief  : 设置通道配置
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
int32_t IALG_SetChannelConfig(int32_t iChannelNo, const char *pcConfigJson)
{
    return gpt_chnManager->channalHandle(iChannelNo, [=](Detail::AlgoChannel &chn) {
        return chn.setConfig(pcConfigJson);
    });
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
    return gpt_chnManager->channalHandle(iChannelNo, [=](Detail::AlgoChannel &chn) {
        return chn.putFrame(pstImgInfo, pfnFreeMem, pPrivateData);
    });
}

/*****************************************************************************************************
* @Name   IALG_ShutdownEngine
* @Brief  : 关闭算法分析引擎
* @Param  : iMode  引擎模式	1:代表图片流 2：代表视频流
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
int32_t IALG_ShutdownEngine(int32_t iMode)
{
    // 去初始化操作,IALG_StartupEngine()的反操作
    if (gpt_chnManager)
    {
        delete gpt_chnManager;
        gpt_chnManager = nullptr;
    }
    ctx.deinitQueue();
    return IALG_OK;
}
