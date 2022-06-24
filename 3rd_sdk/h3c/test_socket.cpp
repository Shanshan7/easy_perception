#include <cstdio>
#include <WinSock2.h>
#include <string>
#include <sstream>
#include <json/json.h>
#include <thread>
#include <list>

#include "h3cprocess.h"
#include "person_Features.h"

#define BUFFER_SIZE 1024
#define PORT 5000
#define IP "127.0.0.1"
#define HEADER "\
HTTP/1.1 200 OK\r\n\
Server:nginx\r\n\
Access-Control-Allow-Origin: *\n\
Access-Control-Allow-Headers: *\n\
Content-Type: application/json; charset=UTF-8\r\n\
Content-Length: %d\r\n\r\n%s\
"
#pragma comment(lib, "WS2_32") 

static H3CProcess h3cProcess;

static GlobalControlParam global_control_param;

void get_connect()
{
    while(1)
    {
        int connect = accept(global_control_param.socketServer, (struct sockaddr*)&global_control_param.addrServer, \
                      &global_control_param.nServerAddrLen);
        global_control_param.client_list.push_back(connect);
    }
}
 
void receive_data()
{
    int rval = 0;
    struct timeval tv;
    tv.tv_sec = 0; //闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰版煛瀹€瀣М妞ゃ垺枪椤︽彃霉濠婂懎浠﹂柕鍥у閺佹劖鎯旈埄鍐綆婵°倗濮烽崑娑氭崲濮椻偓楠炲啫饪伴崼鐔锋疅闂侀潧顧€闂勫嫰顢旈悢鍏尖拻濞达絽鎲￠幆鍫ユ煕婵犲倻鍩ｇ€规洘鍔欏浠嬵敃閻旇渹澹曞┑顔斤供閸撴稓绮旈搹鍏夊亾濞堝灝娅橀柛锝忕秮楠炲﹤饪伴崼婵嗙€銈嗗姦閸嬪懓鈪堕梻鍌氬€搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮婚妸鈺傚亞闁稿本绋戦锟�
    tv.tv_usec = 2000;
    printf("-------------------receive data-------------------\n");
    while(1)
    {
        std::list<int>::iterator it;
        for(it=global_control_param.client_list.begin(); it!=global_control_param.client_list.end(); ++it)
        {
            fd_set rfds; 
            FD_ZERO(&rfds);
            FD_SET(*it, &rfds);

            int maxfd = 0;
            int retval = 0;
            if(maxfd < *it)
            {
                maxfd = *it;
            }
            retval = select(maxfd+1, &rfds, NULL, NULL, &tv);
            if(retval == -1)
            {
                printf("select errorn");
            }
            else if(retval == 0)
            {
                // printf("not messagen");
                continue;
            }
            else
            {
                char buffer[BUFFER_SIZE];
                memset(buffer, 0, BUFFER_SIZE);
                rval = recv(*it, buffer, sizeof(buffer), 0);
                if (rval > 0)
                {
                    // get http method
                    http_method_get(buffer, global_control_param.http_method);

                    // if(strcmp(buffer, "exitn") == 0) break;
                    // analysis input json
                    http_request_message(buffer, global_control_param.task_response_id);

                    if (global_control_param.http_method == "POST")
                    {
                        char buffer_response[BUFFER_SIZE];
                        memset(buffer_response, 0, BUFFER_SIZE);
                        char* response = "ready!";
                        sprintf_s(buffer_response, HEADER, strlen(response), response);
                        send(*it, buffer_response, sizeof(buffer_response), 0);
                        // printf("Response!!!!!\n");
                    }
                }
            }
        }
        Sleep(1);
    }
}
 
void send_message()
{
    int rval = 0;
    h3cProcess.alconfig.face_snap=false;
    h3cProcess.alconfig.person_attribute=false;  
    if (global_control_param.task_response_id["person_attribute"])
    {
        h3cProcess.alconfig.person_attribute=true;
    }
    if (global_control_param.task_response_id["face_attribute"])
    {
        h3cProcess.alconfig.face_snap=true;
    }
    if (global_control_param.task_response_id["face_compare"])
    {
         h3cProcess.alconfig.face_compare=true;
    }
    while(1)
    {
        //h3cProcess.getResult();
        
        h3cProcess.getResult();  
              //printf("-------------------send message-------------------\n");
        if (global_control_param.task_response_id["person_attribute"] && global_control_param.http_method == "GET")
        {
            char buffer[BUFFER_SIZE];
            memset(buffer, 0, BUFFER_SIZE);

            Json::Value jsPerson;
            jsPerson["person_content"]["person_feature"]["gender"] = h3cProcess.infor_Zs.ones.sex;
            jsPerson["person_content"]["person_feature"]["agegroup"] = h3cProcess.infor_Zs.ones.ageGroup;
            jsPerson["person_content"]["person_feature"]["coatcolor"] = h3cProcess.infor_Zs.ones.coatcolor;
            jsPerson["person_content"]["person_feature"]["trousersColor"] = h3cProcess.infor_Zs.ones.trousersColor;
            jsPerson["person_content"]["person_feature"]["hairStyle"] = h3cProcess.infor_Zs.ones.hairlen;
            jsPerson["person_content"]["person_feature"]["Orientation"] = h3cProcess.infor_Zs.ones.Orientation;

            jsPerson["person_content"]["device_information"]["dev_id"] = h3cProcess.infor_Zs.cammers.dev_id;

            jsPerson["person_content"]["sub_image_information"]["snap_path"] = h3cProcess.infor_Zs.sub_img_info.person_snap_path;

            Json::FastWriter json_write;
            std::string strPerson = json_write.write(jsPerson);

            std::list<int>::iterator it;
            for(it=global_control_param.client_list.begin(); it!=global_control_param.client_list.end(); ++it)
            {
                sprintf_s(buffer, HEADER, strlen(strPerson.c_str()), strPerson.c_str());
                send(*it, buffer, sizeof(buffer), 0);
                // printf("Sending data\n");
            }
            Sleep(100);
        }
         if (global_control_param.task_response_id["face_attribute"] && global_control_param.http_method == "GET")
        {
            char buffer[BUFFER_SIZE];
            memset(buffer, 0, BUFFER_SIZE);

            Json::Value jsPerson;
            jsPerson["person_content"]["face_feature"]["sex"] = h3cProcess.infor_Zs.person_Fs.sex;
            jsPerson["person_content"]["face_feature"]["glasses"] = h3cProcess.infor_Zs.person_Fs.glasses;
            jsPerson["person_content"]["face_feature"]["cap"] = h3cProcess.infor_Zs.person_Fs.cap;
            jsPerson["person_content"]["face_feature"]["respitator"] = h3cProcess.infor_Zs.person_Fs.respitator;
            Json::FastWriter json_write;
            std::string strPerson = json_write.write(jsPerson);

            std::list<int>::iterator it;
            for(it=global_control_param.client_list.begin(); it!=global_control_param.client_list.end(); ++it)
            {
                sprintf_s(buffer, HEADER, strlen(strPerson.c_str()), strPerson.c_str());
                send(*it, buffer, sizeof(buffer), 0);
                // printf("Sending data\n");
            }
            Sleep(100);
        }

        // Sleep(100);
    }
}
 
int main()
{
    int rval = 0;

    rval = h3cProcess.loginCamera();
    if(rval < 0)
    {
        std::cout << "闂佸搫顦弲婵嬪磻閻愬灚鏆滈柟缁㈠枛缁狅綁鏌熼柇锕€澧い顐嫹" << std::endl;
    }
    else
    {
        std::cout << "闂佸搫顦弲婵嬪磻閻愬灚鏆滈悗娑櫭欢鐐哄级閸偄浜悮锟�" << std::endl;
    }

    h3cProcess.startEvent();
    //h3cProcess.playvideo();

    //new socket
    global_control_param.socketServer = socket(AF_INET, SOCK_STREAM, 0);
    memset(&global_control_param.addrServer, 0, sizeof(global_control_param.addrServer));
    global_control_param.addrServer.sin_family = AF_INET;
    global_control_param.addrServer.sin_port = htons(PORT);
    global_control_param.addrServer.sin_addr.s_addr = INADDR_ANY;
    if(bind(global_control_param.socketServer, (struct sockaddr* ) &global_control_param.addrServer, 
        sizeof(global_control_param.addrServer))==-1)
    {
        perror("bind");
        exit(1);
    }
    if(listen(global_control_param.socketServer, 20) == -1)
    {
        perror("listen");
        exit(1);
    }
    global_control_param.nServerAddrLen = sizeof(global_control_param.addrServer);
    
    //thread : while ==>> accpet
    std::thread t(get_connect);
    t.detach();//detach闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鈧箍鍎遍ˇ浼村吹閹达箑绠规繛锝庡墮閻忊晜銇勯弴锟犲摵缂佺粯鐩畷鍗炍熼崫鍕垫綋婵＄偑鍊曠换鎰版偉閻撳寒娼栭柧蹇氼潐瀹曞鏌曟繛鍨姕闁诲酣绠栧娲传閸曢潧鍓板銈庡幘閸忔﹢鎮伴璺ㄧ杸婵炴垶鐟ラ埀顒€顭烽弻銈夊箒閹烘垵濮曞┑鐐叉噷閸ㄨ棄顫忓ú顏勪紶闁告洦鍋€閸嬫捇宕奸弴鐐碉紮闂佸搫绋侀崑鈧柛瀣尭椤繈顢楅崒婧炪劑姊洪崫鍕潶闁告梹鍨块獮鍐閵堝懎绐涙繝鐢靛Т鐎氼亞妲愰弴銏♀拻濞达絿鍎ら崵鈧梺鎼炲灪閻撯€崇暦閺屻儲鍤戦棅顒冨紦濮规姊婚崒姘卞缂佸鐗撳畷濂割敂閸喓鍘撻梺鍛婄箓鐎氼參宕冲ú顏呯厸闁告侗鍠楅崐鎰叏婵犲啯銇濋柟顔ㄥ洤閱囨い鎰╁灪濞呮盯姊绘担鍛婂暈闁圭ǹ鐖煎畷婵嬪箣閿旀儳绁﹂棅顐㈡处缁嬫帡宕愭繝姘厾闁诡厽甯掗崝婊勭箾閸涱偄鐏查柡宀嬬秮閹垽宕妷锕€娅楅梺姹囧焺閸ㄨ京鏁Δ鍛畾闁哄倸绨遍崼顏堟煕椤愶絿绠樻い鏂挎濮婅櫣绱掑Ο铏逛紘婵犳鍠撻崐婵嗩嚕閼碱剛鐭欓悷浣靛€愰崑鎾诲箳閹搭厽鍍甸梺闈涱槶閸庤鲸鎱ㄩ弴銏♀拺闁告繂瀚烽崕蹇斻亜椤撶姴鍘撮柣娑卞枦缁犳稑鈽夊▎蹇婂亾婵犳碍鐓犻柟顓熷笒閸旀粍绻涢崨顐㈢伈婵﹥妞藉畷顐﹀礋椤愮喎浜鹃柛顐ｆ礀绾惧綊鏌″搴′簮闁稿鎸搁～婵嬵敆閸屾簽銊╂⒑閸濆嫯顫﹂柛鏃€鍨块獮鍐閵堝懎绐涙繝鐢靛Т鐎氼亞妲愰弴銏♀拻濞达絽鎲＄拹锟犳煕韫囨棑鑰块挊鐔兼煙閻愵剙澧柛銈嗘礋閺岋綁骞嬮敐鍛呮捇鏌ｉ幘鍗炲姦闁哄矉缍佸顒勫箰鎼粹剝娈橀梻浣筋嚙濞存岸宕抽敐澶婅摕闁绘柨澹婂Σ褰掑箹缁厜鍋撻崘鑼吅闂傚倷娴囧銊╂倿閿曞倸绠查柛銉戝苯娈ㄩ梺鍓插亝濞叉﹢宕戠€ｎ喗鐓曟い鎰╁€曢弸鏂棵归悡搴☆伃婵﹪缂氶妵鎰板箳濠靛浂妫栭梻浣呵圭换鎴︽晝閵忕媭鍤曟い鎰剁畱缁犲鎮归崶顏勭毢闁绘挻鍨甸—鍐Χ閸℃袝闂佸憡鎸荤粙鎾跺垝閺冨牆閱囬柡鍥╁枔閸樺灚淇婇悙宸剰濡ょ姵鎮傚畷銏ゅ箻椤旂晫鍘告繛杈剧秬椤鐣峰畝鈧埀顒侇問閸ｎ噣宕滈悢闀愮箚闁割偅娲栭獮銏′繆閵堝拑姊楃紒鎲嬬畵濮婂宕掑▎鎰偘濠电偛顦板ú鐔风暦娴兼潙绠涢柡澶婄仢閸ゆ垶绻濋悽闈浶ｉ柤鐟板⒔缁顢涢悙瀵稿幗闂佺鎻徊鍊燁暱闂備礁鎼幊蹇涘箖閸岀偛钃熼柨鐔哄Т绾惧吋淇婇婵勨偓鈧柛瀣崌瀵粙顢曢悢铚傚濠殿喗锕╅崜娑氱矓濞差亝鐓涢悘鐐额嚙婵倿鏌熼鍝勭伈鐎规洏鍔嶇换婵嬪礋椤忓棛锛熼梻鍌氬€风粈浣革耿闁秴钃熷璺呵归埀顒佸浮瀹曠兘顢橀悢濂変紩闂備胶绮敮妤冪箔濠曨柎闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄦ娊鍩€椤掑倹鍤€閻庢凹鍘奸…鍨潨閳ь剝鐭鹃梺鍛婄☉閻°劑鍩涢幋锔藉仯闁诡厽甯掓俊濂告煛鐎ｎ偄鐏撮柡宀嬬磿閳ь剨缍嗘禍鍫曟偂閸忕⒈娈介柣鎰皺缁犲鏌熼鐣岀煉闁瑰磭鍋ゆ俊鐑芥晜缁涘鎴烽梻鍌氬€风粈渚€骞栭锕€纾归柛顐ｆ礀绾惧綊鏌″搴′簮闁稿鎸搁～婵嬵敆閸屾簽銊╂倵鐟欏嫭绌跨紓宥勭窔瀵宕卞Δ濠傛倯闂佺硶鍓濊摫妞ゅ繒鍠栧缁樻媴閸涘﹥鍎撻梺绋匡工閻栫厧鐣烽幇鏉垮唨妞ゆ劗鍠庢禍鐐叏濮楀棗澧扮紒澶嬫そ閺岀喓绮欓幐搴℃畬闂侀潧娲﹂崝娆撶嵁閹烘绠婚悗娑欙供娴犙囨⒒閸屾瑧顦﹂柟纰卞亰瀹曟劙宕奸弴鐐碉紮闂佸搫绋侀崑鈧柛瀣尭椤繈顢楅崒婧炪劑鎮楃憴鍕５闁逞屽墯閸撴艾顭囬幍顔瑰亾閸忓浜鹃梺閫炲苯澧撮柡宀嬬畵楠炴帒螖娴ｅ弶瀚奸梺鑽ゅУ娴滀粙宕濆畝鍕嚑闁硅揪闄勯悡娑㈡倵閿濆簼鎲鹃柣鎾冲悑椤ㄣ儵鎮欓懠顒傤啋闂佽鍠楃划鎾诲箠閻愬搫唯闁挎梻鐡旀禒褔姊婚崒娆戭槮闁圭⒈鍋婂畷鎰板醇閺囩偟锛欓梺鍝勭▉閸嬧偓闁稿鎸搁～婵嬵敆閸屾簽銊╂⒑閸濆嫯顫﹂柛鏃€鍨块獮鍐閵堝懎绐涙繝鐢靛Т鐎氼亞妲愰弴銏♀拻濞达絽鎲＄拹锟犳煕韫囨棑鑰块挊鐔兼煙閸撗呭笡闁稿锚閳规垿鎮╅崣澶婎槱闂佺粯鎸堕崕鐢稿蓟閿濆鍗抽柣鎰ゴ閸嬫捇宕烽娑樹壕婵鍋撶€氾拷
    //濠电姷鏁搁崑鐐哄垂閸洖绠伴柟缁㈠枛绾惧鏌熼崜褏甯涢柣鎾跺枛閺屽秹宕崟顐熷亾閸︻厺鐒婇柨鏇炲€归悡娑樏归敐鍫綈閻忓繒澧楅妵鍕Ω閿斿墽鐓侀梺闈涚墳缁犳捇鐛幇顓熷劅闁斥晛鍟撮弫顏堟⒒閸屾瑧顦︾紓宥咃躬钘熼柟鎹愵嚙缁€鍌涙叏濡炶浜鹃悗娈垮櫘閸嬪﹪鐛Ο鍏煎珰闁告瑥顦藉Λ鐔兼⒒娓氣偓濞佳囨偋閸℃あ娑樜旈崪浣规櫆闂佸壊鍋呭ú姗€鍩涢幋锔藉仯闁诡厽甯掓俊濂告煛鐎ｎ偄鐏撮柡宀嬬磿閳ь剨缍嗘禍鍫曟偂閸忕⒈娈介柣鎰皺缁犲鏌熼鐣岀煉闁瑰磭鍋ゆ俊鐑芥晜缁涘鎴烽梻鍌氬€搁崐鐑芥嚄閸洖纾婚柟鎯ь嚟閻濆爼鏌涢埄鍐噥闁绘帒锕弻鏇熺箾閻愵剚鐝旂紓浣哄閸ㄥ爼寮婚敐澶婄闁挎繂鎲涢幘缁樼厸闁告侗鍠楅崐鎰版煛瀹€瀣瘈鐎规洘甯掕灒閻炴稈鈧厖澹曢梺鍝勭▉閸嬧偓闁稿鎸搁～婵嬵敆閸屾簽銊╂⒑閸濆嫯顫﹂柛鏃€鍨块獮鍐閵堝懎绐涙繝鐢靛Т鐎氼亞妲愰弴銏♀拻濞达絿鍎ら崵鈧梺鎼炲灪閻撯€崇暦閺屻儲鍤戦棅顒冨紦濮规姊哄Ч鍥х仼闁硅绻濋幃锟犲Ψ閳哄倻鍘介梺鍝勫€圭€笛囧疮閻愮儤鐓熼煫鍥风到濞呭秹鏌″畝鈧崰鎰八囬悧鍫熷劅闁靛繈鍨规慨浠嬫⒒娴ｅ憡鎯堥柣顒€銈稿畷浼村冀椤撶偠鎽曢梺闈浥堥弲婊堝磻閸曨垱鐓曟繛鎴烇公閸旂喎霉閻撳骸顏慨濠囩細閵囨劙骞掑┑鍥舵闂備胶枪缁绘垿鏁冮姀鐙€鍤曟い鎰剁畱缁犲鎮归崶褍绾фい銉︾箞濮婃椽妫冨☉姘暫濠碘槅鍋呴〃濠囥€侀弮鍫濋唶闁哄洨鍠撻崢鐐箾閹炬潙鍤柛銊ゅ嵆閹﹢鏁冮崒娑氬幐婵炶揪缍€椤鐣峰畝鈧埀顒侇問閸ｎ噣宕滈悢闀愮箚闁割偅娲栭獮銏′繆閵堝拑姊楃紒鎲嬬畵濮婄粯鎷呴挊澹捇鏌ｅΔ浣圭缂佽京鍋炵换婵嬪炊瑜庨悗顒勬⒑鐟欏嫬鍔ゅ褍娴锋竟鏇熺附閸涘﹦鍘介梺褰掑亰閸欏酣宕箛娑欑厸闁糕剝顭囬惌鎺楁煛瀹€鈧崰鏍箖濠婂牊鍤嶉柕澶涢檮濞堝憡淇婇悙顏勨偓褔姊介崟顖涘亗闁跨喓濮撮拑鐔兼煏婵炵偓娅嗛柛瀣閺屾稑鈽夊鍫濆婵炲鍘х€氭澘顫忔ウ瑁や汗闁圭儤绻冮ˉ鏍⒑缁嬭法绠查柨鏇樺灩椤曪綁顢曢敃鈧粻鑽ょ磽娴ｈ偂鎴濃枍閵忋倖鈷戦悹鎭掑妼濞呮劙鏌熼崙銈嗗
    //printf("donen");
    //thread : input ==>> send
    std::thread t1(send_message);
    t1.detach();
    //thread : recv ==>> show
    std::thread t2(receive_data);
    t2.detach();
    while(1)//闂傚倸鍊搁崐宄懊归崶褉鏋栭柡鍥ュ灩缁愭鏌熼悧鍫熺凡闁告垹濮撮埞鎴︽偐鐎圭姴顥濈紓浣哄閸ㄥ爼寮昏椤繈顢楅埀顒勫焵椤掆偓椤兘銆侀弮鍫熺劶鐎广儱妫岄幏娲煟閻樺厖鑸柛鏂跨焸瀵悂骞嬮敂鐣屽幐闁诲函缍嗘禍鍫曟偂閸忕⒈娈介柣鎰皺缁犲鏌熼鐣岀煉闁瑰磭鍋ゆ俊鐑芥晜缁涘鎴烽梻鍌氬€风粈渚€骞栭锕€纾归柛顐ｆ礀绾惧綊鏌″搴′簮闁稿鎸搁～婵囨綇閵娿儲鐣俊鐐€栧ú蹇涘磿閻㈡悶鈧線寮崼婵嬪敹闂佺粯鏌ㄩ幉锟狀敂閻斿吋鈷掑ù锝呮啞閹牓鏌涙繝鍌滃煟鐎规洘鍔欏浠嬵敃閻旇渹澹曞┑顔缴戦崜姘焽閹扮増鐓欑€瑰嫮澧楅崵鍥┾偓瑙勬礈鏋摶鏍归敐鍫綈妞ゅ繒鍠栧缁樻媴閸涘﹥鍎撻梺绋匡工閻栫厧鐣烽幇鏉垮唨妞ゆ劗鍠庢禍鐐叏濮楀棗澧扮紒澶嬫そ閺岋紕浠︾拠鎻掝潎闂佽鍠撻崹钘夌暦閵婏妇绡€闁告洦浜炵壕鍧楁⒒閸屾瑧顦︾紓宥咃躬钘熼柟鎹愵嚙缁€鍌涙叏濡炶浜鹃悗娈垮櫘閸嬪﹪鐛鈧畷婊勬媴缁嬭儻袝濠碉紕鍋戦崐鏍箰閻愵剚鍙忔い鎾卞灩閽冪喖鏌曢崼婵愭Ч闁稿缍侀弻娑㈠Ψ閵夛附鎮欓柣搴㈠嚬閸撶喖銆侀弮鍫濋唶闁哄洨鍟块幏娲煟閻樺厖鑸柛鏂跨焸瀵悂骞嬮敂鐣屽幐闁诲函缍嗘禍鍫曟偂閸忕⒈娈介柣鎰皺缁犲鏌熼鐣岀煉闁瑰磭鍋ゆ俊鐑芥晜缁涘鎴烽梻鍌氬€风粈渚€骞栭锕€纾归柛顐ｆ礀绾惧綊鏌″搴′簮闁稿鎸搁～婵嬵敆閸屾簽銊╂倵鐟欏嫭绀冪紒顔肩焸椤㈡﹢宕楅悡搴ｇ獮闁诲函缍嗛崑鍛搭敂閻斿吋鈷掑ù锝呮啞閸熺偤鏌熼柅娑氼槮闁崇粯妫冨鎾閳哄倹娅岄梻浣告惈鐞氼偊宕曟潏鈺冧笉妞ゆ牜鍋為悡銉╂煟閺囩偛鈧湱鈧熬鎷�
    {
        // printf("------------------main-----------------");
    }
    closesocket(global_control_param.socketServer);
    h3cProcess.stopEvent();
    return 0;
}