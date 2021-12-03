#include "../common/data_struct.h"
#include "network_transmission.h"


NetWorkTransmission::NetWorkTransmission()
{
    tcp_socket_fd = -1;
    tcp_port = 1234;

    // accept_socket_fd = -1;
}

NetWorkTransmission::~NetWorkTransmission()
{
    // if(accept_socket_fd > 0)
	// {
	// 	close(accept_socket_fd);
	// }
	if(tcp_socket_fd > 0)
	{
		close(tcp_socket_fd);
	}
    // pthread_mutex_destroy(&send_mutex);
    // LOG(INFO) << "~TCPProcess()";
}

int NetWorkTransmission::socket_init()
{
    int rval = 0;
    int opt = 1;
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

	tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket_fd < 0) {
        LOG(ERROR) << "ERROR: Unable to create socket!";
		return -1;
	}
	
    char ip_adress[] = "10.0.0.10";
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(ip_adress);
	addr.sin_port = htons(tcp_port);
	
	// setsockopt(tcp_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	// rval = bind(tcp_socket_fd, (struct sockaddr *)&addr, sizeof(addr));
	// if (rval < 0) {
    //     printf("[ERROR]: Unable to bind socket!");
	// 	return -1;
	// }
	
	// rval = listen(tcp_socket_fd, 1);
	// if (rval < 0) {
    //     printf("[ERROR]: Unable to listen at socket!");
	// 	return -1;
	// }
    
    if( inet_pton(AF_INET, ip_adress, &addr.sin_addr) <= 0){
        printf("inet_pton error for %s\n", ip_adress);
        return -1;
    }

    if( connect(tcp_socket_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        printf("connect error: %s(errno: %d)\n",strerror(errno),errno);
        return -1;
    }

	signal(SIGPIPE, SIG_IGN);

    // pthread_mutex_init(&send_mutex, NULL);

    printf("tcp socket init success!");
    return rval;
}

// int NetWorkTransmission::accept_connect()
// {
//     accept_socket_fd = accept(tcp_socket_fd, (struct sockaddr *)0, (socklen_t *)0);
//     if (accept_socket_fd < 0) {
//         LOG(ERROR) << "ERROR: Unable to accept client!";
//         return -1;
//     }
//     printf("accept client success!");
//     return 0;
// }

int NetWorkTransmission::send_data(char* data, size_t count)
{
    int rval = 0;

    rval = send(tcp_socket_fd, data, count, 0);
    if( rval < 0)
    {
        printf("send msg error: %s(errno: %d)\n", strerror(errno), errno);
        exit(0);
    }
    
    return rval;
}