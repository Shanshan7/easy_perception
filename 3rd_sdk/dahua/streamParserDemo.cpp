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

	//objIDΪĿ��ID
	printf("objid:%d,objType:%d\n", pObjInfo->objID, pObjInfo->objType);

	//1.����㶼�ǻ���8192����ϵ
	//2.��Ŀ��������02��05����pointValid�ֶ���Чʱ��IVS_OBJ_INFO.trackPoint������Ч
	//3.���԰��е������,����84��87�е�����㣬��IVS_OBJ_INFO.trackPoint���겻��ͬʱ���ڣ�һ�ֳ����½�һ�������
	if (pObjInfo->objType == 0x02 || pObjInfo->objType == 0x05 || pObjInfo->pointValid == 1)
	{
		printf("trackBox: x:%d,y:%d,xSize:%d,ySize:%d\n", pObjInfo->trackPoint.x, pObjInfo->trackPoint.y, pObjInfo->trackPoint.xSize, pObjInfo->trackPoint.ySize);
	}

	//���԰���Ϣ
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
			//������ɫ
			SP_IVS_TRAFFIC_ATTRIBUTE_80* attr80 = (SP_IVS_TRAFFIC_ATTRIBUTE_80*)(pAttrDataUnitTmp->pAttrBuf);
			printf("80 vehicle color rgb:%d %d %d\n", attr80->color_r, attr80->color_g, attr80->color_b);
		}
		else if (pAttrDataUnitTmp->nAttrFlag == SP_IVS_ATTRIBUTE_FLAG_82)
		{
			//������Ϣ
			SP_IVS_TRAFFIC_ATTRIBUTE_82* attr82 = (SP_IVS_TRAFFIC_ATTRIBUTE_82*)(pAttrDataUnitTmp->pAttrBuf);
			printf("82 plateEncode:%d content:%s\n", attr82->plateEncode, attr82->plateInfo);
		}
		else if (pAttrDataUnitTmp->nAttrFlag == SP_IVS_ATTRIBUTE_FLAG_83)
		{
			//������ɫ
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

	//��ʼ��������
	void* handle = SP_CreateStreamParser(1024*1024);
	//IVS����Ŀ��ص�����
	SP_SetIVSObjAttrCBFun(handle, IVSObjAttrCallBack, NULL);

	int numberOfBytesRead = 0;
	while(numberOfBytesRead = fread(pBuffer,1,100*1024 ,fin))
	{
		if (numberOfBytesRead > 0 )
		{
			//�����������������֡����
			SP_ParseData(handle,pBuffer,numberOfBytesRead);
			SP_FRAME_INFO tmpFrame = {0};

			// һ��Parse���ܻ�ö�֡����ȡһ֡���ݣ�ֱ��ʧ��
			while (SP_SUCCESS == SP_GetOneFrame(handle,&tmpFrame)) 
			{               
				//��Ƶ֡
				if (tmpFrame.frameType == SP_FRAME_TYPE_VIDEO)
				{
					//��ȡ��Ƶ֡��Ӧ�ĺ��뼶ʱ��
					printf("Video frameType:%d,frameSubType:%d,frameTime:%d-%d-%dT%d::%d::%d.%d.\n",tmpFrame.frameType, tmpFrame.frameSubType, tmpFrame.frameTime.nYear,tmpFrame.frameTime.nMonth,tmpFrame.frameTime.nDay,tmpFrame.frameTime.nHour,tmpFrame.frameTime.nMinute,tmpFrame.frameTime.nSecond,tmpFrame.frameTime.nMilliSecond);
				}
				//��Ƶ֡
				else if (tmpFrame.frameType == SP_FRAME_TYPE_AUDIO)
				{
					//printf("Audio frameType:%d,frameSubType:%d,frameTime:%d-%d-%dT%d::%d::%d.%d.\n",tmpFrame.frameType, tmpFrame.frameSubType, tmpFrame.frameTime.nYear,tmpFrame.frameTime.nMonth,tmpFrame.frameTime.nDay,tmpFrame.frameTime.nHour,tmpFrame.frameTime.nMinute,tmpFrame.frameTime.nSecond,tmpFrame.frameTime.nMilliSecond);
				}
				//����֡
				else if (tmpFrame.frameType == SP_FRAME_TYPE_DATA)
				{
					if (tmpFrame.frameSubType == SP_FRAME_SUB_TYPE_DATA_INTL
						|| tmpFrame.frameSubType == SP_FRAME_SUB_TYPE_DATA_INTLEX)
					{
						//��ȡ����֡��Ӧ�ĺ��뼶ʱ��
						printf("Data frameType:%d,frameSubType:%d,frameTime:%d-%d-%dT%d::%d::%d.%d.\n",tmpFrame.frameType, tmpFrame.frameSubType, tmpFrame.frameTime.nYear,tmpFrame.frameTime.nMonth,tmpFrame.frameTime.nDay,tmpFrame.frameTime.nHour,tmpFrame.frameTime.nMinute,tmpFrame.frameTime.nSecond,tmpFrame.frameTime.nMilliSecond);
						//����֡������Ϣ, ͨ��IVSObjAttrCallBack�ص��ӿڸ���
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
