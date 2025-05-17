/*
 * Xqqyt_UserServer
 * Author: XQQYT
 * License: MIT
 * Year: 2025
 */

#ifndef SERVER_H
#define SERVER_H

#include "TcpServer/include/TcpServer.h"
#include "OpensslHandler.h"
#include <string.h>
#include <unordered_map>

class Server:public TcpServer
{
    struct ByteArray {
        const uint8_t* data;
        size_t size;
    
        bool operator==(const ByteArray& other) const {
            return size == other.size && std::memcmp(data, other.data, size) == 0;
        }
    };
    
    struct ByteArrayHash {
        std::size_t operator()(const ByteArray& b) const {
            std::size_t hash = 0;
            for (size_t i = 0; i < b.size; ++i) {
                hash ^= std::hash<uint8_t>()(b.data[i]) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
            }
            return hash;
        }
    };
public:
    Server(const int port, const uint32_t recvbufmax = 1024, const uint32_t clientmax = 256);
    ~Server();
private:
    void haveNewConnection(const int socket) override;
    void haveNewClientMsg(const int socket) override;
    void clientDisconnect(const int socket) override;
    void incompleteMsg(const int socket) override;
    void haveTLSRequest(const int socket) override;
    void ClientAuthentication(const int socket, uint8_t* session_id) override;
    void dealClient(const int socket,uint8_t* msg, uint32_t length , uint8_t* key);
private:
    std::unordered_map<int, uint8_t*> socket_aeskey;
    std::unordered_map<ByteArray, uint8_t*,ByteArrayHash> sessionid_key;
};

#endif // SERVER_H
