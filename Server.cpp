/*
 * Xqqyt_UserServer
 * Author: XQQYT
 * License: MIT
 * Year: 2025
 */

#include "Server.h"
#include "MsgDecoder.h"
#include <functional>

void Server::dealClient(const int socket, uint8_t* msg, uint32_t length ,uint8_t* key, MySqlDriver* mysql_driver)
{
    std::cout<<"进入业务代码解析"<<std::endl;
    auto parsed = OpensslHandler::getInstance().parseMsgPayload(msg,length);

    std::vector<uint8_t> result_vec;
    if(OpensslHandler::getInstance().verifyAndDecrypt(parsed.encrypted_data,socket_aeskey[socket],parsed.iv,result_vec, parsed.sha256))
    {
        result_vec.resize(result_vec.size() - 4);
        
        MsgDecoder::decode(socket, std::move(result_vec), parsed.is_binary, key, mysql_driver);
    }
}

Server::Server(const int port, const uint32_t recvbufmax, const uint32_t clientmax)
    :TcpServer(port,recvbufmax,clientmax)
{

}

Server::~Server()
{
    for (auto& pair : socket_aeskey) {
        delete[] pair.second;
    }
    socket_aeskey.clear();

    for(auto& pair : sessionid_key)
    {
        delete[] pair.second;
    }
    sessionid_key.clear();
}


void Server::haveNewConnection(const int socket)
{
    std::cout << "have new connection" << std::endl;
    addToEpoll(socket);
}

void Server::haveTLSRequest(const int socket)
{
    OpensslHandler::getInstance().dealTls(socket,[this,socket](bool ret, OpensslHandler::TlsInfo tls_info){
        if(ret)
            sessionid_key.insert({{tls_info.session_id , 32},tls_info.key});
        else
            std::cout<<"Failed to dealTls"<<std::endl;
    });
}

void Server::ClientAuthentication(const int socket, uint8_t* session_id)
{
    if(sessionid_key.find({session_id, 32}) == sessionid_key.end())
    {
        std::cout<<"未找到session id"<<std::endl;
        return;
    }
    socket_aeskey[socket] = sessionid_key[{session_id, 32}];
    sessionid_key.erase({session_id,32});
    std::cout<<"client 注册成功"<<std::endl;
}


void Server::haveNewClientMsg(const int socket)
{
    std::cout<<"have client message"<<std::endl;
    RecvMsg msg=recvMsg(socket);
    if(msg.ptr==nullptr)
        return;
    deal_msg_thread_pool->addTask(
        [=, msg = std::move(msg)]() mutable {
            auto conn_guard = MySqlConnGuardPtr(mysql_conn_pool);
            if (!conn_guard.valid()) return;
            dealClient(socket, msg.ptr, msg.len, socket_aeskey[socket], conn_guard.get());
            delete[] msg.ptr;
        }
    );

}

void Server::clientDisconnect(const int socket)
{
    std::cout<<"client disconnect"<<std::endl;
    socket_aeskey.erase(socket);
    this->deleteFromEpoll(socket);
}

void Server::incompleteMsg(const int socket)
{
    std::cout<<"msg is incomplete "<<std::endl;
}
