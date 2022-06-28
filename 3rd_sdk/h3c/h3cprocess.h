#ifndef H3CPROCESS_H
#define H3CPROCESS_H

#include <string>
#include <unistd.h>
#include "idm_netsdk.h"
#include "person_Features.h"
#include "utils.h"
#include <json/json.h>
#include "hz_playplay.h"
#include <windows.h>


class H3CProcess
{
public:
    
   
    H3CProcess();
    ~H3CProcess();
    void getFacesnapResult( struct InformationSaveAndOutput infor_Zs,Json::Value  root);
    void getFaceCompareResult( struct InformationSaveAndOutput infor_Zs,Json::Value  root);
    void getPersonAttributeResult( struct InformationSaveAndOutput infor_Zs,Json::Value  root);

    int loginCamera();

    int startEvent();

    int stopEvent();
    void getResult();



    void playvideo();//播放视频
    void initplaySdklog();//输出视频播放日志
    bool start_netsdk();//推流
    bool start_playsdk(HWND hwnd);//窗口播放
public:
    struct InformationSaveAndOutput infor_Zs;
    struct Alconfig
    {
        bool face_snap;
        bool person_attribute;
        bool face_compare;

    } alconfig;
    
    
private:
    LONG lUserID;
    LONG lAlarmHandle;

    std::string cameraIP;
    std::string cameraUser;
    std::string cameraPassword;
    std::string urlPath;
    std::string save_path;
    int32_t play_chid = -1;

    long login_handle = -1;

    long play_handle = -1;

private:
    int loadConfig();
    int saveConfig();
};

#endif // H3CPROCESS_H
