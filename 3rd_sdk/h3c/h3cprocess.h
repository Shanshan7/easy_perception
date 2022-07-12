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
    void getVehicleResult(struct InformationSaveAndOutput infor_Zs,Json::Value  root);
    void getUnvehicleResult(struct InformationSaveAndOutput infor_Zs,Json::Value  root);

    int loginCamera();

    int startEvent();

    int stopEvent();
    void getResult();
    void playvideo();
    bool start_netsdk();
    void initplaySdklog();
    //void getResult1();
    
    //void outputResult();

public:
    struct InformationSaveAndOutput infor_Zs;
    struct Alconfig
    {
        std::string   face_snap;
        std::string   person_attribute;
        std::string   face_compare;
        std::string   veihicle;
        std::string   unveihicle; 

    } alconfig;
    
    
private:
    LONG lUserID;
    LONG lAlarmHandle;

    std::string cameraIP;
    std::string cameraUser;
    std::string cameraPassword;
    std::string urlPath;
    std::string save_path;


    // QTimer *timer;

private:
    int loadConfig();
    int saveConfig();
    void algorithmflag();
};

#endif // H3CPROCESS_H
