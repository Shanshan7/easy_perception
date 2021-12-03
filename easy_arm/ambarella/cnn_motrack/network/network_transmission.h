#ifndef _NETWORK_TRANSMISSION_H_
#define _NETWORK_TRANSMISSION_H_

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <glog/logging.h>
#include <glog/raw_logging.h>

class NetWorkTransmission
{
public:
    NetWorkTransmission();
    ~NetWorkTransmission();

    int socket_init();

    // int accept_connect();

    int send_data(char* data, size_t count);

private:
    int tcp_socket_fd;
    int tcp_port;

    // int accept_socket_fd;
};

#endif // _NETWORK_TRANSMISSION_H_