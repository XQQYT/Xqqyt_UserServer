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
    std::cout<<"进入业务代码解析"<<std::endl;
    auto parsed = OpensslHandler::getInstance().parseMsgPayload(msg,length);

    std::vector<uint8_t> result_vec;
    if(OpensslHandler::getInstance().verifyAndDecrypt(parsed.encrypted_data,socket_aeskey[socket],parsed.iv,result_vec, parsed.sha256))
    {
        std::string plaintext_str(result_vec.begin(), result_vec.end());
        plaintext_str.resize(plaintext_str.size()-4);
    
        std::cout<<"result "<<plaintext_str<<std::endl;
    }
    // MsgDecoder::decode(socket, std::move(msg));
    delete[] msg;
}

Server::Server(const int port, const uint32_t recvbufmax, const uint32_t clientmax)
    :TcpServer(port,recvbufmax,clientmax)
{
    MsgDecoder::setMySqlConnPool(mysql_conn_pool);

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
    deal_msg_thread_pool->addTask(std::bind(&Server::dealClient,this,socket, msg.ptr, msg.len));
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
