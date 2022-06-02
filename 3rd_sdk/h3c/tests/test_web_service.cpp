#include <cstdio>
#include <WinSock2.h>
#include <string>
#include <sstream>
#include <json/json.h>

#include "h3cprocess.h"
#include "person_Features.h"

#define BUFFER_SIZE 1024
#define HOST "127.0.0.1"
#define PORT 5000
#define HEADER "\
HTTP/1.1 200 OK\r\n\
Server:nginx\r\n\
Access-Control-Allow-Origin: *\n\
Access-Control-Allow-Headers: *\n\
Content-Type: application/json; charset=UTF-8\r\n\
Content-Length: %d\r\n\r\n%s\
"
#pragma comment(lib, "WS2_32") 

int run_flag = -1;

int main(int argc, char **argv)
{
    int rval = 0;

    H3CProcess h3cProcess;
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

    // define and init an server sockaddr
    sockaddr_in addrServer;
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.S_un.S_addr = INADDR_ANY;
    addrServer.sin_port = htons(PORT);
    // init socket dll
    WSADATA wsaData;
    WORD socketVersion = MAKEWORD(2, 0);
    if (WSAStartup(socketVersion, &wsaData) != 0)
    {
        printf("Init socket dll error!");
        exit(1);
    }
    // create socket
    SOCKET socketServer = socket(AF_INET, SOCK_STREAM, 0);
    if (SOCKET_ERROR == socketServer)
    {
        printf("Create socket error!");
        exit(1);
    }
    // bind server socket host
    if (SOCKET_ERROR == bind(socketServer, (LPSOCKADDR)&addrServer, sizeof(addrServer)))
    {
        printf("Bind server host failed!");
        exit(1);
    }
    // listen
    if (SOCKET_ERROR == listen(socketServer, 10))
    {
        printf("Listen failed!");
        exit(1);
    }

    while (true)
    {
        printf("Listening ... \n");
        sockaddr_in addrClient;
        int nClientAddrLen = sizeof(addrClient);
        SOCKET socketClient = accept(socketServer, (sockaddr*)&addrClient, &nClientAddrLen);
        if (SOCKET_ERROR == socketClient)
        {
            printf("Accept failed!");
            break;
        }
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(socketClient, buffer, BUFFER_SIZE, 0) < 0)
        {
            printf("Recvive data failed!");
            break;
        }
        else
        {
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
                        std::string reply_message = root["person_attribute"].asString();
                        run_flag = atoi(reply_message.c_str());
                        std::cout << "Reading Complete!" << std::endl;
                    }
                    else
                    {
                        std::cout << "parse error\n" << std::endl;
                    }
                }
                p=strtok(NULL, d); // break
            }
        }

        // response
        memset(buffer, 0, BUFFER_SIZE);
        if (run_flag > 0)
        {
            h3cProcess.getResult();

            Json::Value jsPerson;
            jsPerson["person_feature"]["gender"] = h3cProcess.infor_Zs.ones.sex;
            jsPerson["person_feature"]["agegroup"] = h3cProcess.infor_Zs.ones.ageGroup;
            jsPerson["person_feature"]["coatcolor"] = h3cProcess.infor_Zs.ones.coatcolor;
            jsPerson["person_feature"]["trousersColor"] = h3cProcess.infor_Zs.ones.trousersColor;
            jsPerson["person_feature"]["hairStyle"] = h3cProcess.infor_Zs.ones.hairlen;
            jsPerson["person_feature"]["Orientation"] = h3cProcess.infor_Zs.ones.Orientation;

            // jsPerson1["age"] = 18;
            // std::string strPerson1 = jsPerson1.toStyledString();

            Json::FastWriter json_write;
            std::string strPerson = json_write.write(jsPerson);

            sprintf_s(buffer, HEADER, strlen(strPerson.c_str()), strPerson.c_str());
            if (send(socketClient, buffer, strlen(buffer), 0) < 0)
            {
                printf("Send data failed!");
                break;
            }
            // printf("Send data : \n%s", buffer);
        }
        else
        {
            Json::Value jsPerson1;
            jsPerson1["name"] = "error";
            // jsPerson1["age"] = 18;
            // std::string strPerson1 = jsPerson1.toStyledString();

            Json::FastWriter json_write;
            std::string strPerson1 = json_write.write(jsPerson1);
            sprintf_s(buffer, HEADER, strlen(strPerson1.c_str()), strPerson1.c_str());
            if (send(socketClient, buffer, strlen(buffer), 0) < 0)
            {
                printf("Send data failed!");
                break;
            }
        }
        
        closesocket(socketClient);
    }
    closesocket(socketServer);
    WSACleanup();
    
    h3cProcess.stopEvent();
    return 0;
}