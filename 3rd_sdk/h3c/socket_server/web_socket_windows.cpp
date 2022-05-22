//*****************************************************************************
//@ProjectName      ZYhttpd
//@Description      my http server
//@Author           NicoleRobin
//@Date             2015/02/09
//*****************************************************************************
#include <cstdio>
#include <WinSock2.h>
#include <string>
using namespace std;
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

// #define HEADER "HTTP/1.1 200 OK\r\n\
// 			  Content-Type: application/json\n\
// 			  Cache-Control: no-cache\n\
// 			  Content-Length: %d\n\
// 			  Access-Control-Allow-Origin: *\n\n"
#define HTML "[{\"a\":1234}]"


#pragma comment(lib, "WS2_32")
int main(int argc, char **argv)
{
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
        std::string ret = ""; // 返回Http Response
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
        // else
        // {
        //     ret = buffer;// 如果接收成功，则返回接收的数据内容
		// 	char *p;
		// 	char *delims= {"\n"} ;
		// 	p=strtok(buffer,"\n");
		// 	int i = 0;
		// 	while(p!=NULL) {
		// 		if(i == 13){
		// 			printf("word: %s\n",p); //对获取的数据进行分割，获取每一行的数据 
		// 			ret = p;
		// 		}
		// 		i++;
		// 		p=strtok(NULL,delims);
		// 	}
        // }
        
        printf("Recv data: %s\n", buffer);
        // response
        memset(buffer, 0, BUFFER_SIZE);
        sprintf_s(buffer, HEADER, strlen(HTML), HTML);
        if (send(socketClient, buffer, strlen(buffer), 0) < 0)
        {
            printf("Send data failed!");
            break;
        }
        // printf("Send data : \n%s", buffer);
        closesocket(socketClient);
    }
    closesocket(socketServer);
    WSACleanup();
    return 0;
}

// #include <stdio.h>  
// #include <winsock2.h>  
  
// #pragma comment(lib,"ws2_32.lib")  
  
// int main(int argc, char* argv[])  
// {  
//     //初始化WSA  
//     WORD sockVersion = MAKEWORD(2,2);  
//     WSADATA wsaData;  
//     if(WSAStartup(sockVersion, &wsaData)!=0)  
//     {  
//         return 0;  
//     }  
  
//     //创建套接字  
//     SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
//     if(slisten == INVALID_SOCKET)  
//     {  
//         printf("socket error !");  
//         return 0;  
//     }  
  
//     //绑定IP和端口  
//     sockaddr_in sin;  
//     sin.sin_family = AF_INET;  
//     sin.sin_port = htons(5000);  
//     sin.sin_addr.S_un.S_addr = INADDR_ANY;   
//     if(bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)  
//     {  
//         printf("bind error !");  
//     }  
  
//     //开始监听  
//     if(listen(slisten, 5) == SOCKET_ERROR)  
//     {  
//         printf("listen error !");  
//         return 0;  
//     }  
  
//     //循环接收数据  
//     SOCKET sClient;  
//     sockaddr_in remoteAddr;  
//     int nAddrlen = sizeof(remoteAddr);  
//     char revData[255];   
//     while (true)  
//     {  
//         printf("Connecting...\n");  
//         sClient = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);  
//         if(sClient == INVALID_SOCKET)  
//         {  
//             printf("accept error !");  
//             continue;  
//         }  
//         printf("Accept a connection: %s \r\n", inet_ntoa(remoteAddr.sin_addr));  
          
//         //接收数据  
//         int ret = recv(sClient, revData, 255, 0);         
//         if(ret > 0)  
//         {  
//             revData[ret] = 0x00;  
//             printf(revData);  
//         }  
  
//         //发送数据  
//         const char * sendData = "Hi，TCP Client！\n";  
//         send(sClient, sendData, strlen(sendData), 0);  
//         closesocket(sClient);  
//     }  
      
//     closesocket(slisten);  
//     WSACleanup();  
//     return 0;  
// }