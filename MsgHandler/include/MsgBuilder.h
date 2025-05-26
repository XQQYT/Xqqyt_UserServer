#ifndef _MSGBUILDER_H
#define _MSGBUILDER_H

#include <vector>
#include <memory>
#include <stdint.h>
#include "GlobalEnum.h"

static const uint16_t magic = 0xABCD;
static const uint8_t version = 0x01; 

struct __attribute__((packed)) Header
{
    uint16_t magic;
    uint8_t version;
    uint32_t length;
};

class MsgBuilder
{

public:
    class UserMsg
    {
    public:
        ~UserMsg()
        {
            delete[] iv;
            delete[] sha256;
        }
    public:
        std::unique_ptr<std::vector<uint8_t>> msg;
        uint8_t* iv;
        uint8_t* sha256;
    };
public:
    static MsgBuilder& getInstance()
    {
        static MsgBuilder instance;
        return instance;
    }
    ~MsgBuilder(){}
    std::unique_ptr<UserMsg> buildMsg(std::string payload, const uint8_t* key);
    std::unique_ptr<UserMsg> buildFile(MessageType type,std::string username, std::string path, uint8_t* key);
    std::unique_ptr<std::vector<uint8_t>> buildHeader(MessageType type, uint32_t totalsize);
    std::unique_ptr<std::vector<uint8_t>> buildPayload(MessageType type, std::vector<char>& data);
    std::unique_ptr<std::vector<uint8_t>> buildEnd(MessageType type);

private:
    MsgBuilder(){};
    uint8_t version;
};

#endif //_MSGBUILDER_H