#include "MsgBuilder.h"
#include <string>
#include <memory.h>
#include <arpa/inet.h>
#include "OpensslHandler.h"
#include <fstream>

static uint16_t no_encrypy_magic = 0xAFAF;

std::unique_ptr<MsgBuilder::UserMsg> MsgBuilder::buildMsg(std::string payload, const uint8_t* key)
{
    Header header;

    uint16_t net_magic = htons(magic);
    uint32_t length_place_holder = 0x0;

    memcpy(&header.magic, &net_magic, sizeof(net_magic));
    memcpy(&header.version, &version, sizeof(version));
    memcpy(&header.length, &length_place_holder, sizeof(length_place_holder));

    uint8_t* iv = nullptr;
    uint8_t* sha256 = nullptr;

    std::vector<uint8_t> vec(reinterpret_cast<const uint8_t*>(payload.data()),
                             reinterpret_cast<const uint8_t*>(payload.data() + payload.size()));

    iv = OpensslHandler::getInstance().aesEncrypt(vec, key);
    std::vector<uint8_t> vi_encrypt(iv,iv+16);
    vi_encrypt.insert(vi_encrypt.end(),vec.begin(),vec.end());
    sha256 = OpensslHandler::getInstance().sha256(vi_encrypt.data(), vi_encrypt.size());

    // 构造最终消息
    std::vector<uint8_t> msg(sizeof(Header) + 1 + 16 + vec.size() + 32);

    uint32_t payload_length = htonl(1 + 16 + 32 + vec.size());
    memcpy(&header.length, &payload_length, sizeof(payload_length));

    size_t offset = 0;
    memcpy(msg.data() + offset, &header, sizeof(Header)); offset += sizeof(Header);
    *(msg.data()+offset) = 0x00; offset += 1;
    memcpy(msg.data() + offset, iv, 16); offset += 16;
    memcpy(msg.data() + offset, vec.data(), vec.size()); offset += vec.size();
    memcpy(msg.data() + offset, sha256, 32);

    auto user_msg = std::make_unique<MsgBuilder::UserMsg>();
    user_msg->iv = iv;
    user_msg->sha256 = sha256;
    user_msg->msg = std::make_unique<std::vector<uint8_t>>(std::move(msg));

    return user_msg;
}

std::unique_ptr<std::vector<uint8_t>> readFileToBytes(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("Failed to open file");

    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        throw std::runtime_error("Failed to read file");
    }

    return std::make_unique<std::vector<uint8_t>>(std::move(buffer));
}

std::streamsize getFileSize(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate); // 打开文件，光标定位到末尾
    if (!file) {
        throw std::runtime_error("Failed to open file");
    }
    return file.tellg(); // 返回当前位置，也就是文件大小
}

std::unique_ptr<MsgBuilder::UserMsg> MsgBuilder::buildFile(MessageType type,std::string username, std::string path, uint8_t* key)
{
    constexpr size_t USERNAME_SIZE = 15;
    if (username.size() > USERNAME_SIZE) {
        username.resize(USERNAME_SIZE);
    }

    Header header;

    uint16_t net_magic = htons(magic);
    uint32_t length_place_holder = 0x0;

    memcpy(&header.magic, &net_magic, sizeof(net_magic));
    memcpy(&header.version, &version, sizeof(version));
    memcpy(&header.length, &length_place_holder, sizeof(length_place_holder));

    uint8_t* iv = nullptr;
    uint8_t* sha256 = nullptr;

    std::unique_ptr<std::vector<uint8_t>> full_content = std::make_unique<std::vector<uint8_t>>(getFileSize(path) + 2 + USERNAME_SIZE);
    uint16_t head_offset = 0;
    uint16_t type_net = htons(static_cast<uint16_t>(type));
    memcpy(full_content->data() + head_offset, &type_net, 2);
    head_offset += 2;
    memcpy(full_content->data() + head_offset, username.data(), username.size());
    if (username.size() < USERNAME_SIZE) {
        memset(full_content->data() + head_offset + username.size(), 0, USERNAME_SIZE - username.size());
    }
    head_offset += USERNAME_SIZE;
    
    auto file_all_content = readFileToBytes(path);

    memcpy(full_content->data() + head_offset, file_all_content->data(), file_all_content->size());

    iv = OpensslHandler::getInstance().aesEncrypt(*full_content, key);
    std::vector<uint8_t> vi_encrypt(iv,iv+16);
    vi_encrypt.insert(vi_encrypt.end(),full_content->begin(),full_content->end());
    sha256 = OpensslHandler::getInstance().sha256(vi_encrypt.data(), vi_encrypt.size());

    // 构造最终消息
    std::vector<uint8_t> msg(sizeof(Header) + 1 + 16 + full_content->size() + 32);

    uint32_t payload_length = htonl(1 + 16 + 32 + full_content->size());
    memcpy(&header.length, &payload_length, sizeof(payload_length));

    size_t offset = 0;
    memcpy(msg.data() + offset, &header, sizeof(Header)); offset += sizeof(Header);
    *(msg.data()+offset) = 0x01; offset += 1;
    memcpy(msg.data() + offset, iv, 16); offset += 16;
    memcpy(msg.data() + offset, full_content->data(), full_content->size()); offset += full_content->size();
    memcpy(msg.data() + offset, sha256, 32);

    auto user_msg = std::make_unique<MsgBuilder::UserMsg>();
    user_msg->iv = iv;
    user_msg->sha256 = sha256;
    user_msg->msg = std::make_unique<std::vector<uint8_t>>(std::move(msg));

    return user_msg;
}

// AFAF size type subtype payload
//  2    4    2      2       N
std::unique_ptr<std::vector<uint8_t>> MsgBuilder::buildHeader(MessageType type, uint32_t totalsize)
{
    uint16_t offset = 0;
    auto msg = std::make_unique<std::vector<uint8_t>>(2 + 4 + 2 + 2 + 4);
    memcpy(msg->data() + offset, &no_encrypy_magic,sizeof(no_encrypy_magic)); offset += sizeof(no_encrypy_magic);
    uint32_t size = 8;
    uint32_t size_be = htonl(size);
    memcpy(msg->data() + offset,&size_be,sizeof(size_be)); offset += sizeof(size_be);
    uint16_t maintype = htons(static_cast<uint16_t>(type));
    memcpy(msg->data() + offset,&maintype, sizeof(maintype));   offset += sizeof(maintype);
    uint16_t subtype = htons(static_cast<uint16_t>(SubMessageType::HEADER));
    memcpy(msg->data() + offset,&subtype,sizeof(subtype)); offset += sizeof(subtype);
    uint32_t total_size_be = htonl(totalsize);
    memcpy(msg->data() + offset,&total_size_be,sizeof(total_size_be));
    return msg;
}

std::unique_ptr<std::vector<uint8_t>> MsgBuilder::buildPayload(MessageType type, std::vector<char>& data)
{
    uint16_t offset = 0;
    auto msg = std::make_unique<std::vector<uint8_t>>(2 + 4 + 2 + 2 + data.size());
    memcpy(msg->data() + offset, &no_encrypy_magic,sizeof(no_encrypy_magic)); offset += sizeof(no_encrypy_magic);
    uint32_t size = 2 + 2 + data.size();
    uint32_t size_be = htonl(size);
    memcpy(msg->data() + offset,&size_be,sizeof(size_be)); 
    offset += sizeof(size_be);
    uint16_t maintype = htons(static_cast<uint16_t>(type));
    memcpy(msg->data() + offset,&maintype, sizeof(maintype));   offset += sizeof(maintype);
    uint16_t subtype = htons(static_cast<uint16_t>(SubMessageType::PAYLOAD));
    memcpy(msg->data() + offset,&subtype,sizeof(subtype)); offset += sizeof(subtype);
    memcpy(msg->data() + offset,data.data(),data.size());
    return msg;
}

std::unique_ptr<std::vector<uint8_t>> MsgBuilder::buildEnd(MessageType type)
{
    uint16_t offset = 0;
    auto msg = std::make_unique<std::vector<uint8_t>>(2 + 4 + 2 + 2 + 0);
    memcpy(msg->data() + offset, &no_encrypy_magic,sizeof(no_encrypy_magic)); offset += sizeof(no_encrypy_magic);
    uint32_t size = 2 + 2 + 0;
    uint32_t size_be = htonl(size);
    memcpy(msg->data() + offset,&size_be,sizeof(size_be)); 
    offset += sizeof(size_be);
    uint16_t maintype = htons(static_cast<uint16_t>(type));
    memcpy(msg->data() + offset,&maintype, sizeof(maintype));   offset += sizeof(maintype);
    uint16_t subtype = htons(static_cast<uint16_t>(SubMessageType::END));
    memcpy(msg->data() + offset,&subtype,sizeof(subtype)); offset += sizeof(subtype);
    return msg;
}
