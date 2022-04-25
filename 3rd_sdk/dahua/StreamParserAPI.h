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

/*�ӿڷ���ֵ*/
enum SP_RESULT
{
    SP_SUCCESS = 0,							/*�ɹ�*/
    SP_ERROR_INVALID_HANDLE = 1,			/*��Ч���*/
    SP_ERROR_FILE_TYPE_NOSUPPORT = 2,		/*�ļ����Ͳ�֧��*/
    SP_ERROR_STREAM_NOSUPPORT = 3,			/*�����Ͳ�֧��*/
    SP_ERROR_PARAMETER = 6,					/*��������*/			
    SP_ERROR_BAD_FORMATTED = 9,     		/*�ļ���ʽ����*/
    SP_ERROR_BUFFER_OVERFLOW = 12,			/*�ڲ����������*/
    SP_ERROR_SYSTEM_OUT_OF_MEMORY = 13,		/*ϵͳ�ڴ治��*/
    SP_ERROR_LIST_EMPTY = 14,				/*�б�Ϊ��*/
};

/*֡����*/
enum SP_FRAME_TYPE
{
	SP_FRAME_TYPE_UNKNOWN = 0,			/*֡���Ͳ���֪*/
	SP_FRAME_TYPE_VIDEO,				/*֡��������Ƶ֡*/
	SP_FRAME_TYPE_AUDIO,				/*֡��������Ƶ֡*/
	SP_FRAME_TYPE_DATA,					/*֡����������֡*/
};

/*֡������*/
enum SP_FRAME_SUB_TYPE
{
	SP_FRAME_SUB_TYPE_DATA_INVALID		= -1,	/*������Ч*/
	SP_FRAME_SUB_TYPE_VIDEO_I_FRAME	= 0 ,		/*I֡*/
	SP_FRAME_SUB_TYPE_VIDEO_P_FRAME,			/*P֡*/
	SP_FRAME_SUB_TYPE_VIDEO_B_FRAME,			/*B֡*/
	SP_FRAME_SUB_TYPE_DATA_INTL =7,				/*���ܷ���֡*/
	SP_FRAME_SUB_TYPE_VIDEO_JPEG_FRAME=8,		/*JPEG֡*/
	SP_FRAME_SUB_TYPE_DATA_INTLEX =11,          /*��չ���ܷ���֡*/
};						

/*��������*/
enum SP_ENCODE_VIDEO_TYPE
{
	SP_ENCODE_VIDEO_UNKNOWN = 0,						/*��Ƶ�����ʽ����֪*/
	SP_ENCODE_VIDEO_MPEG4 ,								/*��Ƶ�����ʽ��MPEG4*/
	SP_ENCODE_VIDEO_HI_H264,							/*��Ƶ�����ʽ�Ǻ�˼H264*/
	SP_ENCODE_VIDEO_JPEG,								/*��Ƶ�����ʽ�Ǳ�׼JPEG*/
	SP_ENCODE_VIDEO_DH_H264,							/*��Ƶ�����ʽ��H264*/
	SP_ENCODE_VIDEO_MPEG2 = 9,          				/*��Ƶ�����ʽ��MPEG2*/
	SP_ENCODE_VIDEO_DH_H265 = 12,						/*��Ƶ�����ʽ��H265*/
	SP_ENCODE_VIDEO_H263 = 35,      					/*��Ƶ�����ʽ��H263*/
};

enum SP_ENCODE_AUDIO_TYPE
{
	SP_ENCODE_AUDIO_UNKNOWN = 0,
	SP_ENCODE_AUDIO_PCM			= 7,					/*��Ƶ�����ʽ��PCM8*/
	SP_ENCODE_AUDIO_G729,								/*��Ƶ�����ʽ��G729*/
	SP_ENCODE_AUDIO_IMA,								/*��Ƶ�����ʽ��IMA*/
	SP_ENCODE_PCM_MULAW,								/*��Ƶ�����ʽ��PCM MULAW*/
	SP_ENCODE_AUDIO_G721,								/*��Ƶ�����ʽ��G721*/
	SP_ENCODE_PCM8_VWIS,								/*��Ƶ�����ʽ��PCM8_VWIS*/
	SP_ENCODE_MS_ADPCM,									/*��Ƶ�����ʽ��MS_ADPCM*/
	SP_ENCODE_AUDIO_G711A,								/*��Ƶ�����ʽ��G711A*/
	SP_ENCODE_AUDIO_AMR,								/*��Ƶ�����ʽ��AMR*/
	SP_ENCODE_AUDIO_PCM16,								/*��Ƶ�����ʽ��PCM16*/
	SP_ENCODE_AUDIO_G711U		= 22,					/*��Ƶ�����ʽ��G711U*/
	SP_ENCODE_AUDIO_G723,								/*��Ƶ�����ʽ��G723*/
	SP_ENCODE_AUDIO_AAC			= 26,					/*��Ƶ�����ʽ��AAC*/
	SP_ENCODE_AUDIO_MP2 = 31,							/*��Ƶ�����ʽ��mp2*/
	SP_ENCODE_AUDIO_OGG,								/*��Ƶ�����ʽ��ogg vorbis*/
	SP_ENCODE_AUDIO_MP3,								/*��Ƶ�����ʽ��mp3*/
	SP_ENCODE_AUDIO_G722_1,								/*��Ƶ�����ʽ��G722.1*/
	SP_ENCODE_AUDIO_AC = 49,							/*��Ƶ�����ʽ��AC3*/
};

/*��������*/
enum SP_ENCRYPT_TYPE
{
    SP_ENCRYPT_UNKOWN = 0,
    SP_ENCRYPT_AES,
};
/*ʱ����Ϣ*/
typedef struct
{
    int nYear;								/*��*/
    int nMonth;								/*��*/
    int nDay;								/*��*/
    int nHour;								/*Сʱ*/
    int nMinute;							/*����*/
    int nSecond;							/*��*/
    int nMilliSecond;						/*����*/
} SP_TIME;

//���԰�ʹ�ܱ�ʶ
#define SP_IVS_ATTRIBUTE_FLAG_80	(1 << 0)
#define SP_IVS_ATTRIBUTE_FLAG_81	(1 << 1)
#define SP_IVS_ATTRIBUTE_FLAG_82	(1 << 2)
#define SP_IVS_ATTRIBUTE_FLAG_83	(1 << 3)
#define SP_IVS_ATTRIBUTE_FLAG_84	(1 << 4)
#define SP_IVS_ATTRIBUTE_FLAG_87	(1 << 7)
#define SP_IVS_ATTRIBUTE_FLAG_90	(1 << 16)

typedef struct
{
	/*�켣����������Ӿ��ε����ģ�����X��Y��XSize��YSize�������������Ӿ�������(left��top��right��bottom)*/
	/*RECT=(X-XSize, Y-YSize, X+XSize, Y+YSize)  */
	unsigned short x;
	unsigned short y;
	unsigned short xSize;
	unsigned short ySize;
}SP_IVS_POINT;

//0x80�������԰�, �������Ͷ���
typedef enum
{
	SP_IVS_TrafficVehicleUnknow = 0,		//δ֪
	SP_IVS_TrafficVehiclePassengerCar,		//�ͳ�
	SP_IVS_TrafficVehicleLargeTruck,		//�����
	SP_IVS_TrafficVehicleMidTruck,			//�л���
	SP_IVS_TrafficVehicleSaloonCar,		//�γ�
	SP_IVS_TrafficVehicleMicrobus,			//�����
	SP_IVS_TrafficVehicleMicroTruck,		//С����
	SP_IVS_TrafficVehicleTricycle,			//���ֳ�
	SP_IVS_TrafficVehicleMotor,			//Ħ�г�
	SP_IVS_TrafficVehiclePasserby,			//����
	SP_IVS_TrafficVehicleSuvMpv,			//SUV-MPV
	SP_IVS_TrafficVehicleMidPassengerCar,	//�пͳ�
	SP_IVS_TrafficVehicleTankCar,			//Σ��Ʒ����(���⳵��)
	SP_IVS_TrafficVehicleSUV,				//SUV
	SP_IVS_TrafficVehicleMPV,				//MPV
	SP_IVS_TrafficVehicleBus,				//������
	SP_IVS_TrafficVehiclePickup,			//Ƥ����
	SP_IVS_TrafficVehicleMiniCarriage,		//΢�ͳ�
	SP_IVS_TrafficVehicleOilTankTruck,		//�͹޳�(���⳵��)
	SP_IVS_TrafficVehicleSlotTankCar,   	//�۹޳�(���⳵��)
	SP_IVS_TrafficVehicleColdChainCar,   	//������(���⳵��)
	SP_IVS_TrafficVehicleDregsCar,			//������(���⳵��)
	SP_IVS_TrafficVehicleConcreteMixerTruck,	//���������賵(���⳵��)
	SP_IVS_TrafficVehicleTaxi,             //���⳵(���⳵��)
	SP_IVS_TrafficVehiclePolice,           //����(���⳵��)
	SP_IVS_TrafficVehicleAmbulance,        //�Ȼ���(���⳵��)
	SP_IVS_TrafficVehicleGeneral,          //��ͨ��(���⳵��)
	SP_IVS_TrafficVehicleWateringCar,      //������(������ˮ��������������ɨ����)(���⳵��)
	SP_IVS_TrafficVehicleReserved2,   		//����2(���⳵��)
	SP_IVS_TrafficVehicleFireEngine,       //������(���⳵��)
	SP_IVS_TrafficVehicleTractor,          //������(���⳵��)
	SP_IVS_TrafficVehicleMachineshopTruck, //���̳�(���⳵��)
	SP_IVS_TrafficVehiclePowerLotVehicle,  //�������ϳ�(���⳵��)
	SP_IVS_TrafficVehicleSuctionSewageTruck,	//���۳�(���⳵��)
	SP_IVS_TrafficNormalVehicleTankTruck,  	//��ͨ�޳�(���⳵��) 
	SP_IVS_TrafficVehicleTwocycle,          	//���ֳ�
	SP_IVS_TrafficVehicleSchoolBus,			//У��(���⳵��) 
	SP_IVS_TrafficVehicleExcavator,			//�ھ�(���⳵��)
	SP_IVS_TrafficVehicleBulldozer,			//������(���⳵��)
	SP_IVS_TrafficVehicleCrane,				//����(���⳵��)
	SP_IVS_TrafficVehiclePumptruck,			//�ó�(���⳵��)
	SP_IVS_TrafficVehicleEscort,				//Ѻ�˳�(���⳵��)
	SP_IVS_TrafficVehicleShovelLoader,			//����(���⳵��)
	SP_IVS_TrafficVehiclePoultry	=  46,		//����(���⳵��)

	SP_IVS_TrafficVehicleBicycle                    = 201, //���г�  
	SP_IVS_TrafficVehicleVanTricycle        		= 202, //��ʽ���ֳ�
	SP_IVS_TrafficVehicleMannedConvertibleTricycle   = 203, //���˳������ֳ�
	SP_IVS_TrafficVehicleNoMannedConvertibleTricycle = 204, //�����˳������ֳ�
	SP_IVS_TrafficVehicleElectricbike = 205, //���ֵ�ƿ��
}SP_IVS_TRAFFIC_VEHICLE_TYPE;

//�������԰�,64�ֽ�
typedef struct
{
	unsigned char colorVailed;	//��ɫ�Ƿ���Ч
	unsigned char carModel;		//����,���SP_IVS_TRAFFIC_VEHICLE_TYPE
	unsigned short brand;		//Ʒ�Ƴ���

	unsigned char color_r;		//��ɫ��Ϣ����
	unsigned char color_g;		//��ɫ��Ϣ����
	unsigned char color_b;		//��ɫ��Ϣ����
	unsigned char color_a;		//��ɫ��Ϣ��͸��

	unsigned short subbrand;	//��Ʒ��
	unsigned short year;		//���
	SP_IVS_POINT windowPosition;//����λ��
	unsigned char nReliabilityOfVehicleBodyDetection;	//���������Ŷȣ�0��ʾ�㷨δ�ṩ��ȡֵ��ΧΪ1-100

	unsigned char reserved[43];	//Ԥ��
}SP_IVS_TRAFFIC_ATTRIBUTE_80;

//������Ա���԰�,64�ֽ�
typedef struct
{
	SP_IVS_POINT mainPosition;		//����ʻλ
	SP_IVS_POINT coPosition;		//����ʻλ
	unsigned char mainSafetyBelt;	//���ݰ�ȫ����Ϣ  0-Unknown 1-not 2-yes
	unsigned char coSafetyBelt;		//���ݰ�ȫ����Ϣ  0-Unknown 1-not 2-yes
	unsigned char mainSunvisor;		//������������Ϣ  0-Unknown 1-not 2-yes
	unsigned char coSunvisor;		//������������Ϣ  0-Unknown 1-not 2-yes
	unsigned char reserved[44];		//����
}SP_IVS_TRAFFIC_ATTRIBUTE_81;

//�����������԰�,256�ֽ�
typedef struct
{
	unsigned char plateEncode;		//���Ʊ��룬 0-ASCII, 1-UCS-4LE
	unsigned char plateInfoLen;		//������Ϣ����
	unsigned char reserved[2];		//����

	unsigned char plateInfo[252];	//������Ϣ
}SP_IVS_TRAFFIC_ATTRIBUTE_82;

//�����������԰�,64�ֽ�
typedef struct
{
	unsigned char colorVailed;		//��ɫ�Ƿ���Ч
	unsigned char reserved[3];		//����
	unsigned int color;				//��ɫ��Ϣ��RGBA

	unsigned char strCountry[4];	//���ƹ����ַ���
	unsigned short plateType;		//��������
	unsigned short plateWidth;		//�������ƻ���ľ��Կ��
	unsigned char  plateConfidence;	//�������Ŷ�
	unsigned char  reserved1[47];	//Ԥ��
}SP_IVS_TRAFFIC_ATTRIBUTE_83;

//�켣����Ϣ,512�ֽ�
typedef struct
{
	unsigned char fatherCount;		//��ID����
	unsigned char pointCount;		//�����ӹ켣�����
	unsigned char trackType;		//�켣���ͣ�0��ʾIVS����켣��1��ʾ��������켣
	unsigned char reserved;			//����

	SP_IVS_POINT trackPoint[32];	//�켣����Ϣ
	unsigned int  fatherID[63];		//��ID�б�

}SP_IVS_TRAFFIC_ATTRIBUTE_84;

//Ŀ����ֲ��켣����,64�ֽ�
typedef struct
{
	SP_IVS_POINT trackPoint;			//Ŀ���λ�ã���ʽ,8192����ϵ��
	unsigned short	stayTime;			//Ŀ������ʱ��,��λ��
	unsigned char reserved1[54];
}SP_IVS_TRAFFIC_ATTRIBUTE_87;

//�Ŷ�����ʱ�����԰�
typedef struct
{
	unsigned int m_QueuingTime;        	//�Ŷ�����ʱ��,��λ��
	unsigned char reserved[12];    		//����; 10�ֽ�Ԥ���ֽ�+2�ֽڶ���
}SP_IVS_ATTRIBUTE_90;

//�������԰��ṹ
typedef struct _SP_IVS_OBJ_ATTR_UNIT
{
	unsigned int nAttrFlag;				// ���԰�ʹ�ܱ�ʶ
	unsigned int nAttrDataLen;			// ���԰�����
	unsigned char* pAttrBuf;			// ���԰����ݣ���ʹ�ܱ�ʶ��ȡ��Ӧ���԰�
}SP_IVS_OBJ_ATTR_UNIT;

//IVS����Ŀ��ṹ��
typedef struct _SP_IVS_OBJ_INFO
{
	unsigned int classID;				// ҵ�����ID
	unsigned int objID;					// Ŀ��ID
	SP_IVS_POINT trackPoint;			// �켣����Ϣ���������복��������Ŀ��

	unsigned char objType;				// Ŀ�����ͣ�����/����/��etc
	unsigned char pointValid;			// �����Ƿ����
	unsigned char operatorType;			// ���²������ͣ���������(1), ��������켣��(2)��ɾ������(3)����������켣(4)
	unsigned char reserved[29];			// �����ֶ�

	unsigned int		nAttrCount;		// Ŀ��Я�����԰�����
	SP_IVS_OBJ_ATTR_UNIT*	pAttrData;	// ���԰�����
	unsigned char reserved1[128];		// �����ֶ�
}SP_IVS_OBJ_INFO;

#pragma pack(1)
typedef struct
{
	/*����*/
	int					frameType;				/*֡����*/
	int					frameSubType;			/*֡������*/
	int					frameEncodeType;		/*֡��������*/
	unsigned char		reserved[4];

	/*����*/
	unsigned char*		streamPointer;			/*ָ����������,NULL��ʾ��Ч����*/
	int					streamLen;				/*��������*/
    unsigned char*		framePointer;			/*ָ��֡ͷ,NULL��ʾ��Ч����*/
    int					frameLen;				/*֡����(����֡ͷ��֡�塢֡β)*/
    
    /*ʱ��*/
    SP_TIME				frameTime;				/*ʱ����Ϣ*/
    int					timeStamp;				/*ʱ���*/
    
    /*���*/
	int					frameSeq;				/*֡���*/
	
	/*��Ƶ���ԣ��ؼ�֡����*/
	int					frameRate;				/*֡��*/
	int					width;					/*��*/
	int					height;					/*��*/

    unsigned char  reserved2[8];

	/*��Ƶ����*/
	int					samplesPerSec;			/*����Ƶ��*/
	int					bitsPerSample;			/*����λ��*/
	int					channels;				/*������*/

	/*�����־*/
	int					isValid;				/*0Ϊ��Ч����0��ʾ֡����*/

 #if defined(_WIN64) || defined(__x86_64__)
 	/*��չ*/
 	unsigned char	reserved3[424];				/*�����ֽ�*/
 #else
	/*��չ*/
 	unsigned char	reserved3[408];				/*�����ֽ�*/
 #endif
} SP_FRAME_INFO;
#pragma pack()

 /********************************************************************
 *	Funcname: 	    	SP_CreateStreamParser
 *	Purpose:				������������
 *  InputParam:         nBufferSize: ��Ҫ���ٵĻ�������С������С��SP_PaseDataÿ�δ��������������
 *  OutputParam:      ��
 *  Return:					NULL: ������������ʧ��
 *								����ֵ�������������   
*********************************************************************/
SP_API void* CALLMETHOD SP_CreateStreamParser(int nBufferSize);

 /********************************************************************
 *	Funcname: 	    	SP_ParseData
 *	Purpose:				����������,��ͬ�����з���
 *  InputParam:         handle:	ͨ��SP_CreateStreamParser���صľ��
 *								stream:	�����������ַ
 *								length:	����������
 *  OutputParam:      ��
 *  Return:					0:���óɹ�
 *								����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������    
*********************************************************************/
SP_API SP_RESULT CALLMETHOD SP_ParseData(void* handle, unsigned char* stream, int length);

/********************************************************************
 *	Funcname: 	    	SP_GetOneFrame
 *	Purpose:				ͬ����ȡһ֡��Ϣ,��������ֱ��ʧ��
 *  InputParam:         handle:	ͨ��SP_CreateStreamParser���صľ��
 *								frameInfo: �ⲿSP_FRAME_INFO��һ���ṹ��ַ��
 *  OutputParam:      ��
 *  Return:					0:���óɹ�
 *								����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������    
*********************************************************************/
SP_API SP_RESULT CALLMETHOD SP_GetOneFrame(void* handle, SP_FRAME_INFO* frameInfo);

/********************************************************************
 *	Funcname: 	    	SP_StreamEncryptKey
 *	Purpose:	        ����ʵʱ��������Կ
 *  InputParam:         handle: ͨ��SP_CreateStreamParser��SP_CreateFileParser���صľ����
 *						type : ��Կ���� ��SP_ENCRYPT
 *						key����Կ����
 *						keylen����Կ����
 *  OutputParam:        ��
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�
*********************************************************************/
SP_API SP_RESULT CALLMETHOD SP_StreamEncryptKey(void* handle, unsigned int type, unsigned char* key, int keylen);

/********************************************************************
 *	Funcname: 	    	SP_Destroy
 *	Purpose:				��������������
 *  InputParam:         handle: ͨ��SP_CreateStreamParser���صľ����
 *  OutputParam:      ��
 *  Return:					0:���óɹ�
 *								����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������     
*********************************************************************/
SP_API SP_RESULT CALLMETHOD SP_Destroy(void* handle);

/********************************************************************
 *	Funcname: 	    	SP_GetLastError
 *	Purpose:				������������������
 *  InputParam:         handle: ͨ��SP_CreateStreamParser��SP_CreateFileParser���صľ����
 *  OutputParam:      ��
 *  Return:					0:���óɹ�
 *								����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������   ֵ  
*********************************************************************/
SP_API SP_RESULT CALLMETHOD SP_GetLastError(void* handle);


/********************************************************************
 *	Funcname: 	    	IVSObjAttrCBFun
 *	Purpose:	        IVS����Ŀ��ص���������
 *  OutputParam:        pIVSBuf:�ص�����ָ�룬���ݽṹ��SP_IVS_OBJ_INFO
 *                      nIVSBuffLen�� ���ݳ��ȡ�
 *						pUserData: �û��Զ���
 *
 *  Return:             ��			
*********************************************************************/
typedef void (CALLMETHOD *IVSObjAttrCBFun)(char* pIVSBuf, int nIVSBufLen, void* pReserved, void* pUserData);

/********************************************************************
 *	Funcname: 	    	SP_SetIVSObjAttrCBFun
 *	Purpose:	        IVS����Ŀ��ص�����
 *  InputParam:         handle: ���
 *                      pCallBack: �ص�����ָ��
 *                      pUserData: �û�����
 *  OutputParam:        ��
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������       
*********************************************************************/
SP_API SP_RESULT CALLMETHOD SP_SetIVSObjAttrCBFun(void* handle, IVSObjAttrCBFun pCallBack, void* pUserData);
															
/********************************************************************
 *	Funcname: 	    	SP_ParseIVSEx
 *	Purpose:	        ����IVS����֡
 *  InputParam:         handle: ���
 *                      pBuffer: IVS����֡��frame����frameBody���ݣ������IVS_PRESET(��δ����)��
 *                               �贫�����֡ͷ��ָ֡�롣�������ͣ�����pFrameBody
 *                      len: pBuffer�ĳ���
 *                      frameSubType: ֡�����͡�
 *  OutputParam:        ��
 *  Return:             0:���óɹ�
 *                      ����ֵ��ʧ�ܣ�ͨ��SP_GetLastError��ȡ������       
*********************************************************************/
SP_API SP_RESULT CALLMETHOD SP_ParseIVSEx(void* handle, unsigned char* pBuffer, int len, int frameSubType);

#ifdef __cplusplus
}
#endif

#endif 



