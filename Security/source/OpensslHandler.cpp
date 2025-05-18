#include "OpensslHandler.h"
#include <iostream>
#include <cstring> 
#include <arpa/inet.h>
#include <openssl/sha.h>
#include <sodium.h>
#include <zlib.h>

OpensslHandler::OpensslHandler()
{
    SSL_library_init();
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    const SSL_METHOD* method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    
    SSL_CTX_use_certificate_file(ctx, "server.crt", SSL_FILETYPE_PEM);
    SSL_CTX_use_PrivateKey_file(ctx, "server.key", SSL_FILETYPE_PEM);

    if (sodium_init() < 0) {
        std::cerr << "libsodium 初始化失败\n";
    }
}

void OpensslHandler::dealTls(int socket, std::function<void(bool, TlsInfo)> callback) {
    SSL* ssl = SSL_new(ctx);
    if (!ssl) {
        std::cerr << "SSL_new failed" << std::endl;
        callback(false, {nullptr, nullptr});
        return;
    }

    SSL_set_fd(ssl, socket);

    if (SSL_accept(ssl) <= 0) {
        std::cerr << "SSL handshake failed" << std::endl;
        ERR_print_errors_fp(stderr);
        SSL_shutdown(ssl);
        SSL_free(ssl);
        callback(false, {nullptr, nullptr});
        return;
    }

    uint8_t*  key = new uint8_t[32];
    uint8_t* session_id = new uint8_t[32];

    if (RAND_bytes(key, 32) != 1 || RAND_bytes(session_id, 32) != 1) {
        std::cerr << "Failed to generate random key or session id" << std::endl;
        SSL_shutdown(ssl);
        SSL_free(ssl);
        callback(false, {nullptr, nullptr});
        return;
    }

    if (SSL_write(ssl, key, 32) <= 0) {
        std::cerr << "Failed to send key to client" << std::endl;
        SSL_shutdown(ssl);
        SSL_free(ssl);
        callback(false, {nullptr, nullptr});
        return;
    }

    if (SSL_write(ssl, session_id, 32) <= 0) {
        std::cerr << "Failed to send session id to client" << std::endl;
        SSL_shutdown(ssl);
        SSL_free(ssl);
        callback(false, {nullptr, nullptr});
        return;
    }

    std::cout << "Sent key and session id to client over TLS" << std::endl;

    SSL_shutdown(ssl);
    SSL_free(ssl);

    callback(true, {key, session_id});
}

OpensslHandler::ParsedPayload OpensslHandler::parseMsgPayload(const uint8_t* full_msg ,const uint32_t length) {
    ParsedPayload result;

    size_t offset = 0;

    memcpy(&result.is_binary, full_msg, 1);
    offset += 1;

    // 3. 解析 IV（16字节）
    result.iv.assign(full_msg + offset, full_msg + offset + 16);
    offset += 16;

    // 4. 解析密文
    size_t cipher_len = length - 16 - 32 - 1;
    result.encrypted_data.assign(full_msg + offset, full_msg + offset + cipher_len);
    offset += cipher_len;

    // 5. 解析 SHA256（32字节）
    result.sha256.assign(full_msg + offset, full_msg + offset + 32);

    return result;
}

bool verify_sha256(const std::vector<uint8_t>& data, const std::vector<uint8_t>& expected_hash) {
    if (expected_hash.size() != SHA256_DIGEST_LENGTH) {
        std::cerr << "Invalid hash length." << std::endl;
        return false;
    }

    // 计算实际的 SHA-256 哈希
    uint8_t hash[SHA256_DIGEST_LENGTH];
    SHA256(data.data(), data.size(), hash);

    // 比较实际哈希和期望哈希
    return std::memcmp(hash, expected_hash.data(), SHA256_DIGEST_LENGTH) == 0;
}

bool OpensslHandler::verifyAndDecrypt(const std::vector<uint8_t>& encrypted_data,
    const uint8_t* key,
    const std::vector<uint8_t>& iv,
    std::vector<uint8_t>& out_plaintext,
    std::vector<uint8_t>& sha256) {
    if (iv.size() != AES_BLOCK_SIZE) {
        std::cerr << "IV size incorrect" << std::endl;
        return false;
    }

    std::vector<uint8_t> iv_encrypted(iv.begin(),iv.end());
    iv_encrypted.insert(iv_encrypted.end(), encrypted_data.begin(), encrypted_data.end());

    if(!verify_sha256(iv_encrypted, sha256))
    {
        std::cout<<"sha256校验失败"<<std::endl;
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

std::unique_ptr<std::string>  OpensslHandler::dealPasswordSafe(std::string& password)
{
    std::unique_ptr<std::string> hashed_password = std::make_unique<std::string>(crypto_pwhash_STRBYTES, '\0');

    if (crypto_pwhash_str(
            hashed_password->data(),
            password.data(),
            password.size(),
            crypto_pwhash_OPSLIMIT_INTERACTIVE,
            crypto_pwhash_MEMLIMIT_INTERACTIVE) != 0) {
        std::cerr << "哈希生成失败（可能是内存不足）\n";
        return nullptr;
    }
    hashed_password->resize(strlen(hashed_password->c_str()));
    return hashed_password;
}

uint8_t* OpensslHandler::aesEncrypt(std::vector<uint8_t>& data, const uint8_t* key)
{
    // 1. 计算 CRC32 并附加到数据末尾
    uLong crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, data.data(), data.size());

    uint8_t* crc_bytes = reinterpret_cast<uint8_t*>(&crc);
    data.insert(data.end(), crc_bytes, crc_bytes + sizeof(crc));

    // 2. 添加 PKCS#7 填充（正确的填充方式）
    size_t blockSize = AES_BLOCK_SIZE;
    size_t paddingLength = blockSize - (data.size() % blockSize);
    data.insert(data.end(), paddingLength, static_cast<uint8_t>(paddingLength));

    size_t paddedLen = data.size();

    // 3. 生成随机 IV
    uint8_t* iv = new uint8_t[blockSize];
    if (RAND_bytes(iv, blockSize) != 1) {
        std::cerr << "Failed to generate IV" << std::endl;
        delete[] iv;
        return nullptr;
    }

    AES_KEY aesKey;
    if (AES_set_encrypt_key(key, 256, &aesKey) < 0) {
        std::cerr << "Failed to set AES key" << std::endl;
        delete[] iv;
        return nullptr;
    }

    std::vector<uint8_t> encrypted(paddedLen);
    uint8_t iv_copy[AES_BLOCK_SIZE];
    memcpy(iv_copy, iv, AES_BLOCK_SIZE);

    AES_cbc_encrypt(data.data(), encrypted.data(), paddedLen, &aesKey, iv_copy, AES_ENCRYPT);

    data = std::move(encrypted);

    return iv;
}

bool OpensslHandler::verifyPassword(const std::string& password, const std::string& hash) {
    return crypto_pwhash_str_verify(
               hash.c_str(),
               password.c_str(),
               password.size()
           ) == 0;
}

uint8_t* OpensslHandler::sha256(uint8_t* str, size_t length)
{
    uint8_t* digest = new uint8_t[SHA256_DIGEST_LENGTH];

    if (!SHA256(str, length, digest)) {
        std::cerr << "SHA256 calculation failed" << std::endl;
        delete[] digest;
        return nullptr;
    }

    return digest;
}