/*
 * Xqqyt_UserServer
 * Author: XQQYT
 * License: MIT
 * Year: 2025
 */

#ifndef _TCPSERVER_H
#define _TCPSERVER_H

#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string>
#include <memory.h>
#include <sys/epoll.h>
#include <memory>
#include <list>
#include <fcntl.h>
#include <sstream>
#include <iomanip>
#include <mutex>

#include "ThreadPool.hpp"
#include "MySqlConnPool.h"
#include <unordered_map>

static const int s_send_block_size=5120;

struct SocketInfo
{
	char ip[INET_ADDRSTRLEN];
	uint16_t port;
};

struct RecvMsg{
    uint8_t* ptr;
    uint32_t len;
};

class TcpServer
{
public:
	TcpServer(const int port, const uint32_t recvbufmax = 1024, const uint32_t clientmax = 256);
    ~TcpServer(){
        closeServer();
    }
	void startListen();

	virtual void haveNewConnection(const int socket) = 0;

    virtual void haveNewClientMsg(const int socket) = 0;

	virtual void haveTLSRequest(const int socket) = 0;

	virtual void ClientAuthentication(const int socket, uint8_t* session_id) = 0;

    virtual void incompleteMsg(const int socket) = 0;

	virtual void clientDisconnect(const int socket);
	bool addToEpoll(const int socket);
	void deleteFromEpoll(const int socket);

    static void write(const int socket, const char* msg,int len);
    static void sendMsg(const int socket, std::vector<uint8_t> msg);
	static void sendMsg(const int socket, const uint8_t* data, uint64_t length);
    RecvMsg recvMsg(const int socket);

	static struct SocketInfo getSocketInfo(const int socket);
protected:
	MySqlConnGuardPtr make_guard_ptr()
	{
		return MySqlConnGuardPtr(mysql_conn_pool);
	};

private:
    void closeServer();
    bool checkClientDisconnected(int socket);

protected:
    std::unique_ptr<ThreadPool<>> deal_msg_thread_pool;
    std::shared_ptr<MySqlConnPool> mysql_conn_pool;
    
private:
	int listen_socket;
	struct sockaddr_in socket_addr;
	uint32_t recv_buf_max;
	uint32_t client_max;
	struct epoll_event* evs;
	int my_epoll;
	std::atomic<bool> shutdown;
    static std::mutex mtx;
};

#endif
