#include <iostream>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>

#define BUFFER_SIZE 1024
#define HOST "127.0.0.1"

#define HEADER "\
HTTP/1.1 200 OK\r\n\
Server:nginx\r\n\
Access-Control-Allow-Origin: *\n\
Access-Control-Allow-Headers: *\n\
Content-Type: application/json; charset=UTF-8\r\n\
Content-Length: %d\r\n\r\n%s\
"

#define HTML "[{\"a\":1234}]"

// int main() {
//     int rval = 0;

//     NetWorkService network_serivce;
//     rval = network_serivce.init_network();

//     while (true)
//     {
//         rval = network_serivce.process_recv();
//     }

//     return rval;
// }

int main(int argc, char **argv)
{
    int rval = 0;
    // define and init an server sockaddr
	struct sockaddr_in dest_addr;
	int dest_port = 5000;
    dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(dest_port);
	dest_addr.sin_addr.s_addr = INADDR_ANY;
    // create socket
	int udp_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if(udp_socket_fd < 0)
    {
        printf("Create socket error!");
        exit(1);
    }
    // bind server socket host
	rval = bind(udp_socket_fd, (struct sockaddr*)&dest_addr, sizeof(dest_addr));
	if(rval < 0)
	{
        printf("Bind server host failed!");
        exit(1);
	}
    // listen
    rval == listen(udp_socket_fd, 20);
    if(rval < 0)
    {
        printf("Listen failed!");
        exit(1);
    }
    while (true)
    {
        printf("Listening ... \n");
        std::string ret = "";
        struct sockaddr_in addrClient;
        socklen_t nClientAddrLen = sizeof(addrClient);
        int socketClient = accept(udp_socket_fd, (struct sockaddr*)&addrClient, &nClientAddrLen);
        if (socketClient < 0)
        {
            printf("Accept failed!\n");
            break;
        }
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        if (recv(socketClient, buffer, BUFFER_SIZE, 0) < 0)
        {
            printf("Recvive data failed!\n");
            // exit(1);
        }
        // else
        // {
        //     ret = buffer;// ������ճɹ����򷵻ؽ��յ���������
        // 	char *p;
        // 	char *delims= {"\n"} ;
        // 	p=strtok(buffer,"\n");
        // 	int i = 0;
        // 	while(p!=NULL) {
        // 		if(i == 13){
        // 			printf("word: %s\n",p); //�Ի�ȡ�����ݽ��зָ��ȡÿһ�е����� 
        // 			ret = p;
        // 		}
        // 		i++;
        // 		p=strtok(NULL,delims);
        // 	}
        // }
        
        printf("Recv data: %s\n", buffer);
        // response
        memset(buffer, 0, BUFFER_SIZE);
        snprintf(buffer, strlen(HTML), HEADER,  HTML);
        if (send(socketClient, buffer, strlen(buffer), 0) < 0)
        {
            printf("Send data failed!\n");
            // exit(1);
        }
        printf("Send data : \n%s", buffer);
        close(socketClient);
    }
    close(udp_socket_fd);
    return 0;
}