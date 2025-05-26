/*
 * Xqqyt_UserServer
 * Author: XQQYT
 * License: MIT
 * Year: 2025
 */

#include "TcpServer.h"
#include <memory>
#include <arpa/inet.h>

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
                // int flags=fcntl(client_socket, F_GETFL, 0);
                // fcntl(client_socket,F_SETFL,flags|O_NONBLOCK);
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



void TcpServer::sendMsg(const int socket, std::vector<uint8_t>  msg)
{
    int total=msg.size();
    int send_done=0;
    int block=0;

    try {
        while(send_done<total)
        {
            if(total-send_done<s_send_block_size)
                block=total-send_done;
            else
                block=s_send_block_size;
            send(socket, msg.data()+send_done, block, 0);
            send_done+=block;
			std::cout<<"send "<<send_done<<" / "<<total<<std::endl;
        }
    } catch (const std::exception& e) {
    }

}

void TcpServer::sendMsg(const int socket, const uint8_t* data, uint64_t length) {
    int total = static_cast<int>(length);
    int send_done = 0;
    int block = 0;

    try {
        while (send_done < total) {
            if (total - send_done < s_send_block_size)
                block = total - send_done;
            else
                block = s_send_block_size;

            int sent = send(socket, data + send_done, block, 0);
            if (sent <= 0) {
                std::cerr << "Send failed or connection closed\n";
                break; 
            }
            send_done += sent;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception during send: " << e.what() << std::endl;
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
		uint8_t peek_buffer[4];
		int peeked = recv(socket, peek_buffer, sizeof(peek_buffer), MSG_PEEK);

		if(peek_buffer[0] == 0xEA && peek_buffer[1] == 0xEA)
		{
			std::cout<<"Tls请求"<<std::endl;
			uint8_t unused[2];
			recv(socket, unused , 2 , 0);
			haveTLSRequest(socket);
			return RecvMsg{nullptr, 0};
		}
		else if(peek_buffer[0] == 0xFA && peek_buffer[1] == 0xFA)
		{
			std::cout<<"用户注册"<<std::endl;

			uint8_t unused[2];
			recv(socket, unused , 2 , 0);

			constexpr int SESSIONID_SIZE = 32;
			uint8_t session_id[SESSIONID_SIZE];
			uint32_t sessionid_received = 0;
			while (sessionid_received < SESSIONID_SIZE) {
				int n = recv(socket, session_id + sessionid_received, SESSIONID_SIZE - sessionid_received, 0);
				if (n <= 0) throw std::runtime_error("Header recv error");
				sessionid_received += n;
			}
			ClientAuthentication(socket, session_id);
			return RecvMsg{nullptr, 0};

		}
		else if(peek_buffer[0] == 0xAB && peek_buffer[1] == 0xCD)
		{
			std::cout<<"用户消息"<<std::endl;
			constexpr int HEADER_SIZE = 7;
			uint8_t buffer[HEADER_SIZE] = {0};
			uint32_t header_received = 0;
			while (header_received < HEADER_SIZE) {
				int n = recv(socket, buffer + header_received, HEADER_SIZE - header_received, 0);
				if (n <= 0) throw std::runtime_error("Header recv error");
				header_received += n;
			}

			uint32_t payload_length = 0;
			memcpy(&payload_length, buffer + 3, sizeof(payload_length));
			payload_length = ntohl(payload_length);

			uint8_t* receive_msg = new uint8_t[payload_length];
			uint32_t readed_length = 0;

			while (readed_length < payload_length) {
				int read_byte = recv(socket, receive_msg + readed_length, payload_length - readed_length, 0);
				if (read_byte == 0) {
					delete[] receive_msg;
					throw std::runtime_error("peer closed");
				}
				if (read_byte < 0) {
					delete[] receive_msg;
					throw std::runtime_error("recv error");
				}
				readed_length += read_byte;
				std::cout<<readed_length<<" / "<<payload_length<<std::endl;
			}

			// 4. 返回完整数据
			return RecvMsg{receive_msg, payload_length};
		}
		else if(peek_buffer[0] == 0x17 && peek_buffer[1] == 0x03)
		{
			std::cout<<"Tls close msg"<<std::endl;
			throw std::runtime_error("");
		}
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        // 清理 socket 中的缓存，防止粘包影响后续
        char unused[128];
        while (recv(socket, unused, sizeof(unused) - 1, 0) > 0) {}

        incompleteMsg(socket);
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
