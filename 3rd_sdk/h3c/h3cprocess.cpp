#include "h3cprocess.h"

#include <json/json.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include<sstream>
#include<time.h>

static std::string result = "";
std::ofstream outFile;

VOID CALLBACK My_Alarm_Callback(
    LONG lUserID,
    ULONG ulCommand,
    VOID *pBuffer,
    ULONG ulBufferSize,
    IDM_DEV_ALARM_DEVICE_INFO_S *pstAlarmDeviceInfo,
    VOID *pUserData)
{
    std::cout << "IDM_DEV_Message_Callback_PF userID:" << lUserID << std::endl;
    // std::cout << "IDM_DEV_Message_Callback_PF userData:" << (char *)pUserData << std::endl;
    std::cout << "IDM_DEV_Message_Callback_PF ulCommand " << ulCommand << std::endl;
    IDM_DEV_ALARM_EVENT_S *pinfo = (IDM_DEV_ALARM_EVENT_S *)pBuffer;
    std::cout << "IDM_DEV_Message_Callback_PF Event Type"<< pinfo->ulEventType << std::endl;
    std::string json;

    


    if ((0 != pinfo->stEvent.ulBufferSize) && (nullptr != pinfo->stEvent.pBuffer)) {
        json = pinfo->stEvent.pBuffer;
        std::cout << "Json------------------------------------------------------------------------------------: " << std::endl;
        //std::cout<<"----------------------------------------------------------------------------------------"<<std::endl;
        std::cout << json << std::endl;

        //------------------------------------------------------------------------------------------------------//
        Json::Reader  reader;
        Json::Value     root;
        Json::Value     rootson;

        if(reader.parse(json,rootson))
        {
            //std::cout<<"json open is ok"<<std::endl;
            std::string sex1="."; 
            std::string  glasses1 =".";
            std::string  cap1=".";
            std::string  respitator1=".";
            char mystr[25]={0};
            long long time =0;
            if(rootson.isMember("event_body"))
            {
                //std::cout<<"+++++++++++++++++++++++++++++++++YES++++++++++++++++++++++++++++++++++"<<std::endl;
                root =rootson["event_body"];
            }
            if(root.isMember("genderCode")){
                int  sex                  =root["genderCode"].asInt();   
                //std::cout<<"__________________________"<<sex<<"_________________________________"<<std::endl;
                if(1==sex){
                       sex1="male";
                 }
                if(2==sex){
                       sex1="female";
                 }
                if(0==sex||9==sex){
                sex1="unknow";
                }
            }   
            if(root.isMember("passTime")){
                double time1=0;
                time1           =root["passTime"].asDouble();       
                time = (long long)time1;
                time/=1000;
                //std::cout<<"+++++++++++++++++++++++++++time+++++++++++++++++++++++++++++"<<std::endl;                
                struct tm *t=gmtime((time_t*)&time);
                //std::cout<<"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"<<std::endl;
                t->tm_hour+=8;
                std::string myFormat = "%Y-%m-%d:%H:%M:%S";
                strftime(mystr,sizeof(mystr),myFormat.c_str(),t);
                std::cout<<mystr<<std::endl;
            }
            if(root.isMember("isGlasses")){
                 int  glasses          =root["isGlasses"].asInt();
                 if(0==glasses){
                    glasses1="NO";
                  }
                 if(1==glasses){
                    glasses1="YES";
                  }
            }
            if(root.isMember("isCap")){
                int  cap                 =root["isCap"].asInt(); 
                 std::cout<<"_____________________________________________"<<std::endl;
                if(0==cap){
                  cap1="NO";
             } 
                if(1==cap){
                cap1="YES";
             }

            }
            else{
                std::cout<<"__________________________json turn  faild__________________________________"<<std::endl;
            }
            if(root.isMember("isRespirator"))     {                                         
            int  respitator    = root["isRespirator"].asInt();                
           
                  if(0==respitator){
                respitator1="NO";
               }    
                  if(1==respitator){
                respitator1="YES";
               } 
            }
            std::string carname=".";
            if(root["vehicleBrandComboName"].type()!=Json::nullValue){
                std::cout<<"_______________________________________________"<<std::endl;
                carname=root["vehicleBrandComboName"].asString();
            }
            
            if( outFile.is_open()){
               std:: cout<<"wenjian yi dakai"<<std::endl;
                outFile<<mystr<<','<<sex1<<','<<glasses1<<','<<cap1<<','<<respitator1<<','<<carname<<std::endl;
            }
            else{
                std::cout<<"dakaishibai"<<std::endl;
            }
           

        }
        else{
            std::cout<<"json  failed"<<std::endl;
        }
        

        

        // mutex.lock();
        // result = QString::fromStdString(json);
        // mutex.unlock();
    }
}

VOID CALLBACK My_Exception_Callback(
    LONG lUserID,
    LONG lHandle,
    ULONG ulType,
    VOID *pUserData) {
    std::cout << "IDM_DEV_Exception_Callback_PF userID:" << lUserID << std::endl;
    std::cout << "IDM_DEV_Exception_Callback_PF lHandle:" << lHandle << std::endl;
    std::cout << "IDM_DEV_Exception_Callback_PF Type:" << ulType << std::endl;
    std::cout << "IDM_DEV_Exception_Callback_PF userData:" << (char *)pUserData << std::endl;

    switch (ulType)
    {
    case EXCEPTION_KEEPALIVE:
    {
        std::cout << "Connection exception! userID:" << lUserID << std::endl;
        //do somthing
    }break;

    case EXCEPTION_RECONNECT:
    {
        std::cout << "Reconnection successful! userID:" << lUserID << std::endl;
        //do somthing
    }break;

    case EXCEPTION_RECONNECT_FAILED:
    {
        std::cout << "Reconnection failure! userID:" << lUserID << std::endl;
        //do somthing
    }break;

    case EXCEPTION_ALARM:
    {
        std::cout << "Alarm connection exception! userID:" << lUserID << ",alarmID:" << lHandle << std::endl;
        //do somthing
    }break;

    case EXCEPTION_ALARM_RECONNECT:
    {
        std::cout << "Alarm reconnection successful! userID:" << lUserID << ",alarmID:" << lHandle << std::endl;
        //do somthing
    }break;

    default:
        break;
    }

    return;
}

H3CProcess::H3CProcess()
{
    outFile.open("data.csv",std::ios::out);
    // timer = new QTimer(this);
    cameraIP = "192.168.13.227";
    cameraUser = "admin";
    cameraPassword = "edge2021";
    urlPath = "rtsp://admin:edge2021@192.168.13.227";
    save_path = "./network_config.json";
    std::fstream file;
    file.open(save_path,std::ios::in);
    if(file){
        loadConfig();
    }
}

H3CProcess::~H3CProcess()
{
    outFile.close();
    // timer->stop();
    // timer->deleteLater();

    stopEvent();
    //退出登录
    IDM_DEV_Logout(lUserID);

    //清理库
    IDM_DEV_Cleanup();

    saveConfig();
}


int H3CProcess::loginCamera()
{
    IDM_DEV_Init();//初始化库

    IDM_DEV_SaveLogToFile(3, 0, "/home/edge/To_H3C/log");

    void *userData = NULL;
    IDM_DEV_SetExceptionCallback(My_Exception_Callback, (void *)userData);

    //开启断线重连
    IDM_DEV_RECONNECT_INFO_S stReconnectInfo = { 0 };
    stReconnectInfo.ucEnable = 1;
    stReconnectInfo.uiInterval = 3000; //3秒
    IDM_DEV_SetReconnect(stReconnectInfo);

    //登录
    IDM_DEV_DEVICE_INFO_S devInfo = { 0 };
    IDM_DEV_USER_LOGIN_INFO_S loginInfo = { 0 };
    strncpy(loginInfo.szTargetIP, this->cameraIP.c_str(), sizeof(loginInfo.szTargetIP) - 1);
    loginInfo.usPort = 9000;
    strncpy(loginInfo.szUsername, this->cameraUser.c_str(), IDM_USERNAME_MAX_LEN - 1);
    strncpy(loginInfo.szPassword, this->cameraPassword.c_str(), IDM_PASSWORD_MAX_LEN - 1);
    loginInfo.lLoginMode = 0;
    int ret = IDM_DEV_Login(loginInfo, &devInfo, &lUserID);
    if (ret != IDM_SUCCESS)
    {
        std::cout << "login failed:" + ret << std::endl;
        IDM_DEV_Cleanup();
        return -1;
    }
    void *sumUserData = NULL;
    //注册回调
    ret = IDM_DEV_SetAlarmCallback(0, My_Alarm_Callback, sumUserData);
    if (ret != IDM_SUCCESS)
    {
        std::cout << "set callback failed:" + ret << std::endl;
        IDM_DEV_Logout(lUserID);
        IDM_DEV_Cleanup();
        return -1;
    }
    return 0;
}

int H3CProcess::startEvent()
{
    int ret = 0;

    lAlarmHandle = 0;
    std::string sub =  "{\"channel_no_list\":[65535],\"event_types\":[0],\"event_levels\":[2]}";
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
        return -1;
    }

    IDM_UNI_ALGO_OP req;
    req.uiChannelNo = 0;
    req.ucOpType = 1;
    ret = IDM_UNI_AlgoOp(lUserID, "_39", &req);

    IDM_DEV_INTELLIGENCE_AIUNITE_CFG_S aiuniteCfg = {0};
    ret = IDM_DEV_GetConfig(lUserID, CONFIG_INTELLIGENCE_AIUNITE, 0, &aiuniteCfg, sizeof(IDM_DEV_INTELLIGENCE_AIUNITE_CFG_S));
    if(ret == IDM_SUCCESS)
    {
        if(aiuniteCfg.ucMaskVerify)
        {
            aiuniteCfg.ucMaskVerify = 0;
        }
        else
        {
            aiuniteCfg.ucMaskVerify = 1;
        }
        aiuniteCfg.usFaceAnaInterval += 20;
        if (aiuniteCfg.usFaceAnaInterval > 600)
        {
            aiuniteCfg.usFaceAnaInterval = 1;
        }
        aiuniteCfg.ucDetectMode = 0;
        aiuniteCfg.ucVehicle = 1;
        aiuniteCfg.ucNonVehicle = 1;
        ret = IDM_DEV_SetConfig(lUserID, CONFIG_INTELLIGENCE_AIUNITE, 0, &aiuniteCfg, sizeof(IDM_DEV_INTELLIGENCE_AIUNITE_CFG_S));
    }
    else
    {
        std::cout << "get CONFIG_INTELLIGENCE_AIUNITE error: " << ret << std::endl;
        return -1;
    }

    // timer->start(10);
    // connect(timer, &QTimer::timeout, this, &H3CProcess::getResult);
    return 0;
}

int H3CProcess::stopEvent()
{
    int ret = 0;
    ret = IDM_DEV_StopAlarmUp(lAlarmHandle);
    // timer->stop();
    // disconnect(timer, &QTimer::timeout, this, &H3CProcess::getResult);
    return 0;
}

// void H3CProcess::getResult()
// {
//     mutex.lock();
//     if(!result.isEmpty())
//     {
//         qDebug() << result <<endl;
//         emit signalsResultMsg(result);
//         result = "";
//     }
//     mutex.unlock();
// }

int H3CProcess::loadConfig()
{
    Json::Reader reader;
	Json::Value root;

    std::ifstream in(this->save_path, std::ios::binary);
 
	if (!in.is_open())
	{
		std::cout << "Error opening file\n";
		return -1;
	}

    if (reader.parse(in, root))
	{
        this->cameraIP = root["cameraIP"].asString();
        this->cameraUser = root["cameraUser"].asString();
        this->cameraPassword = root["cameraPassword"].asString();
        this->urlPath = root["urlPath"].asString();
    }
    return 0;
}

int H3CProcess::saveConfig()
{
    Json::StyledWriter sw;
    Json::Value root;
    root["cameraIP"] = this->cameraIP;
    root["cameraUser"] = this->cameraUser;
    root["cameraPassword"] = this->cameraPassword;
    root["urlPath"] = this->urlPath;

    //输出到文件  
    std::ofstream os;
    os.open(this->save_path, std::ios::out | std::ios::app);
    if (!os.is_open())
        std::cout << "[error: can not find or create the file which named \" ***.json\"]." << std::endl;
    os << sw.write(root);
    os.close();

    return 0;
}