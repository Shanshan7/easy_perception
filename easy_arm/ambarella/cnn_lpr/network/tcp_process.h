#ifndef _TCP_PROCESS_H_
#define _TCP_PROCESS_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <glog/logging.h>
#include <glog/raw_logging.h>

class TCPProcess
{
public:
    TCPProcess();
    ~TCPProcess();

    int socket_init();

    int accept_connect();

    int send_data(const unsigned char* data, const size_t count);

private:
    int tcp_socket_fd;
    int tcp_port;

    int accept_socket_fd;
};

#endif // _TCP_PROCESS_H_