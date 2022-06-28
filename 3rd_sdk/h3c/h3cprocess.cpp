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
//???????+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int32_t play_chid = -1;
// ??????????
long login_handle = -1;
// ?????????
long play_handle = -1;

// ???????????????
void __stdcall nsk_datacallback(LONG lStreamHandle, ULONG ulDataType, UCHAR *pu8Buf, ULONG u32BufLen, VOID* pCbUser)
{
    std::ofstream file("record.hzv",std::ios::binary|std::ios::app);
    file.write((const char*)pu8Buf,u32BufLen);
    
    PLAY_InputData(channelID, pu8Buf, u32BufLen);
}
bool H3CProcess:: start_netsdk()
{
    // 预览拉视频流
    IDM_DEV_PREVIEW_INFO_S sPreviewInfo;
    ::memset(&sPreviewInfo, 0, sizeof(IDM_DEV_PREVIEW_INFO_S));
    sPreviewInfo.ulChannel = 0;     // IPC通道号默认0
    sPreviewInfo.ulStreamType = 0;  // 主码流0
    sPreviewInfo.ulLinkMode = 0;    // TCP拉流0
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
bool H3CProcess::start_playsdk(HWND hwnd)
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
    std::cout<<"播放结果是"<<playflag<<std::endl;
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
    outFile<<"??????"<<','<<"???"<<','<<"???"<<','<<"???"<<','<<"????"<<','<<std::endl;
    outFile1.open("Face_recognition.csv",std::ios::out);
    outFile1<<"??????"<<','<<"????"<<','<<"???"<<','<<"????"<<','<<"????????"<<','<<"???????"<<','<<"?????"<<','<<"?��ID"<<','<<"?��IP"<<std::endl;
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
    //????出登�?????
    IDM_DEV_Logout(lUserID);

    //清理�?????
    IDM_DEV_Cleanup();

    saveConfig();
}
int H3CProcess::loginCamera()
{
    IDM_DEV_Init();//初�???�化�?????

    IDM_DEV_SaveLogToFile(3, 0, "/home/edge/To_H3C/log");

    void *userData = NULL;
    IDM_DEV_SetExceptionCallback(My_Exception_Callback, (void *)userData);

    //????�????�????线重�?????
    IDM_DEV_RECONNECT_INFO_S stReconnectInfo = { 0 };
    stReconnectInfo.ucEnable = 1;
    stReconnectInfo.uiInterval = 3000; //3�????
    IDM_DEV_SetReconnect(stReconnectInfo);

    //�??????
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
    
    //注册回调
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
                getFacesnapResult(infor_Zs,root);
               
            } 
            if(33751046==infor_Zs.event_type&&this->alconfig.person_attribute==true)
            {
                getFaceCompareResult(infor_Zs,root);

            }
            if(33751047==infor_Zs.event_type&&this->alconfig.face_compare==true)
            {
                getPersonAttributeResult(infor_Zs,root);

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
    //输�??到文�?????  
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
	// 在这里处理所有窗口消息
	switch (msg)
	{
	case WM_DESTROY:
		// 当窗口销毁时退出应用程序
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
	HWND hWnd = CreateWindowEx(0, wc.lpszClassName, TEXT("标题"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, nullptr, nullptr);
    
	
    bool retcode1 = start_playsdk(hWnd);
    start_netsdk();
    ShowWindow(hWnd, SW_SHOWNA);
    MSG msg;   //消息机制
	while (GetMessage(&msg, NULL, 0, 0))    //消息循环
	{
		TranslateMessage(&msg);   //将传来的消息翻译
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



    
