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
    tv.tv_sec = 0; //���õ���ʱʱ��
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
    while(1)
    {
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

        // Sleep(100);
    }
}
 
int main()
{
    int rval = 0;

    rval = h3cProcess.loginCamera();
    if(rval < 0)
    {
        std::cout << "��������ͷʧ��" << std::endl;
    }
    else
    {
        std::cout << "��������ͷ�ɹ�" << std::endl;
    }

    h3cProcess.startEvent();

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
    t.detach();//detach�Ļ�������̲߳�ͬ��ǰ��Ľ�����ɺ���ܽ��У���������Ϊjoin��ǰ����߳��޷��жϽ������ͻ�
    //һֱ�ȴ������º�����߳��޷����о��޷�ʵ�ֲ���
    //printf("donen");
    //thread : input ==>> send
    std::thread t1(send_message);
    t1.detach();
    //thread : recv ==>> show
    std::thread t2(receive_data);
    t2.detach();
    while(1)//��һ����ѭ��ʹ�����̲߳�����ǰ�˳�
    {
        // printf("------------------main-----------------");
    }
    closesocket(global_control_param.socketServer);
    h3cProcess.stopEvent();
    return 0;
}