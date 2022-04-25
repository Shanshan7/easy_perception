#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "StreamParserAPI.h"

#ifdef _WIN32
#pragma comment(lib, "StreamParserd.lib")
#endif

void CALLMETHOD IVSObjAttrCallBack(char* pIVSBuf, int nIVSBufLen, void* pReserved, void* pUserData)
{
	SP_IVS_OBJ_INFO* pObjInfo = (SP_IVS_OBJ_INFO*)pIVSBuf;

	//objID为目标ID
	printf("objid:%d,objType:%d\n", pObjInfo->objID, pObjInfo->objType);

	//1.坐标点都是基于8192坐标系
	//2.当目标类型是02和05或者pointValid字段有效时，IVS_OBJ_INFO.trackPoint坐标有效
	//3.属性包中的坐标点,比如84或87中的坐标点，跟IVS_OBJ_INFO.trackPoint坐标不会同时存在，一种场景下仅一个会存在
	if (pObjInfo->objType == 0x02 || pObjInfo->objType == 0x05 || pObjInfo->pointValid == 1)
	{
		printf("trackBox: x:%d,y:%d,xSize:%d,ySize:%d\n", pObjInfo->trackPoint.x, pObjInfo->trackPoint.y, pObjInfo->trackPoint.xSize, pObjInfo->trackPoint.ySize);
	}

	//属性包信息
	unsigned char* pAttrDataUnit = (unsigned char*)pObjInfo->pAttrData;
	if(pAttrDataUnit == NULL)
	{
		return;
	}
	for (int i = 0; i < pObjInfo->nAttrCount; i++)
	{	
		SP_IVS_OBJ_ATTR_UNIT* pAttrDataUnitTmp = (SP_IVS_OBJ_ATTR_UNIT*)pAttrDataUnit;
		if (pAttrDataUnitTmp->nAttrFlag == SP_IVS_ATTRIBUTE_FLAG_80)
		{
			//车身颜色
			SP_IVS_TRAFFIC_ATTRIBUTE_80* attr80 = (SP_IVS_TRAFFIC_ATTRIBUTE_80*)(pAttrDataUnitTmp->pAttrBuf);
			printf("80 vehicle color rgb:%d %d %d\n", attr80->color_r, attr80->color_g, attr80->color_b);
		}
		else if (pAttrDataUnitTmp->nAttrFlag == SP_IVS_ATTRIBUTE_FLAG_82)
		{
			//车牌信息
			SP_IVS_TRAFFIC_ATTRIBUTE_82* attr82 = (SP_IVS_TRAFFIC_ATTRIBUTE_82*)(pAttrDataUnitTmp->pAttrBuf);
			printf("82 plateEncode:%d content:%s\n", attr82->plateEncode, attr82->plateInfo);
		}
		else if (pAttrDataUnitTmp->nAttrFlag == SP_IVS_ATTRIBUTE_FLAG_83)
		{
			//车牌颜色
			SP_IVS_TRAFFIC_ATTRIBUTE_83* attr83 = (SP_IVS_TRAFFIC_ATTRIBUTE_83*)(pAttrDataUnitTmp->pAttrBuf);
			printf("83 plate color:0x%x\n", attr83->color);
		}
		else if (pAttrDataUnitTmp->nAttrFlag == SP_IVS_ATTRIBUTE_FLAG_84)
		{
			SP_IVS_TRAFFIC_ATTRIBUTE_84* attr84 = (SP_IVS_TRAFFIC_ATTRIBUTE_84*)(pAttrDataUnitTmp->pAttrBuf);
			SP_IVS_POINT trackBox = attr84->trackPoint[attr84->pointCount - 1];
			printf("84 trackBox: x:%d,y:%d,xSize:%d,ySize:%d\n", trackBox.x, trackBox.y, trackBox.xSize, trackBox.ySize);
		}
		else if (pAttrDataUnitTmp->nAttrFlag == SP_IVS_ATTRIBUTE_FLAG_87)
		{
			SP_IVS_TRAFFIC_ATTRIBUTE_87* attr87 = (SP_IVS_TRAFFIC_ATTRIBUTE_87*)(pAttrDataUnitTmp->pAttrBuf);
			printf("87 trackBox: x:%d,y:%d,xSize:%d,ySize:%d, stayTime:%d\n", attr87->trackPoint.x, attr87->trackPoint.y, attr87->trackPoint.xSize, attr87->trackPoint.ySize, attr87->stayTime);
		}
		else if (pAttrDataUnitTmp->nAttrFlag == SP_IVS_ATTRIBUTE_FLAG_90)
		{
			SP_IVS_ATTRIBUTE_90* attr90 = (SP_IVS_ATTRIBUTE_90*)(pAttrDataUnitTmp->pAttrBuf);
			printf("90 attribute QueuingTime:%d\n", attr90->m_QueuingTime);
		}
		pAttrDataUnit += sizeof(SP_IVS_OBJ_ATTR_UNIT);
	}
}

int main(int argc, char* argv[])
{
	FILE* fin = fopen("in.dav", "rb");
	if (fin == NULL)
	{
		return -1;
	}

	unsigned char* pBuffer = new unsigned char[100*1024];
	if(pBuffer == NULL)
	{
		printf("out of memory.\n");
		return -1;
	}

	//初始化解析器
	void* handle = SP_CreateStreamParser(1024*1024);
	//IVS智能目标回调设置
	SP_SetIVSObjAttrCBFun(handle, IVSObjAttrCallBack, NULL);

	int numberOfBytesRead = 0;
	while(numberOfBytesRead = fread(pBuffer,1,100*1024 ,fin))
	{
		if (numberOfBytesRead > 0 )
		{
			//送入码流分析库进行帧分析
			SP_ParseData(handle,pBuffer,numberOfBytesRead);
			SP_FRAME_INFO tmpFrame = {0};

			// 一次Parse可能获得多帧，获取一帧数据，直到失败
			while (SP_SUCCESS == SP_GetOneFrame(handle,&tmpFrame)) 
			{               
				//视频帧
				if (tmpFrame.frameType == SP_FRAME_TYPE_VIDEO)
				{
					//获取视频帧对应的毫秒级时间
					printf("Video frameType:%d,frameSubType:%d,frameTime:%d-%d-%dT%d::%d::%d.%d.\n",tmpFrame.frameType, tmpFrame.frameSubType, tmpFrame.frameTime.nYear,tmpFrame.frameTime.nMonth,tmpFrame.frameTime.nDay,tmpFrame.frameTime.nHour,tmpFrame.frameTime.nMinute,tmpFrame.frameTime.nSecond,tmpFrame.frameTime.nMilliSecond);
				}
				//音频帧
				else if (tmpFrame.frameType == SP_FRAME_TYPE_AUDIO)
				{
					//printf("Audio frameType:%d,frameSubType:%d,frameTime:%d-%d-%dT%d::%d::%d.%d.\n",tmpFrame.frameType, tmpFrame.frameSubType, tmpFrame.frameTime.nYear,tmpFrame.frameTime.nMonth,tmpFrame.frameTime.nDay,tmpFrame.frameTime.nHour,tmpFrame.frameTime.nMinute,tmpFrame.frameTime.nSecond,tmpFrame.frameTime.nMilliSecond);
				}
				//智能帧
				else if (tmpFrame.frameType == SP_FRAME_TYPE_DATA)
				{
					if (tmpFrame.frameSubType == SP_FRAME_SUB_TYPE_DATA_INTL
						|| tmpFrame.frameSubType == SP_FRAME_SUB_TYPE_DATA_INTLEX)
					{
						//获取智能帧对应的毫秒级时间
						printf("Data frameType:%d,frameSubType:%d,frameTime:%d-%d-%dT%d::%d::%d.%d.\n",tmpFrame.frameType, tmpFrame.frameSubType, tmpFrame.frameTime.nYear,tmpFrame.frameTime.nMonth,tmpFrame.frameTime.nDay,tmpFrame.frameTime.nHour,tmpFrame.frameTime.nMinute,tmpFrame.frameTime.nSecond,tmpFrame.frameTime.nMilliSecond);
						//智能帧解析信息, 通过IVSObjAttrCallBack回调接口给出
						SP_ParseIVSEx(handle, tmpFrame.streamPointer, tmpFrame.streamLen, tmpFrame.frameSubType);
					}
				}
			}
		}
		else
		{
			break;
		}
	}

	SP_Destroy(handle);
	delete[] pBuffer;
	fclose(fin);

	printf("streamParser Over!\n");

	return 0;
}
