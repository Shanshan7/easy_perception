#pragma once
#ifndef HZ_PLAYPLAY_H_
#define HZ_PLAYPLAY_H_

#ifndef HZ_PLAY_API
# if defined(WIN32) || defined(__WIN32__) || defined(WIN64)
#  ifdef __BORLANDC__
#   ifdef BUILD_ZX_PLAY
#    define HZ_PLAY_API
#   else
#    define HZ_PLAY_API __declspec(dllimport)
#   endif
#  else
#   ifdef HZ_PLAYPLAY_EXPORTS
#    define HZ_PLAY_API __declspec(dllexport)
#   elif defined(HZ_PLAYPLAY_IMPORTS)
#    define HZ_PLAY_API __declspec(dllimport)
#   else
#    define HZ_PLAY_API
#   endif
#  endif
# elif defined(__OS2__)
#  ifdef BUILD_ZX_PLAY
#   define HZ_PLAY_API __declspec(dllimport)
#  else
#   define HZ_PLAY_API
#  endif
# else
#  if defined(__GUNC__) && __GUNC__ >= 4
#   define HZ_PLAY_API __attribute__ ((visibility("default")))
#  else
#   define HZ_PLAY_API
#  endif
# endif
#endif // !HZ_PLAY_API

#ifndef PLAYCALL
#if (defined(WIN32) || defined(WIN64)) && !defined(__GUNC__)
#define PLAYCALL 
#elif defined(__OS2__) || defined(__EMX__)
#define PLAYCALL _System
#if defined(__GUNC__) && !defined(_System)
#define _System
#endif
#else
#define PLAYCALL
#endif
#endif // !PLAYCALL

#include <stdint.h>

// 图片格式
typedef enum ePICTURE_FMT
{
    PICFMT_BMP = 0,                     // BMP类型
    PICFMT_JPEG,                        // JPEG类型
    PICFMT_TIFF,                        // TIFF类型
    PICFMT_PNG                          // PNG类型
} PSK_PICTURE_FMT;

// 打印等级调整
typedef enum eLOG_LEVEL
{
    LLEVEL_UNKNOWN = 0,                 // 未知等级
    LLEVEL_FATAL,                       // fatal等级，当设置为此等级时，有一种打印输出（fatal）都有输出
    LLEVEL_ERROR,                       // error等级，当设置为此等级时，有两种打印输出（fatal，error）都有输出
    LLEVEL_WARN,                        // warn等级，当设置为此等级时，有三种打印输出（fatal，error，warn）都有输出
    LLEVEL_INFO,                        // info等级，当设置为此等级时，有四种打印输出（fatal，error，warn，info）都有输出
    LLEVEL_TRACE,                       // Trace等级，当设置为此等级时，有五种打印输出（fatal，error，warn，info，trace）都有输出
    LLEVEL_DEBUG                        // Debug等级，当设置为此等级时，以上六种打印（fatal，error，warn，info，trace，debug）都有输出
}PSK_LOG_LEVEL;

// 视频宽高比
typedef enum eVIDEO_RATIO
{
    VRATIO_DEFAULT,
    VRATIO_ORIGINAL,
    VRATIO_16X9,
    VRATIO_4X3,
    VRATIO_1X1,
}PSK_VIDEO_RATIO;

typedef enum eHWACCEL_TYPE
{
    HW_DXVA2,
    HW_D3D11,
    HW_CUDA,
}PSK_HWACCEL_TYPE;

typedef enum eTEXT_XALIGN_TYPE
{
    XALIGN_NONE,
    XALIGN_LEFT,
    XALIGN_MIDDLE,
    XALIGN_RIGHT,
}TEXT_XALIGN_TYPE;

typedef enum eTEXT_YALIGN_TYPE
{
    YALIGN_NONE,
    YALIGN_TOP,
    YALIGN_MIDDLE,
    YALIGN_BOTTOM,
}TEXT_YALIGN_TYPE;

// 局部显示区域
typedef struct tagPSKEZOOMRECT
{
    uint32_t wndWidth;          // 窗口宽。当值=0时，下面的局部区域值所在的窗口大小由内部计算；
    uint32_t wndHeight;         // 窗口高。当值=0时，下面的局部区域值所在的窗口大小由内部计算；

    uint32_t left;              // 局部区域左边线相对于窗口左侧距离/比例
    uint32_t top;               // 局部区域上边线相对于窗口顶侧距离/比例
    uint32_t right;             // 局部区域右边线相对于窗口左侧距离/比例
    uint32_t bottom;            // 局部区域底边线相对于窗口顶侧距离/比例
}PSK_RECT;

typedef struct tagPSKPOINT
{
    uint32_t wndX;
    uint32_t wndY;
    uint32_t wndWidth;
    uint32_t wndHeight;
}PSK_POINT;

typedef struct tagPSKPOLYGON
{
    uint32_t wndWidth;
    uint32_t wndHeight;
    uint32_t pointNum;
    PSK_POINT *points;
    char     *name;                 // 名称(文本, 不为空将绘制在中心点)
    uint32_t fontSize;              // 文本字体大小
    uint32_t textColor;             // 文本颜色(argb)
    uint32_t arrowStyle;            // 绊线类型(0:无,1:A->B,2:A<-B,3:A<->B)
    bool     showPixel;             // 显示矩形框宽高
    bool     showSignalLamp;        // 显示交通灯
    uint32_t signalStyle;           // 交通灯样式(0:单个.1:横向3个,2:竖向3个)
    bool     fillLastLine;          // 是否封闭多边形
}PSK_POLYGON;

typedef struct tagPOLYGON_SET
{
    uint32_t id;                    // 取值范围[0,100000),建议分配在10000以上
    uint32_t lineWidth;             // 线宽
    uint32_t color;                 // 颜色
    uint64_t ptsStart;              // 显示起始时间
    uint64_t ptsStop;               // 显示结束时间
    uint32_t polygonNum;            // 多边形个数
    uint32_t subWndId;              //如果有子窗口，则填子窗口ID,否则为0
    PSK_POLYGON *polygons;
}PSK_POLYGON_SET;

typedef struct tagDISPLAY_TEXT
{
    uint32_t id;
    uint32_t size;                  // 字体大小,对应的视频中的大小
    bool actualSize;                // true:内部由size计算实际大小，false:使用size值
    uint32_t wndWidth;
    uint32_t wndHeight;
    uint32_t wndX;                  // x坐标(相对于窗口左上角)
    uint32_t wndY;                  // y坐标(相对于窗口左上角)

    uint32_t color;                 // argb  
    bool outline;                   // 是否包含外轮廓
    uint32_t lineWidth;             // 外轮廓存在时的线宽度
    char *string;
    TEXT_XALIGN_TYPE xalignType;    // x轴方向对齐方式
    TEXT_YALIGN_TYPE yalignType;    // y轴方向对齐方式
}PSK_DISPLAY_TEXT;

typedef enum eMEMSNAP_FMT
{
    YUV420P,
    BGR24,
}PSK_MEMSNAP_FMT;

typedef enum eIMGTYPE
{
    IMG_DEFAULT,                    // 设置为默认，输出则为解码后原本的类型
    IMG_YUV420P,
    IMG_BGR24,
    IMG_RGB24,
    IMG_BGRA32,
    IMG_RGBA32,
    IMG_DXVA2,
    IMG_D3D11VA,
}PSK_IMGTYPE;

typedef enum eERRORCODE
{
    PSKNOERROR = 0,
    CREATEINS_ERR,                  // 创建实例错误
    OPENCODEC_ERR,                  // 打开解码器错误
    STARTCODEC_ERR,                 // 启动解码失败
    CREATEVRENDER_ERR,              // 创建视频渲染错误
    INITVRENDER_ERR,                // 初始化视频渲染器失败
    CREATEARENDER_ERR,              // 创建音频渲染错误
    INITARENDER_ERR,                // 初始化音频渲染器失败
    DECODEFRAME_ERR,                // 解码失败
    ERROR_CHANNEL,                  // 错误通道号
    INVALID_FILENAME,               // 错误文件名

    OUT_OF_CHANNEL_LIMIT,           // 超出通道号限制

    INVALID_POINTER,                // 错误的传入指针

    INPUT_DATA_FMT_ERR,             // 输入数据格式错误，包含数据太短

    // 通用错误
    INVALID_URI = 0x1000,
}PSK_ERRORCODE;

typedef struct tagIMAGE_COLOR_INFO
{
    int32_t contrast = 50;               // 对比度 0至100,默认50
    int32_t brightness = 50;             // 亮度   0至100,默认50
    int32_t saturation = 50;             // 饱和度 0至100,默认50
    int32_t hue = 50;                    // 色调   0至100,默认50
}PSK_IMAGE_COLOR_INFO;

// 音频编码信息
typedef struct tagACODEC_INFO
{
    int32_t codecType;              // 编码类型:0-无流编码类型,1-ADPCM,2-G.722,3-G.711U,4-G.711A,5-G.726,6-AAC,7-MP2L2,8-PCM,9-G.722.1
    int32_t tracks;                 // 声道数
    int32_t sampleRate;             // 采样率
    int32_t bitRate;                // 码率
    int32_t channelCount;           // 通道总数
    int32_t channelNO;              // 通道号
    int32_t bitsPerSample;          // 位每采样
}PSK_ACODEC_INFO;

// 视频编码信息
typedef struct tagVCODEC_INFO
{
    int32_t codecType;              // 编码类型:0-无流编码类型,1-MJPEG,2-H.264,3-H.265,4-MPEG4
    int32_t frameRate;              // IPC设置帧率
    int32_t videoHeight;            // 视频高
    int32_t videoWidth;             // 视频宽
    uint32_t realBitrate;           // 实际码率
    char reserved[0x10];            // 保留
}PSK_VCODEC_INFO;

/////////// 通知回调相关结构 /////////////
typedef enum eNOTIFY_TYPE
{
    VDECODER_INITED = 0,            // 视频解码器初始化成功
    ADECODER_INITED,                // 音频解码器初始化成功
    PLAYBACK_END,                   // 回放结束

    VRENDER_START,                  // 视频渲染开始
    ARENDER_START,                  // 音频渲染开始

    VIDEO_ENC_CHANGED,              // 视频解码信息变更 结构体详见PSK_VENC_CHANGED_NOTIFY_INFO
    AUDIO_ENC_CHANGED,              // 音频解码信息变更 结构体详见PSK_AENC_CHANGED_NOTIFY_INFO
}PSK_NOTIFY_TYPE;

typedef struct tagNOTIFY_PARAM
{
    PSK_NOTIFY_TYPE type;           // 详见PSK_NOTIFY_TYPE
    void *lparam;                   // 对应type的结构体,0-nullptr,1-nullptr
}PSK_NOTIFY_PARAM;

typedef struct tagVENC_CHANGED_INFO
{
    PSK_VCODEC_INFO oldInfo, newInfo;
}PSK_VENC_CHANGED_NOTIFY_INFO;

typedef struct tagAENC_CHANGED_INFO
{
    PSK_ACODEC_INFO oldInfo, newInfo;
}PSK_AENC_CHANGED_NOTIFY_INFO;

typedef struct _PSK_PIC_GET_INFO_
{
    PSK_PICTURE_FMT fmt;
    bool 			enCrop;  //是否启动裁剪，先裁剪后缩放
    bool			enScale; //是否启动缩放	
    int32_t			CropX;
    int32_t			CropY;
    int32_t			CropW;
    int32_t			CropH;
    int32_t			ScaleW;
    int32_t			ScaleH;
    PSK_IMGTYPE		image_type; //BMP编码有效
    int32_t			quality; //JPEG编码有效	图像质量1~100,1最差，100最好
}PSK_PIC_GET_INFO;

/////////// 通知回调相关结构 /////////////

typedef enum e_PRIDATA_TYPE
{
    PRIDATA_RULE = 0x00000001,               // 规则框
    PRIDATA_TARGET = 0x00000002,             // 跟踪框
    PRIDATA_MOBILE_DETECTION = 0x00000004,   // 移动侦测
}PSK_PRIDATA_TYPE;

// 视频宽高比
typedef enum e_BUFFER_TYPE
{
    BUF_VIDEO_SRC,
    BUF_AUDIO_SRC,
    BUF_VIDEO_RENDER,
    BUF_AUDIO_RENDER,
    BUF_ALL,
}PSK_BUFFER_TYPE;

// 缓冲区信息
typedef struct tagBUFFER_INFO
{
    int32_t max_value;              // 缓冲区大小，BUF_VIDEO_SRC和BUF_AUDIO_SRC为最大缓存大小，BUF_VIDEO_RENDER和BUF_AUDIO_RENDER为最大缓存帧数
    int32_t useage;                 // 已使用的缓存大小
}PSK_BUFFER_INFO;

// 视频宽高比
typedef enum e_PLAY_MODE
{
    PLAY_REALTIME_STREAM,
    PLAY_RECORD_STREAM,
}PSK_PLAY_MODE;

typedef void(__stdcall *PLAY_Notify)(PSK_NOTIFY_PARAM *, void *);

typedef void(__stdcall *LOG_Callback)(PSK_LOG_LEVEL logLevel, const char *logText, void *userData);

typedef void(__stdcall *PLAY_DisplayCallback)(
    int32_t     channelID,      // 通道ID
    char        *data,          // 数据指针
    uint32_t    dataLen,        // 数据长度
    uint32_t    width,          // 视频宽
    uint32_t    height,         // 视频高
    uint64_t    timestamp,      // 时间戳
    uint32_t    type,           // PSK_IMGTYPE枚举值
    void        *avframe,       // 解码帧
    void        *userData);     // 用户数据


// 视频编码信息
typedef struct tagDECODEDATA_FRM_INFO
{
    char *data;
    uint32_t dataLen;
    int64_t utcTime;
    uint32_t keyFlag;
    uint32_t frameType;  // 0 - video  1 - audio
    union frame_info
    {
        PSK_VCODEC_INFO videoFrmInfo;
        PSK_ACODEC_INFO audioFrmInfo;
    }frameInfo;
}PSK_DECODEDATA_FRM_INFO;

typedef void(__stdcall *PLAY_DecodeDataCallback)(int32_t channelID, PSK_DECODEDATA_FRM_INFO *info, void *userData);

typedef void(__stdcall *PLAY_DrawCallback)(int32_t channelID, void *hdc, void *userData);

#ifdef __cplusplus
extern "C" {
#endif
// @bref        获取版本号
// $retvalue    返回值为版本号字符串，失败返回NULL
HZ_PLAY_API char *PLAYCALL PLAY_GetVersion();

// @bref        获取错误码
// $retvalue    错误码值
HZ_PLAY_API unsigned long PLAYCALL PLAY_GetLastError();

// @bref        设置日志打印级别
// #logLevel    日志级别，详见PSK_LOG_LEVEL定义
HZ_PLAY_API void PLAYCALL PLAY_SetPrintLogLevel(PSK_LOG_LEVEL logLevel);

// @bref        设置日志回调
// #callback    日志回调函数指针
HZ_PLAY_API void PLAYCALL PLAY_SetLogCallback(LOG_Callback callback, void *userData);


// @bref        获取空闲的解码通道号
// #channelID[out] 空闲的解码通道号
// $retvalue    失败返回FLASE 成功返回TURE
HZ_PLAY_API bool PLAYCALL PLAY_GetFreeChID(int32_t *channelID);

// @bref        释放对应的解码通道号
// #channelID   需要释放的解码通道号
HZ_PLAY_API void PLAYCALL PLAY_ReleaseChID(int32_t channelID);

// @bref        开启播放
// #channelID   解码通道号
// #hwnd        播放窗口句柄,如果hwnd为NULL 则不作显示
// $retvalue    失败返回FLASE 成功返回TURE
HZ_PLAY_API bool PLAYCALL PLAY_StartPlay(int32_t channelID, void *hwnd);

// @bref        关闭解码器
// #channelID   解码通道号
HZ_PLAY_API void PLAYCALL PLAY_StopPlay(int32_t channelID);

// @bref        获取已播放的时间
// #channelID   解码通道号
// $retvalue    已播放时间，单位ms
HZ_PLAY_API int32_t PLAYCALL PLAY_GetPlayedTime(int32_t channelID);

// @bref        获取已播放的帧数
// #channelID   解码通道号
// $retvalue    已播放的帧
HZ_PLAY_API int64_t PLAYCALL PLAY_GetPlayedFrames(int32_t channelID);

// @bref        获取文件的总帧数（针对文件播放）
// #channelID   解码通道号
// $retvalue    文件总帧数
HZ_PLAY_API int64_t PLAYCALL PLAY_GetFileTotalFrames(int32_t channelID);

// @bref        私有流数据帧模式输入
// #channelID   解码通道号
// #buff        数据
// #len         数据长度
// $retvalue    成功返回true,失败返回false
HZ_PLAY_API bool PLAYCALL PLAY_InputData(int32_t channelID, uint8_t *buff, uint32_t len);

// @bref        VMS数据帧模式输入
// #channelID   解码通道号
// #stype       数据类型-96视频-97音频
// #buff        数据
// #len         数据长度
// $retvalue    成功返回true,失败返回false
HZ_PLAY_API bool PLAYCALL PLAY_InputVMSData(int32_t channelID, int32_t stype, uint8_t *buff, uint32_t len);

// @bref        获取播放模式
// #channelID   解码通道号
// $retvalue    0-无播放，1-RTSP，2-标准视频文件，3-私有流模式，4-私有文件模式
HZ_PLAY_API int32_t PLAYCALL PLAY_GetStreamOpenMode(int32_t channelID);

// @bref        暂停或恢复
// #channelID   解码通道号
// #isPause     是否暂停，true-暂停，false-恢复
HZ_PLAY_API void PLAYCALL PLAY_PauseResume(int32_t channelID, bool isPause);

// @bref        获取声音音量
// #channelID   解码通道号
// #volume[out] 音量返回
// $retvalue    true-成功获取，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_GetVolume(int32_t channelID, uint32_t *volume);

// @bref        设置声音音量
// #channelID   解码通道号
// #volume      音量
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_SetVolume(int32_t channelID, uint32_t volume);

// @bref        音频渲染使能
// #channelID   解码通道号
// #isEnable    true-渲染，false-不渲染
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_SetAudioEnable(int32_t channelID, int32_t isEnable);

//*******************************************************************************************************************************
//***********************************************************文件相关************************************************************
//*******************************************************************************************************************************

// @bref        打开文件并返回解码通道号
// #fileName    文件名。
// $retvalue    解码通道号
HZ_PLAY_API int32_t PLAYCALL PLAY_OpenFile(const char *fileName);

// @bref        关闭文件并释放对应的解码通道号
// #channelID   解码通道号
HZ_PLAY_API void PLAYCALL PLAY_CloseFile(int32_t channelID);

// @bref        获取文件总时间（针对文件播放）
// #channelID   解码通道号
// $retvalue    文件时间，单位ms
HZ_PLAY_API int32_t PLAYCALL PLAY_GetFileTime(int32_t channelID);

// @bref        设置文件偏移位置 百分比
// #channelID   解码通道号
// #posNum      文件偏移百分比
// $retvalue    成功-true,失败-false
HZ_PLAY_API bool PLAYCALL PLAY_SetFilePos(int32_t channelID, float posNum);

// @bref        文件时移动到位置
// #channelID   解码通道号
// #location    移动到位置(单位毫秒)
// $retvalue    成功-true,失败-false
HZ_PLAY_API bool PLAYCALL PLAY_Seek(int32_t channelID, uint64_t location);

//*******************************************************************************************************************************
//***********************************************************渲染控制************************************************************
//*******************************************************************************************************************************

// @bref        视频渲染控制
// #channelID   解码通道号
// #state       true-渲染，false-不渲染
HZ_PLAY_API void PLAYCALL PLAY_RenderControl(int32_t channelID, bool state);

// @bref        设置播放速度
// #channelID   解码通道号
// #speed       浮点数的速度值，范围1/8,1/4,1/2,1,2,4,8,16,32,-1,-2,-4,-8,-16,-32,1/100.0 -1/100.0
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_SetPlaySpeed(int32_t channelID, float speed);

// @bref        获取实时帧率
// #channelID   解码通道号
// #frameRate[out]帧率
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_GetRealFrameRate(int32_t channelID, double* frameRate);

// @bref        获取视频分辨率[废弃，查看PLAY_GetVideoCodecInfo]
// #channelID   解码通道号
// #width[out]  视频宽
// #height[out] 视频高
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_GetPictureSize(int32_t channelID, uint32_t *width, uint32_t *height);

// @bref        获取视频编码信息
// #channelID   解码通道号
// #width[out]  视频宽
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_GetAVCodecInfo(int32_t channelID, PSK_VCODEC_INFO *vcodecInfo, PSK_ACODEC_INFO *acodecInfo);

// @bref        启用硬件加速
// #channelID   解码通道号
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_EnableHwAccele(int32_t channelID);

// @bref        设置电子放大（开启或者取消）
// #channelID   解码通道号
// #rect        需要放大的区域（开启时），NULL（取消时）
// #isEnable    开启或者取消电子放大
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_SetEleZoom(int32_t channelID, PSK_RECT* rect, bool isEnable);

// @bref        截图
// #channelID   解码通道号
// #fileName    保存图片的文件名（全路径）
// #picFormat   保存图片的格式，详见PSK_PICTURE_FMT定义
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_CatchPic(int32_t channelID, char* fileName, PSK_PICTURE_FMT picFormat);

// @bref        绘制多边形
// #channelID   解码通道号
// #privPolySet 需要绘制的多边形数据
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_DrawPolygons(int32_t channelID, PSK_POLYGON_SET *privPolySet);

// @bref        删除已绘制的多边形
// #channelID   解码通道号
// #frameID     需要删除的多边形ID
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_DeletePolygons(int32_t channelID, uint32_t frameID);

// @bref        绘制文字
// #channelID   解码通道号
// #text        需要绘制文字相关信息数据
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_DrawText(int32_t channelID, PSK_DISPLAY_TEXT *text);

// @bref        获取已绘制的文字的宽高
// #channelID   解码通道号
// #textID      文字信息对应的ID
// #width[out]  文字占用的宽
// #height[out] 文字占用的高
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_GetTextRect(int32_t channelID, uint32_t textID, PSK_RECT *textRect);

// @bref        删除已绘制的文字
// #channelID   解码通道号
// #textID      文字信息对应的ID
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_DeleteText(int32_t channelID, uint32_t textID);

// @bref        获取视频颜色信息
// #channelID   解码通道号
// #colorInfo   颜色信息
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_GetVideoColor(int32_t channelID, PSK_IMAGE_COLOR_INFO *colorInfo);

// @bref        设置视频颜色信息
// #channelID   解码通道号
// #colorInfo   颜色信息
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_SetVideoColor(int32_t channelID, PSK_IMAGE_COLOR_INFO colorInfo);

// @bref        获取回放时间戳
// #channelID   解码通道号
// #value[out]  获取到当前显示的回放时间戳
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_GetPlayRecordTime(int32_t channelID, uint64_t *value);

// @bref        设置通知回调函数
// #channelID   解码通道号
// #notifyFunc  通知回调函数指针
// #userData    回调函数用户数据输入，用于回调函数第二个参数
// $retvalue    true-成功，false-失败
HZ_PLAY_API void PLAYCALL PLAY_SetNotify(int32_t channelID, PLAY_Notify notifyFunc, void *userData);

// @bref        计算抓图到内存需要的预先分配的内存大小
// #channelID   解码通道号
// #fmt         抓图类型
// $retvalue    成功返回大小；失败返回0
HZ_PLAY_API unsigned long PLAYCALL PLAY_CalcMemSize(int32_t channelID, PSK_MEMSNAP_FMT fmt);

// @bref        抓图到内存
// #channelID   解码通道号
// #buff[out]   保存抓图的内存存放地址 需要调用者预先分配
// #buffLen     保存抓图的内存大小 内存大小小于保存图片需要的内存时，会不成功
// #fmt         抓图类型
// $retvalue    成功返回true；失败返回false
HZ_PLAY_API bool PLAYCALL PLAY_MemSnapshot(int32_t channelID, char* buff, unsigned long buffLen, PSK_MEMSNAP_FMT fmt);

// @bref        显示私有数据，例如规则框，跟踪框，移动侦测等
// #channelID   解码通道号
// #dataType    私有数据类型, 见PSK_PRIDATA_TYPE, 按位表示，每一位标识一种类型
// #enable      true:显示，false:不显示
// $retvalue    成功返回true；失败返回false
HZ_PLAY_API bool PLAYCALL PLAY_RenderPrivateData(int32_t channelID, int32_t dataType, bool enable);

// @bref        设置JPEG的编码图像质量
// #quality     图像质量1~100,1最差，100最好
HZ_PLAY_API void PLAYCALL PLAY_SetJpegQuality(int32_t quality);

// @bref        抓图并可指定宽高，异步抓图
// #channelID   解码通道号
// #fileName    文件名
// #width       图片宽度
// #height      图片高度
// #picFormat   保存图片的格式，详见PSK_PICTURE_FMT定义
// $retvalue    true-成功，false-失败
HZ_PLAY_API bool PLAYCALL PLAY_CatchResizePic(int32_t channelID, char* fileName, int32_t width, int32_t height, PSK_PICTURE_FMT picFormat);

// @bref        设置显示回调
// #channelID   解码通道号
// #displayCallback    回调函数
// #userData    用户数据
HZ_PLAY_API void PLAYCALL PLAY_SetDisplayCallback(int32_t channelID, PLAY_DisplayCallback displayCallback, PSK_IMGTYPE imgType, void *userData);

// @bref        设置缓冲区
// #channelID   解码通道号
// #cacheType   缓冲区类型 0-解码前 1-解码后
// #cacheLevel  缓冲区级别 0-无缓冲 1-低缓冲 2-中缓冲 3-中高缓冲 4-高缓冲
HZ_PLAY_API void PLAYCALL PLAY_SetCacheLevel(int32_t channelID, uint32_t cacheType, uint32_t cacheLevel);

// @bref        电子放大时开启子窗口播放
// subWndID     预留防止后续有多个子窗口需求,子画面序号，从1开始的整数
// #channelID   解码通道号
// #hwnd        播放窗口句柄,如果hwnd为NULL 则不作显示
// #enable      子窗口是否启用
// $retvalue    失败返回FLASE 成功返回TURE
HZ_PLAY_API bool PLAYCALL PLAY_SetSubWindow(int32_t channelID, int32_t subWndID, void *hwnd, bool enable);

// @bref        裁剪图片
// #channelID   解码通道号
// #picBuff     截图图片缓冲区，用户分配
// #buffSize    缓冲区大小，用户分配，JPEG建议分配w*h*3/2,PNG、BMP建议分配w*h*3;
// #picSize     编码后的图片大小
// #picGetInfo  截图参数
// $retvalue    失败返回FLASE 成功返回TURE
HZ_PLAY_API bool PLAYCALL PLAY_GetPicBuf(int32_t channelID, unsigned char **picBuff, int32_t buffSize, int32_t *picSize, PSK_PIC_GET_INFO *picGetInfo);

// @bref        电子放大时开启子窗口播放
// #channelID   解码通道号
// #hwnd        播放窗口句柄,如果hwnd为NULL 则不作显示
// #enable      是否启用不断渲染最后一帧画面
// $retvalue    失败返回FLASE 成功返回TURE
HZ_PLAY_API void PLAYCALL PLAY_SetRenderLastFrame(int32_t channelID, bool enable);

// @bref        清空指定缓冲区的剩余数据
// #channelID   解码通道号
// #buffType    缓冲类型
HZ_PLAY_API bool PLAYCALL PLAY_ResetBuffer(int32_t channelID, PSK_BUFFER_TYPE buffType);

// @bref        清空指定缓冲区的剩余数据
// #channelID   解码通道号
// #buffType    缓冲类型
// #buffInfo    缓冲详情
HZ_PLAY_API bool PLAYCALL PLAY_GetBufferValue(int32_t channelID, PSK_BUFFER_TYPE buffType, PSK_BUFFER_INFO *buffInfo);

// @bref        设置播放模式，预览和回放，播放前设置
// #channelID   解码通道号
// #mode        播放模式
HZ_PLAY_API bool PLAYCALL PLAY_SetPlayMode(int32_t channelID, PSK_PLAY_MODE mode);

// @bref        设置解码前回调
// #channelID   解码通道号
// #dataCallback    解码前数据回调函数
// #userData    用户数据
HZ_PLAY_API void PLAYCALL PLAY_SetDecodeDataCallback(int32_t channelID, PLAY_DecodeDataCallback dataCallback, void *userData);

// @bref        注册一个回调函数,获得当前表面的device context
// #channelID   解码通道号
// #regionNum   显示区域序号，范围[0,(MAX_DISPLAY_WND-1)].如果nRegionNum为0,则将设置的区域显示在主窗口中
// #displayCallback    画图回调函数
// #userData    用户数据
HZ_PLAY_API void PLAYCALL PLAY_RegisterDrawFun(int32_t channelID, int32_t regionNum, PLAY_DrawCallback drawCallback, void *userData);
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !HZ_PLAYPLAY_H_