#ifndef H3CPROCESS_H
#define H3CPROCESS_H

#include <string>
#include <unistd.h>
#include "idm_netsdk.h"
#include "person_Features.h"
#include "utils.h"


class H3CProcess
{
public:
    
   
    H3CProcess();
    ~H3CProcess();

    int loginCamera();

    int startEvent();

    int stopEvent();

    void getResult();
    void outputResult();

public:
    struct InformationSaveAndOutput infor_Zs;
    
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
};

#endif // H3CPROCESS_H
