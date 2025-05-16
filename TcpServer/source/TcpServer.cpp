/*
 * Xqqyt_UserServer
 * Author: XQQYT
 * License: MIT
 * Year: 2025
 */

#include "TcpServer.h"
#include <memory>

std::mutex TcpServer::mtx;

TcpServer::TcpServer(const int port, const uint32_t recvbufmax, const uint32_t clientmax):
	deal_msg_thread_pool(std::make_unique<ThreadPool<>>(8,16,1024,ThreadPoolType::NORMAL)),
	mysql_conn_pool(std::make_shared<MySqlConnPool>())
{
    std::cout<<"User Server is running in "<<port<<std::endl;
	evs = nullptr;
	listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket == -1)
	{
		std::cerr << "socket failed with error: " << errno << std::endl;
		return;
	}
	socket_addr.sin_family = AF_INET;
	socket_addr.sin_port = htons(port);
	socket_addr.sin_addr.s_addr = INADDR_ANY;
	bind(listen_socket, (struct sockaddr*)&socket_addr, sizeof(socket_addr));

	this->my_epoll = epoll_create(256);
	if (this->my_epoll == -1)
	{
		std::cerr << "socket failed with error: " << errno << std::endl;
	}

	struct epoll_event ev;

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = listen_socket;

	epoll_ctl(my_epoll, EPOLL_CTL_ADD, listen_socket, &ev);

    evs = new epoll_event[clientmax];

	if (evs == nullptr)
	{
        std::cerr << "new epoll_event failed with error: " << errno << std::endl;
	}

	this->recv_buf_max = recvbufmax;
	this->client_max = clientmax;
    shutdown=false;
}


void TcpServer::startListen()
{
    int8_t ret = listen(listen_socket, client_max);
    if (ret == -1)
	{
        std::cout << "listen failed with error: " << errno << std::endl;
	}
    while (!shutdown)
	{
		int num = epoll_wait(my_epoll, evs, client_max, -1);

		struct sockaddr_in clientinfo;
		socklen_t len = sizeof(clientinfo);
		if (num < 0)
		{
			if (errno == EINTR) {
				std::cerr << "epoll_wait interrupted by signal\n";
				break;
			} else {
				perror("epoll_wait failed");
				continue;
			}
		}
		for (int i = 0; i < num; i++)
		{
			int curfd = evs[i].data.fd;
			if (curfd == listen_socket && evs[i].events == EPOLLIN)
			{
				int client_socket = accept(curfd, (sockaddr*)&clientinfo, &len);
				char* s_addr = inet_ntoa(clientinfo.sin_addr);
                //int flags=fcntl(client_socket, F_GETFL, 0);
                //fcntl(client_socket,F_SETFL,flags|O_NONBLOCK);
				haveNewConnection(client_socket);
			}
			else
			{
				if (evs[i].events == EPOLLIN)
				{
					char buf[1];
					ssize_t ret = recv(curfd, buf, 1, MSG_PEEK);
					if (ret == 0)
					{
						clientDisconnect(curfd);
					}
					else if (ret > 0)
					{
						haveNewClientMsg(curfd);
					}
					else if (errno == EAGAIN || errno == EWOULDBLOCK)
					{
						char tmp[1];
						recv(curfd, tmp, 1, 0);
						continue;
					}
					else
					{
						std::cerr << "Recv error: " << strerror(errno) << std::endl;
					}
				}
			}
		}
	}
}


void TcpServer::closeServer()
{
	if (listen_socket == -1)
		throw std::runtime_error("socket is null");
	close(listen_socket);
	int tmp = socket(AF_INET, SOCK_STREAM, 0);
    connect(tmp, (struct sockaddr*)&socket_addr, sizeof(socket_addr));
    close(tmp);
}


void TcpServer::clientDisconnect(const int socket)
{
	deleteFromEpoll(socket);
	close(socket);
}


bool TcpServer::addToEpoll(const int socket)
{
	if (socket == -1)
	{
		std::cout << "bad socket can not add into epoll" << std::endl;
		return false;
	}
	struct epoll_event ev;
	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = socket;
	epoll_ctl(my_epoll, EPOLL_CTL_ADD, socket, &ev);
	return true;
}


void TcpServer::deleteFromEpoll(const int socket)
{
	if (socket == -1)
	{
		std::cout << "bad socket can not add into epoll" << std::endl;
		return;
	}

	epoll_ctl(my_epoll, EPOLL_CTL_DEL, socket, nullptr);
}



void TcpServer::sendMsg(const int socket, std::string& msg)
{
    int total=msg.length();
    int send_done=0;
    int block=0;

    try {
        mtx.lock();
        while(send_done<total)
        {
            if(total-send_done<s_send_block_size)
                block=total-send_done;
            else
                block=s_send_block_size;
            send(socket, msg.c_str()+send_done, block, 0);
            send_done+=block;
        }
        mtx.unlock();
    } catch (const std::exception& e) {
        mtx.unlock();
    }

}

void TcpServer::write(const int socket, const char* msg,int len)
{
    send(socket, msg, len, 0);
}

bool TcpServer::checkClientDisconnected(int socket) {
    char buffer[1];
    int result = recv(socket, buffer, 1, MSG_PEEK);
    if (result == 0) {
        return true;
    } else if (result == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return false;
        } else {
            perror("recv");
            return true;
        }
    } else {
        return false;
    }
}

RecvMsg TcpServer::recvMsg(const int socket) {
    try {
        constexpr int BUFFER_SIZE = 100;
        auto buffer = new char[BUFFER_SIZE];
        uint32_t byte_recv = recv(socket, buffer, BUFFER_SIZE, 0);

        if (byte_recv <= 0) {
            std::cerr << "recv() 错误，错误码 -> " << errno
                      << ", 错误信息: " << strerror(errno) << std::endl;

            return RecvMsg{nullptr, 0};
        }

        std::string received_data(buffer, byte_recv);

        return RecvMsg{buffer, byte_recv};

    } catch (const std::exception& e) {
        std::cerr << "recvMsg 异常: " << e.what() << std::endl;

        char unused[128];
        while (recv(socket, unused, sizeof(unused) - 1, 0) > 0) {}

        incompleteMsg(socket);  // 可能需要清理粘包残留或关闭连接等
        return RecvMsg{nullptr, 0};
    }
}



SocketInfo TcpServer::getSocketInfo(const int socket)
{
    SocketInfo info = { 0 };
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);

	if (getpeername(socket, (struct sockaddr*)&client_addr, &addr_len) == -1) {
		std::cerr << "Error getting peer name: " << strerror(errno) << std::endl;
		return info;
	}

	inet_ntop(AF_INET, &(client_addr.sin_addr), info.ip, INET_ADDRSTRLEN);

	info.port = ntohs(client_addr.sin_port);

    return info;
}
