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
        char mystr[25];//闂佺鍩栭幐缁樺垔閸ф绫嶉柛顐ｆ礃閿涳拷
        int  sex; //闂佽鍎搁崘銊у姸
        int  glasses ;//闂佹椿浜滈妶鎼佸汲閿燂拷
        int  cap;//闁汇埄鍨靛Λ鍕偤閿燂拷
        int  respitator;//闂佸憡鐟辩徊鍧楀磽閿燂拷

};
struct person_FaceCompare
{
        std::string  name;//婵犳鍠楅幐鎼佸箖閿燂拷
        int          sex; //闂佽鍎搁崘銊у姸
        int          nativeCity;//缂備緡鍋勭粔鐑芥偪閿燂拷
        std::string  bronDate;//闂佸憡鍨煎▍锝夊极閹捐绫嶉柕澶涢檮閸╋拷
        int          idType;  //闁荤姴娲ｅ鎺戔枎閵忋垻灏甸悹鍥皺閳ь剨鎷�
        std::string  idNumber;//闁荤姴娲ｅ鎺戔枎閵忋倖鏅搁柨鐕傛嫹?
        long long    similarity;//闂佺儵鏅犻弲鏌ュ箮閳ь剟鏌ㄩ悤鍌涘?
        char mystr[25];//闂佺鍩栭幐缁樺垔閸ф绫嶉柛顐ｆ礃閿涳拷

};
struct person_Features
{
        int sex; //闂佽鍎搁崘銊у姸
        long long time;
        int ageGroup ;//濡ょ姷鍋為幐宕囨閿燂拷
        int hairlen;//闂佸憡鐟﹂崹鐢告倵閿燂拷
        int coatcolor;//婵炴垶鎸搁敃銊╂閳哄偊绱ｆ繝濠傛椤ワ拷
        int trousersColor;//婵炴垶鎸搁鍫ユ閳哄偊绱ｆ繝濠傛椤ワ拷
        int Orientation;//闂佸搫鐗婄换鍌炲箖閿燂拷
};
struct device_Information
{
    std::string  dev_id;//闁荤姳鐒﹂幏婵嬪箯閿燂拷?闂佽法鍠撶粋鎶�
    std::string  dev_ip;//闁荤姳鐒﹂幏婵嬪箯閿燂拷?闂佽法鍠撶粋鎶�

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
    struct person_Features ones; //闁荤偞绋戞總鏂啃ф径灞兼勃闁绘垵娲︾€氾拷?
    struct device_Information cammers; //闂佺儵鏅涢幉鈥斥攦閳ь剟鎮橀悙闈涘壋闁瑰嚖鎷�?
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
    std::list<int> client_list; //闂佺儵鎳囬弳鏀妔t闂佸搫顦崕閬嶆偤閵娾晛缁╅柟鎯ф噺鐎氾拷?闂佽法鍠曟慨銈囨暜鐎电硶鍋撳☉娆樻缂佽鲸绻傞埢浠嬪焺閸愨晝鐣抽梻鍌氬閸旀洟宕哄Δ浣洪檮婵°倓绀佹径宥夋倵濞戞瑯娈樻繛鍫熷灩閳ь剛鎳撶紞濠囧闯閾忛€涚剨闁告鍋涚拋鎻捗归悩顔煎姎闁汇倕瀚伴幃铏紣娴ｄ警浼囨繛鎴炴尵鐏忕櫝rver闁荤姾娅ｉ崰鏇犫偓姘ュ灲閻涱噣鐓銏犳疂client闂備緡鍋呴惌顔界┍閿燂拷
    std::string http_method;
};

#endif