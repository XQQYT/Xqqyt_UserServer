#ifndef _MSGDECODER_H_
#define _MSGDECODER_H_

#include "rapidjson/document.h"
#include <string>
#include "JsonStrategy.h"
#include "MySqlConnPool.h"
#include <unordered_map>

class MsgDecoder {
public:
    static void decode(const int socket,std::string msg);
    static void setMySqlConnPool(std::shared_ptr<MySqlConnPool> instance);
    static std::shared_ptr<MySqlConnPool> getMySqlConnPool();
private:
    MsgDecoder();
    MsgDecoder(MsgDecoder&) = delete;
    static std::shared_ptr<MySqlConnPool> mysql_conn_pool_instance;
};

#endif
