#ifndef _IDM_NET_SDK_INTERFACE_H_
#define _IDM_NET_SDK_INTERFACE_H_
#include "idm_netsdk.h"

/* 函数接口 */
#ifdef __cplusplus
extern "C" {
#endif
typedef enum tagIDM_VMS_EXTRACTFEATURE_IMGFORMAT
{
    IMG_OTHER = 0,
    IMG_BMP,
    IMG_GIF,
    IMG_JPEG,
    IMG_JIFF,
    IMG_KDC,
    IMG_PCD,
    IMG_PCX,
    IMG_PIC,
    IMG_PIX,
    IMG_PNG,
    IMG_PSD,
    IMG_TAPGA,
    IMG_TIFF,
    IMG_WMF,
    IMG_JP2
}IDM_VMS_EXTRACTFEATURE_IMGFORMAT_S;

/* 提取特征值-图片信息*/
typedef struct tagIDM_VMS_EXTRACTFEATURE_IMGINFO
{
    UCHAR szImgID[32];                      /* 图片ID*/
    ULONG ulImgType;                        /* 图片格式 IDM_VMS_EXTRACTFEATURE_IMGFORMAT_S*/
    VOID *pImgBuff;                         /* 图片数据指针*/
    ULONG ulImgLen;                         /* 图片数据长度*/
    UCHAR aucRes[32];                       /* 保留*/
}IDM_VMS_EXTRACTFEATURE_IMGINFO_S;
/* 提取特征值-特征值信息*/
typedef struct tagIDM_VMS_EXTRACTFEATURE_FEATUREINFO
{
    UCHAR szImgID[32];                      /* 图片ID*/
    UCHAR szAlgModel[16];                   /* 解析算法版本*/
    VOID *pFeatureBuff;                     /* 特征值数据存放指针，空间由用户申请*/
    ULONG ulFeatureSize;                    /* 特征值数据存放空间大小*/
    ULONG ulFeatureLen;                     /* 特征值数据长度*/
    UCHAR aucRes[32];                       /* 保留*/
}IDM_VMS_EXTRACTFEATURE_FEATUREINFO_S;


/* 回放条件参数结构体 */
typedef struct tagIDM_OCX_PLAYBACK_COND
{
    UCHAR  ucRecordType;                        /* 回放录像类型：0-所有录像 1-常规录像 2-移动侦测 3-报警录像 */  
    IDM_DEV_PLAYBACK_COND_NAME_S stNameCond;    /* 文件名条件 */
    IDM_DEV_PLAYBACK_COND_TIME_S stTimeCond;    /* 时间条件 */
}IDM_OCX_PLAYBACK_COND_S;

/* 数据透传接收到的响应数据 */
typedef struct tagIDM_DATA_PASSTHROUGH_RECV_INFO
{
    UINT     uiRecvSize;                           /* 接收到的返回数据大小 */
    CHAR     *pcRecvBuf;                           /* 接收到的返回数据,
                                                      透传json option接口返回到此处的是响应json,
                                                      透传完整私有协议接口返回到此处的是完整的私有协议响应数据 */
    UCHAR    aucRes[200];
}IDM_DATA_PASSTHROUGH_RECV_INFO_S;

typedef struct tagIDM_BIN_DATA
{
	CHAR* pcData; //数据指针
	UINT uiSize; //数据长度
	USHORT usType; //pcData中的数据类型,1:json, 2:二进制.
	USHORT usSubType;//pcData中的数据子类型. 0:通用二进制,其它类型参考UNCP文档
}IDM_BIN_DATA_S;

/*json + N个进制透传参数*/
typedef struct tagIDM_TRANSPARENT_JSON_BIN
{
	IDM_BIN_DATA_S stJson; //json数据
	IDM_BIN_DATA_S *pstBinData; //N个二进制数据
	UINT uiBinCount; //二进制数据个数
}IDM_TRANSPARENT_JSON_BIN_S;

typedef struct tagIDM_TRANSPARENT_SINGLE_BIN
{
	USHORT usModule;  //转发到目标模块标识
	USHORT usSubModule; //转到目标子模块
	IDM_BIN_DATA_S stData; //二进制数据
}IDM_TRANSPARENT_SINGLE_BIN_S;

IDM_API LONG IDM_DEV_SetupSubLink(LONG lUserID, LONG *plSessionID);

/*
*@brief: VMS申请实时流
*@param: IN lUserID 设备句柄
*@param: IN pstPlayBackCond 实时流相关参数
*@param: OUT plSessionID 申请的子链接校验ID
*@param: OUT pMethodStr 请求实时流JSON字符串，字符串内存由调用者分配，推荐256字节
*@param: IN/OUT plStrLen IN内存长度，OUT传出字符串长度
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_VMS_SetupStreamLink(LONG lUserID, IDM_DEV_PREVIEW_INFO_S *pstPreviewInfo, LONG *plSessionID, CHAR* pMethodStr, LONG *plStrLen);

/*
*@brief: VMS申请回放流
*@param: IN lUserID 设备句柄
*@param: IN pstPlayBackCond 回放相关参数
*@param: OUT plSessionID 申请的子链接校验ID
*@param: OUT pMethodStr 请求实时流JSON字符串，字符串内存由调用者分配，推荐512字节
*@param: IN/OUT plStrLen IN内存长度，OUT传出字符串长度
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_VMS_SetupPlayBackLink(LONG lUserID, IDM_DEV_PLAYBACK_COND_S *pstPlayBackCond, LONG *plSessionID, CHAR* pMethodStr, LONG *plStrLen);

/*
*@brief: VMS申请录像下载
*@param: IN lUserID 设备句柄
*@param: IN pstDownloadCond 录像下载相关参数
*@param: OUT plSessionID 申请的子链接校验ID
*@param: OUT pMethodStr 请求实时流JSON字符串，字符串内存由调用者分配，推荐512字节
*@param: IN/OUT plStrLen IN内存长度，OUT传出字符串长度
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_VMS_SetupDownLoadLink(LONG lUserID, IDM_DEV_PLAYBACK_COND_S *pstDownloadCond, LONG *plSessionID, CHAR* pMethodStr, LONG *plStrLen);

typedef struct tagIDM_VMS_AUTO_LOGIN_PARAM
{
    ULONG   ulSessionID;
    CHAR    szServerIP[IDM_DEVICE_IP_MAX_LEN];
    USHORT  usPort;
    UCHAR   res[10];
}IDM_VMS_AUTO_LOGIN_PARAM_S;

/*
*@brief: VMS 主动注册模式下申请实时流
*@param: IN lUserID 设备句柄
*@param: IN pstPlayBackCond 实时流相关参数
*@param: IN pstLinkInfo 主动注册建立子链接必要信息
*@param: OUT pMethodStr 请求实时流JSON字符串，字符串内存由调用者分配，推荐256字节
*@param: IN/OUT plStrLen IN内存长度，OUT传出字符串长度
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_VMS_AR_SetupStreamLink(LONG lUserID, IDM_DEV_PREVIEW_INFO_S *pstPreviewInfo, IDM_VMS_AUTO_LOGIN_PARAM_S *pstLinkInfo, CHAR* pMethodStr, LONG *plStrLen);

/*
*@brief: VMS 主动注册模式下申请回放流
*@param: IN lUserID 设备句柄
*@param: IN pstPlayBackCond 实时流相关参数
*@param: IN pstLinkInfo 主动注册建立子链接必要信息
*@param: OUT pMethodStr 请求实时流JSON字符串，字符串内存由调用者分配，推荐256字节
*@param: IN/OUT plStrLen IN内存长度，OUT传出字符串长度
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_VMS_AR_SetupPlayBackLink(LONG lUserID, IDM_DEV_PLAYBACK_COND_S *pstPlayBackCond, IDM_VMS_AUTO_LOGIN_PARAM_S *pstLinkInfo, CHAR* pMethodStr, LONG *plStrLen);

/*
*@brief: VMS 主动注册模式下申请录像下载
*@param: IN lUserID 设备句柄
*@param: IN pstDownloadCond 录像下载相关参数
*@param: OUT plSessionID 申请的子链接校验ID
*@param: OUT pMethodStr 请求实时流JSON字符串，字符串内存由调用者分配，推荐512字节
*@param: IN/OUT plStrLen IN内存长度，OUT传出字符串长度
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_VMS_AR_SetupDownLoadLink(LONG lUserID, IDM_DEV_PLAYBACK_COND_S *pstDownloadCond, IDM_VMS_AUTO_LOGIN_PARAM_S *pstLinkInfo, CHAR* pMethodStr, LONG *plStrLen);

/*
*@brief: VMS 获取图片特征值
*@param: IN lUserID 设备句柄
*@param: IN pImgInfo 图片信息
*@param: IN/OUT pFeatureInfo 特征值信息
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_VMS_ExtractImageFeature(LONG lUserID, IDM_VMS_EXTRACTFEATURE_IMGINFO_S* pImgInfo, IDM_VMS_EXTRACTFEATURE_FEATUREINFO_S* pFeatureInfo);

/*
*@brief: web插件 录像回放
*@param: IN lUserID 设备句柄
*@param: IN stPlayBackCond 录像查询条件
*@param: IN pfPlayBackCallback 回放数据回调函数
*@param: IN pUserData 用户数据
*@param: OUT lPlayBackHandle 回放句柄
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_OCX_PlayBack(LONG lUserID, IDM_OCX_PLAYBACK_COND_S stPlayBackCond, IDM_DEV_PlayBack_Callback_PF pfPlayBackCallback, VOID* pUserData, LONG* plPlayBackHandle);

/** 手势登录私有 接口声明 */
#ifndef CONST
#define CONST               const
#endif
/*
*@brief: 设置手势登录密码
*@param: IN lUserID 设备句柄
*@param: IN szUserName 用户名称 目前只能是admin
*@param: IN szPassword 用户手势密码
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_ECS_GestureSetPassword(LONG lUserID, CONST CHAR *szUserName,CONST CHAR *szPassword);

/*
*@brief: 手势密码登录设备
*@param: IN pstLoginInfo 登录结构体
*@param: OUT pstDeviceInfo 设备信息
*@param: OUT plUserID 登录成功分配的设备句柄
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_ECS_GestureLogin(IDM_DEV_USER_LOGIN_INFO_S *pstLoginInfo, IDM_DEV_DEVICE_INFO_S *pstDeviceInfo, LONG *plUserID);


LONG IDM_DEV_Stop(LONG lUserID);

/*
*@brief: 私有协议json option数据透传
*@note : 调用完成后需要调用IDM_FREE(pstRecvData结构体地址)释放内存
*@param: IN lUserID 设备句柄
*@param: IN pcSendJson 发送数据的缓冲指针
*@param: IN ulSendSize 发送数据的缓冲长度
*@param: OUT pstRecvData 接收到的数据(使用完调用IDM_FREE释放)
*@param: IN  ulTimeOut 业务超时时间 默认全局超时时间(10000ms)  单位毫秒(注意: 传0 也表示与全局超时时间一致)
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_DEV_JsonDataPassthrough(LONG lUserID, CHAR *pcSendJson, ULONG ulSendSize, IDM_DATA_PASSTHROUGH_RECV_INFO_S *pstRecvData, ULONG ulTimeOut = 0 /* 传0表示 用系统默认 */);

/*
*@brief: 透传外部封装好的完整私有协议数据报文二进制
*@note : 调用完成后需要调用IDM_FREE(pstRecvData结构体地址)释放内存
*@param: IN lUserID 设备句柄
*@param: IN pcSendBuf 发送数据的缓冲指针
*@param: IN ulSendSize 发送数据的缓冲长度
*@param: OUT pstRecvData 接收到的数据(使用完调用IDM_FREE释放)
*@param: IN  ulTimeOut 业务超时时间 默认全局超时时间(10000ms)  单位毫秒(注意: 传0 也表示与全局超时时间一致)
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_DEV_DataPassthrough(LONG lUserID, CHAR *pcSendBuf, ULONG ulSendSize, IDM_DATA_PASSTHROUGH_RECV_INFO_S *pstRecvData, ULONG ulTimeOut = 0 /* 传0表示 用系统默认 */);

/*
*@brief: 私有协议透传json和二进制数据
*@note : 调用完成后需要调用IDM_FREE(out结构体地址)释放内存
*@param: IN lUserID 设备句柄
*@param: IN pstRequest 要发送的json和二进制数据
*@param: OUT pstReponse 返回json和二进制数据,(使用完调用IDM_FREE释放内部申请的内存)
*@param: IN  ulTimeOut 业务超时时间 默认全局超时时间(10000ms)  单位毫秒(注意: 传0 也表示与全局超时时间一致)
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_DEV_TransparentJsonAndData(LONG lUserID, const IDM_TRANSPARENT_JSON_BIN_S *pstRequest, IDM_TRANSPARENT_JSON_BIN_S *pstResponse, ULONG ulTimeOut = 0 /* 传0表示 用系统默认 */);

/*
*@brief: 私有协议透传二进制数据
*@note : 调用完成后需要调用IDM_FREE(out结构体地址)释放内存
*@param: IN lUserID 设备句柄
*@param: IN pstRequest 请求透传参数,
*@param: OUT pstReponse 返回二进制数据,不需返回二进数据时,此参数可为null.(使用完调用IDM_FREE释放内部申请的内存)
*@param: IN  ulTimeOut 业务超时时间 默认全局超时时间(10000ms)  单位毫秒(注意: 传0 也表示与全局超时时间一致)
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_DEV_TransparentData(LONG lUserID, const IDM_TRANSPARENT_SINGLE_BIN_S *pstRequest, IDM_BIN_DATA_S *pstResponse, ULONG ulTimeOut = 0 /* 传0表示 用系统默认 */);

/*
*@brief: 开始私有帧数据转MP4
*@param: IN pcSavedFileName 保存的文件名(包含路径)
*@param: OUT plFileHandle 文件句柄
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_DEV_StartPrivFrameTransMP4(IDM_DEV_SAVE_DATA_INFO_S *pFileInfo, LONG* plFileHandle);

/*
*@brief: 送入私有帧数据
*@param: IN lFileHandle 文件句柄
*@param: IN pPrivFrameBuf 存放私有帧数据的缓冲区
*@param: IN ulBufSize 私有帧数据大小
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_DEV_PrivFrameInputData(LONG lFileHandle, UCHAR* pucPrivFrameBuf, UINT uiBufSize);

/*
*@brief: 停止私有帧数据转MP4
*@param: IN lFileHandle 文件句柄
*@return: 成功返回IDM_SUCCESS，失败返回错误码
*/
IDM_API LONG IDM_DEV_StopPrivFrameTransMP4(LONG lFileHandle);

#ifdef __cplusplus
}
#endif
#endif
