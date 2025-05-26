#ifndef _GLOBALENUM_H
#define _GLOBALENUM_H

#include <stdint.h>
#include <string>

enum class MessageType : uint16_t {
    USER_AVATAR = 0xAEAE,
    VERSION_PACKAGE = 0xBEBE     
};

enum class SubMessageType : uint16_t{
    HEADER = 0xAAAA,
    PAYLOAD = 0xABAB,
    END = 0xACAC
};

struct DeviceInfo
{
    DeviceInfo(std::string device_name_str,std::string code_str,std::string ip_str,std::string comment_str):
        device_name(std::move(device_name_str)),
        code(std::move(code_str)),
        ip(std::move(ip_str)),
        comment(std::move(comment_str)){}
    std::string device_name;
    std::string ip;
    std::string code;
    std::string comment;
};


#endif //_GLOBALENUM_H