/*
 * Xqqyt_UserServer
 * Author: XQQYT
 * License: MIT
 * Year: 2025
 */

#include "Server.h"
#include "MsgDecoder.h"
#include <functional>

void Server::dealClient(const int socket, uint8_t* msg, uint32_t length)
{
    // 检查是否为业务数据，排除是tls客户端挥手的报文
    if (*(msg) == 0xAB && *(msg + 1) == 0xCD)
    {
        auto parsed = openssl_handler.parseMsgPayload(msg,length);

        std::vector<uint8_t> result_vec;
        openssl_handler.aesDecrypt(parsed.encrypted_data,socket_aeskey[socket],parsed.iv,result_vec);

        std::string plaintext_str(result_vec.begin(), result_vec.end());
        plaintext_str.resize(plaintext_str.size()-4);

        std::cout<<"result "<<plaintext_str<<std::endl;
        // MsgDecoder::decode(socket, std::move(msg));
    }
}

Server::Server(const int port, const uint32_t recvbufmax, const uint32_t clientmax)
    :TcpServer(port,recvbufmax,clientmax)
{
    MsgDecoder::setMySqlConnPool(mysql_conn_pool);

}

Server::~Server()
{
    
}


void Server::haveNewConnection(const int socket)
{
    std::cout << "have new connection" << std::endl;
    uint8_t* key = openssl_handler.dealTls(socket);
    socket_aeskey.insert({socket,key});
    this->addToEpoll(socket);
}


void Server::haveNewClientMsg(const int socket)
{
    RecvMsg msg=recvMsg(socket);
    if(msg.ptr==nullptr)
        return;
    deal_msg_thread_pool->addTask(std::bind(&Server::dealClient,this,socket, reinterpret_cast<uint8_t*>(msg.ptr), msg.len));
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
