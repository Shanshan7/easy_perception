#include "tcp_process.h"

static pthread_mutex_t send_mutex;

TCPProcess::TCPProcess()
{
    tcp_socket_fd = -1;
    tcp_port = 12345;

    accept_socket_fd = -1;
}

TCPProcess::~TCPProcess()
{
    if(accept_socket_fd > 0)
	{
		close(accept_socket_fd);
	}
	if(tcp_socket_fd > 0)
	{
		close(tcp_socket_fd);
	}
    pthread_mutex_destroy(&send_mutex);
    LOG(INFO) << "~TCPProcess()";
}

int TCPProcess::socket_init()
{
    int ret = 0;
    int opt = 1;
    struct sockaddr_in addr;

	tcp_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp_socket_fd < 0) {
        LOG(ERROR) << "ERROR: Unable to create socket!";
		return -1;
	}
	
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(tcp_port);
	
	setsockopt(tcp_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	ret = bind(tcp_socket_fd, (struct sockaddr *)&addr, sizeof(addr));
	if (ret < 0) {
        LOG(ERROR) << "ERROR: Unable to bind socket!";
		return -1;
	}
	
	ret = listen(tcp_socket_fd, 1);
	if (ret < 0) {
        LOG(ERROR) << "ERROR: Unable to listen at socket!";
		return -1;
	}

	signal(SIGPIPE, SIG_IGN);

    pthread_mutex_init(&send_mutex, NULL);

    LOG(WARNING) << "tcp socket init success!";
    return ret;
}

int TCPProcess::accept_connect()
{
    accept_socket_fd = accept(tcp_socket_fd, (struct sockaddr *)0, (socklen_t *)0);
    if (accept_socket_fd < 0) {
        LOG(ERROR) << "ERROR: Unable to accept client!";
        return -1;
    }
    LOG(WARNING) << "accept client success!";
    return 0;
}

int TCPProcess::send_data(const unsigned char* data, const size_t count)
{
    int ret = 0;
    if(accept_socket_fd > 0)
    {
        pthread_mutex_lock(&send_mutex);
        ret = send(accept_socket_fd, data, count, 0);
        if (ret < 0) 
        {
            LOG(ERROR) << "ERROR: send data fail!";
            ret = -1;
        }
        pthread_mutex_unlock(&send_mutex);
    }
    else
    {
        LOG(ERROR) << "ERROR: Unable to connect client!";
        ret = -1;
    }
    return ret;
}