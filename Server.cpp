/*
 * Xqqyt_UserServer
 * Author: XQQYT
 * License: MIT
 * Year: 2025
 */

#include "Server.h"
#include "MsgDecoder.h"
#include <functional>

void Server::dealClient(const int socket,std::string msg)
{
    MsgDecoder::decode(socket, std::move(msg));
}


Server::Server(const int port, const uint32_t recvbufmax, const uint32_t clientmax)
    :TcpServer(port,recvbufmax,clientmax)
{

}


void Server::haveNewConnection(const int socket)
{
    std::cout<<"have new connection"<<std::endl;
    this->addToEpoll(socket);
}


void Server::haveNewClientMsg(const int socket)
{
    RecvMsg msg=recvMsg(socket);
    if(msg.ptr==nullptr)
        return;
    deal_msg_thread_pool->addTask(std::bind(&Server::dealClient,this,socket,std::string(msg.ptr,msg.len)));
}

void Server::clientDisconnect(const int socket)
{
    std::cout<<"client disconnect"<<std::endl;
    this->deleteFromEpoll(socket);
}

void Server::incompleteMsg(const int socket)
{
    std::cout<<"msg is incomplete "<<std::endl;
}
