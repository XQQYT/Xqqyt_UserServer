#ifndef _MSGDECODER_H_
#define _MSGDECODER_H_

#include "rapidjson/document.h"
#include <string>
#include "JsonHandler.h"
#include <unordered_map>


class MsgDecoder {
public:
    static void decode(const int socket,std::string msg);
private:
    MsgDecoder();
    MsgDecoder(MsgDecoder&) = delete;
};

#endif
