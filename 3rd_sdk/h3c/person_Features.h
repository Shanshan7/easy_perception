#include <string>
#include <string.h>
#include <sstream>
#include <time.h>
#include <WinSock2.h>
#include <list>
#include <map>

#include "h3cprocess.h"

#ifndef __TOOLS_H__
#define __TOOLS_H__

struct person_FaceSnap
{
        char mystr[25];//抓拍时间
        int  sex; //性别
        int  glasses ;//眼镜
        int  cap;//帽子
        int  respitator;//口罩

};
struct person_FaceCompare
{
        std::string  name;//姓名
        int          sex; //性别
        int          nativeCity;//籍贯
        std::string  bronDate;//出生日期
        int          idType;  //证件类型
        std::string  idNumber;//证件�?
        long long    similarity;//相似�?
        char mystr[25];//抓拍时间

};
struct person_Features
{
        int sex; //性别
        long long time;
        int ageGroup ;//年龄
        int hairlen;//发型
        int coatcolor;//上身颜色
        int trousersColor;//下身颜色
        int Orientation;//朝向
};
struct device_Information
{
    std::string  dev_id;//设�?�id
    std::string  dev_ip;//设�?�ip

};
struct subImageInformation
{
    std::string person_snap_path;
    std::string person_snap_scene_path;

};
struct InformationSaveAndOutput
{
    long long  event_type;
    struct person_FaceSnap person_Fs;
    struct person_FaceCompare person_Fc;
    struct person_Features ones; //行人属�?
    struct device_Information cammers; //相机属�?
    struct subImageInformation sub_img_info;
};

struct GlobalControlParam
{
    // task id create
    std::map<std::string, bool> task_response_id{{"face_recognition", false}, \
                                                 {"face_attribute", false}, \
                                                 {"person_attribute", false}, \
                                                 {"people_counting", false}, \
                                                 {"region_intrusion", false},
                                                 {"motor_vehicle_attribute", false},
                                                 {"nonmotor_vehicle_attribute", false}};

    // socket create
    int socketServer;
    sockaddr_in addrServer;
    int nServerAddrLen;
    std::list<int> client_list; //用list来存放�?�接字，没有限制套接字的容量就可以实现一个server跟若干个client通信
    std::string http_method;
};

#endif