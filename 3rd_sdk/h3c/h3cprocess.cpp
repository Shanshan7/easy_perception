#include "h3cprocess.h"

//#include <json/json.h>
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
void *sumUserData = NULL;
int32_t channelID = 0;
int32_t *channelid=&channelID;

// define snap picture
std::string person_snap_path;
std::string person_snap_scene_path;

std::ofstream outFile;
std::ofstream outFile1;
std::ofstream outFile2;
std::ofstream outFile3;
//???????+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int32_t play_chid = -1;
// ??????????
long login_handle = -1;
// ?????????
long play_handle = -1;

// ???????????????
void __stdcall nsk_datacallback(LONG lStreamHandle, ULONG ulDataType, UCHAR *pu8Buf, ULONG u32BufLen, VOID* pCbUser)
{
    
    PLAY_InputData(channelID, pu8Buf, u32BufLen);
}
bool H3CProcess::start_netsdk()
{
    // 濠电姷顣藉Σ鍛村磻閸涱収鐔嗘俊顖氱毞閸嬫挸顫濋悡搴ｄ桓闂佹寧绻勯崑娑㈩敇閸忕厧绶炲┑鐘插閸為潧鈹戦悩顔肩伇婵炲绋撻埀顒€鐏氱敮鎺椻€﹂崶顒€绠抽柟瀹犳珪閻╊垶骞冭瀹曠厧鈹戦崼婊勵敇缂傚倸鍊峰鎺旀閵堝绠柨鐕傛嫹
    IDM_DEV_PREVIEW_INFO_S sPreviewInfo;
    ::memset(&sPreviewInfo, 0, sizeof(IDM_DEV_PREVIEW_INFO_S));
    sPreviewInfo.ulChannel = 0;     // IPC闂傚倸鍊搁崐椋庢閿熺姴纾婚柛娑卞幘閺嗗棝鏌涘☉姗堝姛妞も晜褰冭灃闁挎繂鎳庨弳娆撴煟椤撶噥娈滈柡宀嬬秮楠炲鎮╅搹顐ゅ涧闂佹眹鍩勯崹閬嶆儎椤栫偛绠栭悷娆忓閻熺懓鈹戦悩鎻掓殭妞ゅ骏鎷�0
    sPreviewInfo.ulStreamType = 0;  // 濠电姷鏁搁崑鐐哄垂閸洖绠插ù锝呮憸閺嗭箓鏌ｉ姀鐘冲暈闁稿顦扮换婵囩節閸屾艾绠婚悷婊呭鐢寮查幖浣圭叆闁绘洖鍊圭€氾拷0
    sPreviewInfo.ulLinkMode = 0;    // TCP闂傚倸鍊烽懗鍫曞箠閹捐搴婇柡灞诲剸閸ヮ剙绠ユい鏂垮⒔閻掗箖姊虹捄銊ユ珢闁瑰嚖鎷�0
    std::string result1="";
    long retvalue1= IDM_DEV_RealPlay(lUserID, sPreviewInfo, nsk_datacallback, sumUserData, &play_handle);
     if (retvalue1 != IDM_SUCCESS)
    {
        std::cout<<"push faild";
        return false;
    }
    else{
        std::cout<<"push success";

    }
    return 0;
}
bool start_playsdk(HWND hwnd)
{
    
    play_chid = PLAY_GetFreeChID(channelid);
    std::cout<<"play_child is "<<play_chid<<std::endl;
    if(play_chid < 0) return false;
    
    bool xuanranfalge=PLAY_RenderPrivateData(channelID, PRIDATA_TARGET,true);
     if (xuanranfalge != true)
    {
        std::cout<<"xuanran faild";
    }
    else{
        std::cout<<"xuanran success";
    }
    bool playflag = PLAY_StartPlay(channelID, hwnd);
    std::cout<<"闂傚倸鍊风粈浣革耿闁秵鎯為幖娣妼閸屻劌霉閻樺樊鍎忔俊顐Ｃ湁闁挎繂鐗婇鐘电棯閹佸仮闁哄瞼鍠栭獮鍡氼檨闁搞倗鍠庨湁婵犲﹤楠搁悘锕傛煛瀹€瀣瘈鐎规洜鍠栭幊婊堟濞戝彉绱�"<<playflag<<std::endl;
    return playflag;
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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
        // std::cout<<json<<std::endl;
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
    outFile<<"鎶撴媿鏃堕棿"<<','<<"鎬у埆"<<','<<"鐪奸暅"<<','<<"甯藉瓙"<<','<<"鍙ｇ僵"<<','<<std::endl;
    outFile1.open("Face_recognition.csv",std::ios::out);
    outFile1<<"鎶撴媿鏃堕棿"<<','<<"濮撳悕"<<','<<"鎬у埆"<<','<<"绫嶈疮"<<','<<"鍑虹敓鏃ユ湡"<<','<<"璇佷欢绫诲瀷"<<','<<"璇佷欢鍙�"<<','<<"璁惧ID"<<','<<"璁惧IP"<<std::endl;
    outFile2.open("unvehicle.csv",std::ios::out);
    outFile2<<"闈炴満鍔ㄨ溅绫诲瀷"<<','<<"澶寸洈"<<','<<"杞戒汉"<<','<<"杞︽"<<','<<std::endl;
    outFile3.open("vehicle.csv",std::ios::out);
    outFile3<<"杞︾墝棰滆壊"<<','<<"杞﹁締绫诲瀷"<<','<<"杞﹁韩棰滆壊"<<','<<"濮挎€�"<<','<<"鍝佺墝"<<','<<"閬槼鏉�"<<','<<"瀹夊叏甯�"<<','<<"璁惧ID"<<','<<"璁惧IP"<<std::endl;
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
    IDM_DEV_Logout(lUserID);

    
    IDM_DEV_Cleanup();

    saveConfig();
}
int H3CProcess::loginCamera()
{
    IDM_DEV_Init();//闂傚倸鍊风粈渚€骞夐敍鍕殰婵°倕鍟畷鏌ユ煙閻楀牊绶茬紒鐙€鍨堕弻銊╂偆閸屾稑顏�???闂傚倸鍊峰ù鍥р枖閺囥垹绐楅柡鍥╁€ｅ☉銏犵妞ゆ挻绋戞禍楣冩煕椤垵浜為柛娆屽亾闂備浇妗ㄩ悞锕傚箲閸ヮ剙鏋侀柟鍓х帛閺呮悂鏌ㄩ悤鍌涘?????

    IDM_DEV_SaveLogToFile(3, 0, "/home/edge/To_H3C/log");

    void *userData = NULL;
    IDM_DEV_SetExceptionCallback(My_Exception_Callback, (void *)userData);

    //????闂傚倸鍊峰ù鍥р枖閺囥垹闂柨鏇炲€哥粻顖炴煥閻曞倹瀚�????闂傚倸鍊峰ù鍥р枖閺囥垹闂柨鏇炲€哥粻顖炴煥閻曞倹瀚�????缂傚倸鍊搁崐鐑芥倿閿曞倸纾块柛鎰▕閻掍粙鏌ㄩ悢鍝勑㈤梻鍌ゅ灦閺岀喖姊荤€电ǹ濡介梺鍝勬噺閹倿寮婚妸鈺傚亞闁稿本绋戦锟�?????
    IDM_DEV_RECONNECT_INFO_S stReconnectInfo = { 0 };
    stReconnectInfo.ucEnable = 1;
    stReconnectInfo.uiInterval = 3000; //3闂傚倸鍊峰ù鍥р枖閺囥垹闂柨鏇炲€哥粻顖炴煥閻曞倹瀚�????
    IDM_DEV_SetReconnect(stReconnectInfo);
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
    
    //婵犵數濮烽弫鎼佸磻濞戔懞鍥敇閵忕姷顦悗鍏夊亾闁告洦鍋夐崺鐐寸箾鐎电ǹ孝妞ゆ垵鎳橀幃娆愮節閸ャ劎鍘介梺褰掑亰閸撴瑩銆傞搹顐ょ闁割偒鍓氱€氾拷
    ret = IDM_DEV_SetAlarmCallback(0, My_Alarm_Callback, sumUserData);
    if (ret != IDM_SUCCESS)
    {
        std::cout << "set callback failed:" + ret << std::endl;
        IDM_DEV_Logout(lUserID);
        IDM_DEV_Cleanup();
        return -1;
    }
    long result =-1;
    return result == IDM_SUCCESS;
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
void H3CProcess::getFacesnapResult( struct InformationSaveAndOutput infor_Zs,Json::Value     root)
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
void H3CProcess:: getFaceCompareResult( struct InformationSaveAndOutput infor_Zs,Json::Value     root)

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
    

}
void H3CProcess::getPersonAttributeResult( struct InformationSaveAndOutput infor_Zs,Json::Value     root)
{
    std::cout<<"行人解析已开始"<<std::endl;
                if(root.isMember("ageGroup")){
                infor_Zs.ones.ageGroup=root["ageGroup"].asInt();
                std::cout<<infor_Zs.ones.ageGroup<<std::endl;
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
void H3CProcess::getVehicleResult(struct InformationSaveAndOutput infor_Zs,Json::Value  root)
{
                if(root.isMember("vehicleColor")){
                infor_Zs.vehicle_info.vehiclecolor=root["vehicleColor"].asInt();
                }
                if(root.isMember("vehicleClass")){
                infor_Zs.vehicle_info.vehicleClass=root["vehicleClass"].asString();
                }
                if(root.isMember("plateColor")){
                infor_Zs.vehicle_info.vehiclecardcolor= root["plateColor"].asInt();
                }
                if(root.isMember("vehicleBrand")){
                infor_Zs.vehicle_info.vehicleBrand=root["vehicleBrand"].asInt();
                }
                if(root.isMember("vehicleAspect")){
                infor_Zs.vehicle_info.vehicleAspect=root["vehicleAspect"].asInt();
                }
                if(root.isMember("sunvisor")){
                infor_Zs.vehicle_info.sunvisor=root["sunvisor"].asInt();
                }
                if(root.isMember("safetyBelt")){
                infor_Zs.vehicle_info.safetyBelt=root["safetyBelt"].asInt();
                }


}
void H3CProcess::getUnvehicleResult(struct InformationSaveAndOutput infor_Zs,Json::Value  root)
{
    
                if (root.isMember("IsCanopy"))
                {
                    infor_Zs.unvehicle_info.IsCanopy =root["IsCanopy"].asInt();
                }
                 if(root.isMember("IsCarryPeople")){
                    infor_Zs.unvehicle_info.IsCarryPeople=root["IsCarryPeople"].asInt();
                }
                if(root.isMember("IsHelemt")){ 
                    infor_Zs.unvehicle_info.IsHelemt =root["IsHelemt"].asInt();
                }
                if(root.isMember("NonVehicleType")) {                                         
                    infor_Zs.unvehicle_info.NonVehicleType = root["NonVehicleType"].asInt();             
                }


}


void H3CProcess::getResult()
{
    mutex.lock();
    //std::cout<<"+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"<<std::endl;
    if(!result.empty())
    {
        std::cout << "parse result start!" << std::endl;
        // qDebug() << result <<endl;
        Json::Reader    reader;
        Json::Value     root;
        Json::Value     rootson;
        algorithmflag();
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
            // if(33751045==infor_Zs.event_type&&this->alconfig.face_snap=="1")
            // {
            //     getFacesnapResult(infor_Zs,root);
               
            // } 
            // if(33751046==infor_Zs.event_type&&this->alconfig.person_attribute=="1")
            // {
            //     getFaceCompareResult(infor_Zs,root);

            // }
            if(33751047==infor_Zs.event_type)
            {
                getPersonAttributeResult(this->infor_Zs,root);
            }
            if(33751049==infor_Zs.event_type&&this->alconfig.unveihicle=="1")
            {
                getUnvehicleResult(infor_Zs,root);
            }
            if(33751051==infor_Zs.event_type&&this->alconfig.veihicle=="1")
            {
                getVehicleResult(infor_Zs,root);
            }
            std::cout<<"行人属性测试 is"<<this->infor_Zs.ones.ageGroup<<std::endl;
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
void H3CProcess::algorithmflag()
{
    Json::Reader reader;
	Json::Value root;

    std::ifstream in("./algorithmflag.json", std::ios::binary);
 
	if (!in.is_open())
	{
		std::cout << "Error opening file\n";
	}

    if (reader.parse(in, root))
	{
        alconfig.face_snap = root["face"].asString();
        alconfig.person_attribute = root["personfeature"].asString();
        alconfig.face_compare=root["face_compare"].asString();
        alconfig.unveihicle=root["unveihicle"].asString();
        alconfig.veihicle=root["veihicle"].asString();
    }
}

int H3CProcess::saveConfig()
{
    Json::StyledWriter sw;
    Json::Value root;
    root["cameraIP"] = this->cameraIP;
    root["cameraUser"] = this->cameraUser;
    root["cameraPassword"] = this->cameraPassword;
    root["urlPath"] = this->urlPath;  
    std::ofstream os;
    os.open(this->save_path, std::ios::out );
    if (!os.is_open())
        std::cout << "[error: can not find or create the file which named \" ***.json\"]." << std::endl;
    os << sw.write(root);
    os.close();

    return 0;
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	// 闂傚倸鍊风欢姘焽閼姐倖瀚婚柣鏃傚帶缁€澶愮叓閸ャ劍绀冪€规洘鐓￠弻鈥愁吋鎼粹€崇闂佺粯鎸搁崐鍧楀箖濮椻偓閹瑩鎮介崹顐ｅ晵婵犵鍓濋悡鈩冪閸洖钃熼柨婵嗩槹閸婄兘鏌涘▎蹇ｆ▓婵☆偀鍓濈换婵嬪煕閳ь剟宕橀顖嗗應鍋撳▓鍨灈闁诲繑鑹鹃銉╁礋椤愩倖娈曢梺閫炲苯澧柣锝囧厴瀹曞ジ寮撮悢鍝勫箞婵犵數鍋涘Λ娆撴偡瑜忕划娆撴嚒閵堝洨锛滄繝銏ｆ硾閼活垶宕㈢€涙﹩娈介柣鎰缁愭棃鏌℃担瑙勫磳闁轰焦鎹囬弫鎾绘晸閿燂拷
	switch (msg)
	{
	case WM_DESTROY:
		// 闂備浇宕甸崰鎰垝鎼淬垺娅犳俊銈呮噹缁犳澘螖閿濆懎鏆為柛瀣耿閺屾洘寰勯崱妯荤彅闂佹椿鍘介〃濠囧蓟閵娿儮鏀介柛鈾€鏅滄晥闂備浇妗ㄩ悞锕佹懌闂侀€涚┒閸旀垿寮幇鏉跨妞ゆ挾鍋涢‖澶嬬節绾板纾块柛瀣洴瀹曟劙宕稿Δ鈧拑鐔兼煕濞戞瑦缍戦柣鎺戠仛閵囧嫰骞掗幋婵愪患缂備讲鍋撻柛鏇ㄥ灡閻撴洘绻濋棃娑欘棞妞ゅ浚鍋婇幃妤呮偡闁附鈻堥梺鍝勭焿缂嶄線宕洪埀顒併亜閹哄棗浜炬繝纰樷偓铏仴闁哄苯绉堕幏鐘诲箵閹烘挸鈧垶姊虹拠鈥崇仭婵☆偄鍟村顐﹀箻缂佹ɑ娅㈤梺璺ㄥ櫐閹凤拷
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
};
void H3CProcess::playvideo()
{
    std::cout<<"ok"<<std::endl;  
    WNDCLASS wc = {};
    HINSTANCE hInstance;
	//WNDCLASSEX wc = { sizeof(WNDCLASSEX) };
	wc.style = CS_VREDRAW | CS_HREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
	wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = CreateSolidBrush(RGB(0, 0,0));
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = TEXT("MainWindow");
	RegisterClass(&wc);

	// Create the window
	HWND hWnd = CreateWindowEx(0, wc.lpszClassName, TEXT("闂傚倸鍊风粈渚€骞栭銈囩煋闁哄鍤氬ú顏嶆晣闁逞屽厴閸嬫捇宕掗悙瀛樻闂佽法鍣﹂幏锟�"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, nullptr, nullptr);
    
	
    bool retcode1 = start_playsdk(hWnd);
    start_netsdk();
    ShowWindow(hWnd, SW_SHOWNA);
    MSG msg;   //婵犵數濮烽弫鎼佸磻閻愬搫鍨傞柣銏犳啞閸嬪鈹戦悩鎻掓殭妞ゆ洟浜堕弻娑樷槈濞嗘劗绋囬柣搴㈣壘椤︾敻寮诲☉妯锋斀闁糕剝顨忔禒濂告⒑缂佹ɑ鎯堥柟鍑ゆ嫹
	while (GetMessage(&msg, NULL, 0, 0))    //婵犵數濮烽弫鎼佸磻閻愬搫鍨傞柣銏犳啞閸嬪鈹戦悩鎻掓殭妞ゆ洟浜堕弻娑樷槈閸楃偛瀛ｉ梺鎼炲€栭〃鍡涘Φ閸曨喚鐤€闁圭偓鎯屽Λ鐐电磽娴ｅ搫顎撻柟鍑ゆ嫹
	{
		TranslateMessage(&msg);   //闂傚倷娴囬褏鎹㈤幇顔藉床闁归偊鍎靛☉銏犵睄闁稿本渚楀鐔兼⒑閸撴彃浜濇繛鍙夌墱閻ヮ亣顦归柡灞炬礃缁旂喖顢涘顒変紑闂佸搫妫涙慨鎾煘閹达附鍊烽柡澶嬪灩娴犳挳姊虹紒姗嗘當闁挎洏鍎抽埀顒勬涧閵堢ǹ鐣烽妸鈺婃晬婵犲﹤鎷嬪Σ閬嶆⒒娴ｅ憡鎯堥悗姘煎墰閹广垽骞掗幊绛圭秮閺佹捇鏁撻敓锟�
		DispatchMessage(&msg);    //
	}
    std::cout<<retcode1<<std::endl;
    
}
void H3CProcess::initplaySdklog(){
    PLAY_SetPrintLogLevel(LLEVEL_DEBUG);
    std::cout<<"playsdklog is ok"<<std::endl;
    PLAY_SetLogCallback([](PSK_LOG_LEVEL loglevel,const char *logText, void *userData){
        if(logText == nullptr)
        {
            std::cout<<"logtext is null"<<std::endl;
            return;
        }
        std::ofstream outFileplayvideo;
        outFileplayvideo.open("play_video1.txt",std::ios::app);
        std::string videolog=logText;
        outFileplayvideo<<videolog<<std::endl;
        outFileplayvideo.close();
        }
        ,nullptr
        );
    
}






    
