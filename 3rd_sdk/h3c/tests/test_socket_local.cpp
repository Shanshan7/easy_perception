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
int socketServer;
struct sockaddr_in addrServer;
int nServerAddrLen;
std::list<int> client_list; //用list来存放套接字，没有限制套接字的容量就可以实现一个server跟若干个client通信
std::string http_method;

int run_receive_message = 1;
int run_send_message = 0;
 
void get_connect()
{
    while(1)
    {
        int connect = accept(socketServer, (struct sockaddr*)&addrServer, &nServerAddrLen);
        client_list.push_back(connect);
    }
}
 
void receive_data()
{
    int rval = 0;
    struct timeval tv;
    tv.tv_sec = 0;//设置倒计时时间
    tv.tv_usec = 2000;
    printf("-------------------receive data-------------------\n");
    while(run_receive_message)
    {
        std::list<int>::iterator it;
        for(it=client_list.begin(); it!=client_list.end(); ++it)
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
                    if (strstr(buffer, "GET") != NULL)
                    {
                        http_method = "GET";
                    }
                    else if(strstr(buffer, "POST") != NULL)
                    {
                        http_method = "POST";
                    }
                    else
                    {
                        printf("Unknown request!\n");
                    }

                    // if(strcmp(buffer, "exitn") == 0) break;
                    // analysis input json
                    const char *d = "\r\n\r\n";
                    char *p;
                    p = strtok(buffer, d);
                    while(p)
                    {
                        if(strstr(p, "{") != NULL)
                        {
                            Json::Reader reader;
                            Json::Value root;

                            if (reader.parse(p, root))
                            {
                                // std::string reply_message = root["person_attribute"].asString();
                                bool reply_message = root["person_attribute"].asBool();
                                run_send_message = int(reply_message);
                                printf("run_send_message: %d\n", run_send_message);
                                std::cout << "Reading Complete!" << std::endl;
                            }
                            else
                            {
                                std::cout << "parse error\n" << std::endl;
                            }
                        }
                        p=strtok(NULL, d); // break
                    }
                    if (http_method == "POST")
                    {
                        char buffer_response[BUFFER_SIZE];
                        memset(buffer_response, 0, BUFFER_SIZE);
                        char* response = "ready!";
                        sprintf_s(buffer_response, HEADER, strlen(response), response);
                        send(*it, buffer_response, sizeof(buffer_response), 0);
                        printf("Response!!!!!\n");
                    }
                }
            }
        }
        // Sleep(1000);
    }
}
 
void send_message()
{
    int rval = 0;
    while(1)
    {
        h3cProcess.getResult();
        // printf("-------------------send data------------------- %d %s\n", run_send_message, http_method);
        if (run_send_message && http_method == "GET")
        {
            char buffer[BUFFER_SIZE];
            memset(buffer, 0, BUFFER_SIZE);

            Json::Value jsPerson;
            // jsPerson["person_content"]["person_feature"]["gender"] = h3cProcess.infor_Zs.ones.sex;
            // jsPerson["person_content"]["person_feature"]["agegroup"] = h3cProcess.infor_Zs.ones.ageGroup;
            // jsPerson["person_content"]["person_feature"]["coatcolor"] = h3cProcess.infor_Zs.ones.coatcolor;
            // jsPerson["person_content"]["person_feature"]["trousersColor"] = h3cProcess.infor_Zs.ones.trousersColor;
            // jsPerson["person_content"]["person_feature"]["hairStyle"] = h3cProcess.infor_Zs.ones.hairStyle;
            // jsPerson["person_content"]["person_feature"]["Orientation"] = h3cProcess.infor_Zs.ones.Orientation;

            // jsPerson["person_content"]["device_information"]["dev_id"] = h3cProcess.infor_Zs.cammers.dev_id;

            // jsPerson["person_content"]["sub_image_information"]["snap_path"] = h3cProcess.infor_Zs.sub_img_info.snap_path;

            jsPerson["person_content"]["person_feature"]["gender"] = "male";
            jsPerson["person_content"]["person_feature"]["agegroup"] = "child";
            jsPerson["person_content"]["person_feature"]["coatcolor"] = "blue";
            jsPerson["person_content"]["person_feature"]["trousersColor"] = "black";
            jsPerson["person_content"]["person_feature"]["hairStyle"] = "short";
            jsPerson["person_content"]["person_feature"]["Orientation"] = "foreward";

            jsPerson["person_content"]["device_information"]["dev_id"] = "aaaaaaaaaaaaaaaaaaaaaaa";

            Json::FastWriter json_write;
            std::string strPerson = json_write.write(jsPerson);

            std::list<int>::iterator it;
            for(it=client_list.begin(); it!=client_list.end(); ++it)
            {
                sprintf_s(buffer, HEADER, strlen(strPerson.c_str()), strPerson.c_str());
                send(*it, buffer, sizeof(buffer), 0);
                printf("Sending data\n");
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
        std::cout << "连接摄像头失败" << std::endl;
    }
    else
    {
        std::cout << "连接摄像头成功" << std::endl;
    }

    h3cProcess.startEvent();

    //new socket
    socketServer = socket(AF_INET, SOCK_STREAM, 0);
    memset(&addrServer, 0, sizeof(addrServer));
    addrServer.sin_family = AF_INET;
    addrServer.sin_port = htons(PORT);
    addrServer.sin_addr.s_addr = INADDR_ANY;
    if(bind(socketServer, (struct sockaddr* ) &addrServer, sizeof(addrServer))==-1)
    {
        perror("bind");
        exit(1);
    }
    if(listen(socketServer, 20) == -1)
    {
        perror("listen");
        exit(1);
    }
    nServerAddrLen = sizeof(addrServer);
    
    //thread : while ==>> accpet
    std::thread t(get_connect);
    t.detach();//detach的话后面的线程不同等前面的进程完成后才能进行，如果这里改为join则前面的线程无法判断结束，就会
    //一直等待，导致后面的线程无法进行就无法实现操作
    //printf("donen");
    //thread : input ==>> send
    std::thread t1(send_message);
    t1.detach();
    //thread : recv ==>> show
    std::thread t2(receive_data);
    t2.detach();
    while(1)//做一个死循环使得主线程不会提前退出
    {
        // printf("------------------h3c-----------------");
    }
    closesocket(socketServer);
    h3cProcess.stopEvent();
    return 0;
}