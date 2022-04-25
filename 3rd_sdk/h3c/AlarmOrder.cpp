#include "idm_netsdk.h"
#include <iostream>
#include <string>
#include <string.h>

VOID CALLBACK My_Alarm_Callback(
    LONG lUserID,
    ULONG ulCommand,
    VOID *pBuffer,
    ULONG ulBufferSize,
    IDM_DEV_ALARM_DEVICE_INFO_S *pstAlarmDeviceInfo,
    VOID *pUserData) {
    std::cout << "IDM_DEV_Message_Callback_PF userID:" << lUserID << std::endl;
    std::cout << "IDM_DEV_Message_Callback_PF userData:" << (char *)pUserData << std::endl;
    std::cout << "IDM_DEV_Message_Callback_PF ulCommand " << ulCommand << std::endl;
    IDM_DEV_ALARM_EVENT_S *pinfo = (IDM_DEV_ALARM_EVENT_S *)pBuffer;
    std::cout << "IDM_DEV_Message_Callback_PF Event Type"<< pinfo->ulEventType << std::endl;
    std::string json;
    if ((0 != pinfo->stEvent.ulBufferSize) && (nullptr != pinfo->stEvent.pBuffer)) {
        json = pinfo->stEvent.pBuffer;
        std::cout << "JSON:" << std::endl;
        std::cout << json << std::endl;
    }
}

void doLoginSample()
{
	LONG lUserID = 0;
	IDM_DEV_Init();//初始化库

	IDM_DEV_SaveLogToFile(3, 0, "/home/edge/To_H3C/log");

	//登录
	IDM_DEV_DEVICE_INFO_S devInfo = { 0 };
	IDM_DEV_USER_LOGIN_INFO_S loginInfo = { 0 };
	strncpy(loginInfo.szTargetIP, "192.168.13.227", sizeof(loginInfo.szTargetIP) - 1);
	loginInfo.usPort = 9000;
	strncpy(loginInfo.szUsername, "admin", IDM_USERNAME_MAX_LEN - 1);
	// std::cout << "Enter password: admin123";
	strncpy(loginInfo.szPassword, "edge2021", IDM_PASSWORD_MAX_LEN - 1);
	loginInfo.lLoginMode = 0;
	int ret = IDM_DEV_Login(loginInfo, &devInfo, &lUserID);
	if (ret != IDM_SUCCESS)
	{
		std::cout << "login failed:" + ret << std::endl;
		IDM_DEV_Cleanup();
		return;
	}
	std::cout << "1111111111111111111111" << std::endl;
	void *sumUserData = NULL;
	//注册回调
	ret = IDM_DEV_SetAlarmCallback(0, My_Alarm_Callback, sumUserData);
	std::cout <<  "ret: " << ret << std::endl;
	if (ret != IDM_SUCCESS)
	{
		std::cout << "---------------------------------------" << std::endl;
		std::cout << "set callback failed:" + ret << std::endl;
		IDM_DEV_Logout(lUserID);
		IDM_DEV_Cleanup();
		return;
	}
	std::cout << "222222222222222222222222222222" << std::endl;
	
	//订阅事件
	LONG lAlarmHandle = 0;
	std::string sub = "{\"channel_no_list\":[0,1,5],\"event_types\":[16842753,16908290],\"event_levels\":[0,2]}";
    IDM_DEV_ALARM_PARAM_S stAlarmParam = {0};
    stAlarmParam.ulLevel = 0;
    stAlarmParam.pcSubscribes = const_cast<char *>(sub.c_str());
    stAlarmParam.ulSubscribesLen = sub.length();
    stAlarmParam.ucType = 0;
    stAlarmParam.ucLinkMode = 0;// 0xFF;
    ret = IDM_DEV_StartAlarmUp(lUserID, stAlarmParam, &lAlarmHandle);
	if (ret != IDM_SUCCESS)
	{
		std::cout << "set callback failed:" + ret << std::endl;
		IDM_DEV_Logout(lUserID);
		IDM_DEV_Cleanup();
		return;
	}
	
	while (1)
	{
		//do somthing
	}
	
	//停止订阅
	ret = IDM_DEV_StopAlarmUp(lAlarmHandle);

	//退出登录
	IDM_DEV_Logout(lUserID);

	//清理库
	IDM_DEV_Cleanup();
}

int main()
{
	int rval = 0;

	doLoginSample();

	return rval;
}