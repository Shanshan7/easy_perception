/********************************************************************************
 Copyright (c) 2019, Chongqing Unisinsight Technologies Co., Ltd. All rights reserved.
------------------------------------------------------------------------------
* @author:lixi<li.xi@unisinsight.com>
* @date: 2019/04/23
*
* @description: 算法开放接口头文件
*
------------------------------------------------------------------------------
********************************************************************************/
#ifndef __IA_OPEN_APP_H__
#define __IA_OPEN_APP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef void (*LOG_FUNC_CB)(int32_t level, const char *format, ...);
enum LOG_LEVEL_E {
        IALG_FATAL = 0,
        IALG_ERROR,
        IALG_WARNING,
        IALG_INFO,
        IALG_DEBUG
};

// 图像格式类型
typedef enum
{
    IALG_PIX_FMT_UNKOWN,
    IALG_PIX_FMT_BGR,
    IALG_PIX_FMT_NV21,
    IALG_PIX_FMT_GRAY,
    IALG_PIX_FMT_YUV420,
    IALG_PIX_FMT_RGB24,
    IALG_PIX_FMT_NV12,
    IALG_PIX_FMT_YUVJ420P,
    IALG_PIX_FMT_YUVJ422P,  //  JPEG图片解码格式
    IALG_PIX_FMT_CUDA
}IALG_IMAGE_FORMAT_E;

// 支持解码图片格式能力列表
#define	IALG_PIX_FMT_BGR_CAP      ( 0x1ull << 0 )
#define	IALG_PIX_FMT_NV21_CAP     ( 0x1ull << 1 )
#define	IALG_PIX_FMT_GRAY_CAP     ( 0x1ull << 2 )
#define	IALG_PIX_FMT_YUV420_CAP   ( 0x1ull << 3 )
#define	IALG_PIX_FMT_RGB24_CAP    ( 0x1ull << 4 )
#define	IALG_PIX_FMT_NV12_CAP     ( 0x1ull << 5 )
#define	IALG_PIX_FMT_YUVJ420P_CAP ( 0x1ull << 6 )
#define	IALG_PIX_FMT_CUDA_CAP     ( 0x1ull << 7 )

// 系统信息类型
typedef enum
{
    IALG_ALG_PLUGIN_PATH = 0,   // 算法插件根目录绝对路径
}IALG_SYS_INFO_TYPE_E;

// 图片类型
typedef enum
{
    IALG_UNKOWN_IMAGE = 0,                          // 未知图片类型
    IALG_VEHICLE_WIDE_SHOT_IMAGE = 1,               // 车辆大图
    IALG_VEHICLE_LICENSE_PLATE_COLOUR_IMAGE = 2,    // 车牌彩色小图
    IALG_VEHICLE_LICENSE_PLATE_BINARY_IMAGE = 3,    // 车牌二值化图
    IALG_PILOT_FACE_IMAGE = 4,                      // 驾驶员面部特征图
    IALG_COPILOT_FACE_IMAGE = 5,                    // 副驾驶员面部特征图
    IALG_VEHICLE_LOGO_IMAGE = 6,                    // 车标
    IALG_ILLEGAL_VHICLE_COMPOSITION_IMAGE = 7,      // 违章合成图
    IALG_VEHICLE_CROSSING_COMPOSITION_IMAGE = 8,    // 过车合成图
    IALG_VEHICLE_CLOSE_UP_IMAGE = 9,                // 车辆特写图
    IALG_PERSON_IMAGE = 10,                         // 人员图
    IALG_FACE_IMAGE = 11,                           // 人脸图
    IALG_NON_VEHICLE_IMAGE = 12,                    // 非机动车图
    IALG_THING_IMAGE = 13,                          // 物品图
    IALG_SCENE_IMAGE = 14,                          // 场景图
    IALG_GENERAL_IMAGE = 100,                       // 一般图
}IALG_IMAGE_TYPE_E;

// 视频图像信息对象
typedef enum
{
    IALG_UNKOWN_OBJ = 0,
    IALG_VIDEO_SLICE_OBJ,               // 视频片段对象
    IALG_IMAGE_OBJ,                     // 图像对象
    IALG_FILE_OBJ,                      // 文件对象
    IALG_PERSON_OBJ,                    // 人员对象
    IALG_FACE_OBJ,                      // 人脸对象
    IALG_VEHICLE_OBJ,                   // 机动车对象
    IALG_NON_VEHICLE_OBJ,               // 非机动车
    IALG_THING_OBJ,                     // 物品对象
    IALG_SCENE_OBJ,                     // 场景对象
    IALG_VIDEO_CASE_OBJ,                // 视频案对象
    IALG_VIDEO_LABEL_OBJ                // 视频图像标签对象
}IALG_VIDEO_IMAGE_INFO_TYPE_E;

typedef void(*IALG_FREE_MEM)(unsigned char*);

typedef struct
{
    unsigned char* pucBuffer;         /* buffer内容首地址 */
    uint32_t uiBufferLength;          /* buffer长度 */
    IALG_FREE_MEM pfnFreeMem;
}IALG_SYS_INFO_S;

typedef struct
{
	IALG_IMAGE_FORMAT_E enImageFormat; // 图片格式
	uint32_t uiWidth;                  // 图片宽度
	uint32_t uiHeight;                 // 图片高度
	unsigned char* pPlanar[4];         // 图片平面指针
	uint32_t uiStride[4];              // 图片跨距
	uint64_t uiFrameIdx;               // 帧序号
	uint64_t dulTimestamp;             // 时间戳(ms)
	uint64_t ulFrameRate;              // 当前帧率
    unsigned char* pPlanarExt[4];         // 图片平面指针
    uint32_t uiStrideExt[4];              // 图片跨距
    uint32_t uiPoolId;
}IALG_IMAGE_INFO_S;

typedef struct
{
    /* 矩形框信息 */
    uint32_t uiLeftX;                                   // 左上角X坐标
    uint32_t uiLeftY;                                   // 左上角Y坐标
    uint32_t uiWidth;                                  // 宽度
    uint32_t uiHeight;                                 // 高度
}IALG_OBJ_RECT_S;

typedef struct
{
    uint64_t uiObjID;                              // 目标 ID
    IALG_VIDEO_IMAGE_INFO_TYPE_E enObjType;        // 目标类型
    IALG_OBJ_RECT_S stObjRect;                  // 目标框位置信息
}IALG_OBJ_OD_INFO_S;

typedef struct  
{
    uint64_t uiTimeStamp;            // 时间戳
    uint64_t uiFrameIndex;           // 帧序号
    int32_t iTotalObjNum;            // 目标总数
    IALG_OBJ_OD_INFO_S* pstObjInfos; // 目标框位置信息
}IALG_OD_RESULT_S;

typedef struct
{
	IALG_IMAGE_TYPE_E enImageType;                     // 图片类型

    /* 坐标信息 */
    uint32_t uiLftX;                                   // 左上角X坐标
    uint32_t uiLftY;                                   // 左上角Y坐标
    uint32_t uiWidth;                                  // 宽度
    uint32_t uiHeight;                                 // 高度
    char * pPrivateData;                               // 私有数据点位信息等

    /* 图片信息为空，算法需要填坐标位置，由调用层根据坐标抠图 */
    IALG_IMAGE_INFO_S * pstImageInfo;                  // 算法插件生成的图片信息，例如算法插件生成的图片信息
}IALG_OBJ_IMAGE_S;

typedef struct
{
    uint64_t uiObjID;                                  // 目标 ID
    IALG_VIDEO_IMAGE_INFO_TYPE_E  enObjType;           // 目标类型
    uint32_t uiEndFlag;                                // 目标结束志位，0表示识别中，1表示识别结束
    uint32_t uiObjImageNum;                            // 目标对象图片个数（比如机动车对象，可以有车牌照图片和车体图片）
    IALG_OBJ_IMAGE_S* pstObjImageInfo;                 // 目标图片信息
    uint32_t uiAccuracy;                               // 准确度，表示该目标识别的可信 准确度，表示该目标识别的可信程度，取值范围为 [0-100]
    uint32_t uiQuality;                                // 目标质量分数，用于局部目标优选，取值范围为 [0-100]
}IALG_OBJ_INFO_S;

typedef struct
{
    IALG_OBJ_INFO_S stObjInfo;
    char *pcResultJson;               // 算法分析属性结果，数据格式为JSON
    void *pFeature;                   // 目标的特征值所在内存地址
    uint64_t uiFeatureLen;            // 目标特征值占用内存空间长度
    void *pShortFeature;              // 目标短特征值所在的内存地址
    uint64_t uiShortFeatureLen;       // 目标短特征值长度
}IALG_REG_RST_S;

typedef void(*IALG_FREE_REG_RST_MEM)(IALG_REG_RST_S*, int32_t iTotalObjNum);

typedef int32_t (*IALG_GET_SYSTEM_INFO) (IALG_SYS_INFO_TYPE_E  enInfoType,  IALG_SYS_INFO_S* const pstSysInfo);
typedef void (*IALG_ANALYZE_RESULT_CB)(int32_t iChannelNo, int32_t iTotalObjNum, const IALG_REG_RST_S* pstRegRsts,
                                       const IALG_IMAGE_INFO_S *pstImage, void *pPrivateData);
typedef void (*IALG_ANALYZE_RESULT_CB_V1)(int32_t iChannelNo, const IALG_REG_RST_S* pstRegRsts,
                                       const IALG_IMAGE_INFO_S* pstImage, void* pPrivateData);
typedef void (*IALG_GET_OBJECT_ID)(int32_t iChannelNo, IALG_VIDEO_IMAGE_INFO_TYPE_E objectType, char* pcObjectId, int32_t* piObjectIdLen);
typedef int32_t (*IALG_RUN_TIME_INFO_CB)(int32_t iChannelNo, int32_t iRunTimeInfo);
typedef void (*IALG_FREE_IMAGE_INFO_MEM)(const IALG_IMAGE_INFO_S *, void* pPrivateData);
typedef void(*IALG_OD_ANALYZE_RESULT_CB)(int32_t iChannelNo, const IALG_OD_RESULT_S* pstOdResult, const IALG_IMAGE_INFO_S *pstImage);

typedef struct
{
    IALG_GET_SYSTEM_INFO pfnGetSysInfo;
    IALG_ANALYZE_RESULT_CB pfnAnalyzeRstCb;
    IALG_ANALYZE_RESULT_CB_V1 pfnAnalyzeRstCbPerObj;
	IALG_OD_ANALYZE_RESULT_CB pfnAnalyzeOdRstCb;
    IALG_GET_OBJECT_ID pfnStGetObjectId;
	IALG_RUN_TIME_INFO_CB pfnReportRunTimeInfoCb;
	int32_t iGpuCardNo;
	int32_t iMode;

}IALG_CONTEXT;

// 算法插件能力
typedef struct
{
    int32_t iAlgoSupplier;      // 算法厂商
    int32_t iAlgoType;          // 算法类型
    const char *pcVersion;      // 算法版本信息
    IALG_FREE_MEM pfnFreeMem;   // 释放算法版本信息回调
    int32_t iMaxTPS;            // 图片流最大并发数
    int32_t iMaxCVS;            // 视频流最大并发数
    int32_t iITD;               // 图片流解码后图像数据位置
    int32_t iVTD;               // 视频流解码后图像数据位置
    int64_t iImageFmts;         // 图片流所支持解码格式能力列表
    int64_t iVideoFmts;         // 视频流所支持解码格式能力列表
    int32_t iOptimize;          // 是否优选 0：算法不优选 1：算法优选
}IALG_Capability_S;

#define VIAS_SYMBOL_EXPORT __attribute__((visibility("default")))
/*****************************************************************************************************
* @Name   IALG_SetLogFunc
* @Brief  : 设置算法插件日志函数
* @Param  : pfnCb  日志记录回调函数
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
VIAS_SYMBOL_EXPORT int32_t IALG_SetLogFunc(const LOG_FUNC_CB pfnCb);

/*****************************************************************************************************
* @Name   IALG_StartupEngine
* @Brief  : 启动算法分析引擎
* @Param  : pstCtx  上下文信息
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
VIAS_SYMBOL_EXPORT int32_t IALG_StartupEngine(IALG_CONTEXT stCtx);

/*****************************************************************************************************
* @Name   IALG_GetCapability
* @Brief  : 获取算法插件运算能力
* @Param  : OUT pAmpCap  算法能力
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
VIAS_SYMBOL_EXPORT int32_t IALG_GetCapability(IALG_Capability_S *const pAmpCap);

/*****************************************************************************************************
* @Name   IALG_AnalyzeImage
* @Brief  : 异步图片流分析
* @Param  : pstImgInfo    图片信息
* @Param  : pcConfigJson  图片参数配置
* @Param  : pPrivateData  软件框架传入的私有数据
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
VIAS_SYMBOL_EXPORT int32_t IALG_AnalyzeImage(const IALG_IMAGE_INFO_S * pstImgInfo, const char * pcConfigJson, void* pPrivateData);

/*****************************************************************************************************
* @Name   IALG_AnalyzeImage_V1
* @Brief  : 视频流特征
* @Param  : pstImgInfo    图片信息
* @Param  : pcConfigJson  图片参数配置
* @Param  : pPrivateData  软件框架传入的私有数据
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
VIAS_SYMBOL_EXPORT int32_t IALG_AnalyzeImage_V1(const IALG_IMAGE_INFO_S* pstImgInfo, const char* pcConfigJson, IALG_FREE_IMAGE_INFO_MEM pfnFreeMem, void* pPrivateData);



/*****************************************************************************************************
* @Name   IALG_AnalyzeImage_V2
* @Brief  : 视频流特征
* @Param  : pstImgInfo    图片信息
* @Param  : pcConfigJson  图片参数配置
* @Param  : pPrivateData  软件框架传入的私有数据
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
VIAS_SYMBOL_EXPORT int32_t IALG_AnalyzeImage_V2(const IALG_IMAGE_INFO_S* pstImgInfo,
                                                uint32_t uiImgNum,
                                                const char* pcConfigJson,
                                                IALG_FREE_IMAGE_INFO_MEM pfnFreeMem,
                                                void* pPrivateData);



/*****************************************************************************************************
* @Name   IALG_AnalyzeImageSync
* @Brief  : 同步图片流分析
* @Param  : pstImgInfo    图片信息
* @Param  : pcConfigJson  图片参数配置
* @Param  : OUT pTotalObjNum 识别目标数
* @Param  : OUT pstRegRsts   识别目标结果
* @Param  : OUT pfnFreeMem   算法插件提供的内存释放函数
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
VIAS_SYMBOL_EXPORT int32_t IALG_AnalyzeImageSync(const IALG_IMAGE_INFO_S * pstImgInfo, const char * pcConfigJson, int32_t* pTotalObjNum,
                                                 IALG_REG_RST_S** ppstRegRsts, IALG_FREE_REG_RST_MEM* pfnFreeMem);

/*****************************************************************************************************
* @Name   IALG_CreateChannel
* @Brief  : 申请通道资源
* @Return : 大于等0,表示返回对应的资源通道号， 失败:errorcode
******************************************************************************************************/
VIAS_SYMBOL_EXPORT int32_t IALG_CreateChannel(void);

/*****************************************************************************************************
* @Name   IALG_SetChannelConfig
* @Brief  : 设置通道配置
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
VIAS_SYMBOL_EXPORT int32_t IALG_SetChannelConfig(int32_t iChannelNo, const char * pcConfigJson);

/*****************************************************************************************************
* @Name   IALG_PutFrame
* @Brief  : 向指定通道推送解码后的数据
* @Param  : iChannelNo       算法通道号
* @Param  : pstImgIALG_INFO  图片信息
* @Param  : pfnFreeMem       释放图片数据的函数指针(算法优选时使用；算法不优选时为NULL，不使用)
* @Param  : pPrivateData     框架传进的私有数据，回调时透传回去
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
VIAS_SYMBOL_EXPORT int32_t IALG_PutFrame(int32_t iChannelNo, const IALG_IMAGE_INFO_S * pstImgInfo, IALG_FREE_IMAGE_INFO_MEM pfnFreeMem, void* pPrivateData);

/*****************************************************************************************************
* @Name   IALG_ReleaseChannel
* @Brief  : 释放通道
* @Param  : iChannelNo  通道号
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
VIAS_SYMBOL_EXPORT int32_t IALG_ReleaseChannel(int32_t iChannelNo);

/*****************************************************************************************************
* @Name   IALG_ShutdownEngine
* @Brief  : 关闭算法分析引擎
* @Param  : iMode  引擎模式	1:代表图片流 2：代表视频流
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
VIAS_SYMBOL_EXPORT int32_t IALG_ShutdownEngine(int32_t iMode);

/*****************************************************************************************************
* @Name   IALG_SetLicense
* @Brief  : 设置算法LICENSE信息
* @Param  : const char* pcLicenseInfo  LICENSE授权 服务地址信息，包含服务器地址
* @Param  : uint32_t uiLen             LICENSE信息长度
* @Return : 成功:0, 失败:errorcode
******************************************************************************************************/
VIAS_SYMBOL_EXPORT int32_t IALG_SetLicense(const char* pcLicenseInfo, uint32_t uiLen);

#ifdef __cplusplus
}
#endif

#endif
