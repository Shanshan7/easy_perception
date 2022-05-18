#include <string>
#include <string.h>
#include<sstream>
#include<time.h>

#ifndef __TOOLS_H__
#define __TOOLS_H__
#endif
struct person_Features
{
        int  sex; //性别
        int  glasses ;//眼镜
        int  cap;//帽子
        int  respitator;//口罩
        std::string  name;//姓名
        int          nativeCity;//籍贯
        std::string  bronDate;//出生日期
        int          idType;  //证件类型
        std::string  idNumber;//证件号
        long long    similarity;//相似度
        char mystr[25];//抓拍时间
        long long time;
        int ageGroup ;//年龄
        std::string hairStyle;//发型
        int coatcolor;//上身颜色
        int trousersColor;//下身颜色
        int Orientation;//朝向
    };
struct device_Information
    {
        std::string  dev_id;//设备id
        std::string  dev_ip;//设备ip

    };
struct InformationSaveAndOutput
{
    long long  event_type;
    struct person_Features ones;//行人属性
    struct device_Information cammers;//相机属性

};