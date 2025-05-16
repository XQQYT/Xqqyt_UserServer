#ifndef _OPENSSLHANDLER_H
#define _OPENSSLHANDLER_H

#include <openssl/rand.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/aes.h>
#include <vector>

class OpensslHandler
{
public:
    struct __attribute__((packed)) Header {
        uint16_t magic;
        uint8_t version;
        uint32_t length;
    };
    
    struct ParsedPayload {
        Header header;
        std::vector<uint8_t> iv;
        std::vector<uint8_t> encrypted_data;
        std::vector<uint8_t> sha256;
    };

public:
    OpensslHandler();
    bool aesDecrypt(const std::vector<uint8_t>& encrypted_data,
        const uint8_t* key,
        const std::vector<uint8_t>& iv,
        std::vector<uint8_t>& out_plaintext);
    ParsedPayload parseMsgPayload(const uint8_t* full_msg ,const uint32_t length);
    uint8_t* dealTls(int socket);
private:
    SSL_CTX* ctx;
};

#endif //_OPENSSLHANDLER_H