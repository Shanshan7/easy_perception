#include "h3cprocess.h"

#include <json/json.h>
#include <iostream>
#include <fstream>
#include <string>
#include <string.h>
#include <sstream>
#include <time.h>
#include <mutex>
//#include"person_Features.h"
#include "outputCsv.h"
//#include "mysql.h"

static std::string result = "";
std::mutex mutex; // lock

// define snap picture
std::string person_snap_path;
std::string person_snap_scene_path;

std::ofstream outFile;
std::ofstream outFile1;

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
    // struct InformationSaveAndOutput infor_Zs;

    if ((0 != pinfo->stEvent.ulBufferSize) && (nullptr != pinfo->stEvent.pBuffer)) {
        json = pinfo->stEvent.pBuffer;
        // std::cout << "Json--------------------------------: " << std::endl;
        mutex.lock();
        result = json;

        // save person snap picture
        Json::Reader    reader;
        Json::Value     root;
        Json::Value     rootson;
        if(reader.parse(result, rootson))
        {
            for (unsigned int i = 0; i < rootson["event_body"]["SubImageInfoList"].size(); i++) {
                std::string sub_image_type = rootson["event_body"]["SubImageInfoList"][i]["Type"].asString();
                if (!strcmp(sub_image_type.c_str(), "10")) {
                    unsigned int index = rootson["event_body"]["SubImageInfoList"][i]["ImageIndex"].asUInt();
                    for (unsigned int j = 0; j < pinfo->ulBufferNumber; j++)
                    {
                        if (index == pinfo->astBuffers[j].ulIndex) {
                            person_snap_path = "person-snap.jpg";
                            SaveOnePicture(pinfo->astBuffers[j], person_snap_path);
                        }
                    }
                }

                if (!strcmp(sub_image_type.c_str(), "14")) {
                    unsigned int index = rootson["event_body"]["SubImageInfoList"][i]["ImageIndex"].asUInt();
                    for (unsigned int j = 0; j < pinfo->ulBufferNumber; j++)
                    {
                        if (index == pinfo->astBuffers[j].ulIndex) {
                            person_snap_scene_path = "person-snap-scene.jpg";
                            SaveOnePicture(pinfo->astBuffers[j], person_snap_scene_path);

                        }
                    }
                }
            }
        }

        mutex.unlock();
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
    outFile1<<"抓拍时间"<<','<<"姓名"<<','<<"性别"<<','<<"籍贯"<<','<<"出生日期"<<','<<"证件类型"<<','<<"证件�???"<<','<<"设�?�ID"<<','<<"设�?�IP"<<std::endl;
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
    else{
        saveConfig();
    }
}

H3CProcess::~H3CProcess()
{
    outFile.close();
    outFile1.close();
    // timer->stop();
    // timer->deleteLater();

    stopEvent();
    //退出登�???
    IDM_DEV_Logout(lUserID);

    //清理�???
    IDM_DEV_Cleanup();

    saveConfig();
}

int H3CProcess::loginCamera()
{
    IDM_DEV_Init();//初�?�化�???

    IDM_DEV_SaveLogToFile(3, 0, "/home/edge/To_H3C/log");

    void *userData = NULL;
    IDM_DEV_SetExceptionCallback(My_Exception_Callback, (void *)userData);

    //开�???�???线重�???
    IDM_DEV_RECONNECT_INFO_S stReconnectInfo = { 0 };
    stReconnectInfo.ucEnable = 1;
    stReconnectInfo.uiInterval = 3000; //3�???
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
        aiuniteCfg.ucFace=1;
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


void H3CProcess::getResult()
{
    mutex.lock();
    std::cout<<"+++++++++++++++++++++++++++解析启用成功++++++++++++++++++++++++++++++++++++++"<<std::endl;
    if(!result.empty())
    {
        std::cout << "parse result start!" << std::endl;
        // qDebug() << result <<endl;
        Json::Reader    reader;
        Json::Value     root;
        Json::Value     rootson;
        // std::cout << "result: " << result << std::endl;
        if(reader.parse(result, rootson))
        {
            //std::cout<<"json open is ok"<<std::endl;
            char mystr[25]={0};
            long long time =0;
            std::cout<<"++++++++++++++++++++++++++++++++++++++++++++++++"<<std::endl;
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
            std::cout<<"++++++++++++++++++++++++++++++++++++++++++++++++"<<std::endl;
            if(rootson.isMember("event_body"))
            {
                //std::cout<<"+++++++++++++++++++++++++++++++++YES++++++++++++++++++++++++++++++++++"<<std::endl;
                root =rootson["event_body"];
            }
            if(33751045==infor_Zs.event_type&&this->alconfig.face_snap==true)
            {
                if (root.isMember("genderCode"))
                {
                    infor_Zs.person_Fs.sex =root["genderCode"].asInt();
                }
                 if(root.isMember("isGlasses")){
                    infor_Zs.person_Fs.glasses=root["isGlasses"].asInt();
                }
                if(root.isMember("isCap")){ 
                    infor_Zs.person_Fs.cap =root["isCap"].asInt();
                }
                if(root.isMember("isRespirator")) {                                         
                    infor_Zs.person_Fs.respitator = root["isRespirator"].asInt();             
                }
            } 
            if(33751046==infor_Zs.event_type&&this->alconfig.person_attribute==true)
            {
                   if(root.isMember("name")){
                   infor_Zs.person_Fc.name=root["name"].asString();
                }
                   if(root.isMember("nativeCityCode")){
                   infor_Zs.person_Fc.nativeCity=root["nativeCityCode"].asInt();

                }  
                   if(root.isMember("bornDate")){
                   infor_Zs.person_Fc.bronDate=root["bornDate"].asString();
                }   
                   if(root.isMember("idType")){
                   infor_Zs.person_Fc.idType=root["idType"].asInt();
                }  
                  if(root.isMember("idNumber")){
                   infor_Zs.person_Fc.idNumber=root["idNumber"].asString();
                }
                  if(root.isMember("similarity")){
                   double a = root["similarity"].asDouble();
                   std::cout<< "a" <<std::endl;
                   infor_Zs.person_Fc.similarity=(long long)a;
            }  
            if(33751047==infor_Zs.event_type&&this->alconfig.face_compare==true)
            {
                if(root.isMember("ageGroup")){
                infor_Zs.ones.ageGroup=root["ageGroup"].asInt();
                }
                if(root.isMember("coatColor")){
                infor_Zs.ones.coatcolor=root["coatClolor"].asInt();
                }
                if(root.isMember("trousersColor")){
                infor_Zs.ones.trousersColor=root["trousersColor"].asInt();
                }
                if(root.isMember("HairLen")){
                infor_Zs.ones.hairlen=root["HairLen"].asInt();
                }
                if(root.isMember("Orientation")){
                infor_Zs.ones.Orientation=root["Orientation"].asInt();
                }
                if(root.isMember("Orientation"))
                infor_Zs.ones.sex =root["genderCode"].asInt();

            }
                

            }

            if(root.isMember("SubImageInfoList")){
                for (unsigned int i = 0; i < root["SubImageInfoList"].size(); i++) {
                    std::string sub_image_type = root["SubImageInfoList"][i]["Type"].asString();
                    if (!strcmp(sub_image_type.c_str(), "10")) {
                        this->infor_Zs.sub_img_info.person_snap_path = person_snap_path;
                    }

                    if (!strcmp(sub_image_type.c_str(), "14")) {
                        this->infor_Zs.sub_img_info.person_snap_scene_path= person_snap_scene_path;
                    }
                }   
            }
        }
        else{
            std::cout<<"json  failed"<<std::endl;
        }
        outputCsv(this->infor_Zs);
        result = "";
    }
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
       
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    mutex.unlock();
}

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

    //输出到文�???  
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


    
