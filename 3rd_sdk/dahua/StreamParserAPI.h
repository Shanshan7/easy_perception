#ifndef __STREAM_PARSER_H_
#define __STREAM_PARSER_H_

#if (defined(WIN32) || defined(WIN64))

	#define SP_API __declspec(dllimport)
	#define CALLMETHOD __stdcall

#else /*linux or mac*/

	#define SP_API
	#define CALLMETHOD

#endif

#ifdef __cplusplus
extern "C" {
#endif

/*接口返回值*/
enum SP_RESULT
{
    SP_SUCCESS = 0,							/*成功*/
    SP_ERROR_INVALID_HANDLE = 1,			/*无效句柄*/
    SP_ERROR_FILE_TYPE_NOSUPPORT = 2,		/*文件类型不支持*/
    SP_ERROR_STREAM_NOSUPPORT = 3,			/*流类型不支持*/
    SP_ERROR_PARAMETER = 6,					/*参数有误*/			
    SP_ERROR_BAD_FORMATTED = 9,     		/*文件格式错误*/
    SP_ERROR_BUFFER_OVERFLOW = 12,			/*内部缓冲区溢出*/
    SP_ERROR_SYSTEM_OUT_OF_MEMORY = 13,		/*系统内存不够*/
    SP_ERROR_LIST_EMPTY = 14,				/*列表为空*/
};

/*帧类型*/
enum SP_FRAME_TYPE
{
	SP_FRAME_TYPE_UNKNOWN = 0,			/*帧类型不可知*/
	SP_FRAME_TYPE_VIDEO,				/*帧类型是视频帧*/
	SP_FRAME_TYPE_AUDIO,				/*帧类型是音频帧*/
	SP_FRAME_TYPE_DATA,					/*帧类型是数据帧*/
};

/*帧子类型*/
enum SP_FRAME_SUB_TYPE
{
	SP_FRAME_SUB_TYPE_DATA_INVALID		= -1,	/*数据无效*/
	SP_FRAME_SUB_TYPE_VIDEO_I_FRAME	= 0 ,		/*I帧*/
	SP_FRAME_SUB_TYPE_VIDEO_P_FRAME,			/*P帧*/
	SP_FRAME_SUB_TYPE_VIDEO_B_FRAME,			/*B帧*/
	SP_FRAME_SUB_TYPE_DATA_INTL =7,				/*智能分析帧*/
	SP_FRAME_SUB_TYPE_VIDEO_JPEG_FRAME=8,		/*JPEG帧*/
	SP_FRAME_SUB_TYPE_DATA_INTLEX =11,          /*扩展智能分析帧*/
};						

/*编码类型*/
enum SP_ENCODE_VIDEO_TYPE
{
	SP_ENCODE_VIDEO_UNKNOWN = 0,						/*视频编码格式不可知*/
	SP_ENCODE_VIDEO_MPEG4 ,								/*视频编码格式是MPEG4*/
	SP_ENCODE_VIDEO_HI_H264,							/*视频编码格式是海思H264*/
	SP_ENCODE_VIDEO_JPEG,								/*视频编码格式是标准JPEG*/
	SP_ENCODE_VIDEO_DH_H264,							/*视频编码格式是H264*/
	SP_ENCODE_VIDEO_MPEG2 = 9,          				/*视频编码格式是MPEG2*/
	SP_ENCODE_VIDEO_DH_H265 = 12,						/*视频编码格式是H265*/
	SP_ENCODE_VIDEO_H263 = 35,      					/*视频编码格式是H263*/
};

enum SP_ENCODE_AUDIO_TYPE
{
	SP_ENCODE_AUDIO_UNKNOWN = 0,
	SP_ENCODE_AUDIO_PCM			= 7,					/*音频编码格式是PCM8*/
	SP_ENCODE_AUDIO_G729,								/*音频编码格式是G729*/
	SP_ENCODE_AUDIO_IMA,								/*音频编码格式是IMA*/
	SP_ENCODE_PCM_MULAW,								/*音频编码格式是PCM MULAW*/
	SP_ENCODE_AUDIO_G721,								/*音频编码格式是G721*/
	SP_ENCODE_PCM8_VWIS,								/*音频编码格式是PCM8_VWIS*/
	SP_ENCODE_MS_ADPCM,									/*音频编码格式是MS_ADPCM*/
	SP_ENCODE_AUDIO_G711A,								/*音频编码格式是G711A*/
	SP_ENCODE_AUDIO_AMR,								/*音频编码格式是AMR*/
	SP_ENCODE_AUDIO_PCM16,								/*音频编码格式是PCM16*/
	SP_ENCODE_AUDIO_G711U		= 22,					/*音频编码格式是G711U*/
	SP_ENCODE_AUDIO_G723,								/*音频编码格式是G723*/
	SP_ENCODE_AUDIO_AAC			= 26,					/*音频编码格式是AAC*/
	SP_ENCODE_AUDIO_MP2 = 31,							/*音频编码格式是mp2*/
	SP_ENCODE_AUDIO_OGG,								/*音频编码格式是ogg vorbis*/
	SP_ENCODE_AUDIO_MP3,								/*音频编码格式是mp3*/
	SP_ENCODE_AUDIO_G722_1,								/*音频编码格式是G722.1*/
	SP_ENCODE_AUDIO_AC = 49,							/*音频编码格式是AC3*/
};

/*加密类型*/
enum SP_ENCRYPT_TYPE
{
    SP_ENCRYPT_UNKOWN = 0,
    SP_ENCRYPT_AES,
};
/*时间信息*/
typedef struct
{
    int nYear;								/*年*/
    int nMonth;								/*月*/
    int nDay;								/*日*/
    int nHour;								/*小时*/
    int nMinute;							/*分钟*/
    int nSecond;							/*秒*/
    int nMilliSecond;						/*毫秒*/
} SP_TIME;

//属性包使能标识
#define SP_IVS_ATTRIBUTE_FLAG_80	(1 << 0)
#define SP_IVS_ATTRIBUTE_FLAG_81	(1 << 1)
#define SP_IVS_ATTRIBUTE_FLAG_82	(1 << 2)
#define SP_IVS_ATTRIBUTE_FLAG_83	(1 << 3)
#define SP_IVS_ATTRIBUTE_FLAG_84	(1 << 4)
#define SP_IVS_ATTRIBUTE_FLAG_87	(1 << 7)
#define SP_IVS_ATTRIBUTE_FLAG_90	(1 << 16)

typedef struct
{
	/*轨迹点是物体外接矩形的中心，根据X，Y及XSize，YSize计算出的物体外接矩形坐标(left，top，right，bottom)*/
	/*RECT=(X-XSize, Y-YSize, X+XSize, Y+YSize)  */
	unsigned short x;
	unsigned short y;
	unsigned short xSize;
	unsigned short ySize;
}SP_IVS_POINT;

//0x80车身属性包, 车型类型定义
typedef enum
{
	SP_IVS_TrafficVehicleUnknow = 0,		//未知
	SP_IVS_TrafficVehiclePassengerCar,		//客车
	SP_IVS_TrafficVehicleLargeTruck,		//大货车
	SP_IVS_TrafficVehicleMidTruck,			//中货车
	SP_IVS_TrafficVehicleSaloonCar,		//轿车
	SP_IVS_TrafficVehicleMicrobus,			//面包车
	SP_IVS_TrafficVehicleMicroTruck,		//小货车
	SP_IVS_TrafficVehicleTricycle,			//三轮车
	SP_IVS_TrafficVehicleMotor,			//摩托车
	SP_IVS_TrafficVehiclePasserby,			//行人
	SP_IVS_TrafficVehicleSuvMpv,			//SUV-MPV
	SP_IVS_TrafficVehicleMidPassengerCar,	//中客车
	SP_IVS_TrafficVehicleTankCar,			//危化品车辆(特殊车辆)
	SP_IVS_TrafficVehicleSUV,				//SUV
	SP_IVS_TrafficVehicleMPV,				//MPV
	SP_IVS_TrafficVehicleBus,				//公交车
	SP_IVS_TrafficVehiclePickup,			//皮卡车
	SP_IVS_TrafficVehicleMiniCarriage,		//微型车
	SP_IVS_TrafficVehicleOilTankTruck,		//油罐车(特殊车辆)
	SP_IVS_TrafficVehicleSlotTankCar,   	//槽罐车(特殊车辆)
	SP_IVS_TrafficVehicleColdChainCar,   	//冷链车(特殊车辆)
	SP_IVS_TrafficVehicleDregsCar,			//渣土车(特殊车辆)
	SP_IVS_TrafficVehicleConcreteMixerTruck,	//混凝土搅拌车(特殊车辆)
	SP_IVS_TrafficVehicleTaxi,             //出租车(特殊车辆)
	SP_IVS_TrafficVehiclePolice,           //警车(特殊车辆)
	SP_IVS_TrafficVehicleAmbulance,        //救护车(特殊车辆)
	SP_IVS_TrafficVehicleGeneral,          //普通车(特殊车辆)
	SP_IVS_TrafficVehicleWateringCar,      //环卫车(包括洒水车、垃圾车、清扫车等)(特殊车辆)
	SP_IVS_TrafficVehicleReserved2,   		//保留2(特殊车辆)
	SP_IVS_TrafficVehicleFireEngine,       //消防车(特殊车辆)
	SP_IVS_TrafficVehicleTractor,          //拖拉机(特殊车辆)
	SP_IVS_TrafficVehicleMachineshopTruck, //工程车(特殊车辆)
	SP_IVS_TrafficVehiclePowerLotVehicle,  //粉粒物料车(特殊车辆)
	SP_IVS_TrafficVehicleSuctionSewageTruck,	//吸污车(特殊车辆)
	SP_IVS_TrafficNormalVehicleTankTruck,  	//普通罐车(特殊车辆) 
	SP_IVS_TrafficVehicleTwocycle,          	//二轮车
	SP_IVS_TrafficVehicleSchoolBus,			//校车(特殊车辆) 
	SP_IVS_TrafficVehicleExcavator,			//挖掘车(特殊车辆)
	SP_IVS_TrafficVehicleBulldozer,			//推土车(特殊车辆)
	SP_IVS_TrafficVehicleCrane,				//吊车(特殊车辆)
	SP_IVS_TrafficVehiclePumptruck,			//泵车(特殊车辆)
	SP_IVS_TrafficVehicleEscort,				//押运车(特殊车辆)
	SP_IVS_TrafficVehicleShovelLoader,			//铲车(特殊车辆)
	SP_IVS_TrafficVehiclePoultry	=  46,		//禽畜车(特殊车辆)

	SP_IVS_TrafficVehicleBicycle                    = 201, //自行车  
	SP_IVS_TrafficVehicleVanTricycle        		= 202, //厢式三轮车
	SP_IVS_TrafficVehicleMannedConvertibleTricycle   = 203, //载人敞篷三轮车
	SP_IVS_TrafficVehicleNoMannedConvertibleTricycle = 204, //不载人敞篷三轮车
	SP_IVS_TrafficVehicleElectricbike = 205, //二轮电瓶车
}SP_IVS_TRAFFIC_VEHICLE_TYPE;

//车身属性包,64字节
typedef struct
{
	unsigned char colorVailed;	//颜色是否有效
	unsigned char carModel;		//车型,详见SP_IVS_TRAFFIC_VEHICLE_TYPE
	unsigned short brand;		//品牌车标

	unsigned char color_r;		//颜色信息，红
	unsigned char color_g;		//颜色信息，绿
	unsigned char color_b;		//颜色信息，蓝
	unsigned char color_a;		//颜色信息，透明

	unsigned short subbrand;	//子品牌
	unsigned short year;		//年款
	SP_IVS_POINT windowPosition;//车窗位置
	unsigned char nReliabilityOfVehicleBodyDetection;	//车身检测置信度，0表示算法未提供，取值范围为1-100

	unsigned char reserved[43];	//预留
}SP_IVS_TRAFFIC_ATTRIBUTE_80;

//车上人员属性包,64字节
typedef struct
{
	SP_IVS_POINT mainPosition;		//主驾驶位
	SP_IVS_POINT coPosition;		//副驾驶位
	unsigned char mainSafetyBelt;	//主驾安全带信息  0-Unknown 1-not 2-yes
	unsigned char coSafetyBelt;		//副驾安全带信息  0-Unknown 1-not 2-yes
	unsigned char mainSunvisor;		//主驾遮阳板信息  0-Unknown 1-not 2-yes
	unsigned char coSunvisor;		//副驾遮阳板信息  0-Unknown 1-not 2-yes
	unsigned char reserved[44];		//对齐
}SP_IVS_TRAFFIC_ATTRIBUTE_81;

//车牌内容属性包,256字节
typedef struct
{
	unsigned char plateEncode;		//车牌编码， 0-ASCII, 1-UCS-4LE
	unsigned char plateInfoLen;		//车牌信息长度
	unsigned char reserved[2];		//对齐

	unsigned char plateInfo[252];	//车牌信息
}SP_IVS_TRAFFIC_ATTRIBUTE_82;

//车牌外形属性包,64字节
typedef struct
{
	unsigned char colorVailed;		//颜色是否有效
	unsigned char reserved[3];		//对齐
	unsigned int color;				//颜色信息，RGBA

	unsigned char strCountry[4];	//车牌国别字符串
	unsigned short plateType;		//车牌类型
	unsigned short plateWidth;		//分析车牌画面的绝对宽度
	unsigned char  plateConfidence;	//车牌置信度
	unsigned char  reserved1[47];	//预留
}SP_IVS_TRAFFIC_ATTRIBUTE_83;

//轨迹点信息,512字节
typedef struct
{
	unsigned char fatherCount;		//父ID个数
	unsigned char pointCount;		//待增加轨迹点个数
	unsigned char trackType;		//轨迹类型，0表示IVS物体轨迹，1表示跟踪物体轨迹
	unsigned char reserved;			//对齐

	SP_IVS_POINT trackPoint[32];	//轨迹点信息
	unsigned int  fatherID[63];		//父ID列表

}SP_IVS_TRAFFIC_ATTRIBUTE_84;

//目标检测局部轨迹属性,64字节
typedef struct
{
	SP_IVS_POINT trackPoint;			//目标框位置（格式,8192坐标系）
	unsigned short	stayTime;			//目标滞留时间,单位秒
	unsigned char reserved1[54];
}SP_IVS_TRAFFIC_ATTRIBUTE_87;

//排队滞留时间属性包
typedef struct
{
	unsigned int m_QueuingTime;        	//排队滞留时间,单位秒
	unsigned char reserved[12];    		//对齐; 10字节预留字节+2字节对齐
}SP_IVS_ATTRIBUTE_90;

//单个属性包结构
typedef struct _SP_IVS_OBJ_ATTR_UNIT
{
	unsigned int nAttrFlag;				// 属性包使能标识
	unsigned int nAttrDataLen;			// 属性包长度
	unsigned char* pAttrBuf;			// 属性包数据，按使能标识读取对应属性包
}SP_IVS_OBJ_ATTR_UNIT;

//IVS智能目标结构体
typedef struct _SP_IVS_OBJ_INFO
{
	unsigned int classID;				// 业务大类ID
	unsigned int objID;					// 目标ID
	SP_IVS_POINT trackPoint;			// 轨迹点信息，仅适用与车身车牌类型目标

	unsigned char objType;				// 目标类型，车牌/车身/人etc
	unsigned char pointValid;			// 坐标是否可信
	unsigned char operatorType;			// 更新操作类型，新增物体(1), 增加物体轨迹点(2)，删除物体(3)，隐藏物体轨迹(4)
	unsigned char reserved[29];			// 保留字段

	unsigned int		nAttrCount;		// 目标携带属性包个数
	SP_IVS_OBJ_ATTR_UNIT*	pAttrData;	// 属性包数据
	unsigned char reserved1[128];		// 保留字段
}SP_IVS_OBJ_INFO;

#pragma pack(1)
typedef struct
{
	/*类型*/
	int					frameType;				/*帧类型*/
	int					frameSubType;			/*帧子类型*/
	int					frameEncodeType;		/*帧编码类型*/
	unsigned char		reserved[4];

	/*数据*/
	unsigned char*		streamPointer;			/*指向码流数据,NULL表示无效数据*/
	int					streamLen;				/*码流长度*/
    unsigned char*		framePointer;			/*指向帧头,NULL表示无效数据*/
    int					frameLen;				/*帧长度(包括帧头、帧体、帧尾)*/
    
    /*时间*/
    SP_TIME				frameTime;				/*时间信息*/
    int					timeStamp;				/*时间戳*/
    
    /*序号*/
	int					frameSeq;				/*帧序号*/
	
	/*视频属性，关键帧才有*/
	int					frameRate;				/*帧率*/
	int					width;					/*宽*/
	int					height;					/*高*/

    unsigned char  reserved2[8];

	/*音频属性*/
	int					samplesPerSec;			/*采样频率*/
	int					bitsPerSample;			/*采样位数*/
	int					channels;				/*声道数*/

	/*错误标志*/
	int					isValid;				/*0为有效，非0表示帧错误*/

 #if defined(_WIN64) || defined(__x86_64__)
 	/*扩展*/
 	unsigned char	reserved3[424];				/*保留字节*/
 #else
	/*扩展*/
 	unsigned char	reserved3[408];				/*保留字节*/
 #endif
} SP_FRAME_INFO;
#pragma pack()

 /********************************************************************
 *	Funcname: 	    	SP_CreateStreamParser
 *	Purpose:				创建流分析器
 *  InputParam:         nBufferSize: 需要开辟的缓冲区大小，不能小于SP_PaseData每次传入的数据流长度
 *  OutputParam:      无
 *  Return:					NULL: 创建流分析器失败
 *								其他值：流解析器句柄   
*********************************************************************/
SP_API void* CALLMETHOD SP_CreateStreamParser(int nBufferSize);

 /********************************************************************
 *	Funcname: 	    	SP_ParseData
 *	Purpose:				输入数据流,并同步进行分析
 *  InputParam:         handle:	通过SP_CreateStreamParser返回的句柄
 *								stream:	数据流缓冲地址
 *								length:	数据流长度
 *  OutputParam:      无
 *  Return:					0:调用成功
 *								其他值：失败，通过SP_GetLastError获取错误码    
*********************************************************************/
SP_API SP_RESULT CALLMETHOD SP_ParseData(void* handle, unsigned char* stream, int length);

/********************************************************************
 *	Funcname: 	    	SP_GetOneFrame
 *	Purpose:				同步获取一帧信息,反复调用直到失败
 *  InputParam:         handle:	通过SP_CreateStreamParser返回的句柄
 *								frameInfo: 外部SP_FRAME_INFO的一个结构地址。
 *  OutputParam:      无
 *  Return:					0:调用成功
 *								其他值：失败，通过SP_GetLastError获取错误码    
*********************************************************************/
SP_API SP_RESULT CALLMETHOD SP_GetOneFrame(void* handle, SP_FRAME_INFO* frameInfo);

/********************************************************************
 *	Funcname: 	    	SP_StreamEncryptKey
 *	Purpose:	        设置实时流解析秘钥
 *  InputParam:         handle: 通过SP_CreateStreamParser或SP_CreateFileParser返回的句柄。
 *						type : 秘钥类型 ：SP_ENCRYPT
 *						key：秘钥数据
 *						keylen：秘钥长度
 *  OutputParam:        无
 *  Return:             0:调用成功
 *                      其他值：失败，
*********************************************************************/
SP_API SP_RESULT CALLMETHOD SP_StreamEncryptKey(void* handle, unsigned int type, unsigned char* key, int keylen);

/********************************************************************
 *	Funcname: 	    	SP_Destroy
 *	Purpose:				销毁码流分析器
 *  InputParam:         handle: 通过SP_CreateStreamParser返回的句柄。
 *  OutputParam:      无
 *  Return:					0:调用成功
 *								其他值：失败，通过SP_GetLastError获取错误码     
*********************************************************************/
SP_API SP_RESULT CALLMETHOD SP_Destroy(void* handle);

/********************************************************************
 *	Funcname: 	    	SP_GetLastError
 *	Purpose:				获得码流分析库错误码
 *  InputParam:         handle: 通过SP_CreateStreamParser或SP_CreateFileParser返回的句柄。
 *  OutputParam:      无
 *  Return:					0:调用成功
 *								其他值：失败，通过SP_GetLastError获取错误码   值  
*********************************************************************/
SP_API SP_RESULT CALLMETHOD SP_GetLastError(void* handle);


/********************************************************************
 *	Funcname: 	    	IVSObjAttrCBFun
 *	Purpose:	        IVS智能目标回调函数定义
 *  OutputParam:        pIVSBuf:回调数据指针，数据结构见SP_IVS_OBJ_INFO
 *                      nIVSBuffLen： 数据长度。
 *						pUserData: 用户自定义
 *
 *  Return:             无			
*********************************************************************/
typedef void (CALLMETHOD *IVSObjAttrCBFun)(char* pIVSBuf, int nIVSBufLen, void* pReserved, void* pUserData);

/********************************************************************
 *	Funcname: 	    	SP_SetIVSObjAttrCBFun
 *	Purpose:	        IVS智能目标回调设置
 *  InputParam:         handle: 句柄
 *                      pCallBack: 回调函数指针
 *                      pUserData: 用户数据
 *  OutputParam:        无
 *  Return:             0:调用成功
 *                      其他值：失败，通过SP_GetLastError获取错误码       
*********************************************************************/
SP_API SP_RESULT CALLMETHOD SP_SetIVSObjAttrCBFun(void* handle, IVSObjAttrCBFun pCallBack, void* pUserData);
															
/********************************************************************
 *	Funcname: 	    	SP_ParseIVSEx
 *	Purpose:	        解析IVS数据帧
 *  InputParam:         handle: 句柄
 *                      pBuffer: IVS数据帧的frame或者frameBody数据，如果是IVS_PRESET(暂未定义)，
 *                               需传入包括帧头的帧指针。其他类型，则传入pFrameBody
 *                      len: pBuffer的长度
 *                      frameSubType: 帧子类型。
 *  OutputParam:        无
 *  Return:             0:调用成功
 *                      其他值：失败，通过SP_GetLastError获取错误码       
*********************************************************************/
SP_API SP_RESULT CALLMETHOD SP_ParseIVSEx(void* handle, unsigned char* pBuffer, int len, int frameSubType);

#ifdef __cplusplus
}
#endif

#endif 



