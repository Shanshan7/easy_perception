#include "h3cprocess.h"

#include <json/json.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include<sstream>
#include<time.h>
//#include"person_Features.h"
#include"outputCsv.h"

static std::string result = "";
std::ofstream outFile;
std::ofstream outFile1;
// struct person_Features
// {
//         int  sex; //性别
//         int  glasses ;//眼镜
//         int  cap;//帽子
//         int  respitator;//口罩
//         std::string  name;//姓名
//         int          nativeCity=0;//籍贯
//         std::string  bronDate;//出生日期
//         int          idType;  //证件类型
//         std::string  idNumber;//证件号
//         long long    similarity;//相似度
//         char mystr[25];//抓拍时间
//         long long time;
//         int age ;//年龄
//         std::string hairStyle;//发型
//         int coatcolor;//上身颜色
//         int trousersColor;//下身颜色
//         int Orientation;//朝向
//     };
// struct device_Information
//     {
//         std::string  dev_id;
//         std::string  dev_ip;

//     };
// struct InformationSaveAndOutput
// {
//     long long  event_type;-
//     struct person_Features ones;
//     struct device_Information cammers;

// };
//void outputCsv(InformationSaveAndOutput edge);


VOID Json_output(std::string json);


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
    struct InformationSaveAndOutput infor_Zs;

    


    if ((0 != pinfo->stEvent.ulBufferSize) && (nullptr != pinfo->stEvent.pBuffer)) {
        json = pinfo->stEvent.pBuffer;
        std::cout << "Json------------------------------------------------------------------------------------: " << std::endl;
        //std::cout<<"----------------------------------------------------------------------------------------"<<std::endl;
        std::cout << json << std::endl;
        Json::Reader    reader;
        Json::Value     root;
        Json::Value     rootson;
        
        if(reader.parse(json,rootson))
        {
            //std::cout<<"json open is ok"<<std::endl;
            char mystr[25]={0};
            long long time =0;
            if(rootson.isMember("event_body"))
            {
                //std::cout<<"+++++++++++++++++++++++++++++++++YES++++++++++++++++++++++++++++++++++"<<std::endl;
                root =rootson["event_body"];
            }
            if(root.isMember("genderCode")){  
                infor_Zs.ones.sex =root["genderCode"].asInt();
            }   
            if(root.isMember("passTime")){
                double time1=0;
                time1           =root["passTime"].asDouble();       
                time = (long long)time1;
                std::cout<<"+++++++++++++++++++++++++++time+++++++++++++++++++++++++++++"<<std::endl; 
                std::cout<<time<<std::endl;
                std::cout<<"+++++++++++++++++++++++++++time+++++++++++++++++++++++++++++"<<std::endl; 
                std::cout<<time<<std::endl;
                time/=1000;
                //std::cout<<"+++++++++++++++++++++++++++time+++++++++++++++++++++++++++++"<<std::endl;                
                struct tm *t=gmtime((time_t*)&time);
                //std::cout<<"++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"<<std::endl;
                t->tm_hour+=8;
                std::string myFormat = "%Y-%m-%d %H:%M:%S";
                strftime(infor_Zs.ones.mystr,sizeof(mystr),myFormat.c_str(),t);
                std::cout<<mystr<<std::endl;
            }
            if(root.isMember("isGlasses")){
                 infor_Zs.ones.glasses=root["isGlasses"].asInt();
            }
            if(root.isMember("isCap")){ 
                infor_Zs.ones.cap =root["isCap"].asInt();

            }
            else{
                std::cout<<"__________________________json turn  faild__________________________________"<<std::endl;
            }
            if(root.isMember("isRespirator"))     {                                         
            infor_Zs.ones.respitator   = root["isRespirator"].asInt();             
            }     
            //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
            if(root.isMember("name")){
                infor_Zs.ones.name=root["name"].asString();
            }
            if(root.isMember("nativeCityCode")){
                infor_Zs.ones.nativeCity=root["nativeCityCode"].asInt();

            }  
            if(root.isMember("bornDate")){
                infor_Zs.ones.bronDate=root["bornDate"].asString();
            }   
            if(root.isMember("idType")){
                infor_Zs.ones.idType=root["idType"].asInt();
            }  
            if(root.isMember("idNumber")){
                infor_Zs.ones.idNumber=root["idNumber"].asString();
            }
            if(root.isMember("similarity")){
                double a =root["similarity"].asDouble();
                std::cout<< "a" <<std::endl;
                infor_Zs.ones.similarity=(long long)a;
            }  
            if(root.isMember("ageGroup")){
                infor_Zs.ones.ageGroup=root["ageGroup"].asInt();
            }
            if(root.isMember("coatColor")){
                infor_Zs.ones.coatcolor=root["coatClolor"].asInt();
            }
            if(root.isMember("trousersColor")){
                infor_Zs.ones.trousersColor=root["trousersColor"].asInt();
            }
            if(root.isMember("trousersColor")){
                infor_Zs.ones.coatcolor=root["trousersColor"].asInt();
            }
            if(root.isMember("hairStyle")){
                infor_Zs.ones.hairStyle=root["hairStyle"].asString();
                std::cout<<std::endl;
                std::cout<<infor_Zs.ones.hairStyle<<std::endl;
            }
            if(root.isMember("Orientation")){
                infor_Zs.ones.Orientation=root["Orientation"].asInt();
            }


            Json::Value event_src;
            event_src=rootson["event_src"];
            if(event_src.isMember("dev_id")){
                infor_Zs.cammers.dev_id=event_src["dev_id"].asString();

            } 
            if(event_src.isMember("dev_ip")){
                infor_Zs.cammers.dev_ip=event_src["dev_ip"].asString();

            } 
            if(rootson.isMember("event_type")){
                double a =rootson["event_type"].asDouble();
                infor_Zs.event_type=(long long )a;

            }
                
        }
        else{
            std::cout<<"json  failed"<<std::endl;
        }
        outputCsv(infor_Zs);

        //------------------------------------------------------------------------------------------------------//
        

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
    outFile<<"抓拍时间"<<','<<"性别"<<','<<"眼镜"<<','<<"帽子"<<','<<"口罩"<<','<<std::endl;
    outFile1.open("Face_recognition.csv",std::ios::out);
    outFile1<<"抓拍时间"<<','<<"姓名"<<','<<"性别"<<','<<"籍贯"<<','<<"出生日期"<<','<<"证件类型"<<','<<"证件号"<<','<<"设备ID"<<','<<"设备IP"<<std::endl;
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
    outFile1.close();
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
        aiuniteCfg.ucDetectMode = 3;
        aiuniteCfg.ucVehicle = 1;
        aiuniteCfg.ucNonVehicle = 1;
        aiuniteCfg.ucPerson =1;
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
// void outputCsv(InformationSaveAndOutput edge)
// {
//             long long  event_type;
//             std::string  sex1="."; 
//             std::string  glasses1 =".";
//             std::string  cap1=".";
//             std::string  respitator1=".";
//             std::string  name=edge.ones.name;
//             int          nativeCity=edge.ones.nativeCity;
//             std::string  bronDate=edge.ones.bronDate;
//             int          idType = edge.ones.idType;
//             std::string  idNumber=edge.ones.idNumber;
//             long long    similarity=edge.ones.similarity;
//             std::string  dev_id=edge.cammers.dev_id;
//             std::string  dev_ip=edge.cammers.dev_ip;
//             char mystr[25]={0};
//             long long time =0;

//             //+++++++++++++++++++++++++++++++++++++++++++++++++++++
//             if (1==edge.ones.sex)
//             {
//                 sex1="male";
//             }
//             if (2==edge.ones.sex)
//             {
//                 sex1="famale";
//             }
//             if(0==edge.ones.sex||9==edge.ones.sex)
//             {
//                 sex1="unknow";
//             }
//             //+++++++++++++++++++++++++++++++++++++++++++++++++++++++
//             if(0==edge.ones.glasses)
//             {
//                 glasses1="NO";
//             }
//             if(1==edge.ones.glasses)
//             {
//                 glasses1="YES";
//             }
//             //++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//             if(0==edge.ones.cap)
//             {
//                 cap1="NO";
//             }
//             if(1==edge.ones.cap)
//             {
//                 cap1="YES";
//             }
//             //++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//             if(0==edge.ones.respitator)
//             {
//                 respitator1="NO";
//             }
//             if(1==edge.ones.respitator)
//             {
//                 respitator1="YES";
//             }
//             //________________________________________________________
//             if(33751045==edge.event_type){
//                std:: cout<<"wenjian yi dakai"<<std::endl;
//                if(outFile.is_open()){
//                    std::cout<<"face_S_success";
//                    outFile<<edge.ones.mystr<<','<<sex1<<','<<glasses1<<','<<cap1<<','<<respitator1<<','<<std::endl;
//                }
//             }
//             if(33751046==edge.event_type){
//                 std::cout<<"sssssssssssss"<<std::endl;
//                 if(outFile1.is_open()){
//                    std::cout<<"face_R_success"<<std::endl;
//                    outFile1<<edge.ones.mystr<<','<<name<<','<<sex1<<','<<nativeCity<<','<<bronDate<<','<<idType<<','<<idNumber<<','<<dev_id<<','<<dev_ip<<std::endl;
//                 }
                
//             }  
            

// }


    
