#ifndef _MSGDECODER_H_
#define _MSGDECODER_H_

#include "rapidjson/document.h"
#include "JsonStrategy.h"
#include "BinaryStrategy.h"
#include "MySqlConnPool.h"

class MsgDecoder {
public:
    static void decode(const int socket, std::vector<uint8_t> msg, bool is_binary, uint8_t* key, MySqlDriver* mysql_driver);
private:
    MsgDecoder();
    MsgDecoder(MsgDecoder&) = delete;
};

#endif
