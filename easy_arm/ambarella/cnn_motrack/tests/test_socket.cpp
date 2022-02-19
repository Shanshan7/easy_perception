#include "network/network_transmission.h"


static NetWorkTransmission network_transmission;

int main() {
    int rval = 0;
    char send_data[100];
    snprintf(send_data, sizeof(send_data), "%s", "Hello world!!!\n");

    if(network_transmission.socket_init() >= 0)
    {
        do {
            network_transmission.send_data(send_data, sizeof(send_data));
            sleep(5);
        } while(1);
    }
    return rval;
}


// #define MAXLINE 4096

// int main(int argc, char** argv)
// {
//     int sockfd, n;
//     char recvline[4096], sendline[4096];
//     struct sockaddr_in servaddr;

//     if( argc != 2){
//         printf("usage: ./client <ipaddress>\n");
//         exit(0);
//     }

//     if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
//         printf("create socket error: %s(errno: %d)\n", strerror(errno),errno);
//         exit(0);
//     }

//     memset(&servaddr, 0, sizeof(servaddr));
//     servaddr.sin_family = AF_INET;
//     servaddr.sin_port = htons(1234);
//     if( inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0){
//         printf("inet_pton error for %s\n",argv[1]);
//         exit(0);
//     }

//     if( connect(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0){
//         printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
//         exit(0);
//     }

//     printf("send msg to server: \n");
//     // fgets(sendline, 4096, stdin);
//     snprintf(sendline, sizeof(sendline), "%s", "Hello world!!!\n");
//     do {
//         if( send(sockfd, sendline, strlen(sendline), 0) < 0)
//         {
//             printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
//             exit(0);
//         }
//         sleep(5);
//     } while(1);

//     close(sockfd);
//     exit(0);
// }


// #include <stdio.h>
// #include <stdlib.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <string.h>

// #define SERVER_IP "127.0.0.1"
// #define PORT 6675

// int main()
// {
//     int socket_fd;
//     socket_fd=socket(AF_INET,SOCK_STREAM,0);

//     struct sockaddr_in server_addr;
//     memset(&server_addr,0,sizeof(server_addr));
//     server_addr.sin_family=AF_INET;
//     server_addr.sin_port=htons(PORT);
//     server_addr.sin_addr.s_addr=inet_addr(SERVER_IP);
//     memset(&(server_addr.sin_zero),0,8);

//     int res =connect(socket_fd,(struct sockaddr *)&server_addr,sizeof(server_addr));
//     printf("connect res is %d\n",res);
    
//     /*
//     while(1)
//     {
//             printf("send send ---->\n");
//             char input[100];
//             char output[100];

//             memset(input,0,sizeof(input));
//             gets(input);
//             res=write(socket_fd,input,strlen(input));
//             printf("the write resoult is %d\n",res);
            
//             res=read(socket_fd,output,100);
//             output[res]=0;
//             printf("server says:'%s'\n",output);
//             sleep(1);
//     }
//     */
//     char path[100];
//     printf("please give the path of the photo\n");
//     char input[100];
//     gets(input);
//     sprintf(path,"/work/myproject/study/photo/%s",input);
//     write(socket_fd,input,strlen(input));

//     FILE *out=fopen(path,"r");
//     FILE *new=fopen("/work/myproject/study/photo/get/new.jpg","w");
//     int c;
//     sleep(1);
//     while((c=fgetc(out))!=EOF)
//     {
//         char photo[100];
//         sprintf(photo,"%d",c);
//         int b=atoi(photo);
//         fputc(b,new);
//                 printf("data %d\n",b);
//                 write(socket_fd,photo,strlen(photo));
//                 usleep(1000);
//     }
//     char *end="#";
//     write(socket_fd,end,strlen(end));
//     close(socket_fd);
// }

