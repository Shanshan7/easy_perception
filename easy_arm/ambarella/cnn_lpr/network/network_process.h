#ifndef _NETWORK_PROCESS_H_
#define _NETWORK_PROCESS_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <glog/logging.h>
#include <glog/raw_logging.h>


class NetWorkProcess
{
public:
    NetWorkProcess();
    ~NetWorkProcess();

    int init_network();
    int start();
    int stop();

    int process_recv();

    int send_result(const std::string &lpr_result, const int code);

    int send_log_path();

    int send_save_data();

private:

    
private:
    int udp_socket_fd;
    int upd_port;
    int dest_port;
	struct sockaddr_in dest_addr;

    int broadcast_socket_fd;
    int broadcast_port;

    pthread_t heart_pthread_id;
	pthread_t recv_thread_id;
};

#endif // _NETWORK_PROCESS_H_