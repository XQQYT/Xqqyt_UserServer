#ifndef _BINARYSTRATEGY_H_
#define _BINARYSTRATEGY_H_

#include <fstream>
#include <cstdio>
#include <stdint.h>
#include <vector>
#include "GlobalEnum.h"
#include "MySqlConnPool.h"

class BinaryStrategy {
public:
    virtual void execute(const int socket,uint8_t* key,std::vector<uint8_t> content, MySqlDriver* mysql_driver) = 0;
    virtual ~BinaryStrategy(){
    }

};

class UserAvatarStrategy : public BinaryStrategy{
public:
    void execute(const int socket,uint8_t* key,std::vector<uint8_t>  content, MySqlDriver* mysql_driver) override;
    ~UserAvatarStrategy(){}
};


class BinaryStrategyFactory {
public:
    static BinaryStrategy* createStrategy(MessageType type);
};

#endif  //_BINARYSTRATEGY_H_
