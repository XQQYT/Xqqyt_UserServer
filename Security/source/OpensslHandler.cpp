#include "OpensslHandler.h"
#include <iostream>
#include <cstring> 
#include <arpa/inet.h>

OpensslHandler::OpensslHandler()
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD* method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    
    SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM);
}

uint8_t* OpensslHandler::dealTls(int socket)
{
    SSL* ssl = SSL_new(ctx);
    SSL_set_fd(ssl, socket);

    if (SSL_accept(ssl) <= 0) {
        std::cerr << "SSL handshake failed" << std::endl;
        SSL_free(ssl);
        return nullptr;
    }

    unsigned char *key = new unsigned char[32]();
    if (RAND_bytes(key, 32) != 1) {
        std::cerr << "Failed to generate random key" << std::endl;
        SSL_free(ssl);
        return nullptr;
    }

    int bytesWritten = SSL_write(ssl, key, 32);
    if (bytesWritten <= 0) {
        std::cerr << "Failed to send key to client" << std::endl;
    } else {
        std::cout << "Sent 256-bit key to client over TLS" << std::endl;
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    return key;
}

OpensslHandler::ParsedPayload OpensslHandler::parseMsgPayload(const uint8_t* full_msg ,const uint32_t length) {
    ParsedPayload result;

    size_t offset = 0;

    // 1. 解析 Header
    if (length < sizeof(Header)) {
        throw std::runtime_error("消息太短，缺少 Header");
    }
    std::memcpy(&result.header, full_msg, sizeof(Header));
    result.header.magic = ntohs(result.header.magic);
    result.header.length = ntohl(result.header.length);
    offset += sizeof(Header);

    // 2. 校验 payload 长度是否符合
    if (length != sizeof(Header) + result.header.length) {
        std::cout<<"length  "<<length<<"  "<<sizeof(Header) + result.header.length<<std::endl;;
        // throw std::runtime_error("消息长度与 Header 中标记不一致");
    }

    // 3. 解析 IV（16字节）
    result.iv.assign(full_msg + offset, full_msg + offset + 16);
    offset += 16;

    // 4. 解析密文
    size_t cipher_len = result.header.length - 16 - 32;
    result.encrypted_data.assign(full_msg + offset, full_msg + offset + cipher_len);
    offset += cipher_len;

    // 5. 解析 SHA256（32字节）
    result.sha256.assign(full_msg + offset, full_msg + offset + 32);

    return result;
}

bool OpensslHandler::aesDecrypt(const std::vector<uint8_t>& encrypted_data,
    const uint8_t* key,
    const std::vector<uint8_t>& iv,
    std::vector<uint8_t>& out_plaintext) {
    if (iv.size() != AES_BLOCK_SIZE) {
        std::cerr << "IV size incorrect" << std::endl;
        return false;
    }

    AES_KEY aesKey;
    if (AES_set_decrypt_key(key, 256, &aesKey) < 0) {
        std::cerr << "Failed to set AES decryption key" << std::endl;
        return false;
    }

    out_plaintext.resize(encrypted_data.size());
    uint8_t iv_copy[AES_BLOCK_SIZE];
    std::memcpy(iv_copy, iv.data(), AES_BLOCK_SIZE);

    AES_cbc_encrypt(encrypted_data.data(),
            out_plaintext.data(),
            encrypted_data.size(),
            &aesKey,
            iv_copy,
            AES_DECRYPT);

    // 1. 移除 PKCS#7 填充
    if (!out_plaintext.empty()) {
        uint8_t padding_size = out_plaintext.back();

        if (padding_size == 0 || padding_size > AES_BLOCK_SIZE) {
        std::cerr << "Invalid padding size" << std::endl;
        return false;
    }

    bool padding_valid = true;
    size_t start = out_plaintext.size() - padding_size;
    for (size_t i = start; i < out_plaintext.size(); ++i) {
        if (out_plaintext[i] != padding_size) {
            padding_valid = false;
            break;
        }
    }

    if (!padding_valid) {
        std::cerr << "Invalid padding content" << std::endl;
        return false;
    }

        out_plaintext.resize(start);
    }

    // 2. 移除 CRC32（4字节）
    if (out_plaintext.size() < sizeof(uint32_t)) {
        std::cerr << "Data too short to contain CRC32" << std::endl;
        return false;
    }
    out_plaintext.resize(out_plaintext.size() - sizeof(uint32_t));

    return true;
}