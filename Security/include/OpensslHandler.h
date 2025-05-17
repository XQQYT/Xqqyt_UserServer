#ifndef _OPENSSLHANDLER_H
#define _OPENSSLHANDLER_H

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <vector>
#include <functional>
#include <memory>

class OpensslHandler
{
public:
    struct ParsedPayload {
        std::vector<uint8_t> iv;
        std::vector<uint8_t> encrypted_data;
        std::vector<uint8_t> sha256;
        uint8_t is_binary;
    };
    struct TlsInfo
    {
        uint8_t* key;
        uint8_t* session_id;
    };

public:
    static OpensslHandler& getInstance()
    {
        static OpensslHandler instance;
        return instance;
    } 
    bool verifyAndDecrypt(const std::vector<uint8_t>& encrypted_data,
        const uint8_t* key,
        const std::vector<uint8_t>& iv,
        std::vector<uint8_t>& out_plaintext,
        std::vector<uint8_t>& sha256);
    ParsedPayload parseMsgPayload(const uint8_t* full_msg ,const uint32_t length);
    void dealTls(int socket,std::function<void(bool,TlsInfo)> callback);
    std::unique_ptr<std::string>  dealPasswordSafe(std::string& password);

private:
    OpensslHandler();
    SSL_CTX* ctx;
};

#endif //_OPENSSLHANDLER_H