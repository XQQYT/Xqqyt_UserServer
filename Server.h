/*
 * Xqqyt_UserServer
 * Author: XQQYT
 * License: MIT
 * Year: 2025
 */

#ifndef SERVER_H
#define SERVER_H

#include "TcpServer/include/TcpServer.h"
#include <string.h>
class Server:public TcpServer
{
public:
    Server(const int port, const uint32_t recvbufmax = 1024, const uint32_t clientmax = 256);

private:
    void haveNewConnection(const int socket) override;
    void haveNewClientMsg(const int socket) override;
    void clientDisconnect(const int socket) override;
    void incompleteMsg(const int socket) override;
    void dealClient(const int socket,std::string msg);

};

#endif // SERVER_H
